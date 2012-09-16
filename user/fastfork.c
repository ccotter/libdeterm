
#include <determinism.h>
#include <debug.h>
#include <string.h>
#include <mman.h>

static inline void die(int r)
{
	iprintf("DIED %d\n",r);
	exit(r);
}

static void test1_leaking(unsigned long len, unsigned long off)
{
	int rc;
	if ((rc = dput(0, 0, 0, 0, 0)) < 0) {
		die(rc);
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
}

int main(int argc, char **argv)
{
	int what = 0;
	int rc;
	sigset_t set;
	if (0 > (rc = become_deterministic()))
		die(rc);

	if (2 != argc) {
		iprintf("usage: copy #\n");
		exit(1);
	}

	if (0 == strcmp("1", argv[1]))
		what = 1;

	sigfillset(&set);
	sigemptyset(&set);
	sigdelset(&set, SIGINT);
	if (0 > (rc = master_allow_signals(&set, sizeof(set))))
		die(rc);

	for (;;)
		test1_leaking(0x1000,0);
	
	return 0;
}

