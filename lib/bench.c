
#include <stdlib.h>
#include <determinism.h>
#include <bench.h>

void bench_fork(pid_t pid, void *(*fn)(void*), void *arg)
{
	struct user_regs_struct regs;

	get_register_state(&regs);
	pid_t rc = dput_regs(pid, &regs, DET_START | DET_SNAP);
	if (!rc) {
		fn(arg);
		exit(0);
	}
}

void bench_join(pid_t pid)
{
	dget(pid, DET_GET_STATUS, 0, 0, 0);
}

