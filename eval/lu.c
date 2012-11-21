
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

pthread_t threads[MAXTHREADS][MAXTHREADS];
pthread_cond_t conds[MAXTHREADS][MAXTHREADS];
pthread_mutex_t mutexes[MAXTHREADS][MAXTHREADS];
int done[MAXTHREADS][MAXTHREADS];
struct luargs args[MAXTHREADS][MAXTHREADS];

mtype A[MAXDIM*MAXDIM], Orig[MAXDIM*MAXDIM],
	  L[MAXDIM*MAXDIM], R[MAXDIM*MAXDIM];
mtype x[MAXDIM];
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

static inline void waitthread(int i, int j)
{
	if (i < 0 || j < 0)
		return;
	pthread_mutex_lock(&mutexes[i][j]);
	while (!done[i][j]) {
		pthread_cond_wait(&conds[i][j], &mutexes[i][j]);
	}
	pthread_mutex_unlock(&mutexes[i][j]);
}

static inline void threaddone(int i, int j)
{
	pthread_mutex_lock(&mutexes[i][j]);
	done[i][j] = 1;
	pthread_mutex_unlock(&mutexes[i][j]);
	pthread_cond_broadcast(&conds[i][j]);
}

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

	waitthread(bi - 1, bj);
	waitthread(bi, bj - 1);

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
	threaddone(bi, bj);
	return NULL;
}



void plu(int nbi, int nbj)
{
	int i;
	for (i = 0; i < nbi; ++i) {
		int j;
		for (j = 0; j < nbj; ++j) {
			args[i][j].bi = i;
			args[i][j].bj = j;
			args[i][j].nbi = nbi;
			args[i][j].nbj = nbj;
			done[i][j] = 0;
			pthread_mutex_init(&mutexes[i][j], NULL);
			pthread_cond_init(&conds[i][j], NULL);
			pthread_create(&threads[i][j], NULL, lu, &args[i][j]);
		}
	}
	for (i = 0; i < nbi; ++i) {
		int j;
		for (j = 0; j < nbj; ++j) {
			pthread_join(threads[i][j], NULL);
		}
	}
	for (i = 0; i < nbi; ++i) {
		int j;
		for (j = 0; j < nbj; ++j) {
			pthread_mutex_destroy(&mutexes[i][j]);
			pthread_cond_destroy(&conds[i][j]);
		}
	}
	/* Clear bottom of A (which is now U) and make L's diagonals 1s. */
	for (i = 0; i < n; ++i) {
		int j;
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
			VAL(Orig, i, j) = VAL(A, i, j) = brand() % 2000;
		}
	}
}

int main(void)
{
	int counter = 0;
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			n, n, n*n, n*n*(int)sizeof(mtype));
		int iter, niter = MAXDIM/n;
		genmatrix(counter);

		int nbi = n / 16, nbj = n / 16;
		if (!nbi)
			nbi = nbj = 1;

		uint64_t td = 0;
		for (iter = 0; iter < niter; iter++) {
			memcpy(Orig, A, sizeof(A));
			uint64_t ts = bench_time();
			plu(nbi, nbj);
			td += bench_time() - ts;
#if 0
			/* Ensure correctness. */
			matmult();
			check(Orig, R);
#endif
		}
		td /= niter;

		printf("  blksize %dx%d itr %d: %lld.%09lld\n",
				n/nbi, n/nbj, niter,
				(long long)td / 1000000000,
				(long long)td % 1000000000);
	}

	return 0;
}

int main1(void)
{
	int nth, nbi, nbj, iter;
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			n, n, n*n, n*n*(int)sizeof(mtype));
		for (nth = 1, nbi = nbj = 1; nth <= MAXTHREADS; ) {
			int niter = MAXDIM/n;
			genmatrix(1);

			if (n < nbi || n < nbj)
				break;

			uint64_t ts = bench_time();
			for (iter = 0; iter < niter; iter++) {
				memcpy(Orig, A, sizeof(A));
				plu(nbi, nbj);
#if 0
				/* Ensure correctness. */
				matmult();
				check(Orig, R);
#endif
			}
			uint64_t td = (bench_time() - ts) / niter;

			printf("blksize %dx%d thr %4d itr %d: %lld.%09lld\n",
				n/nbi, n/nbj, nth, niter,
				(long long)td / 1000000000,
				(long long)td % 1000000000);

			if (nbi == nbj)
				nbi *= 2;
			else
				nbj *= 2;
			nth *= 2;
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
