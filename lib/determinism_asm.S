
#include <asm/determinism.h>
#include <asm/syscall.h>
#include <asm/regs.h>

.globl get_register_state
.globl dput_regs

#define SAVE(ptr, reg, off) movq %reg, off(%ptr)
#define SAVE_IMM(ptr, val, off) movq $val, off(%ptr)

/* Set syscall arguments. */
#define SET_ARG1(val) movq val, %rdi
#define SET_ARG2(val) movq val, %rsi
#define SET_ARG3(val) movq val, %rdx
#define SET_ARG4(val) movq val, %r10
#define SET_ARG5(val) movq val, %r8
#define SET_ARG6(val) movq val, %r9

dput_regs:
	push %rbp
	movq %rsp, %rbp
	SAVE(rsi, rsp, RSP)
	SAVE(rsi, rbp, RBP)
	movq $1f, %rax
	SAVE(rsi, rax, RIP)
	SAVE_IMM(rsi, 0, RAX)
	orq $DET_GENERAL_REGS, %rdx
	movq %rdx, %rax
	SET_ARG3(%rsi)
	SET_ARG2(%rax)
	SET_ARG4($0)
	SET_ARG5($0)
	movq $__NR_dput, %rax
	syscall
1:
	pop %rbp
	ret

get_register_state:
	SAVE(rdi, rbp, RBP);
	push %rbp
	movq %rsp, %rbp
	SAVE(rdi, r15, R15);
	SAVE(rdi, r14, R14);
	SAVE(rdi, r13, R13);
	SAVE(rdi, r12, R12);
	SAVE(rdi, rbx, RBX);
	SAVE(rdi, r11, R11);
	SAVE(rdi, r10, R10);
	SAVE(rdi, r9, R9);
	SAVE(rdi, r8, R8);
	SAVE(rdi, rax, RAX);
	SAVE(rdi, rcx, RCX);
	SAVE(rdi, rax, RAX);
	SAVE(rdi, rax, RAX);
	SAVE(rdi, rcx, RCX);
	SAVE(rdi, rdx, RDX);
	SAVE(rdi, rax, RSI);
	SAVE(rdi, rax, RDI);
	SAVE_IMM(rdi, 0, ORIG_RAX);
	movq 8(%rbp), %rax
	SAVE(rdi, rax, RIP)
	movq %cs, %rax
	SAVE(rdi, rax, CS)
	pushfq
	pop %rax
	SAVE(rdi, rax, EFLAGS)
	movq %rsp, %rax
	addq $16, %rax
	SAVE(rdi, rax, RSP)
	movq %ss, %rax
	SAVE(rdi, rax, SS)
	movq %fs, %rax
	/* TODO what exactly is FS_BASE? See arxh/x86/kernel/ptrace.c */
	SAVE(rdi, rax, FS_BASE)
	movq %gs, %rax
	SAVE(rdi, rax, GS_BASE)
	movq %ds, %rax
	SAVE(rdi, rax, DS)
	movq %es, %rax
	SAVE(rdi, rax, ES)
	movq %fs, %rax
	SAVE(rdi, rax, FS)
	movq %gs, %rax
	SAVE(rdi, rax, GS)
	pop %rbp
	ret

