
#include <errno.h>
#include <stdlib.h>
#include <determinism.h>
#include <stdarg.h>
#include <syscall.h>
#include <stdio.h>
#include <fs.h>

static struct printbuf pb, master_pb;
static void flush_printf_buffer(struct printbuf *pb);

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

void __init_malloc(void);

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

	dfs_setstate(DPROC_READY);
	__init_malloc();

}

void __after_dfork(void)
{
	/* After a dfork(), set ourselves up properly. */
	__det_status = __DET_DETERMINISTIC;
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
		flush_printf_buffer(&pb);
		dret();
		while(1)
			dret();
	} else {
		/* Flush buffers. */
		flush_printf_buffer(&master_pb);
		syscall1(__NR_exit, (long)status);
		/* Shouldn't get here. */
		while(1)
			;
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
	exit(1);
}

#define PRINT_BUFFER_SIZE 512
struct printbuf
{
	char buf[PRINT_BUFFER_SIZE];
	int idx;
};
static void putch_printf(int ch, void *data)
{
	pb.buf[pb.idx] = (char)ch;
	++pb.idx;
	++*(int*)data;
	if (PRINT_BUFFER_SIZE - 1 == pb.idx) {
		dfs_write(1, pb.buf, PRINT_BUFFER_SIZE - 1);
		pb.idx = 0;
	}
}

static void putch_master_printf(int ch, void *data)
{
	master_pb.buf[master_pb.idx] = (char)ch;
	++master_pb.idx;
	++*(int*)data;
	if (PRINT_BUFFER_SIZE - 1 == master_pb.idx)
		flush_printf_buffer(&master_pb);
}

static void flush_printf_buffer(struct printbuf *pb)
{
	if (__DET_MASTER == __det_status) {
		write(1, pb->buf, pb->idx);
	}
	else
		dfs_write(1, pb->buf, pb->idx);
	pb->idx = 0;
}

int printf(const char *fmt, ...)
{
	int count;
	va_list ap;
	va_start(ap, fmt);

	if (__det_status == __DET_MASTER) {
		vprintfmt(putch_master_printf, &count, fmt, ap);
		flush_printf_buffer(&master_pb);
	} else {
		vprintfmt(putch_printf, &count, fmt, ap);
	}

	va_end(ap);
	return 0;
}

int io_sync(int needinput)
{
	long off = dfs_tell(0);
	if (needinput)
		dfs_setstate(DPROC_INPUT);
	flush_printf_buffer(&pb);
	dret();
	dfs_seek(0, off, DSEEK_SET);
	dfs_setstate(DPROC_READY);
	return 0;
}

int getchar(void)
{
	int n;
	char c;
begin:
	n = dfs_read(0, &c, 1);
	if (!n) {
		io_sync(1);
		n = dfs_read(0, &c, 1);
	}
	if (1 == n)
		return (int)c;
	else if (n < 0)
		return EOF;
	else
		return EOF;
}

