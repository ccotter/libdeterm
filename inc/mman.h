
#ifndef _INC_MMAN_H
#define _INC_MMAN_H

#include <syscall.h>

#define PROT_READ    0x1
#define PROT_WRITE   0x2
#define PROT_EXEC    0x4

#define MAP_PRIVATE     0x02      /* Changes are private */
#define MAP_FIXED       0x10      /* Interpret addr exactly */
#define MAP_ANONYMOUS   0x20      /* don't use a file */

static inline void *mmap(void *addr, size_t len, int prot, int flags,
		int fildes, off_t off)
{
	return (void*)syscall6(__NR_mmap,
			(unsigned long)addr, (unsigned long)len, (unsigned long)prot,
			(unsigned long)flags, (unsigned long)fildes, (unsigned long)off);
}

static inline int munmap(void *addr, size_t len)
{
	return (int)syscall2(__NR_munmap, (unsigned long)addr, (unsigned long)len);
}

#define PAGE_SIZE 0x1000
#define PAGE_MASK (~(PAGE_SIZE-1))

#define PAGE_ALIGN(addr) ((addr) + (PAGE_SIZE - (((addr)-1) & ~PAGE_MASK)) - 1)
#define LOWER_PAGE(addr) PAGE_ALIGN((addr) - PAGE_SIZE + 1)

#endif

