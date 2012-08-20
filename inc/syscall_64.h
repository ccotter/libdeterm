
#ifndef _INC_SYSCALL_64_H
#define _INC_SYSCALL_64_H

#define __NR_write 1
#define __NR_exit 60

#define syscall0(N) \
    __syscall(0, 0, 0, 0, 0, 0, N)
#define syscall1(N, a1) \
    __syscall(a1, 0, 0, 0, 0, 0, N)
#define syscall2(N, a1, a2) \
    __syscall(a1, a2, 0, 0, 0, 0, N);
#define syscall3(N, a1, a2, a3) \
    __syscall(a1, a2, a3, 0, 0, 0, N);
#define syscall4(N, a1, a2, a3, a4) \
    __syscall(a1, a2, a3, a4, 0, 0, N);
#define syscall5(N, a1, a2, a3, a4, a5) \
    __syscall(a1, a2, a3, a4, a5, 0, N);
#define syscall6(N, a1, a2, a3, a4, a5, a6) \
    __syscall(a1, a2, a3, a4, a5, a6, N);

long __syscall(long a1, long a2, long a3, long a4, long a5, long a6, long N);

#endif // _INC_SYSCALL_64_H

