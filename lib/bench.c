
#include <debug.h>
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
		dret();
	}
	iprintf("dput(%d): %d\n", pid, rc);
}

void bench_join(pid_t pid)
{
	iprintf("dget(%d)=%d\n",pid, dget(pid, DET_MERGE, 0x40000000, 0xc0000000, 0));
	dput(pid, DET_KILL, 0, 0, 0);
}

