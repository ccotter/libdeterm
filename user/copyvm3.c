
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>

#define OFFSET (0x52380000)
//#define OFFSET 0
#define DST(x) ((void*)(x) + OFFSET)

#define A1 (0x10000000)
#define S1 (0x1000)

#define LARGE ((void*)0x10000000)
#define LS (0x1000*1000*5)
/* 20 MB */

void testAutomap(void)
{
    void *a;
    unsigned long *la;
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        int i;
        dret();
        la = DST((unsigned long*)A1);
        for (i = 0; i < 0x1000/sizeof(unsigned long); ++i)
        {
            if (10 == i) continue;
            assert(0 == la[i]);
        }
        assert(0xdeadbeef == la[10]);
        dret();
        while(1);
    }
    a = mmap((void*)A1, S1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    assert((void*)A1 == a);
    la = (unsigned long*)a;
    memset(a, 0, S1);
    la[10] = 0xdeadbeef;
    assert(1 == dput(0, DETERMINE_START | DETERMINE_VM_COPY, &la[10], sizeof(unsigned long), DST(&la[10])));
    dwaitpid(0);
    assert(0xdeadbeef == la[10]);
    munmap(a, S1);
    printf("testAutomap Success\n");
}

void fill(unsigned long *addr, size_t size, int off)
{
    int i;
    for (i = 0; i < size/sizeof(unsigned long); ++i)
    {
        addr[i] = 0xdeadbeef * (off+i);
    }
}

void checkDeadbeef(unsigned long *a, size_t s)
{
    int i;
        printf("%08lx\n", a);
    for (i = 0; i < s/sizeof(unsigned long); ++i)
    {
        if ((0xdeadbeef * i) != a[i])
            printf("deadbeef at %d %08lx\n", i, &a[i]);
        assert((0xdeadbeef * i) == a[i]);
    }
}

void testLargeCopy(int mapFirst)
{
    void *a;
    if (mapFirst)
    {
        a = mmap((void*)LARGE, LS, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
        assert((void*)LARGE == a);
        fill((unsigned long*)a, LS, 0);
    }
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        int i;
        unsigned long *la = DST((unsigned long*)LARGE);
        dret();
        checkDeadbeef(la, LS);
        dret();
        while(1);
    }
    if (!mapFirst)
    {
        a = mmap((void*)LARGE, LS, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
        assert((void*)LARGE == a);
        fill((unsigned long*)a, LS, 0);
    }
    dput(0, DETERMINE_START | DETERMINE_VM_COPY, (void*)LARGE, LS, DST((void*)LARGE));
    dwaitpid(0);
    munmap(a, LS);
    printf("testLargeCopy %d success\n", mapFirst);
}

void testAlternating(void)
{
    int i;
    void *a;
    for (i = 0; i < LS; i += 0x2000)
    {
        void *a = mmap((void*)(LARGE + i), 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        assert((void*)(LARGE + i) == a);
        fill(a, 0x1000, i/sizeof(unsigned long));
    }
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        dret();
        //dput(0, DETERMINE_DEBUG, (void*)1, 0, NULL);
        checkDeadbeef(DST((unsigned long*)LARGE), LS);
        dret();
        while(1);
    }
    for (i = 0; i < LS; i += 0x2000)
    {
        munmap((void*)(LARGE + i), 0x1000);
    }
    a = mmap((void*)(LARGE), LS, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    assert((void*)LARGE == a);
    fill(a, LS, 0);
    checkDeadbeef((unsigned long*)LARGE, LS);
    printf("Parent yes\n");
    dput(0, DETERMINE_START | DETERMINE_VM_COPY, a, LS, DST(a));
    dwaitpid(0);
    printf("testAlternating success\n");
}

void firstTest(void)
{
    int i;
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        unsigned long *la = DST(LARGE);
        printf("Child la is *%08lx\n", la);
        dret();
        printf("Child la is *%08lx=%08lx\n", la, *la);
        dret();
    }
    unsigned long *la = mmap((void*)LARGE, 0x10000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    for (i = 0; i < 0x10000/sizeof(unsigned long); ++i)
    {
        la[i] = 0xdeadbeef;
    }
    dput(0, DETERMINE_START | DETERMINE_VM_COPY, la, 0x10000, DST(la));
    munmap(la, 0x10000);
}

void testOldRegions(void)
{
    /* This test requires us to look at the console output... */
    
}

int main2(void)
{
    become_deterministic();
    firstTest();
    testAutomap();
    testLargeCopy(0);
    testLargeCopy(1);
    testAlternating();
    testOldRegions();
    printf("All tests success!\n");
    while(1);
    return 0;
}

int main(void)
{
    become_deterministic();
    if (!dfork(0, DETERMINE_SNAP | DETERMINE_START))
    {
        int i = 10000000;
        while(i-->0);
        dret();
    }
    dget(0, 0, 0, 0, 0);
    if (!dfork(0, DETERMINE_SNAP | DETERMINE_START))
    {
        int i = 10000000;
        while(i-->0);
        dret();
    }
    dget(0, 0, 0, 0, 0);
    return 0;
}

