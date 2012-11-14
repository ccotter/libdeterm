
#include <syscall.h>
#include <sys/time.h>

int gettimeofday(struct timeval *restrict tp, void *restrict tzp)
{
	return (int)syscall2(__NR_gettimeofday,
			(unsigned long)tp, (unsigned long)tzp);
}

