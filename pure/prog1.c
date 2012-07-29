
#include <inc/determinism_pure.h>
#include <inc/fs.h>
#include <inc/assert.h>
#include <inc/io.h>
#include <inc/mman.h>
#include <inc/syscall.h>

void sp2(unsigned char *a)
{
    while ((unsigned long)a != 0xc0000000)
    {
        printf("*%08lx= %02x\n", a,*a);
        ++a;
    }
}

void sp(unsigned long *a)
{
    a = (unsigned long*)((unsigned long)a&~3);
    while ((unsigned long)a != 0xc0000000)
    {
        printf("*%08lx= %08lx\n", a,*a);
        ++a;
    }
}

    char Test[10000];
int main(int argc, char **argv, char **envp)
{
/*    dput(0, DETERMINE_DEBUG | 0x5000, 0x08048000,0x4000,0);
    dput(0, DETERMINE_DEBUG | 0x6000, 0x08048000,0x4000,0);
    dput(0, DETERMINE_DEBUG | 0x5000, 0xbfffe000,0x1000,0);
    dput(0, DETERMINE_DEBUG | 0x6000, 0xbfffe000,0x1000,0);*/
    int i;
    for (i = 0; i < 10000-1;++i) Test[i] = 'A'+(i%26);
    printf("ARGC=%d\n", argc);
    printf("ARGV[0] = %s\n", argv[0]);
    printf("ARGV[1] %s\n", strcmp(Test, argv[1])==0 ? "yes" : "no");
    i = 0;
    while (envp[i])
    {
        printf("ENV[%d] = %s\n", i, envp[i]);
        ++i;
    }
    dret();
    dret();
    dret();
    //dret();

    return 0;
}

