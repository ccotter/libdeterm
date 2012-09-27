
#include <determinism.h>
#include <debug.h>
#include <syscall.h>
#include <stdlib.h>

int main(void)
{
	become_deterministic();
	if (0 == dput(0, 1, 0, 0, 0)) {
		int a, b, c;
		a = syscall3(__NR_write, 1, (long)"hello\n", 6);
		//b = syscall0(57 /* fork */);
		b = syscall0(12 /* brk */);
		iprintf("In child %d %d\n", a, b);
		c = dret();
		iprintf("SDF %d\n\n\n",c);
	} else {
		iprintf("In parent.\n");
	}
	dput(0, 0, 0, 0, 0);
	iprintf("Done\n");
	exit(0);
	return 0;
}


