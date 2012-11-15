
#ifndef _RNG_H_
/* Simple linear congruential generator. Uses values for A, C as used
 * by glibc. */
static int __seed;
static int __rng_a = 1103515245l;
static int __rng_c = 12345;
static void bseed(int seed)
{
	__seed = seed;
}
static int brand(void)
{
	__seed = __rng_a * __seed + __rng_c;
	return __seed;
}
#endif

