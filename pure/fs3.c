
#include <inc/determinism_pure.h>
#include <inc/assert.h>
#include <inc/fs.h>
#include <inc/string.h>

const char str1[] = "This is f1.";
const char str2[] = "This is f2.";

int main(void)
{
    int i;
    int fd;
    char buf[100];
    become_deterministic();
    assert(0 == dfs_init_clean());

    fd = dfs_open("/f1", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    assert(strlen(str1) == dfs_write(fd, str1, strlen(str1)));
    dfs_close(fd);
    fd = dfs_open("/f1", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    dfs_read(fd, buf, strlen(str1));
    assert(0 == strcmp(buf, str1));
    dfs_close(fd);
    
    fd = dfs_open("/f2", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    assert(strlen(str2) == dfs_write(fd, str2, strlen(str2)));
    dfs_close(fd);
    fd = dfs_open("/f2", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    dfs_read(fd, buf, strlen(str2));
    assert(0 == strcmp(buf, str2));
    dfs_close(fd);

    /* Now fork off child and place FS. */
    if (!dput(0, DETERMINE_REGS, NULL, 0, NULL))
    {
        int fd;
        char ibuf[100];
        fd = dfs_open("/f1", DFS_O_RDWR);
        assert(fd >= 0);
        dfs_read(fd, ibuf, strlen(str1));
        //iprintf("Hello\n");
        int rc = dfs_write(fd, ibuf, strlen(ibuf));
        dfs_close(fd);
        iprintf("FD=%d, str=%s wrote=%d\n", fd, ibuf, rc);
        dret();
    }
    assert(0 == dfs_put(0));
    assert(1 == dput(0, DETERMINE_START, NULL, 0, NULL));
    iprintf("get fs = %d\n", dfs_get(0));
    fd = dfs_open("/f1", DFS_O_RDONLY);
    assert(fd >= 0);
    i = dfs_read(fd, buf, strlen(str1)*2+2);
    iprintf("IT IS AFTER %d(%s)\n",i, buf);
    dfs_close(fd);

    iprintf("Done\n");
    while(1);
    return 0;
}

