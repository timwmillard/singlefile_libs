#ifndef LINALG_H
#define LINALG_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>

// ── vec2 ──

typedef struct {
    union {
        struct {
            uint8_t x;
            uint8_t y;
        };
        uint8_t v[2];
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

static inline vec2 vec2_scale(vec2 a, uint8_t s) {
    vec2 r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint8_t vec2_dot(vec2 a, vec2 b) {
    uint8_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline uint8_t vec2_len(vec2 a) {
    uint8_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec2 vec2_norm(vec2 a) {
    return vec2_scale(a, 1.0f / vec2_len(a));
}

// ── vec3 ──

typedef struct {
    union {
        struct {
            uint8_t x;
            uint8_t y;
            uint8_t z;
        };
        uint8_t v[3];
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

static inline vec3 vec3_scale(vec3 a, uint8_t s) {
    vec3 r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint8_t vec3_dot(vec3 a, vec3 b) {
    uint8_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline uint8_t vec3_len(vec3 a) {
    uint8_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * a.v[i];
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
            uint8_t x;
            uint8_t y;
            uint8_t z;
            uint8_t w;
        };
        uint8_t v[4];
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

static inline vec4 vec4_scale(vec4 a, uint8_t s) {
    vec4 r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint8_t vec4_dot(vec4 a, vec4 b) {
    uint8_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline uint8_t vec4_len(vec4 a) {
    uint8_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec4 vec4_norm(vec4 a) {
    return vec4_scale(a, 1.0f / vec4_len(a));
}

// ── vec2i ──

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
        };
        int32_t v[2];
    };
} vec2i;

static inline vec2i vec2i_add(vec2i a, vec2i b) {
    vec2i r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec2i vec2i_sub(vec2i a, vec2i b) {
    vec2i r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec2i vec2i_scale(vec2i a, int32_t s) {
    vec2i r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline int32_t vec2i_dot(vec2i a, vec2i b) {
    int32_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline int32_t vec2i_len(vec2i a) {
    int32_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec2i vec2i_norm(vec2i a) {
    return vec2i_scale(a, 1.0f / vec2i_len(a));
}

// ── vec3i ──

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        int32_t v[3];
    };
} vec3i;

static inline vec3i vec3i_add(vec3i a, vec3i b) {
    vec3i r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec3i vec3i_sub(vec3i a, vec3i b) {
    vec3i r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec3i vec3i_scale(vec3i a, int32_t s) {
    vec3i r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline int32_t vec3i_dot(vec3i a, vec3i b) {
    int32_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline int32_t vec3i_len(vec3i a) {
    int32_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec3i vec3i_norm(vec3i a) {
    return vec3i_scale(a, 1.0f / vec3i_len(a));
}

static inline vec3i vec3i_cross(vec3i a, vec3i b) {
    return (vec3i){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── vec4i ──

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
} vec4i;

static inline vec4i vec4i_add(vec4i a, vec4i b) {
    vec4i r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec4i vec4i_sub(vec4i a, vec4i b) {
    vec4i r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec4i vec4i_scale(vec4i a, int32_t s) {
    vec4i r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline int32_t vec4i_dot(vec4i a, vec4i b) {
    int32_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline int32_t vec4i_len(vec4i a) {
    int32_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec4i vec4i_norm(vec4i a) {
    return vec4i_scale(a, 1.0f / vec4i_len(a));
}

// ── vec2u ──

typedef struct {
    union {
        struct {
            uint32_t x;
            uint32_t y;
        };
        uint32_t v[2];
    };
} vec2u;

static inline vec2u vec2u_add(vec2u a, vec2u b) {
    vec2u r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec2u vec2u_sub(vec2u a, vec2u b) {
    vec2u r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec2u vec2u_scale(vec2u a, uint32_t s) {
    vec2u r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint32_t vec2u_dot(vec2u a, vec2u b) {
    uint32_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline uint32_t vec2u_len(vec2u a) {
    uint32_t r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec2u vec2u_norm(vec2u a) {
    return vec2u_scale(a, 1.0f / vec2u_len(a));
}

// ── vec3u ──

typedef struct {
    union {
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t z;
        };
        uint32_t v[3];
    };
} vec3u;

static inline vec3u vec3u_add(vec3u a, vec3u b) {
    vec3u r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec3u vec3u_sub(vec3u a, vec3u b) {
    vec3u r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec3u vec3u_scale(vec3u a, uint32_t s) {
    vec3u r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint32_t vec3u_dot(vec3u a, vec3u b) {
    uint32_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline uint32_t vec3u_len(vec3u a) {
    uint32_t r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec3u vec3u_norm(vec3u a) {
    return vec3u_scale(a, 1.0f / vec3u_len(a));
}

static inline vec3u vec3u_cross(vec3u a, vec3u b) {
    return (vec3u){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── vec4u ──

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
} vec4u;

static inline vec4u vec4u_add(vec4u a, vec4u b) {
    vec4u r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec4u vec4u_sub(vec4u a, vec4u b) {
    vec4u r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec4u vec4u_scale(vec4u a, uint32_t s) {
    vec4u r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline uint32_t vec4u_dot(vec4u a, vec4u b) {
    uint32_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline uint32_t vec4u_len(vec4u a) {
    uint32_t r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec4u vec4u_norm(vec4u a) {
    return vec4u_scale(a, 1.0f / vec4u_len(a));
}

// ── vec2f ──

typedef struct {
    union {
        struct {
            float x;
            float y;
        };
        float v[2];
    };
} vec2f;

static inline vec2f vec2f_add(vec2f a, vec2f b) {
    vec2f r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec2f vec2f_sub(vec2f a, vec2f b) {
    vec2f r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec2f vec2f_scale(vec2f a, float s) {
    vec2f r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline float vec2f_dot(vec2f a, vec2f b) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float vec2f_len(vec2f a) {
    float r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec2f vec2f_norm(vec2f a) {
    return vec2f_scale(a, 1.0f / vec2f_len(a));
}

// ── vec3f ──

typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float v[3];
    };
} vec3f;

static inline vec3f vec3f_add(vec3f a, vec3f b) {
    vec3f r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec3f vec3f_sub(vec3f a, vec3f b) {
    vec3f r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec3f vec3f_scale(vec3f a, float s) {
    vec3f r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline float vec3f_dot(vec3f a, vec3f b) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float vec3f_len(vec3f a) {
    float r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec3f vec3f_norm(vec3f a) {
    return vec3f_scale(a, 1.0f / vec3f_len(a));
}

static inline vec3f vec3f_cross(vec3f a, vec3f b) {
    return (vec3f){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── vec4f ──

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
} vec4f;

static inline vec4f vec4f_add(vec4f a, vec4f b) {
    vec4f r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec4f vec4f_sub(vec4f a, vec4f b) {
    vec4f r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec4f vec4f_scale(vec4f a, float s) {
    vec4f r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline float vec4f_dot(vec4f a, vec4f b) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline float vec4f_len(vec4f a) {
    float r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec4f vec4f_norm(vec4f a) {
    return vec4f_scale(a, 1.0f / vec4f_len(a));
}

// ── vec2d ──

typedef struct {
    union {
        struct {
            double x;
            double y;
        };
        double v[2];
    };
} vec2d;

static inline vec2d vec2d_add(vec2d a, vec2d b) {
    vec2d r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec2d vec2d_sub(vec2d a, vec2d b) {
    vec2d r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec2d vec2d_scale(vec2d a, double s) {
    vec2d r;
    for (int i = 0; i < 2; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline double vec2d_dot(vec2d a, vec2d b) {
    double r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline double vec2d_len(vec2d a) {
    double r = 0;
    for (int i = 0; i < 2; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec2d vec2d_norm(vec2d a) {
    return vec2d_scale(a, 1.0f / vec2d_len(a));
}

// ── vec3d ──

typedef struct {
    union {
        struct {
            double x;
            double y;
            double z;
        };
        double v[3];
    };
} vec3d;

static inline vec3d vec3d_add(vec3d a, vec3d b) {
    vec3d r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec3d vec3d_sub(vec3d a, vec3d b) {
    vec3d r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec3d vec3d_scale(vec3d a, double s) {
    vec3d r;
    for (int i = 0; i < 3; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline double vec3d_dot(vec3d a, vec3d b) {
    double r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline double vec3d_len(vec3d a) {
    double r = 0;
    for (int i = 0; i < 3; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec3d vec3d_norm(vec3d a) {
    return vec3d_scale(a, 1.0f / vec3d_len(a));
}

static inline vec3d vec3d_cross(vec3d a, vec3d b) {
    return (vec3d){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
}

// ── vec4d ──

typedef struct {
    union {
        struct {
            double x;
            double y;
            double z;
            double w;
        };
        double v[4];
    };
} vec4d;

static inline vec4d vec4d_add(vec4d a, vec4d b) {
    vec4d r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] + b.v[i];
    return r;
}

static inline vec4d vec4d_sub(vec4d a, vec4d b) {
    vec4d r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] - b.v[i];
    return r;
}

static inline vec4d vec4d_scale(vec4d a, double s) {
    vec4d r;
    for (int i = 0; i < 4; i++)
        r.v[i] = a.v[i] * s;
    return r;
}

static inline double vec4d_dot(vec4d a, vec4d b) {
    double r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * b.v[i];
    return r;
}

static inline double vec4d_len(vec4d a) {
    double r = 0;
    for (int i = 0; i < 4; i++)
        r += a.v[i] * a.v[i];
    return sqrtf(r);
}

static inline vec4d vec4d_norm(vec4d a) {
    return vec4d_scale(a, 1.0f / vec4d_len(a));
}

#endif // LINALG_H
