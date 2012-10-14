
#ifndef _INC_STDLIB_H
#define _INC_STDLIB_H

#include <sys/types.h>

void exit(int status);

char *getenv(const char *name);

long strtol(const char *s, char **endptr, int base);

int io_sync(int needinput);

void *malloc(ssize_t size);
void free(void *ptr);
void *malloc_aligned(ssize_t size, uint32_t alignment, void **actual);

#endif /* _INC_STDLIB_H */

