
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MINDIM 4
#define MAXDIM 4

typedef float mtype;
struct luargs
{
	int i, j, partition;
};

static void print(mtype *arr);
void *lu(void*);

pthread_t threads[MAXDIM][MAXDIM];
pthread_cond_t conds[MAXDIM][MAXDIM];
pthread_mutex_t mutexes[MAXDIM][MAXDIM];
int done[MAXDIM][MAXDIM];
struct luargs args[MAXDIM][MAXDIM];

mtype A[MAXDIM*MAXDIM];
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

void waitthread(int a, int b,int i, int j)
{
	if (i < 0 || j < 0)
		return;
	pthread_mutex_lock(&mutexes[i][j]);
	while (!done[i][j]) {
		pthread_cond_wait(&conds[i][j], &mutexes[i][j]);
	}
	pthread_mutex_unlock(&mutexes[i][j]);
}

/* Serial LU decomposition. */
void *lu(void *_arg)
{
	struct luargs *args = _arg;
	int partition = args->partition;
	int probsize = n / partition;
	int row0 = args->i * probsize;
	int col0 = args->j * probsize;
	int row1 = (args->i + 1) * probsize;
	int col1 = (args->j + 1) * probsize;
	int k;

	/* Permutation matrix. */
	for (k = row0; k < row1; ++k) {
		P[k] = k;
	}

	waitthread(args->i,args->j,args->i - 1, args->j);
	waitthread(args->i,args->j,args->i, args->j - 1);

	//printf("Doing (%d,%d) -> (%d, %d) [%d %d]\n",row0,col0, row1,col1,args->i,args->j);
	/* Loop over each column. */
	for (k = col0; k < col1; ++k) {
		int i;
		int maxrow = k;
		for (i = k + 1; i < row1; ++i) {
			if (VAL(A, maxrow, k) < fabs(VAL(A, i, k)))
				maxrow = i;
		}
		if (!VAL(A, maxrow, k)) {
			printf("[%d, %d] [%d, %d] \n", row0,col0, row1, col1);
			printf("searched %d %d\n", k+1,row1);
			printf("Singular matrix %d %d.\n",args->i,args->j);
			exit(1);
		}
		swap_rows(k, maxrow);
		SWAP(P[k], P[maxrow]);
		int j;
		for (i = k + 1; i < row1; ++i) {
			float o=VAL(A,i,k);
			VAL(A, i, k) = VAL(A, i, k) / VAL(A, k, k);
			if (o!=VAL(A,i,k)) {
				;//printf("A(%d,%d)=%f->%f\n", i,k,o,VAL(A,i,k));
			}
			for (j = k + 1; j < col1; ++j) {
				o = VAL(A,i,j);
				//printf("i,j,k=%d,%d,%d\n",i,j,k);
				VAL(A, i, j) = VAL(A, i, j) - VAL(A, i, k) * VAL(A, k, j);
				if(o!=VAL(A,i,j)) {
					;//printf("A(%d,%d)=%f->%f\n",i,j,o,VAL(A,i,j));
				}
			}
		}
	}
	/* Signal to other threads. */
	pthread_mutex_lock(&mutexes[args->i][args->j]);
	done[args->i][args->j] = 1;
	pthread_mutex_unlock(&mutexes[args->i][args->j]);
	pthread_cond_broadcast(&conds[args->i][args->j]);

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

int main(void)
{
	n = MINDIM;
	int i;
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
	for (i = 0; i < n; ++i) {
		int j;
		for (j = 0; j < n; ++j) {
			printf("%8.3f ", VAL(arr, i, j));
		}
		printf("\n");
	}
}

