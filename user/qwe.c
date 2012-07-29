
#include <inc/mman.h>
#include <inc/string.h>
#include <inc/fork.h>
#include <inc/assert.h>

#if 0

/* 100 MB */
#define ADDR 0x30000000
#define SZ (0x1000 * 10000 * 3)

void fillpage(int c)
{
    unsigned char *va = (void*)ADDR;
    if (!dfork(c, DETERMINE_START | DETERMINE_SNAP))
    {
        int i;
        for (i = c; i < SZ; i += 2)
            va[i] = c;
        dret();
    }
}

void checkpage(int c)
{
    const unsigned char *va = (void*)ADDR;
    int i;
    dget(c, DETERMINE_MERGE, (void*)va, SZ, (void*)va);
    for (i = 0; i < SZ; i += 2)
        assert(va[i] == c);
}

int main(void)
{
    void *va = (void*)ADDR;
    become_deterministic();
    assert((void*)ADDR == mmap(va, SZ, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0));
    fillpage(0);
    fillpage(1);
    dget(0, 0, NULL, 0, NULL);
    dget(1, 0, NULL, 0, NULL);
    //dput(0, DETERMINE_DEBUG | 0x5000, (void*)0x30000000, 0x1000*10, 0);
    //dput(0, DETERMINE_DEBUG | 0x6000, (void*)0x30000000, 0x1000*10, 0);
    checkpage(0);
    //checkpage(1);
    printf("Done\n");
    return 0;
}

#endif

#define ADDR 0x10000000
#define SZ (0x1000 * 100)
static void go(void)
{
    void *va;
    assert((void*)ADDR == (va = mmap((void*)ADDR, SZ, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0)));
    memset(va,0,SZ);
    //if (!dfork(0, DETERMINE_START))
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
        printf("sdf\n");
        memset(va, 0xef, SZ);
        dret();
    }
    printf("Waiting\n");
    int i = 10000000;
    while(i-->0);
    int Q=dget(0,0,0,0,0);
    printf("Done waiting %d\n",Q);
}

int main(void)
{
    become_deterministic();
    go();
        dput(0, DETERMINE_DEBUG, (void*)1, 0, 0);
    return 0;
}

