
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>
#include <assert.h>

#define ADDR1 (0x20000000)
#define SIZE1 0x1000

#define ADDR (0x10000000)
#define SIZE (0x1000 * 16)
/* size is 20 MB */

static void fill(unsigned char *A, size_t size, int entropy)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        A[i] = entropy * i;
    }
}

static int check(unsigned char *A, size_t size, int entropy)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        if ((unsigned char)(entropy * i) != A[i])
            return 0;
    }
    return 1;
}

/*static void child1(int id)
{
    pid_t childid = dput(id, DETERMINE_START | DETERMINE_REGS , (void*)0, 0, (void*)0);
    if (0 == childid)
    {
        int i;
        void *maddr = (void*)ADDR;
        if (check((unsigned char*)maddr, SIZE, 13))
            printf("Check 1 failed\n");
        //if (check((unsigned char*)maddr, SIZE, 12))
        //    printf("Check 2 failed\n");
        dret();
        printf("String is (%s)\n", (char*)ADDR1);
        for (i = 0; i < SIZE/0x1000; ++i)
        {
            if (i%2)
                mprotect((void*)(ADDR + i * 0x1000), 0x1000, PROT_READ);
        }
        dret();
        if (!check((unsigned char*)maddr, SIZE, 13))
            printf("Check 3 failed\n");
        if (check((unsigned char*)maddr, SIZE, 12))
            printf("Check 4 failed\n");
        printf("Child done\n");
        printf("String is (%s)\n", (char*)ADDR1);
        strcpy((char*)ADDR1, "Bye bye");
        printf("String is (%s)\n", (char*)ADDR1);
        dret();
        while(1);
    }
}*/

static void fill2(unsigned long *addr, size_t size)
{
    int i;
    size /= sizeof(unsigned long);
    for (i = 0; i < size; ++i)
    {
        addr[i] = i * i;
    }
}

static int check2(unsigned long *addr, size_t size)
{
    int i;
    size /= sizeof(unsigned long);
    for (i = 0; i < size; ++i)
    {
        if ((i*i) != addr[i])
            return 0;
    }
    return 1;
}

static void child(int id)
{
    pid_t childid = dput(id, DETERMINE_START | DETERMINE_REGS, 0, 0, 0);
    if (0 == childid)
    {
        unsigned long *laddr = (unsigned long*)ADDR;
        char *caddr = (char*)ADDR;
        dret();
        printf("Child %d\n", id);
        while(1);
        if (!check2(laddr, SIZE))
            printf("Child %d filed mem check\n", id);
        dret();
        printf("Child again %d\n", id);
        laddr[1024*4*id] = 0;
        printf("Addr is %08lx\n", (unsigned long)&laddr[1024*4*id]);
        dret();
        printf("OOPS\n");
    }
}

int main(void)
{
    int i, len, rc;
    void *maddr, *maddr2;
    unsigned long *laddr, *laddr2;
    char *caddr, *caddr2;

    //rc = dput(0, DETERMINE_BECOME_MASTER, 0, 0, (void*)0x33000);
    //assert(1 == rc);
    become_deterministic();

    assert((void*)ADDR == (maddr = mmap((void*)ADDR, SIZE, PROT_READ |
                    PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
                    -1, 0)));
    /*maddr2 = mmap((void*)ADDR1, SIZE1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if ((void*)-1 == maddr2)
    {
        printf("Bad map2.\n");
        goto out;
    }*/
    laddr = (unsigned long*)maddr;
    printf("laddr is %08lx\n", maddr);
    fill2(laddr, SIZE);
    child(1);
    child(2);
    child(3);
    dput(1, DETERMINE_START, maddr, SIZE, maddr);
    return 0;
    //dput(2, DETERMINE_START | DETERMINE_VM_COPY, maddr, SIZE, maddr);
    //dput(3, DETERMINE_START | DETERMINE_VM_COPY, maddr, SIZE, maddr);
    dput(1, DETERMINE_START, 0, 0, (void*)0x33000);
    dput(1, DETERMINE_START, 0, 0, (void*)0x33000);
    dput(1, DETERMINE_START, 0, 0, (void*)0x33000);
    dput(1, DETERMINE_START, 0, 0, (void*)0x33000);
    dput(1, DETERMINE_START, 0, 0, (void*)0x33000);
    //dput(2, DETERMINE_START, 0, 0, 0);
    //dput(3, DETERMINE_START, 0, 0, 0);
    printf("Parent Done!\n");
out:
    while(1);
    return 0;
}
#if 0
int main1(void)
{
    int i;
    void *maddr, *maddr2;
    maddr = mmap((void*)ADDR, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if ((void*)-1 == maddr)
    {
        printf("Bad mmap.\n");
        goto out;
    }
    maddr2 = mmap((void*)ADDR1, SIZE1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if ((void*)-1 == maddr2)
    {
        printf("Bad map2.\n");
        goto out;
    }
    child(1);
    strcpy((char*)ADDR1, "Hello VM_COPY");
    fill((unsigned char*)maddr, SIZE, 13);
    dput(1, DETERMINE_START | DETERMINE_VM_COPY, (void*)(ADDR1), strlen((char*)ADDR1)-5);
    //dput(1, DETERMINE_DEBUG, (void*)2, 0);
    for (i = 0; i < SIZE; i+=12)
    {
        dput(1, DETERMINE_VM_COPY, (void*)(ADDR+i), 1);
        dput(1, DETERMINE_VM_COPY, (void*)(ADDR+i+1), 2);
        dput(1, DETERMINE_VM_COPY, (void*)(ADDR+i+3), 3);
        dput(1, DETERMINE_VM_COPY, (void*)(ADDR+i+6), 4);
        dput(1, DETERMINE_VM_COPY, (void*)(ADDR+i+10), 2);
    }
    dput(1, DETERMINE_VM_COPY, (void*)(ADDR1+strlen((char*)ADDR1)-5), 5);
    dput(1, DETERMINE_START | DETERMINE_VM_COPY, (void*)(ADDR+SIZE-1), 1);
    dget(1, 0, 0, 0);
    printf("String final is (%s)\n", (char*)ADDR1);
out:
    while(1);
    return 0;
}
#endif
