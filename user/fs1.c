
//#include <inc/assert.h>
#include <assert.h>
#include <inc/fs.h>

//int printf(const char *format, ...);
#define printf(...)

uint8_t data[0x10000];
uint8_t buffer[0x10000];

void fill(uint8_t *a, int len)
{
    int i;
    for (i = 0; i < len; ++i)
        a[i] = (uint8_t)(0xdeadbeef * i);
}

int check(uint8_t *a, int len)
{
    int i;
    for (i = 0; i < len; ++i)
        if (((uint8_t)(0xdeadbeef * i)) != a[i])
            return 1;
    return 0;
}

int open(const char*, int flags);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void*buf, size_t);
#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	/* not fcntl */
#define O_EXCL		   0200	/* not fcntl */
#define O_NOCTTY	   0400	/* not fcntl */
#define O_TRUNC		  01000	/* not fcntl */
#define O_APPEND	  02000
#define O_NONBLOCK	  04000
#define O_NDELAY	O_NONBLOCK
#define O_SYNC	       04010000
#define O_FSYNC		 O_SYNC
#define O_ASYNC		 020000

// 
#define INC 0x10000
void readin(int fd, int ld, int len)
{
    int pos;
    for (pos = 0; pos + INC < len; pos += INC)
    {
        assert(INC == read(ld, buffer, INC));
        assert(INC == dfs_write(fd, buffer, INC));
    }
    if (pos < len)
    {
        // get rest
        int rest = len - pos;
        assert(rest == read(ld, buffer, rest));
        assert(rest == dfs_write(fd, buffer, rest));
    }
}

void writeout(int fd, int lf, int len)
{
    int pos = 0;
    for (pos = 0; pos +INC < len; pos += INC)
    {
        assert(INC == dfs_read(fd, buffer, INC));
        assert(INC == write(lf, buffer, INC));
    }
    if (pos < len)
    {
        int rest = len - pos;
        assert(rest == dfs_read(fd, buffer, rest));
        assert(rest == write(lf, buffer, rest));
    }
}

int main(void)
{
    int fd, lf, rc, i;
    int len = 128239250;
    dfs_init_clean();
    fd = dfs_open("/large", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    /* Read in large file and put it in our private dfs. */
    lf = open("fff", O_RDONLY);
    assert(lf >= 0);
    readin(fd, lf, len);
    printf("Read in file\n");
    dfs_close(fd);
    fd = dfs_open("/large", DFS_O_RDONLY);
    assert(fd >= 0);
    lf = open("fff2", O_WRONLY | O_CREAT);
    assert(lf >= 0);
    writeout(fd, lf, len);
    close(lf);
    return 0;
}

int main2(void)
{
    int rc, fd, i, tot;
    dfs_init_clean();
    fd = dfs_open("/file", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    fill(data, sizeof(data));
    tot = 0;
    for (i = 0; i < 100; ++i)
    {
        rc = dfs_write(fd, data, sizeof(data));
        assert(rc >= 0);
        tot += rc;
    }
    printf("Wrote %d bytes\n", tot);
    dfs_close(fd);
    fd = dfs_open("/file", DFS_O_RDONLY);
    assert(fd >= 0);
    tot = 0;
    for (i = 0; i < 100; ++i)
    {
        rc = dfs_read(fd, buffer, sizeof(buffer));
        assert(sizeof(buffer) == rc);
        assert(0 == check(buffer, sizeof(buffer)));
        tot += rc;
    }
    printf("Read in %d bytes\n", tot);
    printf("Success\n");
    return 0;
}

int main1(void)
{
    int fd1, fd2;
    dfs_init_clean();
    fd1 = dfs_mkdir("/test");
    fd2 = dfs_open("/test/sub", DFS_O_CREAT | DFS_O_WRONLY);
    if (fd1 < 0 || fd2 < 0)
    {
        printf("invalid fd %d %d\n", fd1, fd2);
        return 0;
    }
    printf("FDs are %d %d\n", fd1, fd2);
    int rc = dfs_write(fd2, "hello world", 10);
    dfs_close(fd2);
    fd2 = dfs_open("/test/sub", DFS_O_RDONLY);
    printf("New fd is %d rc=%d\n", fd2,rc);
    char BUF[10];
    rc = dfs_read(fd2, BUF, sizeof(BUF));
    printf("is \"%s\" %d\n", BUF, rc);
    printf("Success.\n");
    return 0;
}

