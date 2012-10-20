
#include <assert.h>
#include <debug.h>
#include <sys/mman.h>
#include <determinism.h>
#include <stdlib.h>

/* Original credit to PIOS:
 * Bryan Ford (https://github.com/bford/Determinator) */

#define ADDR1 (0x30000000)
#define SIZE (0x1000 * 5)

unsigned long x, y;

#define SORT_SIZE 16
int randints[16] = {	// some random ints
    20,726,926,682,210,585,829,491,612,744,753,405,346,189,669,416};
const int sortints[16] = {	// sorted array of the same ints
    20,189,210,346,405,416,491,585,612,669,682,726,744,753,829,926};
/*
int randints[8] = {	// some random ints
    20,726,926,682,210,585,829,491};
const int sortints[8] = {	// sorted array of the same ints
    20,210,491,585,682,726,829,926};*/
/*
int randints[4] = {	// some random ints
    20,726,926,682};
const int sortints[4] = {	// sorted array of the same ints
    20,682,726,926};
*/
#define swapints(a,b) ({ int t = (a); (a) = (b); (b) = t; })
#define VAL(x) (x - randints)

void
pqsort(int *lo, int *hi)
{
	if (lo >= hi)
    {
		return;
    }

	int pivot = *lo;	// yeah, bad way to choose pivot...
	int *l = lo+1, *h = hi;
	while (l <= h) {
		if (*l < pivot)
			l++;
		else if (*h > pivot)
			h--;
		else
			swapints(*h, *l), l++, h--;
	}
	swapints(*lo, l[-1]);
	// Now recursively sort the two halves in parallel subprocesses
	if (!dput(0, DET_START | DET_SNAP,0,0,0)) {
		pqsort(lo, l-2);
		dret();
	}
	if (!dput(1, DET_START | DET_SNAP,0,0,0)) {
		pqsort(h+1, hi);
		dret();
	}
    assert(0 < dget(0, DET_MERGE, (unsigned long)randints, sizeof(randints), 0));
    assert(0 < dget(1, DET_MERGE, (unsigned long)randints, sizeof(randints), 0));
	dput(0, DET_KILL, 0, 0, 0);
	dput(1, DET_KILL, 0, 0, 0);
}

int ma[8][8] = {	// First matrix to multiply
	{146, 3, 189, 106, 239, 208, 8, 122},
	{200, 225, 94, 74, 143, 3, 127, 59},
	{32, 127, 52, 205, 0, 86, 143, 213},
	{159, 135, 45, 198, 152, 70, 116, 234},
	{238, 68, 215, 168, 79, 235, 15, 189},
	{82, 160, 97, 132, 186, 1, 220, 48},
	{178, 39, 153, 15, 16, 227, 251, 198},
	{148, 1, 239, 153, 39, 137, 42, 161}};
int mb[8][8] = {	// Second matrix to multiply
	{75, 95, 165, 229, 14, 90, 222, 236},
	{171, 131, 12, 84, 120, 147, 76, 69},
	{235, 51, 255, 250, 222, 64, 9, 1},
	{206, 7, 13, 120, 23, 137, 178, 81},
	{57, 184, 224, 142, 22, 184, 3, 132},
	{49, 30, 70, 28, 239, 52, 217, 13},
	{217, 50, 44, 35, 216, 134, 49, 123},
	{119, 13, 157, 196, 37, 87, 38, 126}};
int mr[8][8];		// Result matrix
const int mc[8][8] = {	// Matrix of correct answers
	{117783, 76846, 161301, 157610, 108012, 106677, 104090, 94046},
	{133687, 87306, 107725, 133479, 85848, 115848, 85063, 110783},
	{139159, 36263, 68482, 104757, 91270, 95127, 87477, 78517},
	{151485, 75381, 122694, 156229, 86758, 131671, 111429, 127648},
	{156375, 68452, 161574, 189491, 131222, 113401, 148992, 113825},
	{147600, 80499, 100851, 115856, 98545, 123124, 68112, 98854},
	{149128, 54805, 130652, 140309, 157630, 99208, 115657, 106951},
	{136163, 42930, 132817, 154486, 107399, 83659, 100339, 80010}};

void
matmult(int a[8][8], int b[8][8], int r[8][8])
{
	int i,j,k;

	// Fork off a thread to compute each cell in the result matrix
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++) {
			int child = i*8 + j;
			if (!dput(child, DET_START | DET_SNAP,0,0,0)) {
				int sum = 0;	// in child: compute cell i,j
				for (k = 0; k < 8; k++)
					sum += a[i][k] * b[k][j];
				r[i][j] = sum;
				dret();
			}
		}

	// Now go back and merge in the results of all our children
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++) {
			int child = i*8 + j;
			assert(-1 != dget(child, DET_MERGE, (unsigned long)mr, sizeof(mr), 0));
		}
		
}

#define N 2
//#define NPAGES 30000
#define NPAGES 300
#define MEM_SIZE (N * 0x1000 * NPAGES)

void fillpage(int c)
{
    if (!dput(c, DET_START | DET_SNAP,0,0,0))
    {
        unsigned char *a = mmap((void*)0x30000000, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
        assert((void*)0x30000000 == a);
        int i;
        for (i = c; i < MEM_SIZE; i += N)
        {
            a[i] = 1;
        }
        dret();
    }
}

void fillonepage(int c)
{
    if (!dput(c, DET_START | DET_SNAP,0,0,0))
    {
        unsigned i;
        unsigned long *la = mmap((void*)(0x30000000L + c * 0x1000),
				0x1000, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        assert((void*)(0x30000000L + c * 0x1000) == la);
        for (i = 0; i < 0x1000/sizeof(unsigned long); ++i)
            la[i] = 0xdeadbeef * c * i;
        dret();
    }
}

/* 4 MB */
unsigned char QWE[0x1000 * 20000];

int main(int argc, char **argv)
{
    int i;
    unsigned char *ba = (void*)0x30000000;
    int rc;
	while(1);

    x = y = 0;
    if (!(rc=dput(0, DET_START | DET_SNAP,0,0,0)))
    {
        x = 0xdeadbeef;
        dret();
    }
    if (!(rc=dput(1, DET_START | DET_SNAP,0,0,0)))
    {
        y = 0xabadcafe;
        dret();
    }
    assert(0 == x); assert(0 == y);
    assert(0 < dget(0, DET_MERGE, (unsigned long)&x, sizeof(x), 0));
    assert(0 < dget(1, DET_MERGE, (unsigned long)&y, sizeof(y), 0));
    assert(0xdeadbeef == x); assert(0xabadcafe == y);
    dput(0, DET_KILL, 0, 0, 0);
    dput(1, DET_KILL, 0, 0, 0);

	x=0xdeadbeef;
	y=0xabadcafe;
    if (!dput(0, DET_START | DET_SNAP,0,0,0)) { x = y; dret(); }
    if (!dput(1, DET_START | DET_SNAP,0,0,0)) { y = x; dret(); }
    assert(0xdeadbeef == x); assert(0xabadcafe == y);
    assert(0 < dget(0, DET_MERGE, (unsigned long)&x, sizeof(x), 0));
    assert(0 < dget(1, DET_MERGE, (unsigned long)&y, sizeof(y), 0));
	iprintf("%lx %lx\n",x,y);
    assert(0xdeadbeef == y); assert(0xabadcafe == x);
    dput(0, DET_KILL, 0, 0, 0);
    dput(1, DET_KILL, 0, 0, 0);
f1:
    /* More complicated merge opportunities. */
    pqsort(&randints[0], &randints[SORT_SIZE-1]);
    assert(0 == memcmp(randints, sortints, SORT_SIZE*sizeof(int)));
	iprintf("PQsort success\n");

    /* Matrix multiplication. */
    matmult(ma, mb, mr);
    assert(sizeof(mr) == sizeof(int)*8*8); /* These can be
                                              determined statically...? */
    assert(sizeof(mc) == sizeof(int)*8*8);
    assert(0 == memcmp(mr, mc, sizeof(mr)));
	iprintf("Mat mult success\n");
    return 0;
    iprintf("Start large\n");

    /* Merge N processes where the N-th process touches bytes in the address
       space modulo N. */
    /*ba = mmap(ba, MEM_SIZE, PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    assert(ba==(void*)0x10000000);
    for (i = 0; i < MEM_SIZE; ++i)
        ba[i] = 0;*/
    for (i = 0; i < N; ++i)
    {
        fillpage(i);
    }
    //dput(0, DETERMINE_DEBUG, (void*)1, 0, NULL);
    //dput(0, DETERMINE_DEBUG, (void*)2, 0, NULL);
    //dput(1, DETERMINE_DEBUG, (void*)2, 0, NULL);
    dput(0, 0, 0, 0, 0);
    dput(1, 0, 0, 0, 0);
    iprintf("Starting\n");
    for (i = 0; i < N; ++i)
    {
        assert(0 < dget(i, DET_MERGE, 0x30000000,
                    MEM_SIZE, 0));
    }
    iprintf("Done\n");
    for (i = 0; i < MEM_SIZE; ++i)
    {
        assert(!!ba[i]);
    }
#if 0
    /* If we merge the same child again we should get a conflict! */
    assert(-1 == dget(0, DET_MERGE, (void*)0x30000000, MEM_SIZE, NULL));
    munmap((void*)0x30000000, MEM_SIZE);

    /* Merge N processes that have set a distinct page of memory to be
       filled uniquely. */
    for (i = 0; i < N; ++i)
    {
        fillonepage(i);
    }
    for (i = 0; i < N; ++i)
    {
        assert(-1 != dget(i, DET_MERGE, (void*)0x30000000,
                    0x1000 * N, NULL));
    }
    for (i = 0; i < N; ++i)
    {
        int j;
        unsigned long *la = (void*)(0x30000000 + i * 0x1000);
        for (j = 0; j < 0x1000/sizeof(unsigned long); ++j)
        {
            assert((0xdeadbeef * i * j) == la[j]);
        }
    }
#endif

    iprintf("Merge success.\n");

    return 0;
}

