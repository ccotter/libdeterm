
/* For use with ptrace and dput/dget syscalls.
 *
 * As yet incomplete. */

#ifndef _INC_SYS_USER_H
#define _INC_SYS_USER_H

#ifdef __i386__

#error "Not yet implemented."

#else /* not __i386__ */

struct user_regs_struct
{
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long rbp;
	unsigned long rbx;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long rax;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsi;
	unsigned long rdi;
	unsigned long orig_rax;
	unsigned long rip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long rsp;
	unsigned long ss;
	unsigned long fs_base;
	unsigned long gs_base;
	unsigned long ds;
	unsigned long es;
	unsigned long fs;
	unsigned long gs;
};

#endif

#endif /* _INC_SYS_USER_H */

