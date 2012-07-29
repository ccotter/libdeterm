
#include <inc/mman.h>
#include <inc/determinism_pure.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/fork.h>

#define WORK() do { } while(0)

const char *tabs = "                                                                    ";

void go(int l, int n)
{
    if (l == 3) return;
    printf("%.*s %d\n", l*4, tabs, n);
    if (!dfork(n*2+1, DETERMINE_START | DETERMINE_SNAP))
    {
        WORK();
        go(l+1, n * 2 + 1);
        dret();
    }
    if (!dfork(n*2+2, DETERMINE_START | DETERMINE_SNAP))
    {
        WORK();
        go(l+1, n * 2 + 2);
        dret();
    }
    if (l!=23) printf("%.*s %d wait for %d\n", l*4, tabs, n, n*2+1);
    dget(n*2+1, 0, 0, 0, 0);
    if (l!=23) printf("%.*s %d wait for %d\n", l*4, tabs, n, n*2+2);
    dget(n*2+2, 0, 0, 0, 0);
    printf("%.*s %d ret\n", l*4, tabs, n);
}

int main1(void)
{
    become_deterministic();
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        dret();
        dret();
        dret();
        dret();
    }
    if (!dfork(1, DETERMINE_START | DETERMINE_SNAP))
    {
        dret();
        dret();
        dret();
        dret();
    }
    dget(0, 0, 0, 0, 0);
    dget(1, 0, 0, 0, 0);
    dget(0, 0, 0, 0, 0);
    dget(1, 0, 0, 0, 0);
    dget(0, 0, 0, 0, 0);
    dget(1, 0, 0, 0, 0);
    dget(0, 0, 0, 0, 0);
    dget(1, 0, 0, 0, 0);
    printf("Done\n");
    return 0;
}

int main2(void)
{
    become_deterministic();
    go(0, 0);

    printf("Done\n");
    return 0;
}

int x, y;

#if 0
int main(void)
{
    become_deterministic();
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        dret();
        dret();
        printf("BYE\n");
    }
    dput(0, DETERMINE_VM_COPY, (void*)0x10000000, 0x10000, (void*)0x10000000);
    while(1);
    return 0;
}
#endif

int main(void)
{
#define MB (4096*1000*10)
    void *va = (void*)0x10000000;
    become_deterministic();
    if (!dfork(0, 0))
    {
        
        printf("chidl is %08lx\n", *(unsigned long*)0x20000000);
        dret();
    }
    assert((void*)0x10000000 == mmap(va, 10*MB, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS |
            MAP_FIXED, -1, 0));
    memset(va, 0xab, 10*MB);
    dput(0, DETERMINE_VM_COPY|DETERMINE_START, va, 10*MB, (void*)0x20000000);
    while(1);
#undef MB
    return 0;
}

int main3(void)
{
    become_deterministic();
    x = y = 0;
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        x = 456;
        dret();
    }
    if (!dfork(1, DETERMINE_START | DETERMINE_SNAP))
    {
        y = 123;
        dret();
    }
    dget(0, DETERMINE_MERGE, (void*)&x, sizeof(int), 0);
    dget(1, DETERMINE_MERGE, (void*)&y, sizeof(int), 0);
    assert(x == 456);
    assert(y == 123);

    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP))
    {
        x = y;
        dret();
    }
    if (!dfork(1, DETERMINE_START | DETERMINE_SNAP))
    {
        y = x;
        dret();
    }
    dget(0, DETERMINE_MERGE, &x, sizeof(int), 0);
    dget(1, DETERMINE_MERGE, &y, sizeof(int), 0);
    assert(y == 456);
    assert(x == 123);
    return 0;
}
#if 0

#define SIZE (0x1000 * 10000*2)
int main(void)
{
    become_deterministic();
    if (!dfork(0, DETERMINE_START))
    {
        dret();
        void *A = (void*)0x10000000;
        memset(A, 0, SIZE);
        dret();
    }
    void *A = mmap((void*)0x10000000, SIZE, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    assert((void*)0x10000000 == A);
    memset(A, 0, SIZE);
    dput(0, DETERMINE_VM_COPY | DETERMINE_START,
            (void*)0x10000000, SIZE, (void*)0x10000000);
    dget(0, 0, 0, 0, 0);
    return 0;
}
#endif

