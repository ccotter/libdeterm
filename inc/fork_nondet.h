
#ifndef _INC_FORK_H
#define _INC_FORK_H

#include <inc/determinism.h>

/* We provide a runtime library that allows for simple process manipulation
   and communication. Familiar functions are prefixed with a "d". */

/* Higher level abstractions. */
int become_deterministic(void);
pid_t dfork(pid_t id, int flags);
int dwaitpid(pid_t childid);
int dsyncchild(pid_t childid);
/* No dwait - this abstraction inherently breaks deterministic execution. */
int dkill(pid_t childid);

/* TODO describe dexec */
int dexec(pid_t childid, const char *file, char **argv, char **env);

#endif

