
#ifndef _INC_DETERMINISM_H
#define _INC_DETERMINISM_H

#include <types.h>

#define DET_START                0x1
#define DET_COPY_REGS            0x2
#define DET_BECOME_MASTER        0x20
#define DET_GET_STATUS           0x80

/* Direct interface to syscalls. */
long dput(pid_t childid, long flags, unsigned long start, size_t size,
		unsigned long dststart);
long dget(pid_t childid, long flags, unsigned long start, size_t size,
		unsigned long dststart);
long dret(void);

int become_deterministic(void);

/* More user-friendly library functions that interface to the syscalls.
 * These library functions are designed to emulate familiar idioms (fork, etc.
 */

#endif

