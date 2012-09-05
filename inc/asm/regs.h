
#ifndef _INC_ASM_REGS_H
#define _INC_ASM_REGS_H

#ifdef __i386__
#error "i386"
#endif

#define R15 0
#define R14 8
#define R13 16
#define R12 24
#define RBP 32
#define RBX 40
#define R11 48
#define R10 56
#define R9 64
#define R8 72
#define RAX 80
#define RCX 88
#define RDX 96
#define RSI 104
#define RDI 112
#define ORIG_RAX 120
#define RIP 128
#define CS 136
#define EFLAGS 144
#define RSP 152
#define SS 160
#define FS_BASE 168
#define GS_BASE 176
#define DS 184
#define ES 192
#define FS 200
#define GS 208

#define S_R15 "R15"
#define S_R14 "R14"
#define S_R13 "R13"
#define S_R12 "R12"
#define S_RBP "RBP"
#define S_RBX "RBX"
#define S_R11 "R11"
#define S_R10 "R10"
#define S_R9 "R9"
#define S_R8 "R8"
#define S_RAX "RAX"
#define S_RCX "RCX"
#define S_RDX "RDX"
#define S_RSI "RSI"
#define S_RDI "RDI"
#define S_ORIG_RAX "ORIG_RAX"
#define S_RIP "RIP"
#define S_CS "CS"
#define S_EFLAGS "EFLAGS"
#define S_RSP "RSP"
#define S_SS "SS"
#define S_FS_BASE "FS_BASE"
#define S_GS_BASE "GS_BASE"
#define S_DS "DS"
#define S_ES "ES"
#define S_FS "FS"
#define S_GS "GS"

#define REG_OFF_0 R15
#define REG_OFF_1 R14
#define REG_OFF_2 R13
#define REG_OFF_3 R12
#define REG_OFF_4 RBP
#define REG_OFF_5 RBX
#define REG_OFF_6 R11
#define REG_OFF_7 R10
#define REG_OFF_8 R9
#define REG_OFF_9 R8
#define REG_OFF_10 RAX
#define REG_OFF_11 RCX
#define REG_OFF_12 RDX
#define REG_OFF_13 RSI
#define REG_OFF_14 RDI
#define REG_OFF_15 ORIG_RAX
#define REG_OFF_16 RIP
#define REG_OFF_17 CS
#define REG_OFF_18 EFLAGS
#define REG_OFF_19 RSP
#define REG_OFF_20 SS
#define REG_OFF_21 FS_BASE
#define REG_OFF_22 GS_BASE
#define REG_OFF_23 DS
#define REG_OFF_24 ES
#define REG_OFF_25 FS
#define REG_OFF_26 GS

#define REG_OFF(i) REG_OFF_##i

#endif

