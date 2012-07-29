
#include <stdio.h>
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <assert.h>
#include <inc/fs_nondet.h>
#include <unistd.h>
#include <fcntl.h>

char buf[0x1000];

static int read_file_to_memory(const char *actualf, const char *f)
{
    int actualfd, fd;
    int readin;
    actualfd = open(actualf, O_RDONLY);
    if (actualfd < 0)
        return -1;
    fd = dfs_open(f, DFS_O_CREAT | DFS_O_WRONLY);
    if (fd < 0)
    {
        close(actualfd);
        return -1;
    }
    while (0 < (readin = read(actualfd, buf, sizeof(buf))))
    {
        int togo = readin;
        while (togo > 0)
        {
            int written = dfs_write(fd, buf, togo);
            if (written <= 0)
            {
                close(actualfd);
                dfs_close(fd);
                return -1;
            }
            togo -= written;
        }
    }
    close(actualfd);
    dfs_close(fd);
    return 0;
}

int main(int argc, char**argv)
{
    assert(0 == become_deterministic());
    assert(0 == dfs_init_clean());
    assert(0 == _init_dfs());
    assert(0 == _init_io());
    /* Read in prog1 into the file system. */
    read_file_to_memory("./root/prog1", "/prog1");
    if (!dfork(0, DETERMINE_START))
    {
        /*
           dret();
        int i;
        char lon[10000];
        for (i = 0; i < sizeof(lon)-1; ++i)
            lon[i] = 'A' + (i % 26);
        lon[sizeof(lon)-1] = 0;
        char *A[] = {"hello1", lon, NULL};
        char *B[] = {"env1", "env2", NULL};
        dexec(1234, "/prog1", A, B);
        // */
        execl("./root/exec", "./root/exec", "hello", (char*)0);
    }
    assert(0 == dfs_put(0));
    assert(DETERMINE_S_RUNNING == dput(0, DETERMINE_START, 0, 0, 0));
    while (1)
    {
        int st = dfs_childstate(0);
        if (DPROC_EXITED == st)
            break;
        dput(0, DETERMINE_START, NULL, 0, NULL);
    }
    dfs_close(0);
    dfs_close(1);
    printf("Parent back get=%d\n", dfs_get(0));
    dkill(0);
    dput(0, DETERMINE_DEBUG | 0x7000,3,0,0);
    /* Get contents of __stdout and print it to the actual console. */
    int fd, rd;
    char buf[0x1000];
    assert(0 <= (fd = dfs_open("/__stdout", DFS_O_RDONLY)));
    while (0 < (rd = dfs_read(fd, buf, sizeof(buf))))
    {
        write(1, buf, rd);
    }
    return 0;






    if (!dfork(0, 0))
    {
        execl("/prog1", "/prog1", "hello", (char*)0);
    }
    if (!dfork(1, 0))
    {
        execl("/prog2", "/prog2", (char*)0);
    }

    assert(0 == dfs_put(0));
    assert(0 == dfs_put(1));
    assert(1 == dput(0, DETERMINE_START, 0, 0, 0));
    assert(1 == dput(1, DETERMINE_START, 0, 0, 0));
    dfs_close(0);
    dfs_close(1);
    /* Get child results. */
    assert(0 == dfs_get(0));
    assert(0 == dfs_get(1));

    dfs_open("/__stdout", DFS_O_RDONLY);
    int W=dfs_read(0, buf, sizeof(buf));
    printf("%s",buf);

    while(1);
    return 0;
}

