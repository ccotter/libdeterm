
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <determinism.h>
#include <bench.h>

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

struct luargs args[MAXTHREADS][MAXTHREADS];
int status[MAXTHREADS][MAXTHREADS];
uint64_t times[MAXTHREADS][MAXTHREADS];
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

static inline long abs(long a)
{
	return a > 0 ? a : -a;
}
static inline long double fabs(long double a)
{
	return a > 0 ? a : -a;
}
static void print(mtype *arr)
{
	int i;
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			long l = (long)(VAL(arr, i, j) * 1000000000);
			if (l<0)
				printf("-");
			l=abs(l);
			printf("%ld.%08ld ", l / 1000000000, l % 1000000000);
		}
		printf("\n");
	}
}

static void printl(mtype a)
{
	long l = (long)(a * 1000000000);
	printf("%ld.%09ld",
			l / 1000000000,
			l % 1000000000);
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
				long l1 = VAL(a,i,j) * 1000000000;
				long l2 = VAL(b,i,j) * 1000000000;
				printf("%ld.%08ld %ld.%08ld\n",
						l1 / 1000000000, l1 % 1000000000,
						l2 / 1000000000, l2 % 1000000000);
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
	times[bi][bj] = bench_time();

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
	times[bi][bj] = bench_time() - times[bi][bj];
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

uint64_t tm,tt;
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
#if 0
	// This stategy fails - incorrect synchronization order. */
	/* Start the first row. */
	for (i = j = 0; j < nbj; ++j) {
		printf("Forked (%d,%d)\n", i, j);
		bench_fork(i * n + j, lu, &args[i][j]);
	}
	while (i < nbi) {
		for (j = 0; j < nbj; ++j) {
			printf(" join on (%d,%d)\n", i, j);
			bench_join(i * n + j);
			/* Start the next threads: remaining n-1 threads on the currrent row
			 * and 1 thread on the next row. */
			if (!j && i) {
				int k;
				for (k = 1; k < nbj; ++k) {
					printf("Forke2 (%d,%d)\n", i, k);
					bench_fork(i * n + k, lu, &args[i][k]);
				}
			}
			if (!j && i + 1 < nbi) {
				printf("Forke3 (%d,%d)\n", i+1, 0);
				bench_fork((i+1) * n, lu, &args[i+1][0]);
			}
		}
		++i;
	}
	/* Join last row threads. */
	/*for (i = nbi - 1, j = 1; j < nbj; ++j) {
		printf(" join on (%d,%d)\n", i, j);
		bench_join(i * n + j);
	}*/
#endif
#define USE_FORK 1
#define RBOUNDS(__x) (0 <= (__x) && (__x) < nbi)
#define CBOUNDS(__x) (0 <= (__x) && (__x) < nbj)
#define READY(_x, _y) ( \
		(!RBOUNDS((_x)-1) || DONE == status[(_x)-1][(_y)]) && \
		(!CBOUNDS((_y)-1) || DONE == status[(_x)][(_y)-1]))
	memset(status, 0, sizeof(status));
	memset(times, 0, sizeof(times));
#if 1
	while (!alldone(nbi, nbj)) {
		for (i = 0; i < nbi; ++i) {
			for (j = 0; j < nbj; ++j) {
				if (!READY(i, j) || DONE == status[i][j])
					continue;
				status[i][j] = RUNNING;
#if USE_FORK
				bench_fork(nbj * i + j, lu, &args[i][j]);
#else
				if (!dput(nbj * i + j, DET_START, 0, 0, 0)) {
					lu(&args[i][j]);
					dret();
				}
#endif
			}
		}
		//for (i = nbi - 1; i >= 0; ++i) {
		//	for (j = nbj - 1; j >= 0; --j) {
		for (i = 0; i < nbi; ++i) {
			for (j = 0; j < nbj; ++j) {
				if (RUNNING != status[i][j])
					continue;
				long addrA = (long)A;
				long addrL = (long)L;
				long row1 = n * (i+1) / nbi-1;
				long col1 = n * (j+1) / nbj-1;
				long size = &VAL(A, row1, col1) - &VAL(A, 0, 0)+1;
				size *= sizeof(mtype);
#if USE_FORK
				dget(nbj * i + j, 0, 0, 0, 0);
				uint64_t ts = bench_time();
				bench_join(nbj * i + j);
				tm += bench_time() - ts;
#else
				dget(nbj * i + j, DET_VM_COPY, addrA, size, addrA);
				dget(nbj * i + j, DET_VM_COPY, addrL, size, addrL);
				dput(nbj * i + j, DET_KILL, 0, 0, 0);
#endif
				tt += times[i][j];
				status[i][j] = DONE;
			}
		}
	}
#endif
#if 0
outer:
	while (!alldone(nbi, nbj)) {
		/* Create all that we can. */
		for (i = 0; i < nbi; ++i) {
			for (j = 0; j < nbj; ++j) {
				if (!READY(i, j) || NOTRUN != status[i][j])
					continue;
				status[i][j] = RUNNING;
				bench_fork(nbj * i + j, lu, &args[i][j]);
			}
		}
		/* Join on next thread, then continue outer loop. */
		for (i = 0; i < nbi; ++i) {
			for (j = 0; j < nbj; ++j) {
				if (RUNNING != status[i][j])
					continue;
				bench_join(nbj * i + j);
				status[i][j] = DONE;
				goto outer;
			}
		}
	}
#endif
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
			//VAL(Orig, i, j) = VAL(A, i, j) = brand() % 2000;
			VAL(A, i, j) = brand() % 2000;
#if CHECK_CORRECTNESS
			VAL(Orig, i, j) = VAL(A, i, j);
#endif
		}
	}
}

static void usage(char **argv)
{
	printf("usage: %s N\n", argv[0]);
	printf("Runs LU decomposition with block size of NxN (N must be a "
			"power of 2)\n");
	exit(1);
}

int main1(int argc, char **argv)
{
	if (2 != argc)
		usage(argv);
	int blocksize = strtol(argv[1], NULL, 10);
	if (blocksize <= 0 || blocksize != (blocksize & -blocksize))
		usage(argv);
#if 0
	n = 1024;
	genmatrix(1);
	uint64_t ts = bench_time();
	plu(n/16,n/16);
	ts = bench_time() - ts;
	printf("In MERGE: %ld.%09ld\n",
			tm/1000000000,
			tm%1000000000);
	printf("Doing work: %ld.%09ld\n",
			tt/1000000000,
			tt%1000000000);
	printf("Overall: %ld.%09ld\n",
			ts/1000000000,
			ts%1000000000);
	exit(0);
#endif
	int counter = 0;
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			n, n, n*n, n*n*(int)sizeof(mtype));
		int iter, niter = MAXDIM/n;

		int nbi = n / blocksize, nbj = n / blocksize;
		nbi = nbj = 16;
		if (!nbi)
			nbi = nbj = 1;

		uint64_t td = 0;
		tt = tm = 0;
		for (iter = 0; iter < niter; iter++) {
			genmatrix(counter++);
			uint64_t ts = bench_time();
			plu(nbi, nbj);
			td += bench_time() - ts;
#if CHECK_CORRECTNESS
			/* Ensure correctness. */
			matmult();
			check(Orig, R);
#endif
		}
		td /= niter;
		tm /= niter;

		printf("  blksize %dx%d itr %d: %lld.%09lld\n",
				n/nbi, n/nbj, niter,
				(long long)td / 1000000000,
				(long long)td % 1000000000);
		printf("  time in MERGE: %ld.%09ld\n",
				(long long)tm / 1000000000,
				(long long)tm % 1000000000);
	}

	return 0;
}

int main(void)
{
	int nth, nbi, nbj, iter;
	int counter = 0;
	nbi = nbj = 16;
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			n, n, n*n, n*n*(int)sizeof(mtype));
		//for (nth = 1, nbi = nbj = 1; nth <= MAXTHREADS; ) {
			//int niter = MAXDIM/n;
			int niter = 5;
			nth = nbi * nbj;

			uint64_t td = 0;
			tm = 0;
			for (iter = 0; iter < niter; iter++) {
				genmatrix(counter++);
				uint64_t t = bench_time();
				plu(nbi, nbj);
				td += bench_time() - t;
#if CHECK_CORRECTNESS
				/* Ensure correctness. */
				matmult();
				printf("Check %d %d\n",nbi,nbj);
				check(Orig, R);
#endif
			}
			td /= niter;
			tm /= niter;

			printf("blksize %dx%d thr %4d itr %d: %lld.%09lld\n",
				n/nbi, n/nbj, nth, niter,
				(long long)td / 1000000000,
				(long long)td % 1000000000);
			printf("  time in MERGE: %ld.%09ld\n",
					(long long)tm / 1000000000,
					(long long)tm % 1000000000);

			/*if (nbi == nbj)
				nbi *= 2;
			else
				nbj *= 2;
			nth *= 2;
		}*/
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
