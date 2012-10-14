
#ifndef _INC_BENCH_H_
#define _INC_BENCH_H_

#include <sys/types.h>

void bench_fork(pid_t pid, void *(*fn)(void*), void* arg);
void bench_join(pid_t pid);

#endif /* _INC_BENCH_H_ */

