
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <determinism.h>
#include <bench.h>

typedef int	KEY_T;

/*
 * These are the COMPARISON macros
 * Replace these macros by YOUR comparison operations.
 * e.g. if you are sorting an array of pointers to strings
 * you should define:
 *
 *	GT(x, y)  as   (strcmp((x),(y)) > 0) 	Greater than
 *	LT(x, y)  as   (strcmp((x),(y)) < 0) 	Less than
 *	GE(x, y)  as   (strcmp((x),(y)) >= 0) 	Greater or equal
 *	LE(x, y)  as   (strcmp((x),(y)) <= 0) 	Less or equal
 *	EQ(x, y)  as   (strcmp((x),(y)) == 0) 	Equal
 *	NE(x, y)  as   (strcmp((x),(y)) != 0) 	Not Equal
 */
#define GT(x, y) ((x) > (y))
#define LT(x, y) ((x) < (y))
#define GE(x, y) ((x) >= (y))
#define LE(x, y) ((x) <= (y))
#define EQ(x, y) ((x) == (y))
#define NE(x, y) ((x) != (y))

static int is_sorted(KEY_T *k, int sz)
{
	int i;
	for (i = 0; i < sz - 1; ++i) {
		if (!LT(k[i], k[i+1]))
			return 0;
	}
	return 1;
}

void print(int *a, int sz)
{
	int i;
	for (i = 0; i < sz; ++i) {
		printf("%12d", a[i]);
	}
	printf("\n");
}

/*
 * This is the SWAP macro to swap between two keys.
 * Replace these macros by YOUR swap macro.
 * e.g. if you are sorting an array of pointers to strings
 * You can define it as:
 *
 *	#define SWAP(x, y) temp = (x); (x) = (y); (y) = temp
 *
 * Bug: 'insort()' doesn't use the SWAP macro.
 */
#define SWAP(x, y) temp = (x); (x) = (y); (y) = temp

void  insort (array, len)
register KEY_T  array[];
register int    len;
{
	register int	i, j;
	register KEY_T	temp;

	for (i = 1; i < len; i++) {
		/* invariant:  array[0..i-1] is sorted */
		j = i;
		/* customization bug: SWAP is not used here */
		temp = array[j];
		while (j > 0 && GT(array[j-1], temp)) {
			array[j] = array[j-1];
			j--;
		}
		array[j] = temp;
	}
}

#ifndef CUTOFF
#  define CUTOFF 15
#endif

/*
 |  void  partial_quickersort (array, lower, upper)
 |  KEY_T  array[];
 |  int    lower, upper;
 |
 |  Abstract:
 |	Sort array[lower..upper] into a partial order
 |     	leaving segments which are CUTOFF elements long
 |     	unsorted internally.
 |
 |  Efficiency:
 |	Could be made faster for _worst_ cases by selecting
 |	a pivot using median-of-3. I don't do it because
 |	in practical cases my pivot selection is arbitrary and
 |	thus pretty random, your mileage may vary.
 |
 |  Method:
 |	Partial Quicker-sort using a sentinel (ala Robert Sedgewick)
 |
 |  BIG NOTE:
 |	Precondition: array[upper+1] holds the maximum possible key.
 |	with a cutoff value of CUTOFF.
 */

void  partial_quickersort (array, lower, upper)
register KEY_T  array[];
register int    lower, upper;
{
    register int	i, j;
    register KEY_T	temp, pivot;

    if (upper - lower > CUTOFF) {
	SWAP(array[lower], array[(upper+lower)/2]);
	i = lower;  j = upper + 1;  pivot = array[lower];
	while (1) {
	    /*
	     * ------------------------- NOTE --------------------------
	     * ignoring BIG NOTE above may lead to an infinite loop here
	     * ---------------------------------------------------------
	     */
	    do i++; while (LT(array[i], pivot));
	    do j--; while (GT(array[j], pivot));
	    if (j < i) break;
	    SWAP(array[i], array[j]);
	}
	SWAP(array[lower], array[j]);
	partial_quickersort (array, lower, j - 1);
	partial_quickersort (array, i, upper);
    }
}
/*
 |  void  sedgesort (array, len)
 |  KEY_T  array[];
 |  int    len;
 |
 |  Abstract:
 |	Sort array[0..len-1] into increasing order.
 |
 |  Method:
 |	Use partial_quickersort() with a sentinel (ala Sedgewick)
 |	to reach a partial order, leave the unsorted segments of
 |	length == CUTOFF to a simpler low-overhead, insertion sort.
 |
 |	This method is the fastest in this collection according
 |	to my experiments.
 |
 |  BIG NOTE:
 |	precondition: array[len] must hold a sentinel (largest
 |	possible value) in order for this to work correctly.
 |	An easy way to do this is to declare an array that has
 | 	len+1 elements [0..len], and assign MAXINT or some such
 |	to the last location before starting the sort (see sorttest.c)
 */
void  sedgesort (array, len)
register KEY_T  array[];
register int    len;
{
    /*
     * ------------------------- NOTE --------------------------
     * ignoring BIG NOTE above may lead to an infinite loop here
     * ---------------------------------------------------------
     */
    partial_quickersort (array, 0, len - 1);
    insort (array, len);
}

#define swapints(a,b) ({ int t = (a); (a) = (b); (b) = t; })

typedef struct pqsort_args
{
	KEY_T *lo, *hi;
	int nth, cn;
} pqsort_args;

void *
pqsort(void *arg)
{
	pqsort_args *a = arg;
	KEY_T *lo = a->lo;
	KEY_T *hi = a->hi;
	int nth = a->nth;
	int cn = a->cn;

	//printf("cn %d, lo = %x, hi = %x, num = %d\n", cn, lo, hi, hi-lo + 1);
	if (lo >= hi)
		return NULL;
	
	if(/*hi <= lo + MIN_STRIDE ||*/ nth <= 1) { // now, MIN_STRIDE is controlled by nthread...
		//int tmp = hi[1];
		//hi[1] = INT_MAX;
		sedgesort(lo, hi - lo + 1);
		//assert(hi[1] == INT_MAX);
		//hi[1] = tmp;
		//insort(lo, hi-lo + 1);
		return NULL;
	}

	int pivot = *lo;	// yeah, bad way to choose pivot...
	KEY_T *l = lo+1, *h = hi;
	while (l <= h) {
		if (*l < pivot)
			l++;
		else if (*h > pivot)
			h--;
		else
			swapints(*h, *l), l++, h--;
	}
	swapints(*lo, l[-1]);

	// Now recursively sort the two halves in parallel subprocesses
	int lcn = (cn*2)+0, rcn = (cn*2)+1;
	pqsort_args larg = {
		.lo = lo, .hi = l-2, .nth = nth >> 1, .cn = lcn };
	pqsort_args rarg = {
		.lo = h+1, .hi = hi, .nth = nth >> 1, .cn = rcn };
#if 1
	bench_fork(lcn, pqsort, &larg);
	bench_fork(rcn, pqsort, &rarg);
	bench_join(lcn);
	bench_join(rcn);
#elif 1
	int rc1,rc2;
	if (!(rc1=dput(lcn, DET_START, 0, 0, 0))) { pqsort(&larg); dret(); }
	if (!(rc2=dput(rcn, DET_START, 0, 0, 0))) { pqsort(&rarg); dret(); }
	if (rc1<0 || rc2<0)printf("first %d %d\n",rc1,rc2);
	rc1=dget(lcn, DET_VM_COPY, (unsigned long)lo,
			sizeof(KEY_T) * (l - 1 - lo), (unsigned long)lo);
	rc2=dget(rcn, DET_VM_COPY, (unsigned long)(h + 1),
			sizeof(KEY_T) * (hi - h), (unsigned long)(h + 1));
	if (rc1<0 || rc2<0)printf("%d %d\n",rc1,rc2);
	dput(lcn, DET_KILL, 0, 0, 0);
	dput(rcn, DET_KILL, 0, 0, 0);
#else
	if (!dput(lcn, DET_START|DET_SNAP, 0, 0, 0)) { pqsort(&larg); dret(); }
	if (!dput(rcn, DET_START|DET_SNAP, 0, 0, 0)) { pqsort(&rarg); dret(); }
	dget(lcn, DET_MERGE, (unsigned long)lo,
			sizeof(KEY_T) * (l - 1 - lo), (unsigned long)lo);
	dget(rcn, DET_MERGE, (unsigned long)(h + 1),
			sizeof(KEY_T) * (hi - h), (unsigned long)(h + 1));
	dput(lcn, DET_KILL, 0, 0, 0);
	dput(rcn, DET_KILL, 0, 0, 0);
#endif
	return NULL;
}

#define MAX_ARRAY_SIZE	10000000
#define MAXTHREADS	8
#define NITER		10

KEY_T randints[NITER][MAX_ARRAY_SIZE+1];

#include <rng.h>
void
gen_randints(ssize_t n, int seed) {
	bseed(seed);
	int i,j;
	for(j = 0; j < NITER; j++) {
		for(i = 0; i < n; ++i) {
			randints[j][i] = (KEY_T)(UINT_MAX * brand());
		}
		randints[j][n] = INT_MAX;	// sentinel at end of array
	}
}

void
testpqsort(int array_size, int nthread)
{	
	assert(nthread <= MAXTHREADS);
	int iter;

	gen_randints(array_size, 1);
	uint64_t ts = bench_time();
	for(iter = 0; iter < NITER; ++iter) {
		pqsort_args arg = {
			.lo = randints[iter],
			.hi = randints[iter] + array_size - 1,
			.nth = nthread,
			.cn = 1 };
		pqsort(&arg);
		if (!is_sorted(randints[iter], array_size)) {
			printf("BAD");
		}
	}
	uint64_t td = bench_time();
	uint64_t tt = (td - ts);

	long long t1 = (tt / NITER) / 1000000000;
	long long t2 = (tt / NITER) % 1000000000;
	printf("array_size: %d\tavg. time: %lld.%09lld\n", array_size, t1, t2);
}

int
main()
{
	int nth;
	for (nth = 1; nth <= MAXTHREADS; nth *= 2) {
		printf("nthreads %d...\n", nth);
		testpqsort(1000, nth);
		testpqsort(5000, nth);
		testpqsort(10000, nth);
		testpqsort(50000, nth);
		testpqsort(100000, nth);
		testpqsort(500000, nth);
		testpqsort(1000000, nth);
		testpqsort(5000000, nth);
		testpqsort(10000000, nth);
	}
	return 0;
}

