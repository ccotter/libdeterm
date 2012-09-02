
#include <string.h>
#include <determinism.h>

#include <debug.h>

static void conv(char *buf, int num)
{
	int at = 0, i, l;
	while (num > 0)
	{
		buf[at++] = "0123456789"[num % 10];
		num /= 10;
	}
	buf[at] = 0;
	for (i = 0, l = at; i < l/2; ++i)
	{
		char t = buf[i];
		buf[i] = buf[l-i-1];
		buf[l-i-1] = t;
	}
}

int main(void)
{
	int rc;
	if ((rc = become_deterministic())) {
        iprintf("Failed to become deterministic\n");
	}
	long ret = dput(0, DET_START, 0, 0, 0);
	if (0 == ret) {
		write(1, "child\n", 6);
		while(1);
		dret();
		exit(0);
	} else if (ret < 0) {
		write(1, "failed\n", 7);
	} else {
		write(1, "started\n", 7);
		dput(0, DET_START, 0, 0, 0);
	}
    char msg[100];
	strcpy(msg, "hello");
	char buf[15];
	conv(buf, ret);
	strcpy(msg+5, buf);
	strcpy(msg+5+strlen(buf), "x\n");
    write(1, msg, strlen(msg));
    /* exit(91) */
	exit(91);
    return 0;
}

