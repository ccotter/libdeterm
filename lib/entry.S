
#include <asm/syscall.h>

.globl _start

_start:
	/* Setup arguments to main(argc=%rdi, argv=%rsi, env=%rdx). */
	xor %rbp, %rbp
	pop %rdi
	leaq (%rsp), %rsi /* argv */
	leaq 8(%rsp, %rax, 8), %rdx /* env */
	call main
	movq %rax, %rdi
	call exit
	hlt

