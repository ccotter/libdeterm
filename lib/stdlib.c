
#include <errno.h>
#include <stdlib.h>
#include <determinism.h>
#include <stdarg.h>
#include <syscall.h>
#include <stdio.h>

/* Declare functions used internally by this library. */
void __init(int argc, char **argv, char **envp);
void __panic(const char *msg, ...);
int __dfs_init_clean(void);

#define __DET_MASTER 1
#define __DET_DETERMINISTIC 2

/* Internal variables. */
int __is_init;
int __dfs_init;
int __det_status;
char **__envp;
int __exit_status;

void __init(int argc, char **argv, char **envp)
{
	if (__is_init)
		__panic("Do not call __init() directly!");
	__is_init = 1;
	__envp = envp;

	/* Are we master? */
	switch (become_deterministic()) {
		case 0:
			__det_status = __DET_MASTER;
			break;
		case -EACCES: /* Failure - we are deterministic. */
			__det_status = __DET_DETERMINISTIC;
			break;
		default: /* Error - we can't become master. */
			__panic("Can not become deterministic: illegal memory maps.");
			break;
	}

	if (__dfs_init_clean())
		__panic("Error initializing deterministic file system.");
	__dfs_init = 1;

}

char *getenv(const char *name)
{
	char *at;
	int len = strlen(name);
	for (at = *__envp; at; ++at) {
		if (!strncmp(at, name, len))
			return at + len + 1;
	}
	return NULL;
}

void exit(int status)
{
	if (__DET_DETERMINISTIC == __det_status) {
		__exit_status = 0xff & status;
		dret();
		while(1) dret();
	} else {
		syscall1(__NR_exit, (long)status);
		while(1);
	}
}

char __panic_buffer[1024];
void __panic(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vsnprintf(__panic_buffer, sizeof(__panic_buffer), msg, ap);
	write(2, __panic_buffer, strlen(__panic_buffer));
	write(2, "\n", 1);
	va_end(ap);
	exit(-1);
}

