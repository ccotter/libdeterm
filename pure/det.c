
#define DETERMINE_PURE 1

#include <inc/determinism_pure.h>

#define ARRAY 0x10000000
#define RESULT (ARRAY + 0x1000)

int calc(int *array)
{
    int i = 0, r = 0;
    while(-1 != array[i])
    {
        r += array[i++];
    }
    return r;
}

int main(int argc, char **argv)
{
    int *array = (int*)ARRAY;
    int *result = (int*)RESULT;
    if (1 != argc)
        while(1);
    *result = calc(array);
    dret();
    return 0;
}

