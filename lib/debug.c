
/*
 * Debugging support code for user-space programs.
 *
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Derived from the MIT Exokernel and JOS.
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

/*
 * Adapted from PIOS for use in libdeterm by Chris Cotter.
 * Aug 2012.
 */

#include <debug.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <syscall.h>

char *argv0;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: <message>", then causes a breakpoint exception.
 */
void
debug_panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;
	va_start(ap, fmt);

	// Print the panic message
	if (argv0)
		iprintf("%s: ", argv0);
	iprintf("user panic at %s:%d: ", file, line);
	ivprintf(fmt, ap);
	iprintf("\n");

    exit(-1);
	while(1)
		;
}

/* like panic, but don't */
void
debug_warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	iprintf("user warning at %s:%d: ", file, line);
	ivprintf(fmt, ap);
	iprintf("\n");
	va_end(ap);
}

// Dump a block of memory as 32-bit words and ASCII bytes
void
debug_dump(const char *file, int line, const void *ptr, int size)
{
	iprintf("user dump at %s:%d of memory %08x-%08x:\n",
		file, line, ptr, ptr + size);
	for (size = (size+15) & ~15; size > 0; size -= 16, ptr += 16) {
		char buf[100];

		// ASCII bytes
		int i; 
		const uint8_t *c = ptr;
		for (i = 0; i < 16; i++)
			buf[i] = isprint(c[i]) ? c[i] : '.';
		buf[16] = 0;

		// Hex words
		const uint32_t *v = ptr;

		// Print each line atomically to avoid async mixing
		iprintf("%08x: %08x %08x %08x %08x %s",
			ptr, v[0], v[1], v[2], v[3], buf);
	}
}

int iprintf(const char *fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = ivprintf(fmt, ap);
    va_end(ap);
    return ret;
};

int ivprintf(const char *fmt, va_list ap)
{
	int ret;
    static char buffer[4096];
    ret = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	write(1, buffer, ret);
	return ret;
}

static const char *regnames[] = {
	"R15",
	"R14",
	"R13",
	"R12",
	"RBP",
	"RBX",
	"R11",
	"R10",
	"R9",
	"R8",
	"RAX",
	"RCX",
	"RDX",
	"RSI",
	"RDI",
	"ORIG_RAX",
	"RIP",
	"CS",
	"EFLAGS",
	"RSP",
	"SS",
	"FS_BASE",
	"GS_BASE",
	"DS",
	"ES",
	"FS",
	"GS",
};

static inline void print_reg(const struct user_regs_struct *regs, int off)
{
	iprintf("%10s: %016lx    ", regnames[off],
			*(unsigned long*)(((char*)regs) + off * sizeof(long)));
}

void print_regs(const struct user_regs_struct *regs)
{
	unsigned int i;
	for (i = 0; i < sizeof(*regs) / sizeof(long); ++i) {
		print_reg(regs, i);
		if (((i+1) % 3) == 0)
			iprintf("\n");
	}
	if ((i % 3) != 0)
		iprintf("\n");
}

