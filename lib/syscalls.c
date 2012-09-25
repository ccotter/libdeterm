
#include <syscall.h>

ssize_t write(int fd, const void *buf, size_t count)
{
	return syscall3(__NR_write, (long)fd, (long)buf, (long)count);
}

