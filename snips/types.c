
#ifndef BASE_TYPES_H
#define BASE_TYPES_H
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
typedef intptr_t isize;

// integer type capable of holding a pointer.
typedef uintptr_t uptr;
typedef intptr_t iptr;

// floating point
typedef float float32;
typedef double float64;
typedef long double float128;

/* end standard types */

#endif // BASE_TYPES_H

