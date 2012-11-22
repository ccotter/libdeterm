
#include <bench.h>
#include <stdio.h>
#include <determinism.h>
#include <debug.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>
#include <stdlib.h>

#define SZ (0x1000 * 0x1)
unsigned char A[SZ];

static inline int put(long a, long b, long c, long d, long e)
{
	int rc = dput(a,b,c,d,e);
	if (rc<0) {
		printf("PUT RC=%d\n", rc);
		exit(1);
	}
	return rc;
}
static inline int get(long a, long b, long c, long d, long e)
{
	int rc = dget(a,b,c,d,e);
	if (rc<0) {
		printf("GET RC=%d\n", rc);
		exit(1);
	}
	return rc;
}

int main(void)
{
	int i;
	memset(A, 3, sizeof(A));
	if (!put(0, DET_START | DET_SNAP, 0, 0, 0)) {
		//memset(A, 4, sizeof(A));
		dret();
	}
	get(0,0,0,0,0);
	long t=bench_time();
	get(0, DET_MERGE, (long)A, SZ, 0);
	t=bench_time()-t;
	printf("Took %ld.%09ld\n",
			t/1000000000,
			t%1000000000);
	return 0;
}

