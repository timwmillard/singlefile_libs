#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

// ─── vec2 (uint8_t) ───────────────────────────────────────────────────────────

static void test_vec2(void) {
    vec2 a = {3, 4};
    vec2 b = {1, 2};

    vec2 r = vec2_add(a, b);
    ASSERT(r.x == 4 && r.y == 6);

    r = vec2_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2);

    r = vec2_scale(a, 2);
    ASSERT(r.x == 6 && r.y == 8);

    ASSERT(vec2_dot(a, b) == 3*1 + 4*2);

    ASSERT_FEQ(vec2_len(a), 5.0f);
}

// ─── vec3 (uint8_t) ───────────────────────────────────────────────────────────

static void test_vec3(void) {
    vec3 a = {1, 2, 3};
    vec3 b = {4, 5, 6};

    vec3 r = vec3_add(a, b);
    ASSERT(r.x == 5 && r.y == 7 && r.z == 9);

    r = vec3_sub(b, a);
    ASSERT(r.x == 3 && r.y == 3 && r.z == 3);

    r = vec3_scale(a, 3);
    ASSERT(r.x == 3 && r.y == 6 && r.z == 9);

    ASSERT(vec3_dot(a, b) == 1*4 + 2*5 + 3*6);

    ASSERT_FEQ(vec3_len((vec3){0, 3, 4}), 5.0f);

    vec3 cx = vec3_cross((vec3){1,0,0}, (vec3){0,1,0});
    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);
}

// ─── vec4 (uint8_t) ───────────────────────────────────────────────────────────

static void test_vec4(void) {
    vec4 a = {1, 2, 3, 4};
    vec4 b = {5, 6, 7, 8};

    vec4 r = vec4_add(a, b);
    ASSERT(r.x == 6 && r.y == 8 && r.z == 10 && r.w == 12);

    r = vec4_sub(b, a);
    ASSERT(r.x == 4 && r.y == 4 && r.z == 4 && r.w == 4);

    r = vec4_scale(a, 2);
    ASSERT(r.x == 2 && r.y == 4 && r.z == 6 && r.w == 8);

    ASSERT(vec4_dot(a, b) == 1*5 + 2*6 + 3*7 + 4*8);

    ASSERT_FEQ(vec4_len((vec4){0, 0, 3, 4}), 5.0f);
}

// ─── vec2i (int32_t) ──────────────────────────────────────────────────────────

static void test_vec2i(void) {
    vec2i a = {-3, 4};
    vec2i b = {1, -2};

    vec2i r = vec2i_add(a, b);
    ASSERT(r.x == -2 && r.y == 2);

    r = vec2i_sub(a, b);
    ASSERT(r.x == -4 && r.y == 6);

    r = vec2i_scale(a, -1);
    ASSERT(r.x == 3 && r.y == -4);

    ASSERT(vec2i_dot(a, b) == (-3)*1 + 4*(-2));

    ASSERT_FEQ(vec2i_len((vec2i){3, 4}), 5.0f);
}

// ─── vec3i (int32_t) ──────────────────────────────────────────────────────────

static void test_vec3i(void) {
    vec3i a = {1, -2, 3};
    vec3i b = {4, 5, -6};

    vec3i r = vec3i_add(a, b);
    ASSERT(r.x == 5 && r.y == 3 && r.z == -3);

    r = vec3i_sub(a, b);
    ASSERT(r.x == -3 && r.y == -7 && r.z == 9);

    r = vec3i_scale(a, 2);
    ASSERT(r.x == 2 && r.y == -4 && r.z == 6);

    ASSERT(vec3i_dot(a, b) == 1*4 + (-2)*5 + 3*(-6));

    ASSERT_FEQ(vec3i_len((vec3i){0, 3, 4}), 5.0f);

    vec3i cx = vec3i_cross((vec3i){1,0,0}, (vec3i){0,1,0});
    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);
}

// ─── vec4i (int32_t) ──────────────────────────────────────────────────────────

static void test_vec4i(void) {
    vec4i a = {1, -2, 3, -4};
    vec4i b = {-1, 2, -3, 4};

    vec4i r = vec4i_add(a, b);
    ASSERT(r.x == 0 && r.y == 0 && r.z == 0 && r.w == 0);

    r = vec4i_sub(a, b);
    ASSERT(r.x == 2 && r.y == -4 && r.z == 6 && r.w == -8);

    r = vec4i_scale(a, 3);
    ASSERT(r.x == 3 && r.y == -6 && r.z == 9 && r.w == -12);

    ASSERT(vec4i_dot(a, b) == 1*(-1) + (-2)*2 + 3*(-3) + (-4)*4);

    ASSERT_FEQ(vec4i_len((vec4i){0, 0, 3, 4}), 5.0f);
}

// ─── vec2u (uint32_t) ─────────────────────────────────────────────────────────

static void test_vec2u(void) {
    vec2u a = {3, 4};
    vec2u b = {1, 2};

    vec2u r = vec2u_add(a, b);
    ASSERT(r.x == 4 && r.y == 6);

    r = vec2u_sub(a, b);
    ASSERT(r.x == 2 && r.y == 2);

    r = vec2u_scale(a, 3);
    ASSERT(r.x == 9 && r.y == 12);

    ASSERT(vec2u_dot(a, b) == 3*1 + 4*2);

    ASSERT_FEQ(vec2u_len(a), 5.0f);
}

// ─── vec3u (uint32_t) ─────────────────────────────────────────────────────────

static void test_vec3u(void) {
    vec3u a = {1, 2, 3};
    vec3u b = {4, 5, 6};

    vec3u r = vec3u_add(a, b);
    ASSERT(r.x == 5 && r.y == 7 && r.z == 9);

    r = vec3u_sub(b, a);
    ASSERT(r.x == 3 && r.y == 3 && r.z == 3);

    r = vec3u_scale(a, 4);
    ASSERT(r.x == 4 && r.y == 8 && r.z == 12);

    ASSERT(vec3u_dot(a, b) == 1*4 + 2*5 + 3*6);

    ASSERT_FEQ(vec3u_len((vec3u){0, 3, 4}), 5.0f);

    vec3u cx = vec3u_cross((vec3u){1,0,0}, (vec3u){0,1,0});
    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);
}

// ─── vec4u (uint32_t) ─────────────────────────────────────────────────────────

static void test_vec4u(void) {
    vec4u a = {1, 2, 3, 4};
    vec4u b = {5, 6, 7, 8};

    vec4u r = vec4u_add(a, b);
    ASSERT(r.x == 6 && r.y == 8 && r.z == 10 && r.w == 12);

    r = vec4u_sub(b, a);
    ASSERT(r.x == 4 && r.y == 4 && r.z == 4 && r.w == 4);

    r = vec4u_scale(a, 2);
    ASSERT(r.x == 2 && r.y == 4 && r.z == 6 && r.w == 8);

    ASSERT(vec4u_dot(a, b) == 1*5 + 2*6 + 3*7 + 4*8);

    ASSERT_FEQ(vec4u_len((vec4u){0, 0, 3, 4}), 5.0f);
}

// ─── vec2f ────────────────────────────────────────────────────────────────────

static void test_vec2f(void) {
    vec2f a = {3.0f, 4.0f};
    vec2f b = {1.0f, 2.0f};

    vec2f r = vec2f_add(a, b);
    ASSERT_FEQ(r.x, 4.0f); ASSERT_FEQ(r.y, 6.0f);

    r = vec2f_sub(a, b);
    ASSERT_FEQ(r.x, 2.0f); ASSERT_FEQ(r.y, 2.0f);

    r = vec2f_scale(a, 2.0f);
    ASSERT_FEQ(r.x, 6.0f); ASSERT_FEQ(r.y, 8.0f);

    ASSERT_FEQ(vec2f_dot(a, b), 3.0f*1.0f + 4.0f*2.0f);

    ASSERT_FEQ(vec2f_len(a), 5.0f);

    vec2f n = vec2f_norm(a);
    ASSERT_FEQ(vec2f_len(n), 1.0f);
    ASSERT_FEQ(n.x, 3.0f/5.0f);
    ASSERT_FEQ(n.y, 4.0f/5.0f);
}

// ─── vec3f ────────────────────────────────────────────────────────────────────

static void test_vec3f(void) {
    vec3f a = {1.0f, 2.0f, 3.0f};
    vec3f b = {4.0f, 5.0f, 6.0f};

    vec3f r = vec3f_add(a, b);
    ASSERT_FEQ(r.x, 5.0f); ASSERT_FEQ(r.y, 7.0f); ASSERT_FEQ(r.z, 9.0f);

    r = vec3f_sub(b, a);
    ASSERT_FEQ(r.x, 3.0f); ASSERT_FEQ(r.y, 3.0f); ASSERT_FEQ(r.z, 3.0f);

    r = vec3f_scale(a, 2.0f);
    ASSERT_FEQ(r.x, 2.0f); ASSERT_FEQ(r.y, 4.0f); ASSERT_FEQ(r.z, 6.0f);

    ASSERT_FEQ(vec3f_dot(a, b), 1*4 + 2*5 + 3*6);

    ASSERT_FEQ(vec3f_len((vec3f){0, 3.0f, 4.0f}), 5.0f);

    vec3f n = vec3f_norm((vec3f){0.0f, 3.0f, 4.0f});
    ASSERT_FEQ(vec3f_len(n), 1.0f);
    ASSERT_FEQ(n.y, 3.0f/5.0f);
    ASSERT_FEQ(n.z, 4.0f/5.0f);

    // cross: (1,0,0) x (0,1,0) = (0,0,1)
    vec3f cx = vec3f_cross((vec3f){1,0,0}, (vec3f){0,1,0});
    ASSERT_FEQ(cx.x, 0.0f); ASSERT_FEQ(cx.y, 0.0f); ASSERT_FEQ(cx.z, 1.0f);

    // cross: a x a = (0,0,0)
    cx = vec3f_cross(a, a);
    ASSERT_FEQ(cx.x, 0.0f); ASSERT_FEQ(cx.y, 0.0f); ASSERT_FEQ(cx.z, 0.0f);
}

// ─── vec4f ────────────────────────────────────────────────────────────────────

static void test_vec4f(void) {
    vec4f a = {1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b = {5.0f, 6.0f, 7.0f, 8.0f};

    vec4f r = vec4f_add(a, b);
    ASSERT_FEQ(r.x, 6.0f); ASSERT_FEQ(r.y, 8.0f);
    ASSERT_FEQ(r.z, 10.0f); ASSERT_FEQ(r.w, 12.0f);

    r = vec4f_sub(b, a);
    ASSERT_FEQ(r.x, 4.0f); ASSERT_FEQ(r.y, 4.0f);
    ASSERT_FEQ(r.z, 4.0f); ASSERT_FEQ(r.w, 4.0f);

    r = vec4f_scale(a, 2.0f);
    ASSERT_FEQ(r.x, 2.0f); ASSERT_FEQ(r.y, 4.0f);
    ASSERT_FEQ(r.z, 6.0f); ASSERT_FEQ(r.w, 8.0f);

    ASSERT_FEQ(vec4f_dot(a, b), 1*5 + 2*6 + 3*7 + 4*8);

    ASSERT_FEQ(vec4f_len((vec4f){0, 0, 3.0f, 4.0f}), 5.0f);

    vec4f n = vec4f_norm((vec4f){0, 0, 3.0f, 4.0f});
    ASSERT_FEQ(vec4f_len(n), 1.0f);
    ASSERT_FEQ(n.z, 3.0f/5.0f);
    ASSERT_FEQ(n.w, 4.0f/5.0f);
}

// ─── mat2f ────────────────────────────────────────────────────────────────────

static void test_mat2f(void) {
    mat2f I = mat2f_identity();
    ASSERT_FEQ(I.m[0][0], 1.0f); ASSERT_FEQ(I.m[0][1], 0.0f);
    ASSERT_FEQ(I.m[1][0], 0.0f); ASSERT_FEQ(I.m[1][1], 1.0f);

    mat2f a = {.v = {1, 2, 3, 4}};
    mat2f b = {.v = {5, 6, 7, 8}};

    mat2f r = mat2f_add(a, b);
    ASSERT_FEQ(r.v[0], 6.0f); ASSERT_FEQ(r.v[1], 8.0f);
    ASSERT_FEQ(r.v[2], 10.0f); ASSERT_FEQ(r.v[3], 12.0f);

    r = mat2f_sub(b, a);
    ASSERT_FEQ(r.v[0], 4.0f); ASSERT_FEQ(r.v[1], 4.0f);
    ASSERT_FEQ(r.v[2], 4.0f); ASSERT_FEQ(r.v[3], 4.0f);

    r = mat2f_scale(a, 2.0f);
    ASSERT_FEQ(r.v[0], 2.0f); ASSERT_FEQ(r.v[1], 4.0f);
    ASSERT_FEQ(r.v[2], 6.0f); ASSERT_FEQ(r.v[3], 8.0f);

    // identity * a = a
    r = mat2f_mul(I, a);
    for (int i = 0; i < 4; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    // known multiply: [[1,2],[3,4]] * [[5,6],[7,8]] = [[19,22],[43,50]]
    r = mat2f_mul(a, b);
    ASSERT_FEQ(r.m[0][0], 19.0f); ASSERT_FEQ(r.m[0][1], 22.0f);
    ASSERT_FEQ(r.m[1][0], 43.0f); ASSERT_FEQ(r.m[1][1], 50.0f);

    // transpose
    r = mat2f_transpose(a);
    ASSERT_FEQ(r.m[0][0], a.m[0][0]); ASSERT_FEQ(r.m[0][1], a.m[1][0]);
    ASSERT_FEQ(r.m[1][0], a.m[0][1]); ASSERT_FEQ(r.m[1][1], a.m[1][1]);

    // identity * vec = vec
    vec2f v = {3.0f, 4.0f};
    vec2f rv = mat2f_mul_vec(I, v);
    ASSERT_FEQ(rv.x, 3.0f); ASSERT_FEQ(rv.y, 4.0f);

    // [[1,0],[0,2]] * (3,4) = (3,8)
    mat2f s = {.m = {{1,0},{0,2}}};
    rv = mat2f_mul_vec(s, v);
    ASSERT_FEQ(rv.x, 3.0f); ASSERT_FEQ(rv.y, 8.0f);
}

// ─── mat3f ────────────────────────────────────────────────────────────────────

static void test_mat3f(void) {
    mat3f I = mat3f_identity();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            ASSERT_FEQ(I.m[i][j], i == j ? 1.0f : 0.0f);

    mat3f a = {.v = {1,2,3, 4,5,6, 7,8,9}};
    mat3f b = {.v = {9,8,7, 6,5,4, 3,2,1}};

    mat3f r = mat3f_add(a, b);
    for (int i = 0; i < 9; i++) ASSERT_FEQ(r.v[i], 10.0f);

    r = mat3f_sub(a, b);
    ASSERT_FEQ(r.v[0], -8.0f); ASSERT_FEQ(r.v[8], 8.0f);

    r = mat3f_scale(a, 2.0f);
    for (int i = 0; i < 9; i++) ASSERT_FEQ(r.v[i], a.v[i] * 2.0f);

    // identity * a = a
    r = mat3f_mul(I, a);
    for (int i = 0; i < 9; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    // transpose
    r = mat3f_transpose(a);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            ASSERT_FEQ(r.m[i][j], a.m[j][i]);

    // identity * vec = vec
    vec3f v = {1.0f, 2.0f, 3.0f};
    vec3f rv = mat3f_mul_vec(I, v);
    ASSERT_FEQ(rv.x, 1.0f); ASSERT_FEQ(rv.y, 2.0f); ASSERT_FEQ(rv.z, 3.0f);

    // diagonal scale matrix
    mat3f ds = {.m = {{2,0,0},{0,3,0},{0,0,4}}};
    rv = mat3f_mul_vec(ds, v);
    ASSERT_FEQ(rv.x, 2.0f); ASSERT_FEQ(rv.y, 6.0f); ASSERT_FEQ(rv.z, 12.0f);
}

// ─── mat4f ────────────────────────────────────────────────────────────────────

static void test_mat4f(void) {
    mat4f I = mat4f_identity();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ASSERT_FEQ(I.m[i][j], i == j ? 1.0f : 0.0f);

    mat4f a = {.v = {1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16}};

    mat4f r = mat4f_scale(a, 0.0f);
    for (int i = 0; i < 16; i++) ASSERT_FEQ(r.v[i], 0.0f);

    // identity * a = a
    r = mat4f_mul(I, a);
    for (int i = 0; i < 16; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    // transpose
    r = mat4f_transpose(a);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ASSERT_FEQ(r.m[i][j], a.m[j][i]);

    // add then sub roundtrip
    mat4f b = mat4f_scale(I, 2.0f);
    r = mat4f_sub(mat4f_add(a, b), b);
    for (int i = 0; i < 16; i++) ASSERT_FEQ(r.v[i], a.v[i]);

    // identity * vec = vec
    vec4f v = {1.0f, 2.0f, 3.0f, 4.0f};
    vec4f rv = mat4f_mul_vec(I, v);
    ASSERT_FEQ(rv.x, 1.0f); ASSERT_FEQ(rv.y, 2.0f);
    ASSERT_FEQ(rv.z, 3.0f); ASSERT_FEQ(rv.w, 4.0f);

    // diagonal scale matrix
    mat4f ds = {.m = {{2,0,0,0},{0,3,0,0},{0,0,4,0},{0,0,0,5}}};
    rv = mat4f_mul_vec(ds, v);
    ASSERT_FEQ(rv.x, 2.0f); ASSERT_FEQ(rv.y, 6.0f);
    ASSERT_FEQ(rv.z, 12.0f); ASSERT_FEQ(rv.w, 20.0f);
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main(void) {
    printf(COLOR_BOLD "\nlinalg.h tests\n" COLOR_RESET "\n");

    test_vec2();  PASS("vec2");
    test_vec3();  PASS("vec3");
    test_vec4();  PASS("vec4");
    test_vec2i(); PASS("vec2i");
    test_vec3i(); PASS("vec3i");
    test_vec4i(); PASS("vec4i");
    test_vec2u(); PASS("vec2u");
    test_vec3u(); PASS("vec3u");
    test_vec4u(); PASS("vec4u");
    test_vec2f(); PASS("vec2f");
    test_vec3f(); PASS("vec3f");
    test_vec4f(); PASS("vec4f");
    test_mat2f(); PASS("mat2f");
    test_mat3f(); PASS("mat3f");
    test_mat4f(); PASS("mat4f");

    if (_failed == 0)
        printf("\n" COLOR_BOLD COLOR_GREEN "All tests passed." COLOR_RESET "\n\n");
    else
        printf("\n" COLOR_BOLD COLOR_RED "%d test(s) failed." COLOR_RESET "\n\n", _failed);
    return _failed != 0;
}
