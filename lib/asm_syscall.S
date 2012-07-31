
/* 32-bit implementation. Jumps into linux kernel mode. */

.global syscall6

.data
return_addr:
    .long 0

.text
/* System calls with more than 5 args must push the args onto the stack. We accomplish
   this by moving the stack pointer back 12 bytes since the stack is already aligned
   to go into the syscall. We save the old return address of the calling code. */
syscall6:
    push %rbp
    movl %esp, %ebp
    movl %ebx, %ecx /* Save %ebx. */

    movl 4(%esp), %edx
    movl %edx, return_addr
    movl 8(%ebp), %eax
    addl $12, %esp
    movl %esp, %ebx
    int $0x80
    addl $12, %esp
    movl return_addr, %edx
    movl %edx, 4(%ebp)

    movl %ecx, %ebx
    movl %ebp, %esp
    pop %rbp
    ret

