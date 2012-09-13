
.globl __syscall

/* From abi-0.99.pdf:
 * Arguments passed in left to right order. INTEGER class arguments are passed
 * in %rdi, %rsi, %rdx, %rcx, %r8, %r9. Any other arguments are pushed onto the
 * stack in reverse order.
 *
 * AMD x86_64 ABI uses          %rdi %rsi %rdx %rcx %r8 %r9
 * Linux x86_64 common ABI uses %rdi %rsi %rdx %r10 %r8 %r9
 *
 * To be saved by the callee:
 * %rbx, %rsp, %rbp, %r12-%r15
 */
__syscall:
	push %rbp
	mov %rsp, %rbp
	movq 16(%rsp), %rax
	movq %rcx, %r10
	syscall
	pop %rbp
	ret

