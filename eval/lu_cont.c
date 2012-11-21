
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
 *
 * We partition the blocks so they reside contiguously in memory.
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
};

static void *lu(void*);
static void plu(void);
static void partition(void);
static void unpartition(void);

pthread_t threads[MAXTHREADS][MAXTHREADS];
pthread_cond_t conds[MAXTHREADS][MAXTHREADS];
pthread_mutex_t mutexes[MAXTHREADS][MAXTHREADS];
int done[MAXTHREADS][MAXTHREADS];
struct luargs args[MAXTHREADS][MAXTHREADS];

#define CHECK_CORRECTNESS 1

mtype A[MAXDIM*MAXDIM], Orig[MAXDIM*MAXDIM],
	  L[MAXDIM*MAXDIM];

#if CHECK_CORRECTNESS
mtype RA[MAXDIM*MAXDIM], RL[MAXDIM*MAXDIM], R[MAXDIM*MAXDIM];
#endif

int n, nbi, nbj;
int rlen, clen, partsize;
#define VAL(_a, _i, _j) ((_a)[(_i) * MAXDIM + (_j)])
/* PVAL takes coords within partition */
#define PVAL(_a, _i, _j) ((_a)[(_i) * rlen + (_j)])
/* Find partition coordinates given general coordinates. */
#define CBLOCK(_i) ((_i) / clen)
#define RBLOCK(_j) ((_j) / rlen)
#define CCOORD(_i) ((_i) % clen)
#define RCOORD(_j) ((_j) % rlen)
#define POFFSET(_a, _bi, _bj) (&(_a)[partsize * (nbj * (_bi) + (_bj))])
#define GVAL(_a, _i, _j) PVAL(POFFSET(_a, CBLOCK(_i), RBLOCK(_j)), \
		CCOORD(_i), RCOORD(_j))

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

/* Multiply R = RL*RA */
static void matmult(void)
{
	int i,j,k;
	memset(R, 0, sizeof(R));
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			for (k = 0; k < n; ++k) {
				VAL(R, i, j) += VAL(RL, i, k) * VAL(RA, k, j);
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
				printf("Not correct %d %d %d\n", n, nbi, nbj);
				printf("%Lf %Lf\n", VAL(a,i,j), VAL(b,i,j));
				print(R);
				printf("\n");
				print(Orig);
				printf("\n");
				print(RL);
				printf("\n");
				print(RA);
				printf("\n");
				printf("Not correct %d %d %d\n", n, nbi, nbj);
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
	int i, j;
	int row0 = clen * bi;
	int col0 = rlen * bj;
	mtype *PA = POFFSET(A, bi, bj);
	mtype *PL = POFFSET(L, bi, bj);

	waitthread(bi - 1, bj);
	waitthread(bi, bj - 1);

	for (i = 0; i < clen; ++i) {
		int j;
		for (j = 0; j < rlen; ++j) {
			int k;
			int m = min(i + row0, j + col0);
			mtype *ptr = &PVAL(PA, i, j);
			//ptr = &GVAL(A, i+row0,j+col0);
			for (k = 0; k < m; ++k) {
				*ptr -= GVAL(L, i+row0, k) * GVAL(A, k, j+col0);
			}
			if (i + row0 > j + col0) {
				PVAL(PL, i, j) = *ptr / GVAL(A, j+col0, j+col0);
				//GVAL(L, i+row0, j+col0) = *ptr / GVAL(A, j+col0, j+col0);
			}
		}
	}
	threaddone(bi, bj);
	return NULL;
}

static void plu(void)
{
	int bi, bj, i ,j;
	partition();
	for (i = 0; i < nbi; ++i) {
		for (j = 0; j < nbj; ++j) {
			args[i][j].bi = i;
			args[i][j].bj = j;
			done[i][j] = 0;
			pthread_mutex_init(&mutexes[i][j], NULL);
			pthread_cond_init(&conds[i][j], NULL);
			pthread_create(&threads[i][j], NULL, lu, &args[i][j]);
		}
	}
	for (i = 0; i < nbi; ++i) {
		for (j = 0; j < nbj; ++j) {
			pthread_join(threads[i][j], NULL);
		}
	}
	for (i = 0; i < nbi; ++i) {
		for (j = 0; j < nbj; ++j) {
			pthread_mutex_destroy(&mutexes[i][j]);
			pthread_cond_destroy(&conds[i][j]);
		}
	}

	/* Clear bottom of A (which is now U) and make L's diagonals 1s. */
	for (bi = 0; bi < nbi; ++bi) {
		for (bj = 0; bj < nbj; ++bj) {
			/* Walk each partition, copying its contents into the contiguous
			 * array. */
			int row0 = clen * bi;
			int col0 = rlen * bj;
			mtype *PA = POFFSET(A, bi, bj);
			mtype *PL = POFFSET(L, bi, bj);
			for (i = 0; i < clen; ++i) {
				for (j = 0; j < rlen; ++j) {
					if (row0+i>col0+j)
						PVAL(PA, i, j) = 0;
					else if (row0+i == col0+j)
						PVAL(PL, i, i) = 1;
				}
			}
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

/* Matrix stored in Orig, we partition it into A. */
static void partition(void)
{
	int bi, bj, i ,j;
	for (bi = 0; bi < nbi; ++bi) {
		for (bj = 0; bj < nbj; ++bj) {
			/* Walk each partition, copying its contents into the contiguous
			 * array. */
			int row0 = clen * bi;
			int col0 = rlen * bj;
			mtype *P = POFFSET(A, bi, bj);
			for (i = 0; i < clen; ++i) {
				for (j = 0; j < rlen; ++j) {
					PVAL(P, i, j) = VAL(Orig, row0+i, col0+j);
				}
			}
		}
	}
}

/* Matrix stored in A and L, we unpartition into RA, RL. */
static void unpartition(void)
{
	int bi, bj, i ,j;
	for (bi = 0; bi < nbi; ++bi) {
		for (bj = 0; bj < nbj; ++bj) {
			/* Walk each partition, copying its contents into the contiguous
			 * array. */
			int row0 = clen * bi;
			int col0 = rlen * bj;
			mtype *PA = POFFSET(A, bi, bj);
			mtype *PL = POFFSET(L, bi, bj);
			for (i = 0; i < clen; ++i) {
				for (j = 0; j < rlen; ++j) {
					VAL(RA, i+row0, j+col0) = PVAL(PA, i, j);
					VAL(RL, i+row0, j+col0) = PVAL(PL, i, j);
				}
			}
		}
	}
}

static inline void setup(int _n, int _nbi, int _nbj)
{
	n = _n;
	nbi = _nbi;
	nbj = _nbj;
	clen = n / nbi;
	rlen = n / nbj;
	partsize = clen * rlen;
}

int main(void)
{
#if 0
	setup(64, 4, 4);
	genmatrix(0);
	plu();
	unpartition();
	matmult();
	check(Orig, R);
	exit(0);
#endif
	int counter = 0;
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			n, n, n*n, n*n*(int)sizeof(mtype));
		int iter, niter = MAXDIM/n;

		nbi = n / 16, nbj = n / 16;
		if (!nbi)
			nbi = nbj = 1;
		setup(n, nbi, nbj);
		genmatrix(counter);

		uint64_t td = 0;
		for (iter = 0; iter < niter; iter++) {
			memcpy(Orig, A, sizeof(A));
			uint64_t ts = bench_time();
			plu();
			td += bench_time() - ts;
#if 0
			/* Ensure correctness. */
			unpartition();
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
	int nth, iter;
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
				plu();
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
