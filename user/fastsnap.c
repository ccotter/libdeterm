
#include <determinism.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define die(x) do { iprintf("error at line %d: %d\n", __LINE__, x), exit(x); } while(0)
#define tassert(x) do { if (!(x)) { iprintf("ooops %d\n", __LINE__); asm("hlt"); }} while (0)

static void test1(void)
{
	int rc;
	unsigned dummy=0;
	struct user_regs_struct regs;
	if ((rc = dput(0, DET_START, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		dummy = 1;
		rc = dput(0, DET_SNAP, 0, 0, 0);
		if (!rc)
			goto child;
		tassert(rc > 0);
		get_register_state(&regs);
		rc = dput_regs(0, &regs, DET_START);
		tassert(rc > 0);
	} else {
		/* Child. */
		tassert(!dummy);
		dummy = 0xdeadbeef;
		tassert(0xdeadbeef == dummy);
		dret();
child:
		tassert(1 == dummy);
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
}

long a[2];
static void test_swap_a_b(void)
{
	a[0] = 0xaaaa;
	a[1] = 0x2222;
	iprintf("%lx %lx\n",a[0],a[1]);
	int rc;
	unsigned dummy=0;
	struct user_regs_struct regs;
	rc = dput(0, DET_START | DET_SNAP, 0, 0, 0);
	if (!rc) {
		a[0] = a[1];
		dret();
	}
	rc = dput(1, DET_START | DET_SNAP, 0, 0, 0);
	if (!rc) {
		a[1] = a[0];
		dret();
	}
	rc = dget(0, DET_MERGE, (unsigned long)a, sizeof(a), 0);
	tassert(rc > 0);
	rc = dget(1, DET_MERGE, (unsigned long)a, sizeof(a), 0);
	tassert(rc > 0);
	iprintf("%lx %lx\n",a[0],a[1]);
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
}

unsigned char b[0x1000*2000];
static void test_interleave(void)
{
	memset(b, 0, sizeof(b));
	int rc;
	unsigned dummy=0;
	struct user_regs_struct regs;
	rc = dput(0, DET_START | DET_SNAP, 0, 0, 0);
	if (!rc) {
		unsigned i;
		for (i = 0; i < sizeof(b); i+=2)
			b[i] = 0xff;
		dret();
	}
	rc = dput(1, DET_START | DET_SNAP, 0, 0, 0);
	if (!rc) {
		unsigned i;
		for (i = 1; i < sizeof(b); i+=2)
			 b[i] = 0xff;
		b[0] = 1;
		dret();
	}
	rc = dget(0, DET_MERGE, (unsigned long)b, sizeof(b), 0);
	tassert(rc > 0);
	rc = dget(1, DET_MERGE, (unsigned long)b, sizeof(b), 0);
	tassert(rc > 0);
	for (dummy = 0; dummy < sizeof(b); ++dummy) {
		tassert(0xff==b[dummy]);
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
}

static void test_interleave1(void)
{
	memset(b, 0, sizeof(b));
	int rc;
	unsigned dummy=0;
	struct user_regs_struct regs;
	rc = dput(0, DET_START | DET_SNAP, 0, 0, 0);
	iprintf("DPUT=%d\n",rc);
	if (!rc) {
		dret();
		unsigned i;
		for (i = 0; i < sizeof(b); ++i)
			 b[i] = 0xff;
		dret();
	}
	rc = dget(0, DET_MERGE, (unsigned long)b, sizeof(b), 0);
	iprintf("%d %lx %lx\n", rc, (unsigned long)b, (unsigned long)b + sizeof(b));
	tassert(rc > 0);
	for (dummy = 0; dummy < sizeof(b)/sizeof(long); ++dummy) {
		iprintf("%lx\n",b[dummy]);
		tassert(0xff==b[dummy]);
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
}

int main(void)
{
	int rc, i, j;
	sigset_t set;
	if (0 > (rc = become_deterministic()))
		die(rc);

	sigemptyset(&set);
	if (0 > (rc = master_allow_signals(&set, sizeof(set))))
		die(rc);

	for (i = 0; i < 1; ++i) {
		test_interleave();
	}

out:
	iprintf("Success\n");
	return 0;
}

