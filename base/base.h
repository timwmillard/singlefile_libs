#ifndef BASE_H
#define BASE_H

#ifdef __cplusplus
extern "C" {
#endif

// #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#    define THREAD_LOCAL __thread
#elif defined(COMPILER_MSVC)
#    define THREAD_LOCAL __declspec(thread)
#elif (__STDC_VERSION__ >= 201112L)
#    define THREAD_LOCAL _Thread_local
#else
#    error "Invalid compiler/version for thead variable; Use Clang, GCC, or MSVC, or use C11 or greater"
#endif

typedef unsigned char byte;

/* standard types */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// unsigned integer type with width of exactly 8, 16, 32, 64 & 128 bits respectively.
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef __uint128_t uint128;

// signed integer type with width of exactly 8, 16, 32, 64 & 128 bits respectively.
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

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define ARENA_REGION_DEFAULT_SIZE_BYTES 4096


typedef struct arena_region {
    struct arena_region *next;
    usize len;
    usize cap;
    uptr data[];
} arena_region;

typedef struct arena {
    arena_region *start;
    arena_region *end;
} arena;

void *arena_alloc(arena *a, usize size_bytes);
void *arena_realloc(arena *a, void *oldptr, usize oldsz, usize newsz);
void arena_reset(arena *a);
void arena_release(arena *a);


typedef struct {
    char* data;
    usize len;
    usize cap;
    arena *arena;
} string;

string string_new(arena *a, usize capacity);
string string_from(arena *a, const char* str);
void string_append(string* s, const char* str);
string string_view(const char* str);
string cstringl(arena *a, const char* str, int len);
string cstring(arena *a, const char* str);
void string_push(string* s, char c);
string string_slice(string s, int start, int end);
bool string_starts_with(string s, const char* prefix);
bool string_ends_with(string s, const char* suffix);
int string_find(string s, const char* needle);
string string_trim(string s);
string string_upper(string s);
string string_lower(string s);
const char *cstr(string *s);

#ifdef BASE_IMPL

arena_region *_arena_new_region(usize size) {
    usize region_cap = ARENA_REGION_DEFAULT_SIZE_BYTES / sizeof(uintptr_t);
    if (region_cap < size) region_cap = size;
    usize size_bytes = sizeof(uptr)*region_cap;
    arena_region *r = (arena_region*)malloc(size_bytes);
    assert(r);
    r->next = NULL;
    r->len = 0;
    r->cap = region_cap - sizeof(arena_region) / sizeof(uptr);
    return r;
}

void _arena_free_region(arena_region *r) {
    free(r);
}

void *arena_alloc(arena *a, usize size_bytes) {
    usize size = (size_bytes + sizeof(uptr) - 1)/sizeof(uptr);

    if (a->end == NULL) {
        assert(a->start == NULL);
        a->end = _arena_new_region(size);
        a->start = a->end;
    }

    while (a->end->len + size > a->end->cap && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->len + size > a->end->cap) {
        assert(a->end->next == NULL);
        a->end->next = _arena_new_region(size);
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->len];
    a->end->len += size;
    return result;
}

void *arena_realloc(arena *a, void *oldptr, usize oldsz, usize newsz) {
    if (newsz <= oldsz) return oldptr;
    void *newptr = arena_alloc(a, newsz);
    char *newptr_char = (char*)newptr;
    char *oldptr_char = (char*)oldptr;
    for (usize i = 0; i < oldsz; ++i) {
        newptr_char[i] = oldptr_char[i];
    }
    return newptr;
}

void arena_reset(arena *a) {
    for (arena_region *r = a->start; r != NULL; r = r->next)
        r->len = 0;
    a->end = a->start;
}

void arena_release(arena *a) {
    arena_region *r = a->start;
    while (r) {
        arena_region *r0 = r;
        r = r->next;
        _arena_free_region(r0);
    }
    a->start = NULL;
    a->end = NULL;
}

string string_new(arena *a, usize capacity) {
    char* data = arena_alloc(a, capacity + 1);
    data[0] = '\0';
    return (string){
        .data = data,
        .len = 0,
        .cap = capacity,
        .arena = a,
    };
}

// Create owned string from C string
string string_from(arena *a, const char* str) {
    usize len = strlen(str);
    string s = string_new(a, len);
    strcpy(s.data, str);
    s.len = len;
    return s;
}

// Ensure capacity for owned strings
void _string_reserve(string* s, arena *a, usize new_cap) {
   if (new_cap > s->cap) {
        if (s->arena) {
            // Arena-allocated string: use arena_realloc
            s->data = arena_realloc(s->arena, s->data, s->cap + 1, new_cap + 1);
        } else {
            // Heap-allocated string: use realloc
            s->data = realloc(s->data, new_cap + 1);
        }
        s->cap = new_cap;
   }
}

// Append to string
void string_append(string* s, const char* str) {
    usize add_len = strlen(str);
    usize new_len = s->len + add_len;
    
    if (new_len > s->cap) {
        _string_reserve(s, s->arena, new_len * 2);  // double capacity
    }
    
    strcpy(s->data + s->len, str);
    s->len = new_len;
}

// Create string from literal (view - doesn't own memory)
string string_view(const char* str) {
    return (string){
        .data = (char*)str,
        .len = strlen(str),
    };
}

// Create owned string from C string with len
string cstringl(arena *a, const char* str, int len) {
    string s = string_new(a, len);
    strcpy(s.data, str);
    s.len = len;
    return s;
}

// Create owned string from C string
string cstring(arena *a, const char* str) {
    usize len = strlen(str);
    return cstringl(a, str, len);
}

// Append character
void string_push(string* s, char c) {
    if (s->len + 1 > s->cap) {
        _string_reserve(s, s->arena, (s->cap + 1) * 2);
    }
    s->data[s->len++] = c;
    s->data[s->len] = '\0';
}

// String manipulation functions
string string_slice(string s, int start, int end) {
    if (start < 0) start = s.len + start;
    if (end < 0) end = s.len + end;
    if (start < 0) start = 0;
    if (end > (int)s.len) end = s.len;
    if (start >= end) return cstring(s.arena, "");
    
    return (string){
        .data = s.data + start,
        .len = end - start,
    };
}

bool string_starts_with(string s, const char* prefix) {
    usize prefix_len = strlen(prefix);
    return s.len >= prefix_len && strncmp(s.data, prefix, prefix_len) == 0;
}

bool string_ends_with(string s, const char* suffix) {
    usize suffix_len = strlen(suffix);
    return s.len >= suffix_len && 
           strcmp(s.data + s.len - suffix_len, suffix) == 0;
}

int string_find(string s, const char* needle) {
    char* found = strstr(s.data, needle);
    return found ? (found - s.data) : -1;
}

string string_trim(string s) {
    char* start = s.data;
    char* end = s.data + s.len - 1;
    
    while (start <= end && isspace(*start)) start++;
    while (end > start && isspace(*end)) end--;
    
    return (string){
        .data = start,
        .len = end - start + 1,
        .cap = 0,
    };
}

string string_upper(string s) {
    string result = string_from(s.arena, s.data);
    for (usize i = 0; i < result.len; i++) {
        result.data[i] = toupper(result.data[i]);
    }
    return result;
}

string string_lower(string s) {
    string result = string_from(s.arena, s.data);
    for (usize i = 0; i < result.len; i++) {
        result.data[i] = tolower(result.data[i]);
    }
    return result;
}

// Returns the string as a null-terminated C string
const char *cstr(string *s) {
    // Make sure we have capacity for the null terminator
    if (s->len > s->cap) {
        _string_reserve(s, s->arena, s->len * 2);  // double capacity
    }
    
    // Add null terminator
    s->data[s->len] = '\0';
    
    return s->data;
}


// Split string into array
typedef struct {
    string* items;
    usize count;
    arena *arena;
} string_array;

#define string_array_arena_from(a, array) string_array_arena_from_count(a, array, ARRAY_SIZE(array))
string_array string_array_arena_from_count(arena *a, const char* array[], int count) {
    string_array sa;
    sa.items = arena_alloc(a, sizeof(*sa.items) * count);
    for (int i = 0; i < count; i++) {
        sa.items[i] = string_from(a, array[i]);
    }
    sa.count = count;
    return sa;
}

#define string_array_from(array) string_array_from_count(array, ARRAY_SIZE(array))
string_array string_array_from_count(const char* array[], int count) {
    return string_array_arena_from_count(NULL, array, count);
}

string_array string_array_arena_view(arena *a, char* array, int count) {
    string_array sa;
    sa.items = arena_alloc(a, sizeof(*sa.items) * count);
    for (int i = 0; i < count; i++) {
        sa.items[i] = string_view(&array[i]);
    }
    sa.count = count;
    return sa;
}

string_array string_array_view(char* array, int count) {
    return string_array_arena_view(NULL, array, count);
}

string_array string_split(string s, const char* delimiter) {
    // Count occurrences
    usize count = 1;
    char *tmp = s.data;
    while ((tmp = strstr(tmp, delimiter)) != NULL) {
        count++;
        tmp += strlen(delimiter);
    }
    
    string *items = malloc(sizeof(string) * count);
    usize idx = 0;
    
    char *start = s.data;
    char *end;
    
    while ((end = strstr(start, delimiter)) != NULL) {
        items[idx++] = (string){
            .data = start,
            .len = end - start,
            .cap = 0,
        };
        start = end + strlen(delimiter);
    }
    
    // Last piece
    items[idx++] = string_view(start);
    
    return (string_array){.items = items, .count = count};
}

void string_array_free(string_array* arr) {
    free(arr->items);
}

// Join strings
string string_join(string_array arr, const char* separator) {
    usize total_len = 0;
    usize sep_len = strlen(separator);
    
    for (usize i = 0; i < arr.count; i++) {
        total_len += arr.items[i].len;
        if (i > 0) total_len += sep_len;
    }
    
    string result = string_new(arr.arena, total_len);
    
    for (usize i = 0; i < arr.count; i++) {
        if (i > 0) string_append(&result, separator);
        string_append(&result, arr.items[i].data);
    }
    
    return result;
}
// ------------ STRING END

#endif // BASE_IMPL

#ifdef __cplusplus
}
#endif

#endif // BASE_H
