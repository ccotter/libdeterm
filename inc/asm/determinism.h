
#ifndef _INC_ASM_DETERMINISM_H
#define _INC_ASM_DETERMINISM_H

#if __i386__
#error "not supported yet"
#endif

#define DET_REGS                 1
#define DET_BECOME_MASTER        2
#define DET_GET_STATUS           4
#define DET_KILL                 8
#define DET_ALLOW_SIGNALS        16
#define DET_VM_ZERO              32
#define DET_VM_COPY              64
#define DET_SNAP                 128
#define DET_MERGE                256

#define DET_DONT_WAIT            0x10000
#define DET_KILL_POISON          0x10000

#define DET_PROT_READ            0x10000
#define DET_PROT_WRITE           0x20000
#define DET_PROT_EXEC            0x40000
#define DET_PROT_NONE            0x00000

#define DET_START                0x01000000
#define DET_DEBUG     	   	     0x80000000

#define DET_GENERAL_REGS         (DET_REGS | 0x10000)
#define DET_FP_REGS              (DET_REGS | 0x20000)

#define DET_S_READY   1 /* Alive and runnable (not in run queue). */
#define DET_S_RUNNING 2 /* Alive and in run queue. */
#define DET_S_EXCEPT  3 /* Process killed due to illegal behavior. */
#define DET_S_EXIT_NORMAL    4 /* Process exited normally. */
#define DET_S_EXCEPT_DEAD    5 /* When an excepted task was killed explicitly by the parent. */

#endif

