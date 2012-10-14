
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static void usage(char **argv)
{
    fprintf(stderr, "Usage: %s <n>\n", argv[0]);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv);
    int N = strtol(argv[1], NULL, 10);
    srand(time(NULL));
    int i;
    printf("%d ", N);
    for (i = 0; i < N; ++i) {
        printf("%d \n", rand() % 1000000);
    }
    return 0;
}

