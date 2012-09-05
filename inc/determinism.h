
#ifndef _INC_DETERMINISM_H
#define _INC_DETERMINISM_H

#include <sys/user.h>
#include <types.h>
#include <signal.h>
#include <asm/determinism.h>

/* Direct interface to syscalls. */
long dput(pid_t childid, long flags, unsigned long start, size_t size,
		unsigned long dststart);
long dget(pid_t childid, long flags, unsigned long start, size_t size,
		unsigned long dststart);
long dret(void);

long become_deterministic(void);

/* More user-friendly library functions that interface to the syscalls.
 * These library functions are designed to emulate familiar idioms (fork, etc.
 */
void get_register_state(struct user_regs_struct *regs);
long dput_regs(pid_t pid, const struct user_regs_struct *regs, unsigned long flags);
long dget_regs(pid_t pid, struct user_regs_struct *regs, unsigned long flags);
long master_allow_signals(sigset_t *set, size_t size);

#endif

