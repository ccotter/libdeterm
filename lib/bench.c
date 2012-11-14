
#include <stdio.h>
#include <debug.h>
#include <stdlib.h>
#include <determinism.h>
#include <bench.h>

#include <sys/time.h>

/*
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file NOTICES for licensing terms.
 *
 * Original author: Bryan Ford at Yale University.
 * Adopted by Chris Cotter for deterministic linux evaluation.
 */


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
	dget(pid, DET_MERGE, (unsigned long)_etext, (unsigned long)_end, 0);
	dput(pid, DET_KILL, 0, 0, 0);
}

/* This will cause a fault if called by deterministic processes, since it
 * uses a disallowed syscall. Instead, we could be nice and return an error. */
uint64_t
bench_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec * 1000000 + tv.tv_usec) * 1000; // ns
}

