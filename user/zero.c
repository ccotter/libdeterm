
#include <determinism.h>
#include <debug.h>
#include <string.h>

#define die(x) do { iprintf("error at line %d: %d\n", __LINE__, x), exit(x); } while(0)
#define tassert(x) do { if (!(x)) { iprintf("ooops %d\n", __LINE__); asm("hlt"); }} while (0)

#define ADDR1 0x10000000L

static inline void is_all_x(unsigned char *ptr, unsigned char val, size_t len)
{
	size_t i;
	for (i = 0; i < len; ++i) {
		tassert(ptr[i]==val);
	}
}

static void test1(void)
{
	int rc;
	if ((rc = dput(0, 0, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		rc = dput(0, DET_VM_ZERO | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				ADDR1-8, 0x1000+16, 0);
		tassert(rc > 0);
		rc = dput(0, DET_VM_ZERO | DET_PROT_WRITE | DET_PROT_READ,
				ADDR1-8, 8, 0);
		tassert(rc > 0);
		rc = dput(0, DET_VM_ZERO | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				ADDR1+0x1000, 8, 0);
		tassert(rc > 0);
	} else {
		/* Child. */
		unsigned long *addr = (unsigned long*)(ADDR1-8);
		unsigned long len = 0x1000+16;
		is_all_x((unsigned char*)addr, 0, len);
		memset(addr, 0xab, len);
		dret();
		is_all_x((unsigned char*)addr + 8, 0xab, 0x1000);
		tassert(!addr[0]);
		tassert(!addr[(len-1)/sizeof(unsigned long)]);
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
	iprintf("test1 success\n");
}

static void test2(void)
{
	int rc;
	if ((rc = dput(0, 0, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		rc = dput(0, DET_VM_ZERO | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				ADDR1, 0x10000000, 0);
		tassert(rc > 0);
	} else {
		/* Child. */
		unsigned long *addr = (unsigned long*)(ADDR1);
		unsigned long len = 0x10000000;
		is_all_x((unsigned char*)addr, 0, len);
		memset(addr, 0xab, len);
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
	iprintf("test2 success\n");
}

static void test3(void)
{
	int rc;
	if ((rc = dput(0, 0, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		int i;
		for (i = 0; i < 0x100; ++i) {
			int flag = (i % 2) ? DET_PROT_WRITE : 0;
			rc = dput(0, DET_VM_ZERO | flag | DET_PROT_READ,
					ADDR1+i*0x100000, 0x100000, 0);
			tassert(rc > 0);
		}
		rc = dput(0, DET_START, 0, 0, 0);
		tassert(rc > 0);
	} else {
		/* Child. */
		unsigned long *addr = (unsigned long*)(ADDR1);
		unsigned long len = 0x10000000;
		int i;
		is_all_x((unsigned char*)addr, 0, len);
		for (i = 0; i < 0x100; ++i) {
			if (i % 2) 
				memset((void*)(ADDR1+0x100000*i), 0xab, 0x100000);
		}
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
	iprintf("test3 success\n");
}

static void test4(void)
{
	int rc;
	unsigned long var = 0xdeadbeef;
	if ((rc = dput(0, DET_START, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		rc = dput(0, DET_VM_ZERO | DET_START, (unsigned long)&var, sizeof(unsigned long), 0);
		tassert(rc > 0);
		tassert(var==0xdeadbeef);
	} else {
		/* Child. */
		tassert(var==0xdeadbeef);
		dret();
		tassert(!var);
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL) {
		iprintf("didn't exit cleanly\n"), exit(1);
	}
	iprintf("test4 success %d\n", rc);
}

int main(void)
{
	int rc;
	sigset_t set;
	if (0 > (rc = become_deterministic()))
		die(rc);

	sigfillset(&set);
	sigdelset(&set, SIGINT);
	if (0 > (rc = master_allow_signals(&set, sizeof(set))))
		die(rc);

	test1();
	test2();
	test3();
	test4();
	iprintf("Success\n");
	return 0;
}

