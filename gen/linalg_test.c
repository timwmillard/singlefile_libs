/* generated — do not edit; run: ./gen_linalg --test > linalg_test.c */

#include <math.h>
#include <stdio.h>
#include "linalg.h"

#define COLOR_GREEN  "\033[32m"
#define COLOR_RED    "\033[31m"
#define COLOR_BOLD   "\033[1m"
#define COLOR_RESET  "\033[0m"

#define PASS(name) printf(COLOR_GREEN "  PASS" COLOR_RESET "  %s\n", name)

static int _failed = 0;

#define ASSERT(expr) do { \
    if (!(expr)) { \
        printf(COLOR_RED "  FAIL" COLOR_RESET "  %s:%d  " #expr "\n", __FILE__, __LINE__); \
        _failed++; \
    } \
} while(0)

#define EPS 1e-5f
#define ASSERT_FEQ(a, b) ASSERT(fabsf((a) - (b)) < EPS)

// ─── bvec2 (uint8_t) ─────────────────────────────────────────────────────────────

static void test_bvec2(void) {
    bvec2 a = {3, 4};
    bvec2 b = {1, 2};

    bvec2 r = bvec2_add(a, b);
    ASSERT(r.x == 4 && r.y == 6);

    r = bvec2_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2);

    r = bvec2_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8);

    ASSERT(bvec2_dot(a, b) == 11);

    ASSERT_FEQ(bvec2_len((bvec2){3, 4}), 5.0f);
}

// ─── bvec3 (uint8_t) ─────────────────────────────────────────────────────────────

static void test_bvec3(void) {
    bvec3 a = {3, 4, 5};
    bvec3 b = {1, 2, 3};

    bvec3 r = bvec3_add(a, b);
    ASSERT(r.x == 4 && r.y == 6 && r.z == 8);

    r = bvec3_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2 && r.z == 2);

    r = bvec3_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8 && r.z == 10);

    ASSERT(bvec3_dot(a, b) == 26);

    ASSERT_FEQ(bvec3_len((bvec3){0, 3, 4}), 5.0f);

    bvec3 cx = bvec3_cross((bvec3){1,0,0}, (bvec3){0,1,0});
    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);
}

// ─── bvec4 (uint8_t) ─────────────────────────────────────────────────────────────

static void test_bvec4(void) {
    bvec4 a = {3, 4, 5, 6};
    bvec4 b = {1, 2, 3, 4};

    bvec4 r = bvec4_add(a, b);
    ASSERT(r.x == 4 && r.y == 6 && r.z == 8 && r.w == 10);

    r = bvec4_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2 && r.z == 2 && r.w == 2);

    r = bvec4_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8 && r.z == 10 && r.w == 12);

    ASSERT(bvec4_dot(a, b) == 50);

    ASSERT_FEQ(bvec4_len((bvec4){0, 0, 3, 4}), 5.0f);
}

// ─── ivec2 (int32_t) ─────────────────────────────────────────────────────────────

static void test_ivec2(void) {
    ivec2 a = {-3, 4};
    ivec2 b = {1, -2};

    ivec2 r = ivec2_add(a, b);
    ASSERT(r.x == -2 && r.y == 2);

    r = ivec2_sub(a, b);
    ASSERT(r.x == -4 && r.y == 6);

    r = ivec2_scale(a, 2);
    ASSERT(r.x == -6 && r.y == 8);

    ASSERT(ivec2_dot(a, b) == -11);

    ASSERT_FEQ(ivec2_len((ivec2){3, 4}), 5.0f);
}

// ─── ivec3 (int32_t) ─────────────────────────────────────────────────────────────

static void test_ivec3(void) {
    ivec3 a = {-3, 4, -5};
    ivec3 b = {1, -2, 3};

    ivec3 r = ivec3_add(a, b);
    ASSERT(r.x == -2 && r.y == 2 && r.z == -2);

    r = ivec3_sub(a, b);
    ASSERT(r.x == -4 && r.y == 6 && r.z == -8);

    r = ivec3_scale(a, 2);
    ASSERT(r.x == -6 && r.y == 8 && r.z == -10);

    ASSERT(ivec3_dot(a, b) == -26);

    ASSERT_FEQ(ivec3_len((ivec3){0, 3, 4}), 5.0f);

    ivec3 cx = ivec3_cross((ivec3){1,0,0}, (ivec3){0,1,0});
    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);
}

// ─── ivec4 (int32_t) ─────────────────────────────────────────────────────────────

static void test_ivec4(void) {
    ivec4 a = {-3, 4, -5, 6};
    ivec4 b = {1, -2, 3, -4};

    ivec4 r = ivec4_add(a, b);
    ASSERT(r.x == -2 && r.y == 2 && r.z == -2 && r.w == 2);

    r = ivec4_sub(a, b);
    ASSERT(r.x == -4 && r.y == 6 && r.z == -8 && r.w == 10);

    r = ivec4_scale(a, 2);
    ASSERT(r.x == -6 && r.y == 8 && r.z == -10 && r.w == 12);

    ASSERT(ivec4_dot(a, b) == -50);

    ASSERT_FEQ(ivec4_len((ivec4){0, 0, 3, 4}), 5.0f);
}

// ─── uvec2 (uint32_t) ─────────────────────────────────────────────────────────────

static void test_uvec2(void) {
    uvec2 a = {3, 4};
    uvec2 b = {1, 2};

    uvec2 r = uvec2_add(a, b);
    ASSERT(r.x == 4 && r.y == 6);

    r = uvec2_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2);

    r = uvec2_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8);

    ASSERT(uvec2_dot(a, b) == 11);

    ASSERT_FEQ(uvec2_len((uvec2){3, 4}), 5.0f);
}

// ─── uvec3 (uint32_t) ─────────────────────────────────────────────────────────────

static void test_uvec3(void) {
    uvec3 a = {3, 4, 5};
    uvec3 b = {1, 2, 3};

    uvec3 r = uvec3_add(a, b);
    ASSERT(r.x == 4 && r.y == 6 && r.z == 8);

    r = uvec3_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2 && r.z == 2);

    r = uvec3_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8 && r.z == 10);

    ASSERT(uvec3_dot(a, b) == 26);

    ASSERT_FEQ(uvec3_len((uvec3){0, 3, 4}), 5.0f);

    uvec3 cx = uvec3_cross((uvec3){1,0,0}, (uvec3){0,1,0});
    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);
}

// ─── uvec4 (uint32_t) ─────────────────────────────────────────────────────────────

static void test_uvec4(void) {
    uvec4 a = {3, 4, 5, 6};
    uvec4 b = {1, 2, 3, 4};

    uvec4 r = uvec4_add(a, b);
    ASSERT(r.x == 4 && r.y == 6 && r.z == 8 && r.w == 10);

    r = uvec4_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2 && r.z == 2 && r.w == 2);

    r = uvec4_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8 && r.z == 10 && r.w == 12);

    ASSERT(uvec4_dot(a, b) == 50);

    ASSERT_FEQ(uvec4_len((uvec4){0, 0, 3, 4}), 5.0f);
}

// ─── vec2 (float) ─────────────────────────────────────────────────────────────

static void test_vec2(void) {
    vec2 a = {3.0f, 4.0f};
    vec2 b = {1.0f, 2.0f};

    vec2 r = vec2_add(a, b);
    ASSERT_FEQ(r.x, 4.0f);
    ASSERT_FEQ(r.y, 6.0f);

    r = vec2_sub(a, b);
    ASSERT_FEQ(r.x, 2.0f);
    ASSERT_FEQ(r.y, 2.0f);

    r = vec2_scale(a, 2.0f);
    ASSERT_FEQ(r.x, 6.0f);
    ASSERT_FEQ(r.y, 8.0f);

    ASSERT_FEQ(vec2_dot(a, b), 11.0f);

    ASSERT_FEQ(vec2_len((vec2){3.0f, 4.0f}), 5.0f);

    vec2 nv = vec2_norm((vec2){3.0f, 4.0f});
    ASSERT_FEQ(vec2_len(nv), 1.0f);
    ASSERT_FEQ(nv.x, 3.0f / 5.0f);
    ASSERT_FEQ(nv.y, 4.0f / 5.0f);
}

// ─── vec3 (float) ─────────────────────────────────────────────────────────────

static void test_vec3(void) {
    vec3 a = {3.0f, 4.0f, 5.0f};
    vec3 b = {1.0f, 2.0f, 3.0f};

    vec3 r = vec3_add(a, b);
    ASSERT_FEQ(r.x, 4.0f);
    ASSERT_FEQ(r.y, 6.0f);
    ASSERT_FEQ(r.z, 8.0f);

    r = vec3_sub(a, b);
    ASSERT_FEQ(r.x, 2.0f);
    ASSERT_FEQ(r.y, 2.0f);
    ASSERT_FEQ(r.z, 2.0f);

    r = vec3_scale(a, 2.0f);
    ASSERT_FEQ(r.x, 6.0f);
    ASSERT_FEQ(r.y, 8.0f);
    ASSERT_FEQ(r.z, 10.0f);

    ASSERT_FEQ(vec3_dot(a, b), 26.0f);

    ASSERT_FEQ(vec3_len((vec3){0.0f, 3.0f, 4.0f}), 5.0f);

    vec3 nv = vec3_norm((vec3){0.0f, 3.0f, 4.0f});
    ASSERT_FEQ(vec3_len(nv), 1.0f);
    ASSERT_FEQ(nv.y, 3.0f / 5.0f);
    ASSERT_FEQ(nv.z, 4.0f / 5.0f);

    vec3 cx = vec3_cross((vec3){1.0f,0.0f,0.0f}, (vec3){0.0f,1.0f,0.0f});
    ASSERT_FEQ(cx.x, 0.0f); ASSERT_FEQ(cx.y, 0.0f); ASSERT_FEQ(cx.z, 1.0f);
}

// ─── vec4 (float) ─────────────────────────────────────────────────────────────

static void test_vec4(void) {
    vec4 a = {3.0f, 4.0f, 5.0f, 6.0f};
    vec4 b = {1.0f, 2.0f, 3.0f, 4.0f};

    vec4 r = vec4_add(a, b);
    ASSERT_FEQ(r.x, 4.0f);
    ASSERT_FEQ(r.y, 6.0f);
    ASSERT_FEQ(r.z, 8.0f);
    ASSERT_FEQ(r.w, 10.0f);

    r = vec4_sub(a, b);
    ASSERT_FEQ(r.x, 2.0f);
    ASSERT_FEQ(r.y, 2.0f);
    ASSERT_FEQ(r.z, 2.0f);
    ASSERT_FEQ(r.w, 2.0f);

    r = vec4_scale(a, 2.0f);
    ASSERT_FEQ(r.x, 6.0f);
    ASSERT_FEQ(r.y, 8.0f);
    ASSERT_FEQ(r.z, 10.0f);
    ASSERT_FEQ(r.w, 12.0f);

    ASSERT_FEQ(vec4_dot(a, b), 50.0f);

    ASSERT_FEQ(vec4_len((vec4){0.0f, 0.0f, 3.0f, 4.0f}), 5.0f);

    vec4 nv = vec4_norm((vec4){0.0f, 0.0f, 3.0f, 4.0f});
    ASSERT_FEQ(vec4_len(nv), 1.0f);
    ASSERT_FEQ(nv.z, 3.0f / 5.0f);
    ASSERT_FEQ(nv.w, 4.0f / 5.0f);
}

// ─── mat2 ─────────────────────────────────────────────────────────────────────

static void test_mat2(void) {
    mat2 I = mat2_identity();
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            ASSERT_FEQ(I.m[i][j], i == j ? 1.0f : 0.0f);

    mat2 a = {.v = {1,2,3,4}};

    mat2 r = mat2_scale(a, 0.0f);
    for (int i = 0; i < 4; i++) ASSERT_FEQ(r.v[i], 0.0f);

    mat2 b = mat2_scale(I, 2.0f);
    r = mat2_sub(mat2_add(a, b), b);
    for (int i = 0; i < 4; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    r = mat2_mul(I, a);
    for (int i = 0; i < 4; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    r = mat2_transpose(a);
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            ASSERT_FEQ(r.m[i][j], a.m[j][i]);

    vec2 v = {1.0f, 2.0f};
    vec2 rv = mat2_mul_vec(I, v);
    ASSERT_FEQ(rv.x, 1.0f);
    ASSERT_FEQ(rv.y, 2.0f);

    mat2 ds = {.m = {{2,0},{0,3}}};
    rv = mat2_mul_vec(ds, v);
    ASSERT_FEQ(rv.x, 2.0f);
    ASSERT_FEQ(rv.y, 6.0f);
}

// ─── mat3 ─────────────────────────────────────────────────────────────────────

static void test_mat3(void) {
    mat3 I = mat3_identity();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            ASSERT_FEQ(I.m[i][j], i == j ? 1.0f : 0.0f);

    mat3 a = {.v = {1,2,3,4,5,6,7,8,9}};

    mat3 r = mat3_scale(a, 0.0f);
    for (int i = 0; i < 9; i++) ASSERT_FEQ(r.v[i], 0.0f);

    mat3 b = mat3_scale(I, 2.0f);
    r = mat3_sub(mat3_add(a, b), b);
    for (int i = 0; i < 9; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    r = mat3_mul(I, a);
    for (int i = 0; i < 9; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    r = mat3_transpose(a);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            ASSERT_FEQ(r.m[i][j], a.m[j][i]);

    vec3 v = {1.0f, 2.0f, 3.0f};
    vec3 rv = mat3_mul_vec(I, v);
    ASSERT_FEQ(rv.x, 1.0f);
    ASSERT_FEQ(rv.y, 2.0f);
    ASSERT_FEQ(rv.z, 3.0f);

    mat3 ds = {.m = {{2,0,0},{0,3,0},{0,0,4}}};
    rv = mat3_mul_vec(ds, v);
    ASSERT_FEQ(rv.x, 2.0f);
    ASSERT_FEQ(rv.y, 6.0f);
    ASSERT_FEQ(rv.z, 12.0f);
}

// ─── mat4 ─────────────────────────────────────────────────────────────────────

static void test_mat4(void) {
    mat4 I = mat4_identity();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ASSERT_FEQ(I.m[i][j], i == j ? 1.0f : 0.0f);

    mat4 a = {.v = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}};

    mat4 r = mat4_scale(a, 0.0f);
    for (int i = 0; i < 16; i++) ASSERT_FEQ(r.v[i], 0.0f);

    mat4 b = mat4_scale(I, 2.0f);
    r = mat4_sub(mat4_add(a, b), b);
    for (int i = 0; i < 16; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    r = mat4_mul(I, a);
    for (int i = 0; i < 16; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    r = mat4_transpose(a);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ASSERT_FEQ(r.m[i][j], a.m[j][i]);

    vec4 v = {1.0f, 2.0f, 3.0f, 4.0f};
    vec4 rv = mat4_mul_vec(I, v);
    ASSERT_FEQ(rv.x, 1.0f);
    ASSERT_FEQ(rv.y, 2.0f);
    ASSERT_FEQ(rv.z, 3.0f);
    ASSERT_FEQ(rv.w, 4.0f);

    mat4 ds = {.m = {{2,0,0,0},{0,3,0,0},{0,0,4,0},{0,0,0,5}}};
    rv = mat4_mul_vec(ds, v);
    ASSERT_FEQ(rv.x, 2.0f);
    ASSERT_FEQ(rv.y, 6.0f);
    ASSERT_FEQ(rv.z, 12.0f);
    ASSERT_FEQ(rv.w, 20.0f);
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(void) {
    printf(COLOR_BOLD "\nlinalg.h tests\n" COLOR_RESET "\n");

    test_bvec2(); PASS("bvec2");
    test_bvec3(); PASS("bvec3");
    test_bvec4(); PASS("bvec4");
    test_ivec2(); PASS("ivec2");
    test_ivec3(); PASS("ivec3");
    test_ivec4(); PASS("ivec4");
    test_uvec2(); PASS("uvec2");
    test_uvec3(); PASS("uvec3");
    test_uvec4(); PASS("uvec4");
    test_vec2(); PASS("vec2");
    test_vec3(); PASS("vec3");
    test_vec4(); PASS("vec4");

    test_mat2(); PASS("mat2");
    test_mat3(); PASS("mat3");
    test_mat4(); PASS("mat4");

    if (_failed == 0)
        printf("\n" COLOR_BOLD COLOR_GREEN "All tests passed." COLOR_RESET "\n\n");
    else
        printf("\n" COLOR_BOLD COLOR_RED "%d test(s) failed." COLOR_RESET "\n\n", _failed);
    return _failed != 0;
}
