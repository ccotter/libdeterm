
#ifndef _INC_TIME_H_
#define _INC_TIME_H_

#include <sys/time.h>

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

#endif

