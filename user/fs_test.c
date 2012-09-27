
#include <string.h>
#include <fs.h>
#include <determinism.h>
#include <assert.h>
#include <debug.h>

static int dfork(pid_t id, unsigned long flags)
{
	struct user_regs_struct regs;
	get_register_state(&regs);
	return dput_regs(id, &regs, DET_START | flags);
}

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
static void child(int childid)
{
    if (!dfork(childid, 0)) {
        dret(); /* Wait for parent to send us the FS. */
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", childid);
        fd = dfs_open(fname, DFS_O_CREAT | DFS_O_WRONLY);
        fill(array, sizeof(array), childid);
        dfs_write(fd, array, sizeof(array));
        dfs_close(fd);
        dret();
    }

	int rc = dfs_put(childid); /* Put FS into child. */
    iprintf("put %d=%d\n",childid, rc);
    dput(childid, DET_START, 0, 0, 0);
}

#define NCHILDREN 5

int main(void)
{
    int i;

    for (i = 0; i < NCHILDREN; ++i) {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_CREAT)));
        dfs_close(fd);
    }
    for (i = 0; i < NCHILDREN; ++i) {
        child(i);
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
        assert(!dfs_get(i));
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
    iprintf("All tests passed\n");

    return 0;
}


