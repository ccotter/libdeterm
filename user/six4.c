
#include <syscall.h>

int main(void)
{
    syscall1(__NR_exit, 91);
    return 0;
}

