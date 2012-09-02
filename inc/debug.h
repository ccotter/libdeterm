
#ifndef _INC_DEBUG_H
#define _INC_DEBUG_H

#include <stdarg.h>
#include <types.h>

ssize_t write(int fd, const void *buf, size_t count);
void exit(int status);

/* Debug console output functions. These use the illegal write() syscall. */
/*void cputs(const char *str);			* lib/debug.c */
/* The `i' stands for illegal (deterministic programs can't really use them. */
int	iprintf(const char *fmt, ...);
int	ivprintf(const char *fmt, va_list);

#endif

