
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static int sizes[] = {10000, 10000, 10000, 10000, 10000, 10000, 10000};
//static int sizes[] = {100, 1000, 10000, 100000, 1000000, 10000000, 50000000};

int main(int argc, char **argv)
{
    srand(time(NULL));
	int largest = -1;
    int i;
	for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
		int j;
		if (largest < sizes[i])
			largest = sizes[i];
		printf("int len%d = %d;\n", i+1, sizes[i]);
		printf("int array%d[] = {\n", i+1);
		for (j = 0; j < sizes[i]-1; ++j) {
			printf("%d, ", rand() % 1000000);
			if (0 == j % 100)
				printf("\n");
		}
		printf("%d", rand() % 1000000);
		printf("};\n\n");
		printf("int tmpspace[%d];\n", largest);
		printf("int tmpleft[%d];\n", largest);
		printf("int tmpright[%d];\n", largest);
	}
    return 0;
}

