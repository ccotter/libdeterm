
#include <determinism.h>
#include <syscall.h>

long dput(pid_t childid, long flags, unsigned long start, size_t size,
		unsigned long dststart)
{
	return syscall5(__NR_dput, childid, flags, start, size, dststart);
}

long dget(pid_t childid, long flags, unsigned long start, size_t size,
		unsigned long dststart)
{
	return syscall5(__NR_dget, childid, flags, start, size, dststart);
}

long dret(void)
{
	return syscall0(__NR_dret);
}

long become_deterministic(void)
{
	return syscall5(__NR_dput, 0, DET_BECOME_MASTER, 0, 0, 0);
}

long dget_regs(pid_t pid, struct user_regs_struct *regs, unsigned long flags)
{
	return dget(pid, DET_GENERAL_REGS | flags, (unsigned long)regs, 0, 0);
}

long master_allow_signals(sigset_t *set, size_t size)
{
	return dput(0, DET_ALLOW_SIGNALS, (unsigned long)set, size, 0);
}


