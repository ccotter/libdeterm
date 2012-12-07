
#ifndef _INC_UNISTD_H
#define _INC_UNISTD_H

#include <sys/types.h>

#ifdef __i386__
#include <unistd_32.h>
#else
#include <unistd_64.h>
#endif

extern char *optarg;
int getopt(int argc, char * argv[], const char * optstring);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
unsigned int sleep(unsigned int seconds);

#endif /* _INC_UNISTD_H */

