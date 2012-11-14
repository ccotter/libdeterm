
/*
 * Adapted from PIOS for use in libdeterm by Chris Cotter.
 * Aug 2012.
 */

#ifndef _INC_STDINT_H_
#define _INC_STDINT_H_

// Explicitly-sized versions of integer types
typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef short			int16_t;
typedef unsigned short		uint16_t;
typedef int			int32_t;
typedef unsigned int		uint32_t;
typedef long long		int64_t;
typedef unsigned long long	uint64_t;

// Pointers and addresses are 32 bits long.
// We use pointer types to represent virtual addresses,
// and [u]intptr_t to represent the numerical values of virtual addresses.
typedef int			intptr_t;	// pointer-size signed integer
typedef unsigned		uintptr_t;	// pointer-size unsigned integer
typedef int			ptrdiff_t;	// difference between pointers

// size_t is used for memory object sizes, and ssize_t is a signed analog.
typedef unsigned		size_t;
typedef int			ssize_t;

#endif

