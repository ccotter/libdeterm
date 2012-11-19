
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MINDIM 4
#define MAXDIM MINDIM

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })
#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a < _b ? _a : _b; })

int NT;
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

static void print(mtype *arr);
void *lu(void*);

pthread_t threads[MAXDIM][MAXDIM];
pthread_cond_t conds[MAXDIM][MAXDIM];
pthread_mutex_t mutexes[MAXDIM][MAXDIM];
int done[MAXDIM][MAXDIM];
struct luargs args[MAXDIM][MAXDIM];

mtype A[MAXDIM*MAXDIM], L[MAXDIM*MAXDIM], A2[MAXDIM*MAXDIM];
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
	printf("(%d,%d): [%d,%d] [%d,%d]\n", _i,_j,row0,row1-1,col0,col1-1);

	waitthread(_i-1,_j);
	waitthread(_i,_j-1);

	int i, j;
	int QW,WQ;
	QW=2;
	WQ=2;
	for (i = row0; i < row1; ++i) {
		for (j = col0; j < col1; ++j) {
			printf("A:%d %d\n", i,j);
			for (k = 0; k < min(_i, _j); ++k) {
				int l;
				for (l = 0; l < probsize; ++l) {
					int _k = k * probsize + l;
			printf("  (%d,%d)use L:%d %d\n", i,j,i,_k);
			printf("  (%d,%d)use A:%d %d\n", i,j,_k,j);
					if (i==QW&&j==WQ)
						printf("(%d,%d):%d-%d (%d,%d)=%.8f (%d,%d)=%.8f\n",
								i,j,k,
								probsize-row1+i,i,_k,VAL(L,i,_k),_k,j,VAL(A,_k,j));
					VAL(A, i, j) = VAL(A, i, j) - VAL(L, i, _k) * VAL(A, _k, j);
					if (i==QW&&j==WQ)
						printf("  final=%.8f\n", VAL(A,i,j));
				}
			}
			if (i==QW&&j==WQ)
				printf("is %d\n", probsize-row1+i);
			for (k = 0; k < probsize - row1 + i; ++k) {
				int _k = k + row0;
				printf("  use L:%d %d\n", i,_k);
				printf("  use A:%d %d\n", _k,j);
				if (i==QW&&j==WQ)
					printf("o(%d,%d):%d-%d (%d,%d)=%.8f (%d,%d)=%.8f\n",
							i,j,k,
							probsize-row1+i,i,_k,VAL(L,i,_k),_k,j,VAL(A,_k,j));
				VAL(A, i, j) = VAL(A, i, j) - VAL(L, i, _k) * VAL(A, _k, j);
				if (i==QW&&j==WQ)
					printf("  final=%.8f\n", VAL(A,i,j));
			}
			if (i > j) {
#if 0
				if (i==2&&j==1) {
					printf("start\n");
					print(A);
					print(L);
					printf("ok\n");
				}
#endif
				printf("made L %d %d\n",i,j);
				VAL(L, i, j) = VAL(A2, i, j) / VAL(A2, j, j);
			}
		}
	}
#if 0
	for (i = row0; i < row1; ++i) {
		for (j = col0; j < col1; ++j) {
			if (i > j) {
				VAL(L, i, j) = VAL(A, i, j) / VAL(A, j, j);
			}
		}
	}
#endif
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
			++NT;
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
	printf("finished %d\n", partition);
}

#include "../inc/rng.h"
void genmatrix(int seed)
{
	bseed(seed);
	int i;
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			VAL(A2, i, j) = VAL(A, i, j) = brand() % 100 + 100;
		}
	}
}

int intcmp(const void *_a, const void *_b)
{
	int a = *(int*)_a, b = *(int*)_b;
	if (a<b)
		return -1;
	else if (a>b)
		return 1;
	return 0;
}

int main(void)
{
	n = MINDIM;
	int i;
	genmatrix(1);
	print(A);
	plu(MINDIM/2);
	print(L);
	printf("\n");
	print(A);
	printf("Created %d threads\n", NT);

#if 0
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			int k;
			for (k = 1; k < deps[i][j][0]; ++k) {
				int a = deps[i][j][k] / MAXDIM;
				int b = deps[i][j][k] % MAXDIM;
				if (i!=a&&j!=b)
					printf("oops");
			}
#if 0
			qsort(&deps[i][j][1] , deps[i][j][0], sizeof(int), intcmp);
			printf("(%d,%d): ", i, j);
			for (k = 1; k < deps[i][j][0]; ++k) {
				int Q = deps[i][j][k];
				printf("(%d,%d) ", Q/MAXDIM,Q%MAXDIM);
			}
			printf("\n");
#endif
		}
	}
#endif
	return 0;
	for (i = 1; i <= n / 2; i *= 2) {
		genmatrix(1);
		plu(i);
		print(A);
	}
	return 0;
}

static void print(mtype *arr)
{
	int i;
	if (n>8)
		return;
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			printf("%.8f ", VAL(arr, i, j));
		}
		printf("\n");
	}
}

