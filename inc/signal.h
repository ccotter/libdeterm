
#ifndef _INC_SIGNAL_H
#define _INC_SIGNAL_H

#include <string.h>
#include <errno.h>

#ifdef __i386__
#error "not supported yet"
#endif

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO
#define SIGLOST		29
#define SIGPWR		30
#define SIGSYS		31
#define	SIGUNUSED	31

#ifdef __i386__
#define _NSIG_WORDS 2
#else
#define _NSIG_WORDS 1
#endif

typedef struct
{
	unsigned long int sig[_NSIG_WORDS];
} sigset_t;

static inline int sigemptyset(sigset_t *set)
{
	memset(set, 0, sizeof(sigset_t));
	return 0;
}

static inline int sigfillset(sigset_t *set)
{
	memset(set, 0xff, sizeof(sigset_t));
	return 0;
}

static inline int sigaddset(sigset_t *set, int signum)
{
	if (SIGHUP > signum || SIGUNUSED < signum)
		return -EINVAL;
	set->sig[0] |= (1U << (signum-1));
	return 0;
}

static inline int sigdelset(sigset_t *set, int signum)
{
	if (SIGHUP > signum || SIGUNUSED < signum)
		return -EINVAL;
	set->sig[0] &= ~(1U << (signum-1));
	return 0;
}

static inline int sigismember(const sigset_t *set, int signum)
{
	if (SIGHUP > signum || SIGUNUSED < signum)
		return -EINVAL;
	return set->sig[0] & (1U << (signum-1));
}

#endif

