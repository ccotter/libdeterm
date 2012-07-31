
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
#define LS2 (0x1000*2)
/* 20 MB */

void fill(unsigned long *addr, size_t size, int off)
{
    int i;
    for (i = 0; i < size/sizeof(unsigned long); ++i)
        addr[i] = 0xdeadbeef * (off+i);
}

void testAlternating(void)
{
    int i;
    for (i = 0; i < LS; i += 0x2000)
    {
        void *a = mmap((void*)(LARGE + i), 0x1000, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        assert((void*)(LARGE + i) == a);
        fill(a, 0x1000, i / sizeof(unsigned long));
    }
    if (!dfork(0,DETERMINE_SNAP))
    {
        dret();
        *(int*)0xdeadbeef=0;
        while(1);
    }
    dget(0,0,0,0,0);
    if (!dfork(0,DETERMINE_SNAP))
    {
        dret();
        *(int*)0xdeadbeef=0;
        while(1);
    }
    dget(0,0,0,0,0);
}

int main(void)
{
    become_deterministic();
    testAlternating();
    printf("Done\n");
    return 0;
}

