#include <stdio.h>
#include <string.h>

#define BASE_IMPL
#include "base.h"

#define COLOR_GREEN  "\033[32m"
#define COLOR_RED    "\033[31m"
#define COLOR_BOLD   "\033[1m"
#define COLOR_RESET  "\033[0m"

#define PASS(name) printf(COLOR_GREEN "  PASS" COLOR_RESET "  %s\n", name)

static int _failed = 0;

#define ASSERT(expr) do { \
    if (!(expr)) { \
        printf(COLOR_RED "  FAIL" COLOR_RESET "  %s:%d  %s\n", __FILE__, __LINE__, #expr); \
        _failed++; \
    } \
} while(0)

/* basic alloc returns non-null and is writable */
void test_alloc_basic() {
    arena a = {0};
    byte *mem = arena_alloc(&a, 1024);
    ASSERT(mem != NULL);
    memset(mem, 0xcb, 1024);
    ASSERT(mem[0] == (byte)0xcb && mem[1023] == (byte)0xcb);
    arena_release(&a);
    PASS("alloc basic");
}

/* multiple allocations don't overlap */
void test_alloc_no_overlap() {
    arena a = {0};
    byte *a1 = arena_alloc(&a, 64);
    byte *a2 = arena_alloc(&a, 64);
    byte *a3 = arena_alloc(&a, 64);
    memset(a1, 0x11, 64);
    memset(a2, 0x22, 64);
    memset(a3, 0x33, 64);
    ASSERT(a1[0] == (byte)0x11);
    ASSERT(a2[0] == (byte)0x22);
    ASSERT(a3[0] == (byte)0x33);
    arena_release(&a);
    PASS("alloc no overlap");
}

/* returned pointers are word-aligned */
void test_alloc_alignment() {
    arena a = {0};
    /* vary sizes to stress alignment of subsequent allocs */
    (void)arena_alloc(&a, 1);
    int    *p2 = arena_alloc(&a, sizeof(int));
    double *p3 = arena_alloc(&a, sizeof(double));
    (void)arena_alloc(&a, 3);
    int    *p4 = arena_alloc(&a, sizeof(int));
    ASSERT(((uptr)p2 % sizeof(uptr)) == 0);
    ASSERT(((uptr)p3 % sizeof(uptr)) == 0);
    ASSERT(((uptr)p4 % sizeof(uptr)) == 0);
    arena_release(&a);
    PASS("alloc alignment");
}

/* force region growth by exceeding the default region size */
void test_alloc_multi_region() {
    arena a = {0};
    int total = (ARENA_REGION_DEFAULT_SIZE_BYTES / 512) * 4;
    for (int i = 0; i < total; i++) {
        void *p = arena_alloc(&a, 512);
        ASSERT(p != NULL);
    }
    ASSERT(a.start != a.end);   /* must have grown beyond one region */
    arena_release(&a);
    PASS("alloc multi region");
}

/* single allocation larger than the default region size */
void test_alloc_large() {
    arena a = {0};
    usize big = ARENA_REGION_DEFAULT_SIZE_BYTES * 4;
    byte *p = arena_alloc(&a, big);
    ASSERT(p != NULL);
    memset(p, 0xAB, big);
    ASSERT(p[0] == (byte)0xAB && p[big - 1] == (byte)0xAB);
    arena_release(&a);
    PASS("alloc large");
}

/* realloc grows and copies existing data */
void test_realloc_grow() {
    arena a = {0};
    char *p = arena_alloc(&a, 16);
    memcpy(p, "hello world!!!!!", 16);
    char *p2 = arena_realloc(&a, p, 16, 32);
    ASSERT(p2 != NULL);
    ASSERT(memcmp(p2, "hello world!!!!!", 16) == 0);
    arena_release(&a);
    PASS("realloc grow");
}

/* realloc with newsz <= oldsz returns the same pointer unchanged */
void test_realloc_no_shrink() {
    arena a = {0};
    char *p = arena_alloc(&a, 64);
    char *p2 = arena_realloc(&a, p, 64, 32);
    ASSERT(p == p2);
    arena_release(&a);
    PASS("realloc no shrink");
}

/* release zeroes the arena; it can be reused immediately after */
void test_release_and_reuse() {
    arena a = {0};
    for (int i = 0; i < 50; i++) arena_alloc(&a, 256);
    arena_release(&a);
    ASSERT(a.start == NULL && a.end == NULL);
    void *p = arena_alloc(&a, 64);
    ASSERT(p != NULL);
    arena_release(&a);
    PASS("release and reuse");
}

/* reset keeps regions but resets len; memory can be reused without malloc */
void test_reset() {
    arena a = {0};
    for (int i = 0; i < 50; i++) arena_alloc(&a, 256);
    arena_region *first = a.start;   /* remember the region pointer */
    arena_reset(&a);
    ASSERT(a.start == first);        /* regions are still there */
    ASSERT(a.start->len == 0);       /* but len is reset */
    void *p = arena_alloc(&a, 64);
    ASSERT(p != NULL);
    arena_release(&a);
    PASS("reset");
}

int main(void) {
    printf(COLOR_BOLD "\narena tests\n" COLOR_RESET "\n");

    test_alloc_basic();
    test_alloc_no_overlap();
    test_alloc_alignment();
    test_alloc_multi_region();
    test_alloc_large();
    test_realloc_grow();
    test_realloc_no_shrink();
    test_release_and_reuse();
    test_reset();

    if (_failed == 0)
        printf("\n" COLOR_BOLD COLOR_GREEN "All tests passed." COLOR_RESET "\n\n");
    else
        printf("\n" COLOR_BOLD COLOR_RED "%d test(s) failed." COLOR_RESET "\n\n", _failed);
    return _failed != 0;
}
