
#include <determinism.h>
#include <syscall.h>
#include <unistd.h>
#include <fs.h>
#include <stdlib.h>

void __after_dfork(void);

/* Direct access to syscalls. */
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

/* Higher-level wrappers around deterministic syscalls. */
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

/* Even higher-level abstractions. */

/* Returns -ENOMEM when no there are no available threads.
 * Otherwise, returns 1 to the calling process and returns 0
 * into the newly allocated child process.
 */
int dfork(pid_t pid)
{
	struct user_regs_struct regs;

	get_register_state(&regs);
	pid_t rc = dput_regs(pid, &regs, 0);
	if (rc < 0) {
		return rc;
	} else if (rc > 0) {
		dfs_put(pid);
		dput(pid, DET_START, 0, 0, 0);
		return 1;
	} else {
		__after_dfork();
		return 0;
	}
}

int dfork_fn(pid_t pid, void *(*fn)(void*), void *data)
{
	int rc = dfork(pid);
	if (rc < 0)
		return rc;
	;
	return 0;
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
	return state;
}

