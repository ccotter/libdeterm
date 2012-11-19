
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "bench.h"

#define MINDIM 16
#define MAXDIM 1024
#define MAXTHREADS 32

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })
#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a < _b ? _a : _b; })

typedef float mtype;
struct luargs
{
	int i, j, partition;
};

#if 0
int deps[MAXDIM][MAXDIM][MINDIM*MINDIM];
static inline void adddep(int a, int b, int c, int d)
{
	deps[a][b][deps[a][b][0]++] = c * MAXDIM + d;
}
#endif

void *lu(void*);

pthread_t threads[MAXDIM][MAXDIM];
pthread_cond_t conds[MAXDIM][MAXDIM];
pthread_mutex_t mutexes[MAXDIM][MAXDIM];
int done[MAXDIM][MAXDIM];
struct luargs args[MAXDIM][MAXDIM];

mtype A[MAXDIM*MAXDIM], L[MAXDIM*MAXDIM];
mtype x[MAXDIM], P[MAXDIM];
int n;
#define VAL(_a, _i, _j) (_a[_i * MAXDIM + _j])
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
	int partition = args->partition;
	int probsize = n / partition;
	int _i = args->i;
	int _j = args->j;
	int row0 = _i * probsize;
	int col0 = _j * probsize;
	int row1 = (_i + 1) * probsize;
	int col1 = (_j + 1) * probsize;
	int k;

	waitthread(_i-1,_j);
	waitthread(_i,_j-1);

	int i, j;
	for (i = row0; i < row1; ++i) {
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
	threaddone(_i, _j);
	return NULL;
}

void plu(int partition)
{
	int i;
	int probsize = n / partition;
	for (i = 0; i < partition; ++i) {
		int j;
		for (j = 0; j < partition; ++j) {
			args[i][j].i = i;
			args[i][j].j = j;
			args[i][j].partition = partition;
			done[i][j] = 0;
			pthread_mutex_init(&mutexes[i][j], NULL);
			pthread_cond_init(&conds[i][j], NULL);
			pthread_create(&threads[i][j], NULL, lu, &args[i][j]);
		}
	}
	for (i = 0; i < partition; ++i) {
		int j;
		for (j = 0; j < partition; ++j) {
			pthread_join(threads[i][j], NULL);
		}
	}
	for (i = 0; i < partition; ++i) {
		int j;
		for (j = 0; j < partition; ++j) {
			pthread_mutex_destroy(&mutexes[i][j]);
			pthread_cond_destroy(&conds[i][j]);
		}
	}
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			if (i>j)
				VAL(A,i,j)=0;
			if(i==j)
				VAL(L,i,i)=1;
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
			VAL(A, i, j) = brand() % 100 + 100;
		}
	}
}

int main(void)
{
	for (n = MINDIM; n <= MAXDIM; n *= 2) {
		printf("Matrix size: %dx%d = %d (%ld bytes)\n", n, n, n * n,
				n * n * sizeof(mtype));
		int i;
		for (i = 1; i <= n && i*i <= MAXTHREADS; i *= 2) {
			genmatrix(1);
			long tt = bench_time();
			plu(i);
			tt = bench_time() - tt;
			int blksize = n / i;
			printf("blksize %dx%d (nthreads=%d): %ld.%.9ld\n",
					blksize, blksize, i * i,
					tt / 1000000000,
					tt % 1000000000);
		}
		printf("\n");
	}
	return 0;
}

