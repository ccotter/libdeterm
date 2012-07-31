
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>

#define A1 (0x10000000)
#define S1 (0x1000)

#define LARGE ((void*)0x10000000)
#define LS (0x1000*1000*5)
/* 20 MB */

void testAutomap(void)
{
    void *a;
    unsigned long *la;
    if (!dfork(0, DETERMINE_START))
    {
        int i;
        dret();
        la = (unsigned long*)A1;
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
    int rc = dput(0, DETERMINE_START | DETERMINE_VM_COPY, &la[10], sizeof(unsigned long), &la[10]);
    dget(0, 0, 0, 0, 0);
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
    for (i = 0; i < s/sizeof(unsigned long); ++i)
    {
        if ((0xdeadbeef * i) != a[i])
            printf("deadbeef at %d\n", i);
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
    if (!dfork(0, DETERMINE_START))
    {
        unsigned long *la = (unsigned long*)LARGE;
        dret();
        //checkDeadbeef(la, LS);
        dret();
        while(1);
    }
    if (!mapFirst)
    {
        a = mmap((void*)LARGE, LS, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
        assert((void*)LARGE == a);
        fill((unsigned long*)a, LS, 0);
    }
    int i=0x10000;
    dput(0, DETERMINE_START, (void*)LARGE, LS, (void*)LARGE);
    //dput(0, DETERMINE_START | DETERMINE_VM_COPY, (void*)LARGE, LS, (void*)LARGE);
    dget(0, 0, 0, 0, 0);
    munmap(a, LS);
    printf("testLargeCopy %d success\n", mapFirst);
}

#define go(x) \
{ int i=x; while (i-->0); }
void testAlternating(void)
{
    int i;
    void *a;
    for (i = 0; i < LS; i += 0x2000)
    {
        void *a = mmap((void*)(LARGE + i), 0x1000, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        assert((void*)(LARGE + i) == a);
        fill(a, 0x1000, i/sizeof(unsigned long));
    }
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        printf("1\n");
        dret();
        printf("2\n");
        //checkDeadbeef((unsigned long*)LARGE, LS);
        dret();
        printf("HEY\n");
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
    //dput(0, DETERMINE_START | DETERMINE_VM_COPY, a, LS, a);
    dput(0, DETERMINE_START, a, LS, a);
    dget(0, 0, 0, 0, 0);
    printf("testAlternating success\n");
}

void testOldRegions(void)
{
    /* This test requires us to look at the console output... */
    
}

int main(void)
{
    become_deterministic();
    //testAutomap();
    //testLargeCopy(0);
    testLargeCopy(1);
    testAlternating();
    //testOldRegions();
    printf("All tests success!\n");
    return 0;
}

