
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static void gen(int len, char *s)
{
	int i;
	for (i = 0; i < len; ++i) {
		s[i] = rand() % ('~' - ' ') + ' ';
	}
	s[i] = 0;
}

int main(void)
{
	srand(time(0));
	char password[100];
	gen(2, password); printf("%s\n", password);
	gen(2, password); printf("%s\n", password);
	gen(2, password); printf("%s\n", password);
	gen(3, password); printf("%s\n", password);
	gen(3, password); printf("%s\n", password);
	gen(3, password); printf("%s\n", password);
	gen(3, password); printf("%s\n", password);
	gen(4, password); printf("%s\n", password);
	gen(4, password); printf("%s\n", password);
	gen(4, password); printf("%s\n", password);
	gen(5, password); printf("%s\n", password);
	gen(5, password); printf("%s\n", password);
	return 0;
}

