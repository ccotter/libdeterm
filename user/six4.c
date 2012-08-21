
#include <syscall.h>
#include <string.h>

int main(void);

void _start(void)
{
    main();
}

int main(void)
{
    /* write(1, "hello\n") */
    const char* msg = "hello\n";
    syscall3(__NR_write, 1, (long)msg, strlen(msg));
    /* exit(91) */
    syscall1(__NR_exit, 91);
    return 0;
}

