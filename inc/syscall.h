
#ifndef _INC_SYSCALL_H_
#define _INC_SYSCALL_H_

#ifdef __i386__
#include <syscall_32.h>
#else
#include <syscall_64.h>
#endif

#endif // _INC_SYSCALL_H_

