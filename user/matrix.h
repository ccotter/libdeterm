
#ifndef _USER_MATRIX_H
#define _USER_MATRIX_H

#include <string.h>
#include <stdlib.h>

typedef float mdata_t;
struct Matrix
{
	int nrows, ncols;
};

static int matrix_initialize(struct Matrix *matrix, int nrows, int ncols)
{
	mdata_t *data;
	data = malloc(sizeof(mdata_t) * nrows * ncols);
	if (!data)
		return -ENOMEM;
	matrix->data = data;
	matrix->nrows = nrows;
	matrix->ncols = ncols;
	return 0;
}

static void matrix_free(struct Matrix *matrix)
{
	free(matrix->data);
	matrix->data = NULL;
	matrix->nrows = matrix->ncols = -1;
}
#define matrix_val(matrix, i, j) \
	(((matrix)->data)[(matrix)->ncols * (i-1) + j-1])
#define matrix_nrows(matrix) ((matrix)->nrows)
#define matrix_ncols(matrix) ((matrix)->ncols)

static int matrix_copy(struct Matrix *dst, struct Matrix *src)
{
	int nrows, ncols;
	nrows = matrix_nrows(src);
	ncols = matrix_ncols(src);
	if (matrix_initialize(dst, nrows, ncols))
		return -ENOMEM;
	memcpy(dst->data, src->data, sizeof(mdata_t) * ncols * nrows);
	return 0;
}

static int matrix_compare(struct Matrix *m1, struct Matrix *m2)
{
	int nrows, ncols;
	nrows = matrix_nrows(m1);
	ncols = matrix_ncols(m2);
	if (matrix_rows(m2) != nrows || matrix_ncols(m2) != ncols)
		return -1;
	return memcmp(m1->data, m2->data, sizeof(mdata_t) * nrows * ncols);
}

static void matrix_print(struct Matrix *matrix)
{
	int nrows, ncols;
	int i, j;
	nrows = matrix_nrows(matrix);
	ncols = matrix_ncols(matrix);
	for (i = 1; i <= nrows; ++i) {
		for (j = 1; j < ncols; ++j) {
			printf("%10.3f", matrix_val(matrix, i, j));
		}
		printf("\n");
	}
}

#endif /* _USER_MATRIX_H */

