#ifndef LINALG_H
#define LINALG_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#  pragma GCC diagnostic ignored "-Wmissing-braces"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#  ifdef __clang__
#    pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#    pragma GCC diagnostic ignored "-Wnested-anon-types"
#  endif
#endif

// ── bvec2 ──

typedef struct {
    union {
        struct {
            uint8_t x;
            uint8_t y;
        };
        uint8_t v[2];
    };
} bvec2;

static inline bvec2 bvec2_add(bvec2 a, bvec2 b) {
    bvec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline bvec2 bvec2_sub(bvec2 a, bvec2 b) {
    bvec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline bvec2 bvec2_scale(bvec2 a, uint8_t s) {
    bvec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint8_t bvec2_dot(bvec2 a, bvec2 b) {
    uint8_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float bvec2_len(bvec2 a) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

// ── bvec3 ──

typedef struct {
    union {
        struct {
            uint8_t x;
            uint8_t y;
            uint8_t z;
        };
        uint8_t v[3];
    };
} bvec3;

static inline bvec3 bvec3_add(bvec3 a, bvec3 b) {
    bvec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline bvec3 bvec3_sub(bvec3 a, bvec3 b) {
    bvec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline bvec3 bvec3_scale(bvec3 a, uint8_t s) {
    bvec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint8_t bvec3_dot(bvec3 a, bvec3 b) {
    uint8_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float bvec3_len(bvec3 a) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

static inline bvec3 bvec3_cross(bvec3 a, bvec3 b) {
    return (bvec3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── bvec4 ──

typedef struct {
    union {
        struct {
            uint8_t x;
            uint8_t y;
            uint8_t z;
            uint8_t w;
        };
        uint8_t v[4];
    };
} bvec4;

static inline bvec4 bvec4_add(bvec4 a, bvec4 b) {
    bvec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline bvec4 bvec4_sub(bvec4 a, bvec4 b) {
    bvec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline bvec4 bvec4_scale(bvec4 a, uint8_t s) {
    bvec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint8_t bvec4_dot(bvec4 a, bvec4 b) {
    uint8_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float bvec4_len(bvec4 a) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

// ── ivec2 ──

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
        };
        int32_t v[2];
    };
} ivec2;

static inline ivec2 ivec2_add(ivec2 a, ivec2 b) {
    ivec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline ivec2 ivec2_sub(ivec2 a, ivec2 b) {
    ivec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline ivec2 ivec2_scale(ivec2 a, int32_t s) {
    ivec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline int32_t ivec2_dot(ivec2 a, ivec2 b) {
    int32_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float ivec2_len(ivec2 a) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

// ── ivec3 ──

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        int32_t v[3];
    };
} ivec3;

static inline ivec3 ivec3_add(ivec3 a, ivec3 b) {
    ivec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline ivec3 ivec3_sub(ivec3 a, ivec3 b) {
    ivec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline ivec3 ivec3_scale(ivec3 a, int32_t s) {
    ivec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline int32_t ivec3_dot(ivec3 a, ivec3 b) {
    int32_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float ivec3_len(ivec3 a) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

static inline ivec3 ivec3_cross(ivec3 a, ivec3 b) {
    return (ivec3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── ivec4 ──

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
            int32_t w;
        };
        int32_t v[4];
    };
} ivec4;

static inline ivec4 ivec4_add(ivec4 a, ivec4 b) {
    ivec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline ivec4 ivec4_sub(ivec4 a, ivec4 b) {
    ivec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline ivec4 ivec4_scale(ivec4 a, int32_t s) {
    ivec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline int32_t ivec4_dot(ivec4 a, ivec4 b) {
    int32_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float ivec4_len(ivec4 a) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

// ── uvec2 ──

typedef struct {
    union {
        struct {
            uint32_t x;
            uint32_t y;
        };
        uint32_t v[2];
    };
} uvec2;

static inline uvec2 uvec2_add(uvec2 a, uvec2 b) {
    uvec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline uvec2 uvec2_sub(uvec2 a, uvec2 b) {
    uvec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline uvec2 uvec2_scale(uvec2 a, uint32_t s) {
    uvec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint32_t uvec2_dot(uvec2 a, uvec2 b) {
    uint32_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float uvec2_len(uvec2 a) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

// ── uvec3 ──

typedef struct {
    union {
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t z;
        };
        uint32_t v[3];
    };
} uvec3;

static inline uvec3 uvec3_add(uvec3 a, uvec3 b) {
    uvec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline uvec3 uvec3_sub(uvec3 a, uvec3 b) {
    uvec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline uvec3 uvec3_scale(uvec3 a, uint32_t s) {
    uvec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint32_t uvec3_dot(uvec3 a, uvec3 b) {
    uint32_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float uvec3_len(uvec3 a) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

static inline uvec3 uvec3_cross(uvec3 a, uvec3 b) {
    return (uvec3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── uvec4 ──

typedef struct {
    union {
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t z;
            uint32_t w;
        };
        uint32_t v[4];
    };
} uvec4;

static inline uvec4 uvec4_add(uvec4 a, uvec4 b) {
    uvec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline uvec4 uvec4_sub(uvec4 a, uvec4 b) {
    uvec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline uvec4 uvec4_scale(uvec4 a, uint32_t s) {
    uvec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint32_t uvec4_dot(uvec4 a, uvec4 b) {
    uint32_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float uvec4_len(uvec4 a) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

// ── vec2 ──

typedef struct {
    union {
        struct {
            float x;
            float y;
        };
        float v[2];
    };
} vec2;

static inline vec2 vec2_add(vec2 a, vec2 b) {
    vec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec2 vec2_sub(vec2 a, vec2 b) {
    vec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec2 vec2_scale(vec2 a, float s) {
    vec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline float vec2_dot(vec2 a, vec2 b) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float vec2_len(vec2 a) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

static inline vec2 vec2_norm(vec2 a) {
    return vec2_scale(a, 1.0f / vec2_len(a));
}

// ── vec3 ──

typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float v[3];
    };
} vec3;

static inline vec3 vec3_add(vec3 a, vec3 b) {
    vec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
    vec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec3 vec3_scale(vec3 a, float s) {
    vec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline float vec3_dot(vec3 a, vec3 b) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float vec3_len(vec3 a) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

static inline vec3 vec3_norm(vec3 a) {
    return vec3_scale(a, 1.0f / vec3_len(a));
}

static inline vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── vec4 ──

typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        float v[4];
    };
} vec4;

static inline vec4 vec4_add(vec4 a, vec4 b) {
    vec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec4 vec4_sub(vec4 a, vec4 b) {
    vec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec4 vec4_scale(vec4 a, float s) {
    vec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline float vec4_dot(vec4 a, vec4 b) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float vec4_len(vec4 a) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += (float)a.v[i] * (float)a.v[i];
    return sqrtf(r);
}

static inline vec4 vec4_norm(vec4 a) {
    return vec4_scale(a, 1.0f / vec4_len(a));
}

// ── mat2 ──

typedef struct {
    union {
        float m[2][2];
        float v[4];
    };
} mat2;

static inline mat2 mat2_identity(void) {
    mat2 r = {0};
    for (int i = 0; i < 2; i++)
        r.m[i][i] = 1.0f;
    return r;
}

static inline mat2 mat2_add(mat2 a, mat2 b) {
    mat2 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline mat2 mat2_sub(mat2 a, mat2 b) {
    mat2 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline mat2 mat2_scale(mat2 a, float s) {
    mat2 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline mat2 mat2_mul(mat2 a, mat2 b) {
    mat2 r = {0};
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}

static inline mat2 mat2_transpose(mat2 a) {
    mat2 r;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            r.m[i][j] = a.m[j][i];
    return r;
}

static inline vec2 mat2_mul_vec(mat2 a, vec2 b) {
    vec2 r = {0};
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            r.v[i] += a.m[i][j] * b.v[j];
    return r;
}

// ── mat3 ──

typedef struct {
    union {
        float m[3][3];
        float v[9];
    };
} mat3;

static inline mat3 mat3_identity(void) {
    mat3 r = {0};
    for (int i = 0; i < 3; i++)
        r.m[i][i] = 1.0f;
    return r;
}

static inline mat3 mat3_add(mat3 a, mat3 b) {
    mat3 r;
    for (int i = 0; i < 9; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline mat3 mat3_sub(mat3 a, mat3 b) {
    mat3 r;
    for (int i = 0; i < 9; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline mat3 mat3_scale(mat3 a, float s) {
    mat3 r;
    for (int i = 0; i < 9; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline mat3 mat3_mul(mat3 a, mat3 b) {
    mat3 r = {0};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}

static inline mat3 mat3_transpose(mat3 a) {
    mat3 r;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            r.m[i][j] = a.m[j][i];
    return r;
}

static inline vec3 mat3_mul_vec(mat3 a, vec3 b) {
    vec3 r = {0};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            r.v[i] += a.m[i][j] * b.v[j];
    return r;
}

// ── mat4 ──

typedef struct {
    union {
        float m[4][4];
        float v[16];
    };
} mat4;

static inline mat4 mat4_identity(void) {
    mat4 r = {0};
    for (int i = 0; i < 4; i++)
        r.m[i][i] = 1.0f;
    return r;
}

static inline mat4 mat4_add(mat4 a, mat4 b) {
    mat4 r;
    for (int i = 0; i < 16; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline mat4 mat4_sub(mat4 a, mat4 b) {
    mat4 r;
    for (int i = 0; i < 16; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline mat4 mat4_scale(mat4 a, float s) {
    mat4 r;
    for (int i = 0; i < 16; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 r = {0};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}

static inline mat4 mat4_transpose(mat4 a) {
    mat4 r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            r.m[i][j] = a.m[j][i];
    return r;
}

static inline vec4 mat4_mul_vec(mat4 a, vec4 b) {
    vec4 r = {0};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            r.v[i] += a.m[i][j] * b.v[j];
    return r;
}

#endif // LINALG_H
