
#include <stdio.h>
#include <pthread.h>

/* Used
 * http://www.personal.kent.edu/~rmuhamma/Algorithms/MyAlgorithms/Sorting/quickSort.htm
 * as a reference for the quicksort algorithm.
 *
 */

static void quicksort(int *a, int start, int end);

static inline void swap(int *a, int l, int r)
{
	int tmp = a[l];
	a[l] = a[r];
	a[r] = tmp;
}

static void quicksort(int *a, int start, int end)
{
	int n = end - start;
	if (n < 2)
		return;
	int p = a[start + n / 2];
	int l = start;
	int r = end - 1;
	while (l <= r) {
		while (a[l] < p) ++l;
		while (a[r] > p) --r;
		if (l <= r) {
			swap(a, l, r);
			++l;
			--r;
		}
	}
	quicksort(a, start, r + 1);
	quicksort(a, l, end);
}

void print(int *a, int len)
{
	int i;
	for (i = 0; i < len; ++i) {
		printf("%6d ", a[i]);
	}
	printf("\n");
}

int main(void)
{
	int a[] = {6,3,1,6,8,9,67,3,2,21};
	int len = sizeof(a) / sizeof(int);
	quicksort(a, 0, len);
	print(a, len);
	return 0;
}

