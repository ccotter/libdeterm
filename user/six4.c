
#include <syscall.h>
#include <string.h>

int main(void);

void _start(void)
{
	main();
}

void conv(char *buf, int num)
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
	long ret = syscall5(__NR_dput,0,1,0,0,0);
	if (0 == ret)
	{
		syscall0(__NR_dret);
		syscall3(__NR_write, 1, (long)"child\n", 6);
		syscall1(__NR_exit, 91);
	}
	else
	{
		syscall5(__NR_dput,0,1,0,0,0);
	}
    /* write(1, "hello\n") */
    char msg[100];
	strcpy(msg, "hello");
	char buf[15];
	conv(buf, ret);
	strcpy(msg+5, buf);
	strcpy(msg+5+strlen(buf), "x\n");
    syscall3(__NR_write, 1, (long)msg, strlen(msg));
    /* exit(91) */
    syscall1(__NR_exit, 91);
    return 0;
}

