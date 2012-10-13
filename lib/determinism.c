
#include <determinism.h>
#include <syscall.h>
#include <unistd.h>
#include <fs.h>
#include <stdlib.h>

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

static inline void __put_pid(pid_t pid)
{
	__threads[pid].used = 0;
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

int dwait(pid_t pid)
{
	int state;
	for (;;) {
		int out_fd;
		state = dfs_childstate(pid);
		if (state < 0)
			return -1;
		if (dfs_get(pid))
			return -1;
		io_sync(0);
		if ((DPROC_EXITED | DPROC_FAULTED) & state)
			break;
		if (DPROC_INPUT & state)
			io_sync(1);
		if (!dfs_put(pid))
			return -1;
		dput(pid, DET_START, 0, 0, 0);
	}
	dput(pid, DET_KILL, 0, 0, 0);
	__put_pid(pid);
	return state;
}

pid_t dfork_fn(void *(fn)(void*), void *arg)
{
	pid_t rc = dfork(DET_START);
	if (!rc) {
		fn(arg);
		return 0;
	} else {
		return rc;
	}
}

