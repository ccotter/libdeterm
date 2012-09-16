
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
		while(1);
		iprintf("SKLDFDSSDFFNSD F\n\n\n\n");
		int ewq=0,asd;
		iprintf("Child R15=%ld\n", read_r15());
		//while(1);
		asd = (int)(1/ewq);
		(void)asd;
		dret();
		iprintf("FUCK\n");
	} else {
		dget(0,0,0,0,0);
		int i;
		iprintf("parent %d\n", ret);
		for(i = 0; i < 1000000000; ++i);
		iprintf("ret code %d\n", dput(0, DET_GET_STATUS, 0, 0, 0));
		iprintf("ret code %d\n", dput(0, DET_GET_STATUS, 0, 0, 0));
		dget_regs(0, &regs, 0);
		print_regs(&regs);
		//i = dput(0, DET_KILL|DET_KILL_POISON, 0, 0, 0);
		iprintf("KILLLLL %d\n", i);
		while(1);
	}
	return 0;
}

