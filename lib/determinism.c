
#include <determinism.h>
#include <syscall.h>
#include <unistd.h>
#include <fs.h>

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

#define NMAX_THREADS 100
struct dthread_state
{
	pid_t id;
	int used;
};

static struct dthread_state __threads[NMAX_THREADS];

void __dthread_init(void)
{
	/* We disallow pid=0. */
	__threads[0].used = 1;
	__threads[0].id = -1;
}

static inline int __get_avail_pid(void)
{
	size_t i;
	for (i = 0; i < NMAX_THREADS; ++i) {
		if (!__threads[i].used) {
			__threads[i].used = 1;
			__threads[i].id = i;
			return i;
		}
	}
	return -ENOMEM;
}

/* Returns -ENOMEM when no there are no available threads.
 * Otherwise, returns the allocated thread process id. */
pid_t dfork(unsigned long flags)
{
	struct user_regs_struct regs;
	pid_t avail = __get_avail_pid();

	if (avail < 0)
		return avail;

	get_register_state(&regs);
	pid_t rc = dput_regs(avail, &regs, flags);
	if (rc < 0) {
		return rc;
	} else if (rc > 0) {
		dfs_put(avail);
		dput(avail, DET_START, 0, 0, 0);
		return avail;
	} else {
		return 0;
	}
}

