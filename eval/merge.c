
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

/**
 * Adopted the merge-sort algorithm and parallel variant from
 * "Introduction to Algorithms" 3rd edition, by Cormen, Lieseron, Rivest, Stein.
 *
 * The parallel merge-sort only spawns up to 2 ^ MAX_DEPTH threads to reduce
 * the overhead of having too many threads doing little work.
 *
 */

#define MAX_DEPTH 5
static void pmerge_sort(int *A, int p, int r, int depth);

static void merge(int *A, int p, int q, int r)
{
	int i, j, k;
	int n1 = q - p;
	int n2 = r - q;
	int *L = malloc(sizeof(int) * (n1 + 1));
	int *R = malloc(sizeof(int) * (n2 + 1));
	for (i = 0; i < n1; ++i)
		L[i] = A[p + i];
	for (i = 0; i < n2; ++i)
		R[i] = A[q + i];
	L[n1] = R[n2] = INT_MAX;
	i = j = 0;
	for (k = p; k < r; ++k) {
		if (L[i] <= R[j]) {
			A[k] = L[i];
			++i;
		} else {
			A[k] = R[j];
			++j;
		}
	}
	free(L);
	free(R);
}

void merge_sort(int *A, int p, int r)
{
	if (p + 1 < r) {
		int q = (p + r) / 2;
		merge_sort(A, p, q);
		merge_sort(A, q, r);
		merge(A, p, q, r);
	}
}

struct pmerge_data
{
	int *A;
	int p, r, depth;
};

static void *pmerge_trampoline(void *_data)
{
	struct pmerge_data *data = _data;
	pmerge_sort(data->A, data->p, data->r, data->depth);
	return NULL;
}

static void pmerge_sort(int *A, int p, int r, int depth)
{
	if (p + 1 < r) {
		int q = (p + r) / 2;
		if (depth < MAX_DEPTH) {
			pthread_t thread;
			struct pmerge_data data;
			data.A = A;
			data.p = p;
			data.r = q;
			data.depth = depth + 1;
			pthread_create(&thread, NULL, pmerge_trampoline, &data);
			pmerge_sort(A, q, r, depth + 1);
			pthread_join(thread, NULL);
		} else {
			merge_sort(A, p, q);
			merge_sort(A, q, r);
		}
		merge(A, p, q, r);
	}
}

static void print(int *a, int len)
{
	int i;
	for (i = 0; i < len; ++i) {
		printf("%6d ", a[i]);
	}
	printf("\n");
}

static int is_sorted(int *A, int len)
{
	int i;
	for (i = 0; i < len - 1; ++i) {
		if (A[i] > A[i+1])
			return 0;
	}
	return 1;
}

void read_array(int **A, int *len)
{
	int *ar, i;
	scanf("%d", len);
	ar = malloc(sizeof(int) * *len);
	for (i = 0; i < *len; ++i) {
		scanf("%d", &ar[i]);
	}
	*A = ar;
}

static void usage(char **argv)
{
	fprintf(stderr, "Usage: %s <type\n  type: 's' for serial, 'p' for parallel\n",
			argv[0]);
	exit(1);
}

int main(int argc, char **argv)
{
	char ch = argc == 2 ? argv[1][0] : '-';
	if (ch != 'p' && ch != 's')
		usage(argv);
	int *A, len;
	read_array(&A, &len);
	if (ch == 's') {
		merge_sort(A, 0, len);
	} else {
		pmerge_sort(A, 0, len, 0);
	}
	if (is_sorted(A, len)) {
		printf("SORTED\n");
	} else {
		printf("NOT SORTED\n");
	}
	return 0;
}

