
#include <inc/syscall.h>

long __syscall(int num, long a1, long a2, long a3, long a4, long a5)
{
    long ret;
    asm volatile("int %1\n"
            : "=a" (ret)
            : "i" (0x80),
            "a" (num),
            "b" (a1),
            "c" (a2),
            "d" (a3),
            "S" (a4),
            "D" (a5)
            : "cc", "memory");
    return ret;
}

