
.globl __syscall

/* From abi-0.99.pdf:
 * Arguments passed in left to right order. INTEGER class arguments are passed
 * in %rdi, %rsi, %rdx, %rcx, %r8, %r9. Any other arguments are pushed onto the
 * stack in reverse order.
 *
 * To be saved by the callee:
 * %rbx, %rsp, %rbp, %r12-%r15
 */
__syscall:
    movq 8(%rsp), %rax
    movq %rcx, %r10
    syscall
    ret

