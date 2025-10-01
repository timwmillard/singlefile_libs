// Copyright 2025 Tim Millard <timwmillard@gmail.com>
// Copyright 2022 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef LIB_H
#define LIB_H

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

#ifndef ASSERT
#define ASSERT assert
#include <assert.h>
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef ARENA_NOSTDIO
#include <stdarg.h>
#include <stdio.h>
#endif // ARENA_NOSTDIO

#ifndef ARENA_ASSERT
#include <assert.h>
#define ARENA_ASSERT assert
#endif

#define ARENA_BACKEND_LIBC_MALLOC 0
#define ARENA_BACKEND_LINUX_MMAP 1
#define ARENA_BACKEND_WIN32_VIRTUALALLOC 2
#define ARENA_BACKEND_WASM_HEAPBASE 3

#ifndef ARENA_BACKEND
#define ARENA_BACKEND ARENA_BACKEND_LIBC_MALLOC
#endif // ARENA_BACKEND

typedef struct Region Region;

struct Region {
    Region *next;
    size_t count;
    size_t capacity;
    uintptr_t data[];
};

typedef struct {
    Region *begin, *end;
} Arena;

typedef struct  {
    Region *region;
    size_t count;
} Arena_Mark;

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

Region *new_region(size_t capacity);
void free_region(Region *r);

void *arena_alloc(Arena *a, size_t size_bytes);
void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz);
char *arena_strdup(Arena *a, const char *cstr);
void *arena_memdup(Arena *a, void *data, size_t size);
void *arena_memcpy(void *dest, const void *src, size_t n);
#ifndef ARENA_NOSTDIO
char *arena_sprintf(Arena *a, const char *format, ...);
char *arena_vsprintf(Arena *a, const char *format, va_list args);
#endif // ARENA_NOSTDIO

Arena_Mark arena_snapshot(Arena *a);
void arena_reset(Arena *a);
void arena_rewind(Arena *a, Arena_Mark m);
void arena_free(Arena *a);
void arena_trim(Arena *a);


// stretchy buffer
// init: NULL 
// free: array_free() 
// push_back: array_push() 
// size: array_count() 
#define array_free(a)         ((a) ? free(array__raw(a)),0 : 0)
#define array_push(a,v)       (array__maybegrow(a,1), (a)[array__n(a)++] = (v))
#define array_count(a)        ((a) ? array__n(a) : 0)
#define array_capacity(a)     ((a) ? array__m(a) : 0)
#define array_add(a,n)        (array__maybegrow(a,n), array__n(a)+=(n), &(a)[array__n(a)-(n)])
#define array_last(a)         ((a)[array__n(a)-1])
#define array_arena(a, al)    (array__o(a) = al)

#include <stdlib.h>
#define array__raw(a) ((size_t *) (a) - 3)
#define array__m(a)   array__raw(a)[0] // count
#define array__n(a)   array__raw(a)[1] // capacity
#define array__o(a)   array__raw(a)[2] // arena

#define array__needgrow(a,n)  ((a)==0 || array__n(a)+n >= array__m(a))
#define array__maybegrow(a,n) (array__needgrow(a,(n)) ? array__grow(a,n) : 0)
#define array__grow(a,n)  array__growf((void **) &(a), (n), sizeof(*(a)))

static void array__growf(void **arr, int increment, int itemsize)
{
	int m = *arr ? 2*array__m(*arr)+increment : increment+1;
    void *p;
    if (array__o(*arr))
        p = arena_realloc((Arena*)array__o(*arr), *arr ? array__raw(*arr) : 0, array__n(*arr), itemsize * m + sizeof(size_t)*3);
    else
        p = reallocf(*arr ? array__raw(*arr) : 0, itemsize * m + sizeof(size_t)*3);
	assert(p);
	if (p) {
		if (!*arr) ((int *) p)[1] = 0;
		*arr = (void *) ((int *) p + 2);
		array__m(*arr) = m;
	}
}

#endif // LIB_H

#ifdef LIB_IMPLEMENTATION

#if ARENA_BACKEND == ARENA_BACKEND_LIBC_MALLOC
#include <stdlib.h>

// TODO: instead of accepting specific capacity new_region() should accept the size of the object we want to fit into the region
// It should be up to new_region() to decide the actual capacity to allocate
Region *new_region(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t)*capacity;
    // TODO: it would be nice if we could guarantee that the regions are allocated by ARENA_BACKEND_LIBC_MALLOC are page aligned
    Region *r = (Region*)malloc(size_bytes);
    ARENA_ASSERT(r); // TODO: since ARENA_ASSERT is disableable go through all the places where we use it to check for failed memory allocation and return with NULL there.
    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    free(r);
}
#elif ARENA_BACKEND == ARENA_BACKEND_LINUX_MMAP
#include <unistd.h>
#include <sys/mman.h>

Region *new_region(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t) * capacity;
    Region *r = mmap(NULL, size_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    ARENA_ASSERT(r != MAP_FAILED);
    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t) * r->capacity;
    int ret = munmap(r, size_bytes);
    ARENA_ASSERT(ret == 0);
}

#elif ARENA_BACKEND == ARENA_BACKEND_WIN32_VIRTUALALLOC

#if !defined(_WIN32)
#  error "Current platform is not Windows"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define INV_HANDLE(x)       (((x) == NULL) || ((x) == INVALID_HANDLE_VALUE))

Region *new_region(size_t capacity)
{
    SIZE_T size_bytes = sizeof(Region) + sizeof(uintptr_t) * capacity;
    Region *r = VirtualAllocEx(
        GetCurrentProcess(),      /* Allocate in current process address space */
        NULL,                     /* Unknown position */
        size_bytes,               /* Bytes to allocate */
        MEM_COMMIT | MEM_RESERVE, /* Reserve and commit allocated page */
        PAGE_READWRITE            /* Permissions ( Read/Write )*/
    );
    if (INV_HANDLE(r))
        ARENA_ASSERT(0 && "VirtualAllocEx() failed.");

    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    if (INV_HANDLE(r))
        return;

    BOOL free_result = VirtualFreeEx(
        GetCurrentProcess(),        /* Deallocate from current process address space */
        (LPVOID)r,                  /* Address to deallocate */
        0,                          /* Bytes to deallocate ( Unknown, deallocate entire page ) */
        MEM_RELEASE                 /* Release the page ( And implicitly decommit it ) */
    );

    if (FALSE == free_result)
        ARENA_ASSERT(0 && "VirtualFreeEx() failed.");
}

#elif ARENA_BACKEND == ARENA_BACKEND_WASM_HEAPBASE

// Stolen from https://surma.dev/things/c-to-webassembly/

extern unsigned char __heap_base;
// Since ARENA_BACKEND_WASM_HEAPBASE entirely hijacks __heap_base it is expected that no other means of memory
// allocation are used except the arenas.
unsigned char* bump_pointer = &__heap_base;
// TODO: provide a way to deallocate all the arenas at once by setting bump_pointer back to &__heap_base?

// __builtin_wasm_memory_size and __builtin_wasm_memory_grow are defined in units of page sizes
#define ARENA_WASM_PAGE_SIZE (64*1024)

Region *new_region(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t)*capacity;
    Region *r = (void*)bump_pointer;

    // grow memory brk() style
    size_t current_memory_size = ARENA_WASM_PAGE_SIZE * __builtin_wasm_memory_size(0);
    size_t desired_memory_size = (size_t) bump_pointer + size_bytes;
    if (desired_memory_size > current_memory_size) {
        size_t delta_bytes = desired_memory_size - current_memory_size;
        size_t delta_pages = (delta_bytes + (ARENA_WASM_PAGE_SIZE - 1))/ARENA_WASM_PAGE_SIZE;
        if (__builtin_wasm_memory_grow(0, delta_pages) < 0) {
            ARENA_ASSERT(0 && "memory.grow failed");
            return NULL;
        }
    }

    bump_pointer += size_bytes;

    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    // Since ARENA_BACKEND_WASM_HEAPBASE uses a primitive bump allocator to
    // allocate the regions, free_region() does nothing. It is generally
    // not recommended to free arenas anyway since it is better to keep
    // reusing already allocated memory with arena_reset().
    (void) r;
}

#else
#  error "Unknown Arena backend"
#endif

// TODO: add debug statistic collection mode for arena
// Should collect things like:
// - How many times new_region was called
// - How many times existing region was skipped
// - How many times allocation exceeded ARENA_REGION_DEFAULT_CAPACITY

void *arena_alloc(Arena *a, size_t size_bytes)
{
    size_t size = (size_bytes + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);

    if (a->end == NULL) {
        ARENA_ASSERT(a->begin == NULL);
        size_t capacity = ARENA_REGION_DEFAULT_CAPACITY;
        if (capacity < size) capacity = size;
        a->end = new_region(capacity);
        a->begin = a->end;
    }

    while (a->end->count + size > a->end->capacity && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->count + size > a->end->capacity) {
        ARENA_ASSERT(a->end->next == NULL);
        size_t capacity = ARENA_REGION_DEFAULT_CAPACITY;
        if (capacity < size) capacity = size;
        a->end->next = new_region(capacity);
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->count];
    a->end->count += size;
    return result;
}

void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz)
{
    if (newsz <= oldsz) return oldptr;
    void *newptr = arena_alloc(a, newsz);
    char *newptr_char = (char*)newptr;
    char *oldptr_char = (char*)oldptr;
    for (size_t i = 0; i < oldsz; ++i) {
        newptr_char[i] = oldptr_char[i];
    }
    return newptr;
}

size_t arena_strlen(const char *s)
{
    size_t n = 0;
    while (*s++) n++;
    return n;
}

void *arena_memcpy(void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;
    for (; n; n--) *d++ = *s++;
    return dest;
}

char *arena_strdup(Arena *a, const char *cstr)
{
    size_t n = arena_strlen(cstr);
    char *dup = (char*)arena_alloc(a, n + 1);
    arena_memcpy(dup, cstr, n);
    dup[n] = '\0';
    return dup;
}

void *arena_memdup(Arena *a, void *data, size_t size)
{
    return arena_memcpy(arena_alloc(a, size), data, size);
}

#ifndef ARENA_NOSTDIO
char *arena_vsprintf(Arena *a, const char *format, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);
    int n = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    ARENA_ASSERT(n >= 0);
    char *result = (char*)arena_alloc(a, n + 1);
    vsnprintf(result, n + 1, format, args);

    return result;
}

char *arena_sprintf(Arena *a, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char *result = arena_vsprintf(a, format, args);
    va_end(args);

    return result;
}
#endif // ARENA_NOSTDIO

Arena_Mark arena_snapshot(Arena *a)
{
    Arena_Mark m;
    if(a->end == NULL){ //snapshot of uninitialized arena
        ARENA_ASSERT(a->begin == NULL);
        m.region = a->end;
        m.count  = 0;
    }else{
        m.region = a->end;
        m.count  = a->end->count;
    }

    return m;
}

void arena_reset(Arena *a)
{
    for (Region *r = a->begin; r != NULL; r = r->next) {
        r->count = 0;
    }

    a->end = a->begin;
}

void arena_rewind(Arena *a, Arena_Mark m)
{
    if(m.region == NULL){ //snapshot of uninitialized arena
        arena_reset(a);   //leave allocation
        return;
    }

    m.region->count = m.count;
    for (Region *r = m.region->next; r != NULL; r = r->next) {
        r->count = 0;
    }

    a->end = m.region;
}

void arena_free(Arena *a)
{
    Region *r = a->begin;
    while (r) {
        Region *r0 = r;
        r = r->next;
        free_region(r0);
    }
    a->begin = NULL;
    a->end = NULL;
}

void arena_trim(Arena *a){
    Region *r = a->end->next;
    while (r) {
        Region *r0 = r;
        r = r->next;
        free_region(r0);
    }
    a->end->next = NULL;
}

#endif // LIB_IMPLEMENTATION
