
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

#define MINDIM 4
#define MAXDIM 1024
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

mtype A[MAXDIM*MAXDIM], Orig[MAXDIM*MAXDIM],
	  L[MAXDIM*MAXDIM], R[MAXDIM*MAXDIM];
mtype x[MAXDIM];
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
				if (i == 1 && j == 0&&0) {
					printf("Vals are ");
					printl(VAL(L, i, j));
					printf(" ");
					printl(VAL(A, i, j));
					printf(" ");
					printl(VAL(A, j, j));
					printf(" - ");/**/
				}
				VAL(L, i, j) = VAL(A, i, j) / VAL(A, j, j);
				if (i == 1 && j == 0&&0) {
					printl(VAL(L, i, j));
					printf("\n");/**/
				}
			}
		}
	}
	return NULL;
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
			VAL(Orig, i, j) = VAL(A, i, j) = brand() % 2000;
		}
	}
}

int main(void)
{
	int nth, nbi, nbj, iter;
	n = 4;
	genmatrix(1);
	plu(2,2);
				matmult();
				check(Orig, R);
	exit(1);
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			n, n, n*n, n*n*(int)sizeof(mtype));
		for (nth = 1, nbi = nbj = 1; nth <= MAXTHREADS; ) {
			int niter = MAXDIM/n;
			niter = 1;
			genmatrix(1);

			if (n < nbi || n < nbj)
				break;
			//if (n / nbi < 4 || n / nbj < 4)
			//	break;

			plu(nbi, nbj);

			uint64_t ts = bench_time();
			for (iter = 0; iter < niter; iter++) {
				memcpy(Orig, A, sizeof(A));
				plu(nbi, nbj);
#if 1
				/* Ensure correctness. */
				matmult();
				printf("Check %d %d\n",nbi,nbj);
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
