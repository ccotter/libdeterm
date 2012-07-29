
#include <inc/mman.h>
#include <inc/determinism_pure.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/fork.h>

#define ADDR1 (0x30000000)
#define SIZE (0x1000 * 100)

int x, y;

int randints[256] = {	// some random ints
	 20,726,926,682,210,585,829,491,612,744,753,405,346,189,669,416,
	 41,832,959,511,260,879,844,323,710,570,289,299,624,319,997,907,
	 56,545,122,497, 60,314,759,741,276,951,496,376,403,294,395, 96,
	372,402,468,866,782,524,739,273,462,920,965,225,164,687,628,127,
	998,957,973,212,801,790,254,855,215,979,229,234,194,755,174,793,
	367,865,458,479,117,471,113, 12,605,328,231,513,676,495,422,404,
	611,693, 32, 59,126,607,219,837,542,437,803,341,727,626,360,507,
	834,465,795,271,646,725,336,241, 42,353,438, 44,167,786, 51,873,
	874,994, 80,432,657,365,734,132,500,145,238,931,332,146,922,878,
	108,508,601, 38,749,606,565,642,261,767,312,410,239,476,498, 90,
	655,379,835,270,862,876,699,165,675,869,296,163,435,321, 88,575,
	233,745, 94,303,584,381,359, 50,766,534, 27,499,101,464,195,453,
	671, 87,139,123,544,560,679,616,705,494,733,678,927, 26, 14,114,
	140,777,250,564,596,802,723,383,808,817,  1,436,361,952,613,680,
	854,580, 76,891,888,721,204,989,882,141,448,286,964,130, 48,385,
	756,224,138,630,821,449,662,578,400, 74,477,275,272,392,747,394};
const int sortints[256] = {	// sorted array of the same ints
	  1, 12, 14, 20, 26, 27, 32, 38, 41, 42, 44, 48, 50, 51, 56, 59,
	 60, 74, 76, 80, 87, 88, 90, 94, 96,101,108,113,114,117,122,123,
	126,127,130,132,138,139,140,141,145,146,163,164,165,167,174,189,
	194,195,204,210,212,215,219,224,225,229,231,233,234,238,239,241,
	250,254,260,261,270,271,272,273,275,276,286,289,294,296,299,303,
	312,314,319,321,323,328,332,336,341,346,353,359,360,361,365,367,
	372,376,379,381,383,385,392,394,395,400,402,403,404,405,410,416,
	422,432,435,436,437,438,448,449,453,458,462,464,465,468,471,476,
	477,479,491,494,495,496,497,498,499,500,507,508,511,513,524,534,
	542,544,545,560,564,565,570,575,578,580,584,585,596,601,605,606,
	607,611,612,613,616,624,626,628,630,642,646,655,657,662,669,671,
	675,676,678,679,680,682,687,693,699,705,710,721,723,725,726,727,
	733,734,739,741,744,745,747,749,753,755,756,759,766,767,777,782,
	786,790,793,795,801,802,803,808,817,821,829,832,834,835,837,844,
	854,855,862,865,866,869,873,874,876,878,879,882,888,891,907,920,
	922,926,927,931,951,952,957,959,964,965,973,979,989,994,997,998};
int bs[256];

#define swapints(a,b) ({ int t = (a); (a) = (b); (b) = t; })

void
pqsort(int *lo, int *hi)
{
	if (lo >= hi)
		return;

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
	if (!dfork(0, DETERMINE_START | DETERMINE_SNAP)) {
		pqsort(lo, l-2);
		dret();
	}
	if (!dfork(1, DETERMINE_START | DETERMINE_SNAP)) {
		pqsort(h+1, hi);
		dret();
	}
    assert(-1 != dget(0, DETERMINE_MERGE, randints, sizeof(randints), NULL));
    assert(-1 != dget(1, DETERMINE_MERGE, randints, sizeof(randints), NULL));
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
			if (!dfork(child, DETERMINE_START | DETERMINE_SNAP)) {
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
			assert(-1 != dget(child, DETERMINE_MERGE, mr, sizeof(mr), NULL));
		}
}

#define N 2
#define NPAGES 30000
#define MEM_SIZE (N * 0x1000 * NPAGES)

void fillpage(int c)
{
    if (!dfork(c, DETERMINE_START | DETERMINE_SNAP))
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
    if (!dfork(c, DETERMINE_START | DETERMINE_SNAP))
    {
        int i;
        unsigned long *la = mmap((void*)(0x30000000 + c * 0x1000), 0x1000, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        assert((void*)(0x30000000 + c * 0x1000) == la);
        for (i = 0; i < 0x1000/sizeof(unsigned long); ++i)
            la[i] = 0xdeadbeef * c * i;
        dret();
    }
}

int main(void)
{
    int i, rc;
    unsigned char *ba = (void*)0x30000000;
    assert(0 == dput(0, DETERMINE_BECOME_MASTER, NULL, 0, NULL));
    iprintf("Ok1\n");

    x = y = 0;
    if (!dfork(10, DETERMINE_START | DETERMINE_SNAP)) { x = 0xdeadbeef; dret(); iprintf("OOps1\n"); while(1); }
    if (!dfork(11, DETERMINE_START | DETERMINE_SNAP)) { y = 0xabadcafe; dret(); iprintf("OOPs2\n");while(1); }
    assert(0 == x); assert(0 == y);
    assert(-1 != dget(10, DETERMINE_MERGE, (void*)&x, sizeof(int), NULL));
    assert(-1 != dget(11, DETERMINE_MERGE, (void*)&y, sizeof(int), NULL));
    assert(0xdeadbeef == x); assert(0xabadcafe == y);
    printf("Ok2\n");
    return 0;

    if (!dfork(0, DETERMINE_START | DETERMINE_SNAP)) { x = y; dret(); }
    if (!dfork(1, DETERMINE_START | DETERMINE_SNAP)) { y = x; dret(); }
    assert(0xdeadbeef == x); assert(0xabadcafe == y);
    assert(-1 != dget(0, DETERMINE_MERGE, (void*)&x, sizeof(int), NULL));
    assert(-1 != dget(1, DETERMINE_MERGE, (void*)&y, sizeof(int), NULL));
    assert(0xdeadbeef == y); assert(0xabadcafe == x);

    /* More complicated merge opportunities. */
    pqsort(&randints[0], &randints[256-1]);
    assert(0 == memcmp(randints, sortints, 256*sizeof(int)));

    /* Matrix multiplication. */
    matmult(ma, mb, mr);
    assert(sizeof(mr) == sizeof(int)*8*8); /* These can be determined statically...? */
    assert(sizeof(mc) == sizeof(int)*8*8);
    assert(0 == memcmp(mr, mc, sizeof(mr)));
    printf("Start large\n");

    /* Merge N processes where the N-th process touches bytes in the address space modulo N. */
    /*ba = mmap(ba, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
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
    dput(0, 0, NULL, 0, NULL);
    dput(1, 0, NULL, 0, NULL);
    printf("Startin\n");
    for (i = 0; i < N; ++i)
    {
        assert(-1 != dget(i, DETERMINE_MERGE, (void*)0x30000000, MEM_SIZE, NULL));
    }
    printf("Done\n");
    for (i = 0; i < MEM_SIZE; ++i)
    {
        assert(!!ba[i]);
    }
    /* If we merge the same child again we should get a conflict! */
    assert(-1 == dget(0, DETERMINE_MERGE, (void*)0x30000000, MEM_SIZE, NULL));
    munmap((void*)0x30000000, MEM_SIZE);

    /* Merge N processes that have set a distinct page of memory to be filled uniquely. */
    for (i = 0; i < N; ++i)
    {
        fillonepage(i);
    }
    for (i = 0; i < N; ++i)
    {
        assert(-1 != dget(i, DETERMINE_MERGE, (void*)0x30000000, 0x1000 * N, NULL));
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

    printf("Merge success.\n");

    return 0;
}

