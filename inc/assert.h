
#ifndef _INC_ASSERT_H_
#define _INC_ASSERT_H_

#include <inc/std.h>
#include <inc/io.h>

#define assert(x)		\
	do { if (!(x)) {printf("assertion failed(%s:%d): %s\n", __FILE__, __LINE__, #x); _exit(1);}} while (0)

#define assert_concat2(a, b) a##b
#define assert_concat(a, b) assert_concat2(a,b)
#define static_assert(x) enum { assert_concat(__useless, __LINE__) = 1/(!!(x)) };

#endif

