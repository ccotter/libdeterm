
#include <asm/syscall.h>

.globl _start

_start:
	movq $0, %r15
	movq $0, %r14
	call main
	movq %rax, %rdi
	call exit
	hlt

