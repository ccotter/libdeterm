
#include <determinism.h>
#include <debug.h>

int main(void) {
	int i;
	become_deterministic();
	for (i = 0; i < 5; ++i) {
		long ret = dput(0, 1, 0, 0, 0);
		if (ret) {
			iprintf("At %d\n", i);
			int a=dput(0, 0, 0, 0, 0);
			iprintf("DPUT(%d)\n", a);
			break;
		} else { 
			if (i==4)
				while(1);
			continue;
		}
	}
	iprintf("OK DONE %d\n", i);
	iprintf("OK FINISHED\n");
	return 0;
}

