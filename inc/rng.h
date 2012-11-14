
#ifndef _RNG_H_
/* Simple linear congruential generator. Uses values for A, C as used
 * by glibc. */
static unsigned __seed;
static unsigned __rng_a = 1103515245l;
static unsigned __rng_c = 12345;
static void bseed(unsigned seed)
{
	__seed = seed;
}
static unsigned brand(void)
{
	__seed = __rng_a * __seed + __rng_c;
	return __seed;
}
#endif

