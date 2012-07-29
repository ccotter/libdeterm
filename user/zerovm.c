
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>

#define ADDR1 (0x10000000)
//#define SIZE (0x1000*1000*5-1)
#define SIZE (0x1000 * 100)
/* size is 20 MB */

unsigned long count0s(unsigned char *addr, size_t size)
{
    unsigned long r = 0, i;
    for (i = 0; i < size; ++i)
    {
        if (0 == addr[i])
            ++r;
    }
    return r;
}

void fill(unsigned char *addr, size_t size)
{
    unsigned char A = 0;
    unsigned long i;
    for (i = 0; i < size; ++i)
    {
        addr[i] = A++;
    }
}

int check(unsigned char *addr, size_t size)
{
    int ret = 0;
    unsigned long i;
    for (i = 0; i < size; ++i)
    {
        ret += 97 * (addr[i] % (i+3));
    }
    return ret;
}

void child1(int id)
{
    pid_t childid = dput(id, DETERMINE_START | DETERMINE_REGS, (void*)0, 0, 0);
    if (0 == childid)
    {
        printf("Count(1) = %ld\n", count0s((unsigned char*)0x10000000, 0x1000*1234));
        dret();
        printf("Count(1) = %ld\n", count0s((unsigned char*)0x10000000, 0x1000*1234));
        while(1);
    }
}

void child(int id)
{
    pid_t childid = dput(id, DETERMINE_START | DETERMINE_REGS , 0, 0, 0);
    if (0 == childid)
    {
        int i;
        for (i = 0; i < SIZE/0x1000; ++i)
        {
            int rc;
            if ((i+1)%2)
                rc = mprotect((void*)(ADDR1 + 0x1000 * i), 0x1000, PROT_READ);
            else
                rc = mprotect((void*)(ADDR1 + 0x1000 * i), 0x1000, PROT_READ | PROT_WRITE);
            if (rc)
            {
                printf("mprotect failed %08x rc=%d\n", ADDR1 + i*0x1000, rc);
                return;
            }
        }
        printf("Before Check=%d (%d)\n", check((unsigned char*)ADDR1, SIZE), *(unsigned char*)(1+ADDR1));
        dret();
        printf("After Check =%d (%d)\n", check((unsigned char*)ADDR1, SIZE), *(unsigned char*)(1+ADDR1));
        dret();
        while(1);
    }
}

int main(void)
{
    int i;
    void *maddr;
    maddr = mmap((void*)ADDR1, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if ((void*)-1 == maddr)
    {
        printf("Bad mmap.\n");
        goto out;
    }
    //dput(1, DETERMINE_DEBUG, (void*)1, 0);
    fill((unsigned char*)maddr, SIZE);
    child(1);
    //dput(1, DETERMINE_DEBUG, (void*)2, 0);
    /*if (-1 == dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+0x1000-1), SIZE-0x1000+1))
        printf("ZEROING failed\n");
    if (-1 == dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+0x1000-1), SIZE-0x1000+1))
        printf("ZEROING failed\n");
    if (-1 == dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+0x1000-1), SIZE-0x1000+1))
        printf("ZEROING failed\n");
    if (-1 == dput(1, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)(ADDR1), SIZE))
        printf("ZEROING failed\n");
    */
    /*for (i = 0; i < SIZE/0x1000; ++i)
    {
        dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1 + 0x1000 * i), 0x1000);
    }*/
    for (i = 0; i < SIZE-1; ++i)
    {
        dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1 + i), 0x1, 0);
    }
    dput(1, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)(ADDR1 + SIZE-1), 1, 0);
out:
    while(1);
    return 0;
}

int main3(void)
{
    void *A1 = (void*)0x10000000;
    size_t S1 = 0x1000*1234;
    void *maddr;
    maddr = mmap(A1, S1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if ((void*)-1 == maddr)
    {
        printf("Bad mmap.\n");
        goto out;
    }

    memset(maddr, 1, S1);
    child1(1);
    dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+1), 0x1000*2, 0);
    dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+21), 0x1000*4, 0);
    dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+32), 0x1000, 0);
    dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+43), 0x1001, 0);
    dput(1, DETERMINE_ZERO_FILL, (void*)(ADDR1+54), 3, 0);
    dput(1, DETERMINE_START, (void*)0, 0, 0);

out:
    while(1);
    return 0;
}

