
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>

#include "bench.h"

#define MINDIM		16
#define MAXDIM		1024
#define MAXTHREADS	8

/* Original author Bryan Ford <bryan.ford@yale.edu> for Determinator
 * http://github.com/bford/Determinator
 * Adopted for deterministic Linux by Chris Cotter <ccotter@utexas.edu>.
 */

typedef int elt;

elt a[MAXDIM*MAXDIM], b[MAXDIM*MAXDIM], r[MAXDIM*MAXDIM];

struct tharg {
	int bi, bj, nbi, nbj, dim;
};

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
	pthread_t *threads = malloc(sizeof(pthread_t) * nbi * nbj);
	for (bi = 0; bi < nbi; bi++)
		for (bj = 0; bj < nbj; bj++) {
			int child = bi*nbj + bj;
			arg[child].bi = bi;
			arg[child].bj = bj;
			arg[child].nbi = nbi;
			arg[child].nbj = nbj;
			arg[child].dim = dim;
			pthread_create(&threads[child], NULL, blkmult, &arg[child]);
		}

	// Now go back and merge in the results of all our children
	for (bi = 0; bi < nbi; bi++) {
		for (bj = 0; bj < nbj; bj++) {
			int child = bi*nbj + bj;
			pthread_join(threads[child], NULL);
		}
	}
	free(threads);
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

int main(int argc, char **argv)
{
	int nbi = 4;
	int nbj = 4;
	int nth = nbi * nbj;
	int dim = 1024;
	/* Block size = 256x256 */
	int i;
	int niter = 10;
	uint64_t tt = 0;
	for (i = 0; i < niter; ++i) {
		genmatrix(i);
		uint64_t ts = bench_time();
		matmult(nbi, nbj, dim);
		tt = bench_time() - ts;
	}
	tt /= niter;
	printf("blksize %dx%d thr %d itr %d: %lld.%09lld\n",
		dim/nbi, dim/nbj, nth, niter,
		(long long)tt / 1000000000,
		(long long)tt % 1000000000);
	return 0;
}

int main1(int argc, char **argv)
{
	int i;
	for (i = 0; i < MAXDIM*MAXDIM; i++)
		a[i] = b[i] = i;

	int dim, nth, nbi, nbj, iter;
	for (dim = MINDIM; dim <= MAXDIM; dim *= 2) {
		printf("matrix size: %dx%d = %d (%d bytes)\n",
			dim, dim, dim*dim, dim*dim*(int)sizeof(elt));
		for (nth = nbi = nbj = 1; nth <= MAXTHREADS; ) {
			assert(nth == nbi * nbj);
			int niter = MAXDIM/dim;
			niter = niter * niter; // * niter;	// MM = O(n^3)

			matmult(nbi, nbj, dim);	// once to warm up...

			uint64_t ts = bench_time();
			for (iter = 0; iter < niter; iter++)
				matmult(nbi, nbj, dim);
			uint64_t td = (bench_time() - ts) / niter;

			printf("blksize %dx%d thr %d itr %d: %lld.%09lld\n",
				dim/nbi, dim/nbj, nth, niter,
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

