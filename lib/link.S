
#include <inc/determinism_pure.h>

.text
.global _dstart
.global _start_exec
.extern _main_setup
.extern _exit

_dstart:
    xor %ebp, %ebp
    pop %rax # argc
    leal (%esp), %esi # argv
    leal 4(%esp, %eax, 4), %edi # env
    push %rdi
    push %rsi
    push %rax
    call _main_setup
    call main
    push %rax
do_exit:
    call _exit
    jmp do_exit
    hlt

    /* Linux syscall register order: a b c d S D */
/* Similar to _dstart except that this is called when we perform a user level dexec. */
_start_exec:
    movl 4(%esp), %ebx  # childid
    movl 8(%esp), %esp  # new stack
    movl $DETERMINE_COPY_CHILD, %ecx
    movl $SYS_dget, %eax
    movl $0, %edx
    movl $0, %esi
    movl $0, %edi
    int $0x80
    movl $SYS_dput, %eax
    movl $DETERMINE_CHILD_STATUS|DETERMINE_KILL, %ecx
    int $0x80
    jmp _dstart

