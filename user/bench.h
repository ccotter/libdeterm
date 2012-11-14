
#ifndef _BENCH_H_
#define _BENCH_H_

#include <sys/time.h>

/*
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file NOTICES for licensing terms.
 *
 * Original author: Bryan Ford at Yale University.
 * Adopted by Chris Cotter for deterministic linux evaluation.
 */

uint64_t
bench_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec * 1000000 + tv.tv_usec) * 1000; // ns
}

#endif

