
#include <inc/io.h>
#include <inc/std.h>
#include <inc/mman.h>
#include <inc/determinism_pure.h>
#include <inc/types.h>
#include <inc/fork.h>

int main(int argc, char **argv)
{
    dret();
    int i;
    char lon[10000];
    for (i = 0; i < sizeof(lon)-1; ++i)
        lon[i] = 'A' + (i % 26);
    lon[sizeof(lon)-1] = 0;
    char *A[] = {"hello1", lon, NULL};
    char *B[] = {"env1", "env2", NULL};
    dexec(1234, "/prog1", A, B);
    return 0;
}

