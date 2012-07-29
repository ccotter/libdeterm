
#include <inc/determinism_common.h>

.text
.global _dstart
.global _start_exec

/* Non deterministic processes won't ever jump here, but deterministic processes
   will start at this address. */
_dstart:

    /* Linux syscall register order: a b c d S D */
/* Similar to _start except that this is called when we perform a user level dexec. */
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

