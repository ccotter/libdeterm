
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

int become_deterministic(void)
{
	return syscall5(__NR_dput, 0, DET_BECOME_MASTER, 0, 0, 0);
}

