
#ifndef _INC_IO_H_
#define _INC_IO_H_

#include <inc/types.h>

/* This printf version actually uses system call 4 to write directly to
   stdout, violating determinism. Its use is for debugging until I actually
   implement deterministic printf. */
//int iprintf(const char *fmt, ...);

void flush_printf_buffer(void);
int printf(const char *fmt, ...);
int snprintf(char *buf, ssize_t size, const char *fmt, ...);
int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);

int getchar(void);

/* Syncs our file system state with the parent process. */
int io_sync(int needinput);

#endif

