
#ifndef BASE_TYPES_H
#define BASE_TYPES_H
/* standard types */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// unsigned integer type with width of exactly 8, 16, 32 and 64 bits respectively.
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;

// signed integer type with width of exactly 8, 16, 32 and 64 bits respectively.
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef __int128_t i128;
typedef unsigned int uint;

// size is commonly used for array indexing and loop counting.
typedef size_t usize;
typedef intptr_t isize;

// integer type capable of holding a pointer.
typedef uintptr_t uptr;
typedef intptr_t iptr;

// floating point
typedef float f32;
typedef double f64;
typedef long double f128;

/* end standard types */

#endif // BASE_TYPES_H

