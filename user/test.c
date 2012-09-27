
#include <determinism.h>
#include <debug.h>
#include <string.h>
#include <mman.h>
#include <syscall.h>
#include <stdlib.h>

static inline void die(int x)
{
	iprintf("Died: %d\n", x);
	exit(1);
}

static inline int fork(void)
{
	return (int)syscall0(__NR_fork);
}

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2

static inline int open(const char *path, int oflag)
{
	return (int)syscall2(__NR_open, (unsigned long)path, (unsigned long)oflag);
}

int main(void)
{
	int rc, i, j, fd;
	sigset_t set;
	if (0 > (rc = become_deterministic()))
		die(rc);

	sigemptyset(&set);
	sigdelset(&set, SIGINT);
	if (0 > (rc = master_allow_signals(&set, sizeof(set))))
		die(rc);

	fd = open("/root/FILE", O_RDWR);
	iprintf("fd=%d\n", fd);
	//unsigned char *addr = mmap(0, 0, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fork()) {
		while(1);
	} else {
		while(1);
		iprintf("child\n");
		return 0;
	}

out:
	iprintf("Success\n");
	return 0;
}

