
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>

#define A1 (0x10000000)
#define S1 (0x10000)
#define A2 (0x20000000)
#define A3 (0x20010000)
#define S3 (0x2000)

struct region
{
    int map, early, dont;
    void *addr;
    size_t size;
    int off;
};

struct region regions[] = {
    {1, 0, 0, (void*)0x21000000,  0x10000, 0},
    {1, 1, 1, (void*)0x21014000,  0x1,     0},
    {1, 1, 0, (void*)0x21012000,  0x1001,  100},
    {1, 1, 1, (void*)0x21016000,  0x20000, 100},
    {1, 0, 0, (void*)0x21010000,  0x2000,  100},
    {1, 0, 1, (void*)0x21013000,  0x1000,  0},
    {1, 0, 0, (void*)0x21015000,  0x2,     0},
};

static int check0(void *B, size_t size)
{
    int i;
    unsigned char *A = (unsigned char*)B;
    for (i = 0; i < size; i++)
    {
        if (A[i]) return 0;
    }
    return 1;
}

unsigned char bytes[0x1000];

void testautomap(void)
{
    int i;
    unsigned long min = -1, max = 0;
    for (i = 0; i < sizeof(regions)/sizeof(struct region); ++i)
    {
        void *aa;
        if ((unsigned long)regions[i].addr < min)
            min = (unsigned long)regions[i].addr;
        if ((unsigned long)(regions[i].addr + regions[i].size) > max)
            max = (unsigned long)(regions[i].addr + regions[i].size);
        if (!regions[i].map || !regions[i].early)
            continue;
        aa = mmap(regions[i].addr, regions[i].size,
                PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        assert(regions[i].addr == aa);
        memset(aa+regions[i].off, 0xab, regions[i].size - regions[i].off);
    }
    max = (max + 0x1000-1) & 0xfffff000;
    printf("Min max are %08lx %08lx\n", min, max);
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        int i;
        for (i = 0; i < sizeof(regions)/sizeof(struct region); ++i)
        {
            if (!regions[i].map || !regions[i].early)
                continue;
            assert(!check0(regions[i].addr+regions[i].off, regions[i].size - regions[i].off));
        }
        dret();
        dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
        for (i = 0; i < sizeof(regions)/sizeof(struct region); ++i)
        {
            if (!regions[i].map)
                continue;
            assert(check0(regions[i].addr+regions[i].off, regions[i].size - regions[i].off));
        }
        printf("Zero success from child\n");
        dret();
    }
    for (i = 0; i < sizeof(regions)/sizeof(struct region); ++i)
    {
        void *aa;
        if (!regions[i].map || regions[i].early)
            continue;
        aa = mmap(regions[i].addr, regions[i].size,
                PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        assert(regions[i].addr == aa);
        memset(aa+regions[i].off, 0xab, regions[i].size - regions[i].off);
    }
    for (i = 0; i < sizeof(regions)/sizeof(struct region); ++i)
    {
        if (!regions[i].map || regions[i].dont)
            continue;
        dput(0, DETERMINE_ZERO_FILL, regions[i].addr+regions[i].off, regions[i].size - regions[i].off, 0);
    }
    dput(0, DETERMINE_ZERO_FILL, (void*)min, max-min, 0);
    dput(0, DETERMINE_START, (void*)0, 0, 0);
    dput(0, 0, (void*)0, 0, 0); /* Sync. */
    printf("testautomap success\n");
}

unsigned long small;

void testsmall(void)
{
    printf("Addr %08lx\n", &small);
    small = 0xdeadbeef;
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        assert(0xdeadbeef == small);
        dret();
        assert(0 == small);
        dret();
        while(1);
    }
    dput(0, DETERMINE_ZERO_FILL | DETERMINE_START, &small, sizeof(unsigned long), 0);
    assert(0xdeadbeef == small);
    printf("testsmall success\n");
}

int main(void)
{
    int i;

    /* Test that regions not mapped in the destination are created automatically. */
    testautomap();
    testsmall();

    memset(bytes, 0xab, sizeof(bytes));

    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP)) {
        assert(!check0(bytes, 0x1000) && "First check");
        dret();
        assert(check0(bytes, 0x1000) && "Second check");
        dret();
    }

    printf("3\n");
    dput(0, DETERMINE_START | DETERMINE_ZERO_FILL, &bytes, 0x1000, 0);
    printf("Success\n");

out:
    while(1);
    return 0;
}

