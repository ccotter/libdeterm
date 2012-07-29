
#include <stdio.h>
#include <sys/mman.h>
#include <inc/determinism.h>
#include <fcntl.h>
#include <syscall.h>
#include <string.h>

pid_t getpid2(void)
{
    return syscall(SYS_getpid); // getpid caches pid's (how though?)
}

#define ADDR1 (0x10000000)
#define SIZE (0x1000*15)

void child(int id)
{
    pid_t childid = dput(id, DETERMINE_START | DETERMINE_REGS , 0, 0, 0);
    if (0 == childid)
    {
        char BUF[10];
        void *mret;
        int fd;
        snprintf(BUF, sizeof(BUF), "/f%d", id);
        fd = open(BUF, O_CREAT | O_RDWR);
        if (fd < 0)
        {
            printf("Child %d failed with fd=%d\n", id, fd);
            dret();
            return;
        }
        mret = mmap((void*)(ADDR1 * id), SIZE, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
        if ((void*)-1 == mret)
        {
            printf("mmap %d returned failed\n", id);
            return;
        }
        snprintf((char*)mret, SIZE, "Hello from child %d", id);
        dret();
        printf("Child(%d) str (%s)\n", id, (char*)mret);
        snprintf((char*)mret, SIZE, "BYE from child %d", id);
        printf("Child(%d) str (%s)\n", id, (char*)mret);
        msync(mret, SIZE, MS_SYNC);
        dret();
        printf("LAST Child(%d) str (%s)\n", id, (char*)mret);
        munmap(mret, SIZE);
        close(fd);
        while(1);
    }
    //while(1);
}

static void readit(int id)
{
    int fd, rc;
    char BUF[10];
    char BUF2[100];
    void *mret;
    snprintf(BUF, sizeof(BUF), "/f%d", id);
    fd = open(BUF, O_RDONLY);
    if (fd < 0)
    {
        printf("Read %d says fd=%d\n", id, fd);
        return;
    }
        /*mret = mmap((void*)(ADDR1 * (id+2)), SIZE, PROT_READ, MAP_FIXED | MAP_SHARED, fd, 0);
        if ((void*)-1 == mret)
        {
            printf("mmap %d returned failed\n", id);
            return;
        }
        printf("Parent %d string originally is (%s)\n", id, (char*)mret);*/
    rc = read(fd, BUF2, sizeof(BUF2));
    BUF2[rc] = 0;
    printf("File %d (%d) is %s\n", id, rc, BUF2);
}

int main(void)
{
    child(1);
    child(2);
    dput(1, 0, 0, 0, 0);
    dput(2, 0, 0, 0, 0);
    //dput(1, DETERMINE_DEBUG, 2, 0);
    //dput(2, DETERMINE_DEBUG, 2, 0);
    printf("Parent to read files...\n");
    readit(1);
    readit(2);
    dput(1, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)(0x10000000), SIZE, 0);
    dput(2, DETERMINE_START | DETERMINE_ZERO_FILL, (void*)(0x20000000), SIZE, 0);
    printf("Parent to read files...AGAIN\n");
    readit(1);
    readit(2);
    printf("GOing \n");
    dput(1, DETERMINE_START, 0, 0, 0);
    dput(2, DETERMINE_START, 0, 0, 0);
    printf("DONE \n");
    dput(1, DETERMINE_START, 0, 0, 0);
    dput(2, DETERMINE_START, 0, 0, 0);
    //dput(1, DETERMINE_DEBUG, 1, 0);
    while(1);
    return 0;
}

