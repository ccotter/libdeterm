
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "bench.h"

/* LU decomposition algorithm from:
 * http://www.cse.uiuc.edu/courses/cs554/notes/06_lu.pdf
 *
 * Use fine grained LU decomposition with n^2 threads.
 */

#define MINDIM 16
#define MAXDIM 2048
#define MAXTHREADS 256

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })
#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a < _b ? _a : _b; })

typedef long double mtype;
struct luargs
{
	int bi, bj;
	int nbi, nbj;
};

void *lu(void*);

pthread_t threads[MAXTHREADS];
struct luargs args[MAXTHREADS][MAXTHREADS];
int status[MAXTHREADS][MAXTHREADS];
#define NOTRUN 0
#define RUNNING 1
#define DONE 2

#define CHECK_CORRECTNESS 0

mtype A[MAXDIM*MAXDIM], L[MAXDIM*MAXDIM];
#if CHECK_CORRECTNESS
mtype Orig[MAXDIM*MAXDIM], R[MAXDIM*MAXDIM];
#endif

int n;
#define VAL(_a, _i, _j) (_a[_i * MAXDIM + _j])

static void print(mtype *arr)
{
	int i;
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			printf("%.8Lf ", VAL(arr, i, j));
		}
		printf("\n");
	}
}

#if CHECK_CORRECTNESS
/* Multiply R = L*A */
static void matmult()
{
	int i,j,k;
	memset(R, 0, sizeof(R));
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			for (k = 0; k < n; ++k) {
				VAL(R, i, j) += VAL(L, i, k) * VAL(A, k, j);
			}
		}
	}
}

#define EPS .000000001L
static void check(mtype *a, mtype *b)
{
	int i;
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			if (fabs(VAL(a, i, j) - VAL(b, i, j)) > EPS) {
				printf("Not correct %d %d\n", i, j);
				printf("%Lf %Lf\n", VAL(a,i,j), VAL(b,i,j));
				print(R);
				printf("\n");
				print(Orig);
				printf("\n");
				print(L);
				printf("\n");
				print(A);
				printf("\n");
				exit(1);
			}
		}
	}
}
#endif

/* Serial LU decomposition. */
void *lu(void *_arg)
{
	struct luargs *args = _arg;
	int bi = args->bi;
	int bj = args->bj;
	int nbi = args->nbi;
	int nbj = args->nbj;
	int row0 = bi * n / nbi;
	int col0 = bj * n / nbj;
	int row1 = row0 + n / nbi;
	int col1 = col0 + n / nbj;

	int i;
	for (i = row0; i < row1; ++i) {
		int j;
		for (j = col0; j < col1; ++j) {
			int k;
			for (k = 0; k < min(i, j); ++k) {
				VAL(A, i, j) -= VAL(L, i, k) * VAL(A, k, j);
			}
			if (i > j) {
				VAL(L, i, j) = VAL(A, i, j) / VAL(A, j, j);
			}
		}
	}
	return NULL;
}

int alldone(int nbi, int nbj)
{
	int i;
	for (i = 0; i < nbi; ++i) {
		int j;
		for (j = 0; j < nbj; ++j) {
			if (!status[i][j])
				return 0;
		}
	}
	return 1;
}

void plu(int nbi, int nbj)
{
	int i, j;
	for (i = 0; i < nbi; ++i) {
		for (j = 0; j < nbj; ++j) {
			args[i][j].bi = i;
			args[i][j].bj = j;
			args[i][j].nbi = nbi;
			args[i][j].nbj = nbj;
		}
	}
#define USE_FORK 1
#define RBOUNDS(__x) (0 <= (__x) && (__x) < nbi)
#define CBOUNDS(__x) (0 <= (__x) && (__x) < nbj)
#define READY(_x, _y) ( \
		(!RBOUNDS((_x)-1) || DONE == status[(_x)-1][(_y)]) && \
		(!CBOUNDS((_y)-1) || DONE == status[(_x)][(_y)-1]))
	memset(status, 0, sizeof(status));
	while (!alldone(nbi, nbj)) {
		for (i = 0; i < nbi; ++i) {
			for (j = 0; j < nbj; ++j) {
				if (!READY(i, j) || DONE == status[i][j])
					continue;
				status[i][j] = RUNNING;
#if USE_FORK
				pthread_create(&threads[nbj * i + j], NULL, lu, &args[i][j]);
#endif
			}
		}
		for (i = 0; i < nbi; ++i) {
			for (j = 0; j < nbj; ++j) {
				if (RUNNING != status[i][j])
					continue;
#if USE_FORK
				pthread_join(threads[nbj * i + j], NULL);
#endif
				status[i][j] = DONE;
			}
		}
	}
	/* Clear bottom of A (which is now U) and make L's diagonals 1s. */
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			if (i > j)
				VAL(A,i,j) = 0;
			if(i == j)
				VAL(L,i,i) = 1;
		}
	}
}

#include "../inc/rng.h"
void genmatrix(int seed)
{
	bseed(seed);
	int i;
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			VAL(A, i, j) = brand() % 2000;
#if CHECK_CORRECTNESS
			VAL(Orig, i, j) = VAL(A, i, j);
#endif
		}
	}
}

static void usage(char **argv)
{
	printf("usage: %s nbi nbj\n  nbi/nbj Number of row/column partitions.\n",
			argv[0]);
	exit(1);
}

int main(int argc, char **argv)
{
	int nth, nbi, nbj, iter;
	int counter = 0;

	if (3 != argc)
		usage(argv);
	nbi = strtol(argv[1], NULL, 0);
	nbj = strtol(argv[2], NULL, 0);
	if (nbi <= 0 || nbj <= 0)
		usage(argv);
	nth = nbi * nbj;

	int niter = 10;
	for (n = MINDIM; n <= MAXDIM; n *= 2) {

		printf("matrix size: %dx%d = %d (%d bytes) ",
				n, n, n*n, n*n*(int)sizeof(mtype));
		printf("blksize %dx%d thr %4d itr %d:\n",
				n/nbi, n/nbj, nth, niter);
		for (iter = 0; iter < niter; iter++) {
			genmatrix(counter++);
			uint64_t ts = bench_time();
			plu(nbi, nbj);
			ts = bench_time() - ts;
			printf("%lld.%09lld\n",
					(long long)ts / 1000000000,
					(long long)ts % 1000000000);
#if CHECK_CORRECTNESS
			matmult();
			check(Orig, R);
#endif
		}

	}
	return 0;
}

#define SWAP(x, y) do { mtype temp = (x); (x) = (y); (y) = temp;} while(0)

static void swap_rows(int x, int y)
{
	int i;
	if (x == y)
		return;
	for (i = 0; i < n; ++i) {
		SWAP(VAL(A, x, i), VAL(A, y, i));
	}
}
