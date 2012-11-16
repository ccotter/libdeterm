
#include <determinism.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <bench.h>

#define MINDIM		1024
#define MAXDIM		1024
#define MAXTHREADS	16

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

	long waittime=0;

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
			/*if (!dput(child, DET_START, 0, 0, 0)) {
				blkmult(&arg[child]);
				dret();
			}*/
		}

	// Now go back and merge in the results of all our children
	for (bi = 0; bi < nbi; bi++) {
		for (bj = 0; bj < nbj; bj++) {
			int child = bi*nbj + bj;
			dget(child, DET_GET_STATUS,0,0,0);
			long T = bench_time();
			bench_join(child);
			waittime += bench_time()-T;
			/*dget(0, DET_GET_STATUS,0,0,0);
			long T = bench_time();
			int rc=dget(child, DET_VM_COPY, (long)r, sizeof(r), (long)r);
			waittime += bench_time()-T;
			if (rc<0)
				printf("rc=%d\n",rc);
			dput(child, DET_KILL, 0, 0, 0);*/
		}
	}
	printf("waited %ld.%09ld\n", waittime/1000000000,waittime%1000000000);
}

int main(int argc, char **argv)
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

