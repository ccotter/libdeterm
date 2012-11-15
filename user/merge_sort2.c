
#include <bench.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* The following is an auto generated file that contains 8
 * arrays to sort. */
#include "../eval/merge_sort_arrays2.h"

/**
 * Adopted the merge-sort algorithm and parallel variant from
 * "Introduction to Algorithms" 3rd edition, by Cormen, Lieseron, Rivest, Stein.
 *
 * The parallel merge-sort only spawns up to 2 ^ max_depth threads to reduce
 * the overhead of having too many threads doing little work.
 *
 */

/* Describe an array that we can sort. Data #included above. */
struct array_info
{
	int len;
	int *A;
};
struct array_info tosort[7];

static int max_depth;
static void pmerge_sort(int *A, int p, int r, int depth);

static void print(int *a, int len)
{
	int i;
	for (i = 0; i < len; ++i) {
		printf("%6d ", a[i]);
	}
	printf("\n");
}

static void merge(int *A, int p, int q, int r)
{
	int i, j, k;
	int n1 = q - p;
	int n2 = r - q;
	int *L = tmpleft;
	int *R = tmpright;
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

int childid = 0;
static void pmerge_sort(int *A, int p, int r, int depth)
{
	printf("[%d %d)\n", p,r);
	if (p + 1 < r) {
		int q = (p + r) / 2;
		if (depth < max_depth) {
			struct pmerge_data data;
			data.A = A;
			data.p = p;
			data.r = q;
			data.depth = depth + 1;
			int id = ++childid;
			bench_fork(id, pmerge_trampoline, &data);
			pmerge_sort(A, q, r, depth + 1);
			//bench_join(id);
			bench_join2(id, A + p, r - p);
		} else {
			merge_sort(A, p, q);
			merge_sort(A, q, r);
		}
		merge(A, p, q, r);
	}
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
	printf("Usage: %s -t s|p -d max_depth \n", argv[0]);
	printf("  type: 's' for serial, 'p' for parallel\n");
	exit(1);
}

int len0 = 6;
int array0[] = {20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};

static void pass(char type)
{
	unsigned i;
	for (i = 0; i < sizeof(tosort) / sizeof(struct array_info); ++i) {
		int *A, len;
		A = tosort[i].A;
		len = tosort[i].len;

		/* Do a first pass to ensure the integers are loaded in memory. */
		memmove(tmpspace, A, len * sizeof(int));
		if (type == 's') {
			merge_sort(tmpspace, 0, len);
		} else {
			pmerge_sort(tmpspace, 0, len, 0);
		}
		print(tmpspace, len);
goto ok;
		/* The the actual benchmark. */
		memmove(tmpspace, A, len * sizeof(int));
		uint64_t start = bench_time();
		if (type == 's') {
			merge_sort(tmpspace, 0, len);
		} else {
			pmerge_sort(tmpspace, 0, len, 0);
		}
		uint64_t end = bench_time() - start;
		printf("Sorting %15d ints took %lld.%09lld\n",
				len,
				(long long)end / 1000000000,
				(long long)end % 1000000000);

ok:
		if (!is_sorted(tmpspace, len)) {
			printf("NOT SORTED\n");
			exit(1);
		} else {
			printf("array %d\n",i);
		}
	}
}

int main(int argc, char **argv)
{
	int c;
	char type = -1;
	max_depth = -1;
	while ((c = getopt(argc, argv, "t:d:")) != -1) {
		switch (c) {
			case 't':
				type = optarg[0];
				if (type != 'p' && type != 's')
					usage(argv);
				break;
			case 'd':
				max_depth = strtol(optarg, NULL, 10);
				break;
			case '?':
			default:
				usage(argv);
		}
	}
	if (type == -1 || max_depth == -1)
		usage(argv);

	tosort[0].len = len1; tosort[0].A = array1;
	tosort[0].len = len0; tosort[0].A = array0;
	tosort[1].len = len2; tosort[1].A = array2;
	tosort[2].len = len3; tosort[2].A = array3;
	tosort[3].len = len4; tosort[3].A = array4;
	tosort[4].len = len5; tosort[4].A = array5;
	tosort[5].len = len6; tosort[5].A = array6;
	tosort[6].len = len7; tosort[6].A = array7;

	pass(type);
	return 0;
}

