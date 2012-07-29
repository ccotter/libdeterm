
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>
#include <assert.h>

#define ADDR1 (0x10000000)
#define SIZE (0x1000 * 100)

int x, y;

void map1(void)
{
    unsigned long *la;
    void *a = mmap((void*)0x10000000, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    assert((void*)0x10000000 == a);
    la = a;
    la[10] = 0xdeadbeef;
}

void check1(void)
{
    unsigned long *la = (void*)0x10000000;
    int i;
    for (i = 0; i < 0x1000/sizeof(unsigned long); ++i)
    {
        if (10 == i) continue;
        assert(0 == la[i]);
    }
    assert(0xdeadbeef == la[10]);
}

int main(void)
{

    become_deterministic();
    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP)) { map1(); dret(); }
    dget(0, DETERMINE_MERGE, (void*)(0x10000000 + 10 * sizeof(unsigned long)), sizeof(unsigned long), NULL);
    check1();

#if 0
    x = y = 0;
    if (!dfork(0)) { x = 0xdeadbeef; dret(); }
    if (!dfork(1)) { y = 0xabadcafe; dret(); }
    assert(0 == x); assert(0 == y);
    dget(0, DETERMINE_MERGE, (void*)&x, sizeof(int));
    dget(1, DETERMINE_MERGE, (void*)&y, sizeof(int));
    assert(0xdeadbeef == x); assert(0xabadcafe == y);

    if (!dfork(0)) { x = y; dret(); }
    if (!dfork(1)) { y = x; dret(); }
    assert(0xdeadbeef == x); assert(0xabadcafe == y);
    dget(0, DETERMINE_MERGE, (void*)&x, sizeof(int));
    dget(1, DETERMINE_MERGE, (void*)&y, sizeof(int));
    assert(0xdeadbeef == y); assert(0xabadcafe == x);
#endif

    printf("Merge success.\n");

out:
    return 0;
}
