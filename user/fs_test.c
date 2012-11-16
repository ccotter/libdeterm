
#include <string.h>
#include <fs.h>
#include <determinism.h>
#include <assert.h>

unsigned char array[0x2000];
unsigned char array_2[0x2000];
static int fill(unsigned char *A, int len, int off)
{
    int i;
    for (i = 0; i < len; ++i)
        A[i] = (unsigned char)(0xdeadbeef * off);
    return 0;
}
static int check(unsigned char *A, int len, int off)
{
    int i;
    for (i = 0; i < len; ++i) {
        if ((unsigned char)(0xdeadbeef * off) != A[i])
            return 1;
    }
    return 0;
}
static pid_t child(int count)
{
	pid_t id = dfork(count, 0);
    if (!id) {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", count);
        fd = dfs_open(fname, DFS_O_CREAT | DFS_O_WRONLY);
        fill(array, sizeof(array), count);
        dfs_write(fd, array, sizeof(array));
        dfs_close(fd);
        dret();
    }
	return count;
}

#define NCHILDREN 50

int main(void)
{
    int i;
	int ids[NCHILDREN];

    for (i = 0; i < NCHILDREN; ++i) {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_CREAT)));
        dfs_close(fd);
    }
    /* Make sure nothing has been written to the files before we sync. */
    for (i = 0; i < NCHILDREN; ++i) {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_RDONLY)));
        assert(0 == dfs_read(fd, array, sizeof(array)));
        dfs_close(fd);
    }
    for (i = 0; i < NCHILDREN; ++i) {
        ids[i] = child(i);
    }

    /* Make sure nothing has been written to the files before we sync. */
    for (i = 0; i < NCHILDREN; ++i) {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_RDONLY)));
        assert(0 == dfs_read(fd, array, sizeof(array)));
        dfs_close(fd);
    }

    /* Sync with all children. */
    for (i = 0; i < NCHILDREN; ++i) {
        assert(!dfs_get(ids[i]));
    }

    /* Now check each file. */
    for (i = 0; i < NCHILDREN; ++i) {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        fill(array_2, sizeof(array_2), i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_RDONLY)));
        assert(sizeof(array) == dfs_read(fd, array, sizeof(array)));
        assert(!memcmp(array, array_2, sizeof(array)));
        dfs_close(fd);
    }
    printf("All tests passed\n");

    return 0;
}


