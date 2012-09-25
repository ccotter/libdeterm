
#ifndef _INC_UNISTD_H
#define _INC_UNISTD_H

#include <sys/types.h>

#ifdef __i386__
#include <unistd_32.h>
#else
#include <unistd_64.h>
#endif

ssize_t write(int fd, const void *buf, size_t count);

#endif /* _INC_UNISTD_H */

