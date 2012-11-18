
#include <debug.h>

// Brute-force MD5-based "password cracker":
// exhaustively searches for a short ASCII string
// whose MD5 hash yields a given hash output.
// Similar to pwcrack.c, but single-node only,
// and uses pthreads-compatible bench.h for benchmarking.

/**
 * Original program by Bryan Ford <bryan.ford@yale.edu>.
 * Written for Determinator: https://github.com/bford/Determinator
 *
 * This version of the md5 cracker uses deterministic Linux.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <bench.h>

#include "md5.c"	// Bad practice, but hey, it's easy...

#define MINCHR		(' ')	// Minimum ASCII value a string may contain
#define MAXCHR		('~')	// Maximum ASCII value a string may contain

#define MAXTHREADS	16
#define MAXLEN		10
#define BLOCKLEN	3

#define hexdig(c)	((c) >= '0' && (c) <= '9' ? (c) - '0' : \
			 (c) >= 'a' && (c) <= 'f' ? (c) - 'a' + 10 : \
			 (c) >= 'A' && (c) <= 'F' ? (c) - 'A' + 10 : -1)

int found;
char out[MAXLEN+1];
uint64_t tt;

// Increment string 'str' lexicographically,
// starting with the "least significant character" at position 'lo',
// and ending just before limit character position 'hi',
// returning 1 when all characters wrap around to MINCHR.
int
incstr(uint8_t *str, int lo, int hi)
{
	while (lo < hi) {
		assert(str[lo] >= MINCHR && str[lo] <= MAXCHR);
		if (++str[lo] <= MAXCHR)
			return 0;	// no carry; we're done.
		str[lo] = MINCHR;	// wrap around
		lo++;			// carry out to next character
	}
	return 1;			// carry out to hi position
}

typedef struct search_args {
	uint8_t str[MAXLEN+1];
	int len, lo, hi;
	const unsigned char *hash;
} search_args;

// Search all strings of length 'len' for one that hashes to 'hash'.
void *
search(void *args)
{
	search_args *a = args;
	assert(a->lo < a->hi);
	assert(a->hi <= a->len);

	do {
		unsigned char h[16];
		MD5_CTX ctx;
		MD5Init(&ctx);
		MD5Update(&ctx, a->str, a->len);
		MD5Final(h, &ctx);
		if (memcmp(h, a->hash, 16) == 0) {
			strcpy(out, (char*)a->str);
			found = 1;
			return NULL;
		}
	} while (!incstr(a->str, a->lo, a->hi));
	return NULL;
}

// Like 'search', but do in parallel across 2 nodes with 2 threads each
int psearch(uint8_t *str, int len, const unsigned char *hash, int nthreads)
{
	tt = bench_time();
	if (len <= BLOCKLEN) {
		search_args a = { {0}, len, 0, len, hash };
		strcpy((char*)a.str, (char*)str);
		search(&a);
		return found;
	}

	// Iterate over blocks, searching 4 blocks at a time in parallel
	int done = 0;
	do {
		int i;
		search_args a[nthreads];
		for (i = 0; i < nthreads; i++) {
			strcpy((char*)a[i].str, (char*)str);
			a[i].len = len;
			a[i].lo = 0;
			a[i].hi = BLOCKLEN;
			a[i].hash = hash;
			bench_fork(i, search, &a[i]);
			done |= incstr(str, BLOCKLEN, len);
		}
		for (i = 0; i < nthreads; i++)
			bench_join(i);	// collect results
		if (found)
			return 1;
	} while (!done);
	return 0;	// no match at this string length
}

void
crack(char *hashv, int nthreads)
{
	found = 0;
	// Decode the MD5 hash from hex
	unsigned char hash[16];
	int i;
	for (i = 0; i < 16; i++) {
		int hi = hexdig(hashv[i*2+0]);
		int lo = hexdig(hashv[i*2+1]);
		hash[i] = (hi << 4) | lo;
	}

	// Search all strings of length 1, then of length 2, ...
	int len;
	for (len = 1; len < MAXLEN; len++) {
		uint8_t str[len+1];
		str[len] = 0;		// Null terminate for printing match
		memset(str, MINCHR, len);
		if (psearch(str, len, hash, nthreads)) {
			tt = bench_time() - tt;
			printf("  nthreads = %d: found '%5s' in %ld.%09ld seconds.\n",
					nthreads,
					out,
					tt / 1000000000,
					tt % 1000000000);
			return;
		}
	}
}

int main(void)
{
	char *hashes[] = {
		"2bec52c0729e136fcf418b7ee42e51c7",
		"4e50f5e7a29d352bc4c3267cffdd50a4",
		"75cf1d4a01dfd9703c26927574d72230",
		"a6f931611c69f3500ad52e84a0c96d2d",
		"60662e6279e742da121699b10c5a9f54",
		"78d2a8b546632b0df5f0cd12b551998a",
		"dbdd1e7fba9a2ee6dd033af77c8ee398",
		"38f1eebefea62e6affc37d97202df948",
		"6e4eb59d8f151da5ce10866c35f433ab",
		"86f21f30b32ab591e804add38f82d7b8",
		"40cee8c8635cc4e301848e20c4eb840a",
		"fbf45a198e8b5be665c3903ddebb9eff"
	};
	int nthreads[] = {1, 2, 4, 8};
	unsigned i;
	for (i = 0; i < sizeof(hashes) / sizeof(char*); ++i) {
		unsigned j;
		printf("Searching for password to match %s..\n", hashes[i]);
		for (j = 0; j < sizeof(nthreads) / sizeof(int); ++j) {
			crack(hashes[i], nthreads[j]);
		}
		printf("\n");
	}
	return 0;
}

