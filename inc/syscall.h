
#ifndef _INC_SYSCALL_H_
#define _INC_SYSCALL_H_

#define SYS_mmap 90
#define SYS_munmap 91
#define SYS_brk 45
#define SYS_write 4
#define SYS_exit 1

#define syscall0(N) __syscall(N, 0, 0, 0, 0, 0)
#define syscall1(N, a1) __syscall(N, a1, 0, 0, 0, 0)
#define syscall2(N, a1, a2) __syscall(N, a1, a2, 0, 0, 0)
#define syscall3(N, a1, a2, a3) __syscall(N, a1, a2, a3, 0, 0)
#define syscall4(N, a1, a2, a3, a4) __syscall(N, a1, a2, a3, a4, 0)
#define syscall5(N, a1, a2, a3, a4, a5) __syscall(N, a1, a2, a3, a4, a5)

long syscall6(int N,
        unsigned long a1,
        unsigned long a2,
        unsigned long a3,
        unsigned long a4,
        unsigned long a5,
        unsigned long a6);

long __syscall(int num, long a1, long a2, long a3, long a4, long a5);

#endif

