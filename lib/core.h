
#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <stdio.h>

/* standard types */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// unsigned integer type with width of exactly 8, 16, 32 and 64 bits respectively.
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef __uint128_t uint128;

// signed integer type with width of exactly 8, 16, 32 and 64 bits respectively.
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef __int128_t int128;
typedef unsigned int uint;

// size is commonly used for array indexing and loop counting.
typedef size_t usize;

// integer type capable of holding a pointer.
typedef uintptr_t uptr;
typedef intptr_t iptr;
typedef ptrdiff_t ptrdiff;

// floating point
typedef float float32;
typedef double float64;
typedef long double float128;

/* end standard types */

#ifndef CORE_ASSERT
#define CORE_ASSERT assert
#include <assert.h>
#endif


// stretchy buffer
// init: NULL 
// free: array_free() 
// push_back: array_push() 
// size: array_count() 
#define array_free(a)         ((a) ? free(array__raw(a)),0 : 0)
#define array_push(a,v)       (array__maybegrow(a,1), (a)[array__n(a)++] = (v))
#define array_count(a)        ((a) ? array__n(a) : 0)
#define array_add(a,n)        (array__maybegrow(a,n), array__n(a)+=(n), &(a)[array__n(a)-(n)])
#define array_last(a)         ((a)[array__n(a)-1])

#include <stdlib.h>
#define array__raw(a) ((int *) (a) - 2)
#define array__m(a)   array__raw(a)[0]
#define array__n(a)   array__raw(a)[1]

#define array__needgrow(a,n)  ((a)==0 || array__n(a)+n >= array__m(a))
#define array__maybegrow(a,n) (array__needgrow(a,(n)) ? array__grow(a,n) : 0)
#define array__grow(a,n)  array__growf((void **) &(a), (n), sizeof(*(a)))

static void array__growf(void **arr, int increment, int itemsize)
{
	int m = *arr ? 2*array__m(*arr)+increment : increment+1;
	void *p = realloc(*arr ? array__raw(*arr) : 0, itemsize * m + sizeof(int)*2);
	assert(p);
	if (p) {
		if (!*arr) ((int *) p)[1] = 0;
		*arr = (void *) ((int *) p + 2);
		array__m(*arr) = m;
	}
}

#endif // CORE_H
