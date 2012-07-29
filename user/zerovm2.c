
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>

#define ADDR1 (0x10000000)
//#define SIZE (0x1000*1000*5-539)
#define SIZE (0x1000 * 5-123)
/* size is 20 MB */

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
        ret += 97 * ret + addr[i] % (i?i:1);
    }
    return ret;
}

void child(int id)
{
    pid_t childid = dput(id, DETERMINE_START | DETERMINE_REGS , 0, 0, 0);
    if (0 == childid)
    {
        printf("(%d) Before Check=%d\n", id, check((unsigned char*)ADDR1, SIZE));
        dret();
        printf("(%d) After Check =%d\n", id, check((unsigned char*)ADDR1, SIZE));
        fill((unsigned char*)ADDR1, SIZE-id);
        printf("(%d) Second After Check =%d\n", id, check((unsigned char*)ADDR1, SIZE));
        dret();
        printf("(%d) Third After Check =%d\n", id, check((unsigned char*)ADDR1, SIZE));
        while(1);
    }
}

int main(void)
{
    void *maddr;
    maddr = mmap((void*)ADDR1, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if ((void*)-1 == maddr)
    {
        printf("Bad mmap.\n");
        goto out;
    }
    //dput(1, DETERMINE_DEBUG, 1, 0);
    printf("Going to fill... %08lx\n", *(unsigned long*)maddr);
    fill((unsigned char*)maddr, SIZE);
    printf("Finished filling.\n");
    child(1);
    child(2);
    child(3);
    dput(1, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)ADDR1, SIZE, 0);
    dput(2, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)ADDR1, SIZE, 0);
    dput(3, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)ADDR1, SIZE, 0);
    dput(1, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)ADDR1+233, SIZE, 0);
    dput(2, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)ADDR1+233, SIZE, 0);
    dput(3, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)ADDR1+233, SIZE, 0);
out:
    while(1);
    return 0;
}

