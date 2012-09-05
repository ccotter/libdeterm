
#ifndef _INC_ASM_DETERMINISM_H
#define _INC_ASM_DETERMINISM_H

#if __i386__
#error "not supported yet"
#endif

#define DET_REGS                 1
#define DET_BECOME_MASTER        2
#define DET_GET_STATUS           3
#define DET_KILL                 4
#define DET_ALLOW_SIGNALS        5

#define DET_GENERAL_REGS         (DET_REGS | 0x100)
#define DET_FP_REGS              (DET_REGS | 0x200)

#if __i386__
#error "not supported yet"
#else

#ifdef __ASSEMBLER__
#define DET_START                (0x0001 << 16)
#define DET_DEBUG     	   	     (0x8000 << 16)
#else
#define DET_START                (0x0001L << 16)
#define DET_DEBUG     	   	     (0x8000L << 16)
#endif

#endif

#endif

