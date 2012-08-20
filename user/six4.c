
#include <syscall.h>

int main(void)
{
    /* write(1, "hello") */
    const char* msg = "hello";
    syscall3(__NR_write, 1, (long)msg, 6);
    /* exit(91) */
    syscall1(__NR_exit, 91);
    return 0;
}

