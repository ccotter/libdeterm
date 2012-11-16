
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#include <determinism.h>
#include "bench.h"

/* Quicksort benchmark from Determinator.
 * Original credit to PIOS:
 * Bryan Ford (https://github.com/bford/Determinator)
 */

typedef int	KEY_T;
static void merge_sort(int *A, int p, int r);
static void *pmerge_sort(void *_data);
static void merge(int *A, int p, int q, int r);

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

static void print(int *a, int sz)
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

#define MAX_ARRAY_SIZE	10000001
int tmpleft[MAX_ARRAY_SIZE];
int tmpright[MAX_ARRAY_SIZE];

static void merge_sort(int *A, int p, int r)
{
	if (p + 1 < r) {
		int q = (p + r) / 2;
		merge_sort(A, p, q);
		merge_sort(A, q, r);
		merge(A, p, q, r);
	}
}

static void merge(int *A, int p, int q, int r)
{
	int i, j, k;
	int n1 = q - p;
	int n2 = r - q;
	int *L = tmpleft;
	int *R = tmpright;
	for (i = 0; i < n1; ++i)
		L[i] = A[p + i];
	for (i = 0; i < n2; ++i)
		R[i] = A[q + i];
	L[n1] = R[n2] = INT_MAX;
	i = j = 0;
	for (k = p; k < r; ++k) {
		if (LT(L[i],R[j])) {
			A[k] = L[i];
			++i;
		} else {
			A[k] = R[j];
			++j;
		}
	}
}

struct pmerge_data
{
	int *A;
	int p, r, depth;
};

int max_depth;
int childid = 0;
static void *pmerge_sort(void *_data)
{
	struct pmerge_data *data = _data;
	KEY_T *A = data->A;
	int p = data->p;
	int r = data->r;
	int depth = data->depth;
	if (p + 1 < r) {
		int q = (p + r) / 2;
		if (depth < max_depth) {
			struct pmerge_data larg = {
				.A = A,
				.p = p,
				.r = q,
				.depth = depth + 1
			};
			struct pmerge_data rarg = {
				.A = A,
				.p = q,
				.r = r,
				.depth = depth + 1
			};

			int lid = ++childid;
			int rid = ++childid;
			long T;

			int rc = dput(lid, DET_START, 0, 0, 0);
			if (!rc) { pmerge_sort(&larg); dret(); }
			rc = dput(rid, DET_START, 0, 0, 0);
			if (!rc) { pmerge_sort(&rarg); dret(); }

			/* Join data. */
			size_t sz = sizeof(int) * (q - p + 1);
			dget(lid, DET_VM_COPY, (long)(A+p), sz, (long)(A+p));
			sz = sizeof(int) * (r - q + 1);
			dget(rid, DET_VM_COPY, (long)(A+q), sz, (long)(A+q));
			dput(lid, DET_KILL, 0, 0, 0);
			dput(rid, DET_KILL, 0, 0, 0);
		} else {
			/* Once we reach a certain depth, stop forking threads. */
			merge_sort(A, p, q);
			merge_sort(A, q, r);
		}
		merge(A, p, q, r);
	}
	return NULL;
}

#define MAXTHREADS	16
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
testmergesort(int array_size, int nthread)
{	
	assert(nthread <= MAXTHREADS);
	int iter;

	gen_randints(array_size, 2);
	uint64_t ts = bench_time();
	for(iter = 0; iter < NITER; ++iter) {
		struct pmerge_data arg = {
			.A = randints[iter],
			.p = 0,
			.r = array_size,
			.depth = 0
		};
		pmerge_sort(&arg);
		//if (!is_sorted(randints[iter], array_size))
		//	printf("NOT SORTED\n");
	}
	uint64_t td = bench_time();
	uint64_t tt = (td - ts);

	long long t1 = (tt/NITER) / 1000000000;
	long long t2 = (tt/NITER) % 1000000000;
	printf("array_size: %d\tavg. time: %lld.%09lld\n", array_size, t1, t2);
}

int
main(void)
{
	int nth;
	max_depth = 0;
	for (nth = 1; nth <= MAXTHREADS; nth *= 2, ++max_depth) {
		printf("nthreads %d...\n", nth, max_depth);
		testmergesort(1000, nth);
		testmergesort(5000, nth);
		testmergesort(10000, nth);
		testmergesort(50000, nth);
		testmergesort(100000, nth);
		testmergesort(500000, nth);
		testmergesort(1000000, nth);
		testmergesort(5000000, nth);
		testmergesort(10000000, nth);
	}
	return 0;
}

