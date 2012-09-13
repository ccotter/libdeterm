
#include <determinism.h>
#include <debug.h>
#include <string.h>
#include <mman.h>

#define die(x) do { iprintf("error at line %d: %d\n", __LINE__, x), exit(x); } while(0)
#define tassert(x) do { if (!(x)) { iprintf("ooops %d\n", __LINE__); asm("hlt"); }} while (0)

#define ADDR1 0x10000000L

static inline void is_all_x(void *_ptr, unsigned char val, size_t len)
{
	unsigned char *ptr = _ptr;
	size_t i;
	for (i = 0; i < len; ++i) {
		tassert(ptr[i]==val);
	}
}

static void test1(void)
{
	int rc;
	unsigned long *addr = (unsigned long*)(ADDR1-100);
	void *map_addr = (void*)LOWER_PAGE((unsigned long)addr);
	unsigned long len = 0x1000000+200;
	unsigned long map_len = ((unsigned long)addr - (unsigned long)map_addr) + len;
	map_len = PAGE_ALIGN(map_len);
	if ((rc = dput(0, 0, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		unsigned long *tmp = mmap(map_addr, map_len, PROT_READ|PROT_WRITE,
				MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
		tassert(tmp==map_addr);
		memset(addr, 0xab, len);
		rc = dput(0, DET_VM_COPY | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				(unsigned long)addr, len, (unsigned long)addr);
	iprintf("%lx %d\n",len,rc);
		tassert(rc > 0);
		rc = dget(0, 0, 0, 0, 0);
		is_all_x(addr, 0xab, len);
		tassert(rc > 0);

		rc = dput(0, DET_VM_COPY | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				(unsigned long)addr, len, (unsigned long)addr);
		tassert(rc > 0);
	} else {
		/* Child. */
		is_all_x((unsigned char*)addr, 0xab, len);
		memset(addr, 0x12, len);
		is_all_x((unsigned char*)addr, 0x12, len);
		dret();
		is_all_x((unsigned char*)addr, 0xab, len);
		memset(addr, 0x12, len);
		is_all_x((unsigned char*)addr, 0x12, len);
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
	iprintf("test1 success\n");
	munmap(map_addr, map_len);
}

#define CHILD_OFF 0x10000

static void test2(void)
{
	int rc;
	unsigned long *addr = (unsigned long*)(ADDR1);
	unsigned long *caddr = (unsigned long*)((unsigned long)addr + CHILD_OFF);
	void *map_addr = (void*)LOWER_PAGE((unsigned long)addr);
	void *map_caddr = (void*)LOWER_PAGE((unsigned long)caddr);
	unsigned long len = 0x1000000;
	unsigned long map_len = ((unsigned long)addr - (unsigned long)map_addr) + len;
	map_len = PAGE_ALIGN(map_len);
	if ((rc = dput(0, 0, 0, 0, 0)) < 0) {
		die(rc);
	} else if (rc > 0) {
		/* Parent. */
		unsigned long *tmp = mmap(map_addr, map_len, PROT_READ|PROT_WRITE,
				MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
		tassert(tmp==map_addr);
		memset(addr, 0xab, len);
		rc = dput(0, DET_VM_COPY | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				(unsigned long)addr, len, (unsigned long)map_caddr);
		tassert(rc > 0);
		rc = dget(0, 0, 0, 0, 0);
		is_all_x(addr, 0xab, len);
		tassert(rc > 0);

		rc = dput(0, DET_VM_COPY | DET_START | DET_PROT_WRITE | DET_PROT_READ,
				(unsigned long)addr, len, (unsigned long)map_caddr);
		tassert(rc > 0);
	} else {
		/* Child. */
		is_all_x((unsigned char*)caddr, 0xab, len);
		memset(caddr, 0x12, len);
		is_all_x((unsigned char*)caddr, 0x12, len);
		dret();
		while(1);
		is_all_x((unsigned char*)caddr, 0xab, len);
		memset(caddr, 0x12, len);
		is_all_x((unsigned char*)caddr, 0x12, len);
		dret();
	}
	if ((rc = dput(0, DET_KILL, 0, 0, 0)) < 0)
		die(rc);
	else if (rc != DET_S_EXIT_NORMAL)
		iprintf("didn't exit cleanly\n"), exit(1);
	iprintf("test2 success\n");
	munmap(map_addr, map_len);
}

int main(void)
{
	int rc;
	sigset_t set;
	if (0 > (rc = become_deterministic()))
		die(rc);

	sigfillset(&set);
	sigemptyset(&set);
	sigdelset(&set, SIGINT);
	if (0 > (rc = master_allow_signals(&set, sizeof(set))))
		die(rc);

	test1();
	test2();
	iprintf("Success\n");
	return 0;
}

