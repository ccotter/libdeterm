
#include <stdio.h>
#include <rng.h>

int main(void)
{
	bseed(1);
	int i;
	for (i = 0; i < 10; ++i) {
		printf("%u\n", brand());
	}
	return 0;
}

