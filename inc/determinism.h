
#ifndef _INC_DETERMINISM_H_
#define _INC_DETERMINISM_H_

#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <inc/determinism_common.h>

int iprintf(const char *fmt, ...);

// System call declarations.
long dput(pid_t child, int flags, void *start, size_t size, void *dst);

long dget(pid_t child, int flags, void *start, size_t size, void *dst);

long dret(void);

int become_deterministic(void);

#endif

