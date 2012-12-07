
#include <stdio.h>
#include <assert.h>
#include <determinism.h>
#include <stdlib.h>

#include <bench.h>

#define MINDIM		16
#define MAXDIM		1024
#define MAXTHREADS	256

/* Original author Bryan Ford <bryan.ford@yale.edu> for Determinator
 * http://github.com/bford/Determinator
 * Adopted for deterministic Linux by Chris Cotter <ccotter@utexas.edu>.
 */

uint64_t tm;
typedef int elt;

elt a[MAXDIM*MAXDIM], b[MAXDIM*MAXDIM], r[MAXDIM*MAXDIM];

struct tharg {
	int bi, bj, nbi, nbj, dim;
};

void printm(elt *m, int n)
{
	int i, j;
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			printf("%d ", m[i * n + j]);
		}
		printf("\n");
	}
}

void *
blkmult(void *varg)
{
	struct tharg *arg = varg;
	int dim = arg->dim;	// Total matrix size in each dimension
	int nbi = arg->nbi;	// Number of blocks in i dimension
	int nbj = arg->nbj;	// Number of blocks in j dimension
	int bsi = dim/nbi;	// Block size in i dimension
	int bsj = dim/nbj;	// Block size in j dimension

	int il = arg->bi * bsi, jl = arg->bj * bsj;
	int ih = il + bsi,	jh = jl + bsj;
	int ii, jj, kk;
	for (ii = il; ii < ih; ii++)
		for (jj = jl; jj < jh; jj++) {
			elt sum = 0;
			for (kk = 0; kk < dim; kk++)
				sum += a[ii*dim+kk] * b[kk*dim+jj];
			r[ii*dim+jj] = sum;
		}
	return NULL;
}

void
matmult(int nbi, int nbj, int dim)
{
	assert(dim >= 1 && dim <= MAXDIM);
	assert(nbi >= 1 && nbi <= dim); assert(dim % nbi == 0);
	assert(nbj >= 1 && nbj <= dim); assert(dim % nbj == 0);

	int nth = nbi*nbj;
	assert(nth >= 1 && nth <= 256);

	int bi,bj;
	struct tharg arg[256];

	// Fork off a thread to compute each cell in the result matrix
	for (bi = 0; bi < nbi; bi++)
		for (bj = 0; bj < nbj; bj++) {
			int child = bi*nbj + bj;
			arg[child].bi = bi;
			arg[child].bj = bj;
			arg[child].nbi = nbi;
			arg[child].nbj = nbj;
			arg[child].dim = dim;
			bench_fork(child, blkmult, &arg[child]);
		}

	// Now go back and merge in the results of all our children
	for (bi = 0; bi < nbi; bi++) {
		for (bj = 0; bj < nbj; bj++) {
			int child = bi*nbj + bj;
			dget(child, 0, 0, 0, 0);
			uint64_t ts = bench_time();
			bench_join(child);
			tm += bench_time() - ts;
		}
	}
}

#include "../inc/rng.h"
void genmatrix(int seed)
{
	bseed(seed);
	int i;
	for (i = 0; i < MAXDIM*MAXDIM; ++i) {
		a[i] = brand() % 2000 + 1000;
		b[i] = brand() % 2000 + 1000;
	}
}

int main1(int argc, char **argv)
{
	int nbi = 4;
	int nbj = 4;
	int nth = nbi * nbj;
	int dim = 1024;
	/* Block size = 256x256 */
	int i;
	int niter = 10;
	uint64_t tt = 0;
	tm = 0;
	for (i = 0; i < niter; ++i) {
		genmatrix(i);
		uint64_t ts = bench_time();
		matmult(nbi, nbj, dim);
		tt += bench_time() - ts;
	}
	tt /= niter;
	tm /= niter;
	printf("blksize %dx%d thr %d itr %d: %lld.%09lld\n",
		dim/nbi, dim/nbj, nth, niter,
		(long long)tt / 1000000000,
		(long long)tt % 1000000000);
	printf("merge time: %lld.%09lld\n",
			(long long)tm / 1000000000,
			(long long)tm % 1000000000);
	return 0;
}

static void usage(char **argv)
{
	printf("usage: %s nbi nbj\n  nbi/nbj Number of row/column partitions.\n",
			argv[0]);
	exit(1);
}

int main(int argc, char **argv)
{
	int counter = 0;
	int dim, nth, nbi, nbj, iter;

	if (3 != argc)
		usage(argv);
	nbi = strtol(argv[1], NULL, 0);
	nbj = strtol(argv[2], NULL, 0);
	if (nbi <= 0 || nbj <= 0)
		usage(argv);

	int niter = 10;
	for (dim = MINDIM; dim <= MAXDIM; dim *= 2) {
		nth = nbi * nbj;

		/* Once to warm up. */
		genmatrix(counter++);
		matmult(nbi, nbj, dim);

		printf("matrix size: %dx%d = %d (%d bytes)",
				dim, dim, dim*dim, dim*dim*(int)sizeof(elt));
		printf("blksize %dx%d thr %d itr %d:\n",
			dim/nbi, dim/nbj, nth, niter);
		for (iter = 0; iter < niter; iter++) {
			tm = 0;
			genmatrix(counter++);
			uint64_t ts = bench_time();
			matmult(nbi, nbj, dim);
			ts = bench_time() - ts;
			printf("%lld.%09lld %lld.%09lld\n",
					(long long)ts / 1000000000,
					(long long)ts % 1000000000,
					(long long)tm / 1000000000,
					(long long)tm % 1000000000);
		}

	}

	return 0;
}

