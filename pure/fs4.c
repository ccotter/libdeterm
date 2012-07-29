
#include <inc/determinism_pure.h>
#include <inc/assert.h>
#include <inc/fs.h>
#include <inc/string.h>

#define OFF 0x1234
#define SIZE1 (0x1000000-OFF)
#define SIZE2 (0x1000000+OFF)
//#define SIZE (0x1000*0x1000/4/4)
long fc1[SIZE1];
long fc2[SIZE2];

long sc[SIZE1+SIZE2+1];

void fill(long *b, int off, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        b[i] = off*i;
    }
}

int check(long *b, int off, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        if ((off*i)!=b[i])
        {
            iprintf("%d ? %d\n", i*off,b[i]);
            return 1;
            break;
        }
        //assert(i == b[i]);
    }
    return 0;
}

void st(long *b, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        iprintf("%08lx ",b[i]);
    }
    iprintf("\n");
}

int main(void)
{
    int i,fd;

    become_deterministic();
    assert(0 == dfs_init_clean());
    assert(0 <= (fd = dfs_open("/f1", DFS_O_CREAT | DFS_O_RDWR)));
    assert(4 == dfs_write(fd, "123", 4));
    dfs_close(fd);
    fill(fc1, 1234, sizeof(fc1)/sizeof(long));
    fill(fc2, 5678, sizeof(fc2)/sizeof(long));

    if (!dput(0, 0, NULL, 0, NULL))
    {
        fd = dfs_open("/f1", DFS_O_APPEND);
        assert(fd >= 0);
        assert(!check(fc2, 5678, sizeof(fc2)/sizeof(long)));
        assert(sizeof(fc2) == dfs_write(fd, fc2, sizeof(fc2)));
        dfs_close(fd);
        unsigned long *Q = (unsigned long*)(0x10000000 + 3123 * 0x1000);
        iprintf("Child says %08lx %08lx", Q, Q[0]);
        //while(1);
        dret();
        fd = dfs_open("/f1", DFS_O_RDONLY);
        assert(fd >= 0);
        assert(sizeof(sc) == dfs_read(fd, sc, sizeof(sc)));
        assert(!check(sc+1, 5678, sizeof(fc2)/sizeof(long)));
        assert(!check(sc+1+sizeof(fc2)/sizeof(long), 1234, sizeof(fc1)/sizeof(long)));
        iprintf("Child yes\n");
        dfs_close(fd);
        dret();
    }
    int rc;
    assert(0 == (rc=dfs_put(0)));
    iprintf("RC=%d\n",rc);
    assert(1 == dput(0, DETERMINE_START, NULL, 0, NULL));

    dput(0, 0,0,0,0);
    fd = dfs_open("/f1", DFS_O_APPEND);
    assert(fd >= 0);
    assert(sizeof(fc1) == dfs_write(fd, fc1, sizeof(fc1)));
    dfs_close(fd);

    iprintf("Going to get\n");
    assert(0 == dfs_get(0));
    assert(1 == dput(0, DETERMINE_START, NULL, 0, NULL));
    iprintf("Done getting\n");

    fd = dfs_open("/f1", DFS_O_RDONLY);
    assert(0 <= fd);
    int w;
    assert(sizeof(sc) == (w=dfs_read(fd, sc, sizeof(sc))));
    assert(!check(sc+1, 1234, sizeof(fc1)/sizeof(long)));
    assert(!check(sc+1+sizeof(fc1)/sizeof(long), 5678, sizeof(fc2)/sizeof(long)));
    iprintf("parent good %d %d\n",w,sizeof(sc));
    dfs_close(fd);

    iprintf("Done\n");
    while(1);
    return 0;
}

const char str1[] = "xThis is original. x";
const char str2[] = "xFrom parent. x";
const char str3[] = "xFrom child. x";

int main1(void)
{
    int i;
    int fd;
    char buf[100];
    become_deterministic();
    assert(0 == dfs_init_clean());

    fd = dfs_open("/f1", DFS_O_CREAT | DFS_O_WRONLY);
    assert(fd >= 0);
    assert(strlen(str1) == dfs_write(fd, str1, strlen(str1)));
    dfs_close(fd);

    /* Now fork off child and place FS. */
    if (!dput(0, 0, NULL, 0, NULL))
    {
        int fd,i;
        char buf[100];
        fd = dfs_open("/f1", DFS_O_APPEND);
        assert(fd >= 0);
        assert(strlen(str3) == dfs_write(fd, str3, strlen(str3)));
        dfs_close(fd);
        dret();
        fd = dfs_open("/f1", DFS_O_RDONLY);
        assert(fd >= 0);
        i = dfs_read(fd, buf, 100);
        buf[i+1]=0;
        iprintf("    CHILD IT IS AFTER %d(%s)\n",i, buf);
        dret();
    }
    assert(0 == dfs_put(0));
    assert(1 == dput(0, DETERMINE_START, NULL, 0, NULL));

    fd = dfs_open("/f1", DFS_O_APPEND);
    assert(fd >= 0);
    assert(strlen(str2) == dfs_write(fd, str2, strlen(str2)));
    dfs_close(fd);

    iprintf("get fs = %d\n", dfs_get(0));
    fd = dfs_open("/f1", DFS_O_RDONLY);
    assert(fd >= 0);
    i = dfs_read(fd, buf, 100);
    buf[i+1]=0;
    iprintf("IT IS AFTER %d(%s)\n",i, buf);
    dfs_close(fd);
    iprintf("Done\n");
    dput(0, DETERMINE_START, NULL, 0, NULL);

    while(1);
    return 0;
}

