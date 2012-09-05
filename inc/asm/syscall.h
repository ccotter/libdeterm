
#ifndef _INC_ASM_SYSCALL_H
#define _INC_ASM_SYSCALL_H

#ifdef __i386__
#include <asm/syscall_32.h>
#else
#include <asm/syscall_64.h>
#endif

#endif

