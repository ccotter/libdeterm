
.globl _init
.globl _start

_start:
	/* Setup arguments to main(argc=%rdi, argv=%rsi, env=%rdx). */
	xor %rbp, %rbp
	movq 0(%rsp), %rdi
	leaq 8(%rsp), %rsi /* argv */
	leaq 16(%rsp, %rax, 8), %rdx /* env */
	call __init
	movq 0(%rsp), %rdi
	leaq 8(%rsp), %rsi /* argv */
	leaq 16(%rsp, %rax, 8), %rdx /* env */
	call main
	movq %rax, %rdi
	call exit
	hlt

