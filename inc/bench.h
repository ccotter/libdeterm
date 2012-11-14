
#ifndef _INC_BENCH_H_
#define _INC_BENCH_H_

#include <sys/types.h>

void bench_fork(pid_t pid, void *(*fn)(void*), void* arg);
void bench_join(pid_t pid);
void bench_join2(pid_t pid, void *start, size_t len);
uint64_t bench_time(void);

#endif /* _INC_BENCH_H_ */

