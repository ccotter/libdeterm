
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

void printd(int a,int b)
{
	fprintf(stderr, "done(%d,%d): [%d, %d, %d, %d]\n", a,b,done[0][0], done[0][1], done[1][0], done[1][1]);
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
	//printf("(%d,%d): [%d,%d] [%d,%d]\n", _i,_j,row0,row1-1,col0,col1-1);

	waitthread(_i-1,_j);
	waitthread(_i,_j-1);

	for (k = 0; k < min(_i, _j); ++k) {
		//waitthread(k, _j);
		//waitthread(_i, k);
		int i, j;
		for (i = row0; i < row1; ++i) {
			for (j = col0; j < col1; ++j) {
				int __i = i;
				int __j = j;
				//if (__i==3&&__j==3)
				if (_i==1&&_j==1)
					printf("AT IT (%d,%d) (%d,%d) (%d,%d)\n",_i,_j,__i,k,k,__j);
				VAL(A, __i, __j) = VAL(A, __i, __j) -
					VAL(L, __i, k) * VAL(A, k, __j);
				//if (__i > __j)
				//	VAL(L, __i, __j) = VAL(A, __i, __j) / VAL(A, __j, __j);
			}
		}
	}
	if (_i > _j) {
		//waitthread(_j, _j);
		int i, j;
		for (i = row0; i < row1; ++i) {
			for (j = col0; j < col1; ++j) {
				int __i = _i;
				int __j = j;
				//printf("L:(%d %d)\n", __i,__j);
				VAL(L, __i, __j) = VAL(A, __i, __j) / VAL(A, __j, __j);
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
			VAL(A, i, j) = brand() % 100 + 100;
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
	plu(2);
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

