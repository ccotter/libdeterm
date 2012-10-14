
#include <sys/mman.h>

void *mmap(void *addr, size_t len, int prot, int flags,
		int fildes, off_t off)
{
	return (void*)syscall6(__NR_mmap,
			(unsigned long)addr, (unsigned long)len, (unsigned long)prot,
			(unsigned long)flags, (unsigned long)fildes, (unsigned long)off);
}

int munmap(void *addr, size_t len)
{
	return (int)syscall2(__NR_munmap, (unsigned long)addr, (unsigned long)len);
}

