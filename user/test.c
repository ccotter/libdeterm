
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>
#include <assert.h>

#define ADDR1 (0x10000000)
#define SIZE 0x100000
#define ADDR2 (0x20000000)

int x[0x2000];

int main(void)
{
    int rc;
    int fd;
    void *ma;
    unsigned long *la;
    fd = open("/f3", O_RDONLY);
    ma = mmap((void*)ADDR1, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);
    assert((void*)ADDR1 == ma);
    la = ma;
    x[0] = 0;
    dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
    printf("la[1] = %08lx\n", la[1]);
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        la[0] = 0xfacedeed;
        x[0] = 0xdeadbeef;
        assert(0x44434241 == la[1]);
        dret();
        printf("Child says %08lx\n", la[0]);
        dret();
    }
    dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
    dput(0, DETERMINE_DEBUG, (void*)2, 0, 0);
    rc = dget(0, DETERMINE_MERGE, (void*)0, 0xafffffff - 0, 0);
    printf("*%08lx=%08lx rc=%d\n", x, x[0],rc);
    printf("la[0]=%08lx\n", la[0]);
    printf("la[1]=%08lx\n", la[1]);
    la[0] = 0xabababab;
    printf("la[0]=%08lx\n", la[0]);
    dput(0, DETERMINE_START, (void*)0, 0, 0);

    while(1);
    return 0;
}

int main2(void)
{
int x[0x2000];
    dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        printf("Writing child\n");
        x[0x2000-4] = 0xdeadbeef;
        dret();
    }
    dget(0, 0, (void*)0, 0, 0);
    printf("Writing parent\n");
    x[0x2000-4] = 0xdeadbeef;
    dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
    dput(0, DETERMINE_DEBUG, (void*)2, 0, 0);
    while(1);
    return 0;
}

int main1(void)
{
    int i, len, rc, fd1, fd2;
    void *a1, *a2;

    fd1 = open("/test", O_RDONLY);
    assert(fd1 >= 0);
    fd2 = open("/test", O_RDONLY);
    assert(fd2 >= 0);

    a1 = mmap((void*)ADDR1, SIZE, PROT_READ, MAP_SHARED | MAP_FIXED, fd1, 0);
    assert((void*)ADDR1 == a1);
    a2 = mmap((void*)ADDR2, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd2, 0);
    assert((void*)ADDR2 == a2);

    //i = *(int*)a1;
    //i = *(int*)a2;

    dput(0, DETERMINE_DEBUG | 0x5000, a1, 0x1000, 0);
    dput(0, DETERMINE_DEBUG | 0x5000, a2, 0x1000, 0);
    *(unsigned long*)a2 = 0xdeadbeef;
    assert(0xdeadbeef == *(unsigned long*)a2);
    dput(0, DETERMINE_DEBUG | 0x5000, a1, 0x1000, 0);
    dput(0, DETERMINE_DEBUG | 0x5000, a2, 0x1000, 0);

    dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);

out:
    while(1);
    return 0;
}

