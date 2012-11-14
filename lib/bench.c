
#include <stdio.h>
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
}

extern uint8_t _etext[], _end[];
void bench_join(pid_t pid)
{
	//dget(pid, DET_MERGE, 0x00607000, 0x20000000-0x00607000, 0);
	dget(pid, DET_MERGE, (unsigned long)_etext, (unsigned long)_end, 0);
	dput(pid, DET_KILL, 0, 0, 0);
}

