
#ifndef _INC_DETERMINISM_COMMON_H
#define _INC_DETERMINISM_COMMON_H

#define SYS_dput 341
#define SYS_dget 342
#define SYS_dret 343

/* The low two bytes specify general operations. The high two bytes
   specify flags for a specific flag. */

#define DETERMINE_START             0x0001
#define DETERMINE_REGS              0x0002
#define DETERMINE_VM_COPY           0x0004
#define DETERMINE_ZERO_FILL         0x0008
#define DETERMINE_SNAP              0x0010
#define DETERMINE_MERGE             0x0010
#define DETERMINE_BECOME_MASTER     0x0020
#define DETERMINE_COPY_CHILD        0x0040
#define DETERMINE_CLEAR_CHILD       0x0040
#define DETERMINE_CHILD_STATUS      0x0080
#define DETERMINE_DEBUG             0x8000

/* Change or get a process's state. */
#define DETERMINE_KILL              (0x0001 << 16)

/* These constants can be returned by the three syscalls. */

/* Process is alive and in a state ready to begin execution but is not
   on the scheduler queue. */
#define DETERMINE_S_READY           1
/* Process was put onto the scheduluer (may or may not actually be running). */
#define DETERMINE_S_RUNNING         2
/* Process was killed due to illegal behavior (illegal opcode,
   memory violation, permission violation). */
#define DETERMINE_S_EXCEPT          3
/* Process exited normally. */
#define DETERMINE_S_DEAD            4

#endif

