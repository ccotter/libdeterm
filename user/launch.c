
#include <inc/determinism_pure.h>

int main(void)
{
    if (!fork())
    {
        execve("/det", NULL, NULL);
    }
    while(1);
    return 0;
}

