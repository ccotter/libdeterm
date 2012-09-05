
#include <determinism.h>
#include <sys/user.h>
#include <debug.h>
#include <signal.h>

static unsigned long
read_r15(void)
{
        unsigned long r15;
        __asm __volatile("movq %%r15,%0" : "=rm" (r15));
        return r15;
}

int main(void)
{
	struct user_regs_struct regs;
	long ret;
	sigset_t set;

	become_deterministic();
	sigfillset(&set);
	sigdelset(&set, SIGINT);
	ret = master_allow_signals(&set, sizeof(set));
	iprintf("sign ret %ld sigset=%016lx\n",ret,*(unsigned long*)&set);

	get_register_state(&regs);
	iprintf("Before\n");
	print_regs(&regs);
	ret = dput_regs(0, &regs, DET_START);
	if (0 == ret) {
		iprintf("Child R15=%ld\n", read_r15());
		dret();
		iprintf("FUCK\n");
	} else {
		iprintf("parent %d\n", ret);
		dget_regs(0, &regs, 0);
		print_regs(&regs);
		dput(0, DET_KILL, 0, 0, 0);
		while(1);
	}
	return 0;
}

