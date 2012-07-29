
#ifndef _INC_DETERMINISM_PURE_H_
#define _INC_DETERMINISM_PURE_H_

#include <inc/determinism_common.h>

#ifndef __ASSEMBLER__

#include <inc/types.h>
#include <inc/string.h>

typedef int pid_t;

/* Lower level system calls. */
long dput(pid_t child, int flags, void *start, size_t size, void *dst);
long dget(pid_t child, int flags, void *start, size_t size, void *dst);
long dret(void);

int become_deterministic(void);

#endif /* __ASSEMBLER__ */

#endif 

