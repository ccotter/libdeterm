
#ifndef _INC_SYS_TIME_H
#define _INC_SYS_TIME_H

#include <sys/types.h>

struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};

/* tzp must be NULL (undefined otherwise). Always returns 0. */
int gettimeofday(struct timeval *restrict tp, void *restrict tzp);

#endif

