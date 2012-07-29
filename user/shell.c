
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
    read_file_to_memory("/prog3", "/prog3");
    read_file_to_memory("/prog2", "/prog2");
    read_file_to_memory("/database", "/database");
    if (!dfork(0, DETERMINE_START))
    {
        execl("/prog2", "/prog2", (char*)NULL);
    }
    dget(0, 0, NULL, 0, NULL);
    assert(0 == dfs_put(0));
    assert(DETERMINE_S_RUNNING == dput(0, DETERMINE_START, NULL, 0, NULL));
    dwaitpid(0);    
    /* Shell finished, now save file system to persistent storage. */
    printf("Parent finished running prog2, now going to spin.\n");
    return 0;
}

