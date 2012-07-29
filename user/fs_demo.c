
#include <string.h>
#include <inc/determinism.h>
#include <inc/fs_nondet.h>
#include <inc/fork_nondet.h>
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
    for (i = 0; i < len; ++i)
    {
        if ((unsigned char)(0xdeadbeef * off) != A[i])
            return 1;
    }
    return 0;
}
static void child(int childid)
{
    if (!dfork(childid, DETERMINE_START))
    {
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
    dfs_put(childid); /* Put FS into child. */
    dput(childid, DETERMINE_START, NULL, 0, NULL);
}

#define NCHILDREN 5

int main(void)
{
    int i;
    assert(0 == become_deterministic());
    assert(0 == dfs_init_clean()); /* Wipes FS. */

    for (i = 0; i < NCHILDREN; ++i)
    {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_CREAT)));
        dfs_close(fd);
    }
    for (i = 0; i < NCHILDREN; ++i)
    {
        child(i);
    }
    /* Make sure nothing has been written to the files before we sync. */
    for (i = 0; i < NCHILDREN; ++i)
    {
        char fname[100];
        int fd;
        snprintf(fname, sizeof(fname), "/child_%d", i);
        assert(0 <= (fd = dfs_open(fname, DFS_O_RDONLY)));
        assert(0 == dfs_read(fd, array, sizeof(array)));
        dfs_close(fd);
    }
    /* Sync with all children. */
    for (i = 0; i < NCHILDREN; ++i)
    {
        assert(!dfs_get(i));
    }
    /* Now check each file. */
    for (i = 0; i < NCHILDREN; ++i)
    {
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

    while(1); // init can't return
    return 0;
}


