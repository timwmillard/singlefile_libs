/*

linalg.h Linear Alegbra generator.

Usage:
cc -o gen_linalg gen_linalg.c && ./gen_linalg > linalg.h
*/

#include <stdio.h>

// ─── vector table ────────────────────────────────────────────────────────────

typedef struct {
    const char *name;      // "vec3f"
    const char *type;      // "float"
    int         n;         // 3
    int         is_float;  // 0 for integer types; affects len return type and norm
} VecDef;

static VecDef vecs[] = {
    { "bvec2",  "uint8_t",  2, 0 },
    { "bvec3",  "uint8_t",  3, 0 },
    { "bvec4",  "uint8_t",  4, 0 },
    { "ivec2", "int32_t",  2, 0 },
    { "ivec3", "int32_t",  3, 0 },
    { "ivec4", "int32_t",  4, 0 },
    { "uvec2", "uint32_t", 2, 0 },
    { "uvec3", "uint32_t", 3, 0 },
    { "uvec4", "uint32_t", 4, 0 },
    { "vec2", "float",    2, 1 },
    { "vec3", "float",    3, 1 },
    { "vec4", "float",    4, 1 },
};

// ─── matrix table ────────────────────────────────────────────────────────────

typedef struct {
    const char *name;      // "mat3f"
    const char *vec_name;  // "vec3f"
    int         n;         // 3
} MatDef;

static MatDef mats[] = {
    { "mat2f", "vec2f", 2 },
    { "mat3f", "vec3f", 3 },
    { "mat4f", "vec4f", 4 },
};

// ─── field names ─────────────────────────────────────────────────────────────

static const char *fields[] = { "x", "y", "z", "w" };

// ─── helpers ─────────────────────────────────────────────────────────────────

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

static void emit_vec_fields(VecDef *v) {
    for (int i = 0; i < v->n; i++)
        printf("            %s %s;\n", v->type, fields[i]);
}

// ─── vec emitters ─────────────────────────────────────────────────────────────

static void emit_vec_struct(VecDef *v) {
    printf("typedef struct {\n");
    printf("    union {\n");
    printf("        struct {\n");
    emit_vec_fields(v);
    printf("        };\n");
    printf("        %s v[%d];\n", v->type, v->n);
    printf("    };\n");
    printf("} %s;\n\n", v->name);
}

static void emit_vec_add(VecDef *v) {
    printf("static inline %s %s_add(%s a, %s b) {\n",
           v->name, v->name, v->name, v->name);
    printf("    %s r;\n", v->name);
    printf("    for (int i = 0; i < %d; i++)\n", v->n);
    printf("        r.v[i] = a.v[i] + b.v[i];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_vec_sub(VecDef *v) {
    printf("static inline %s %s_sub(%s a, %s b) {\n",
           v->name, v->name, v->name, v->name);
    printf("    %s r;\n", v->name);
    printf("    for (int i = 0; i < %d; i++)\n", v->n);
    printf("        r.v[i] = a.v[i] - b.v[i];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_vec_scale(VecDef *v) {
    printf("static inline %s %s_scale(%s a, %s s) {\n",
           v->name, v->name, v->name, v->type);
    printf("    %s r;\n", v->name);
    printf("    for (int i = 0; i < %d; i++)\n", v->n);
    printf("        r.v[i] = a.v[i] * s;\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_vec_dot(VecDef *v) {
    printf("static inline %s %s_dot(%s a, %s b) {\n",
           v->type, v->name, v->name, v->name);
    printf("    %s r = 0;\n", v->type);
    printf("    for (int i = 0; i < %d; i++)\n", v->n);
    printf("        r += a.v[i] * b.v[i];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_vec_len(VecDef *v) {
    printf("static inline float %s_len(%s a) {\n", v->name, v->name);
    printf("    float r = 0;\n");
    printf("    for (int i = 0; i < %d; i++)\n", v->n);
    printf("        r += (float)a.v[i] * (float)a.v[i];\n");
    printf("    return sqrtf(r);\n");
    printf("}\n\n");
}

static void emit_vec_norm(VecDef *v) {
    // norm requires float arithmetic; skip for integer types
    if (!v->is_float) return;
    printf("static inline %s %s_norm(%s a) {\n",
           v->name, v->name, v->name);
    printf("    return %s_scale(a, 1.0f / %s_len(a));\n",
           v->name, v->name);
    printf("}\n\n");
}

static void emit_vec_cross(VecDef *v) {
    // cross product only makes sense for vec3
    if (v->n != 3) return;
    printf("static inline %s %s_cross(%s a, %s b) {\n",
           v->name, v->name, v->name, v->name);
    printf("    return (%s){\n", v->name);
    printf("        a.y*b.z - a.z*b.y,\n");
    printf("        a.z*b.x - a.x*b.z,\n");
    printf("        a.x*b.y - a.y*b.x,\n");
    printf("    };\n");
    printf("}\n\n");
}

// ─── mat emitters ─────────────────────────────────────────────────────────────

static void emit_mat_struct(MatDef *m) {
    printf("typedef struct {\n");
    printf("    union {\n");
    printf("        float m[%d][%d];\n", m->n, m->n);
    printf("        float v[%d];\n", m->n * m->n);
    printf("    };\n");
    printf("} %s;\n\n", m->name);
}

static void emit_mat_identity(MatDef *m) {
    printf("static inline %s %s_identity(void) {\n", m->name, m->name);
    printf("    %s r = {0};\n", m->name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n);
    printf("        r.m[i][i] = 1.0f;\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_mat_add(MatDef *m) {
    printf("static inline %s %s_add(%s a, %s b) {\n",
           m->name, m->name, m->name, m->name);
    printf("    %s r;\n", m->name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n * m->n);
    printf("        r.v[i] = a.v[i] + b.v[i];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_mat_sub(MatDef *m) {
    printf("static inline %s %s_sub(%s a, %s b) {\n",
           m->name, m->name, m->name, m->name);
    printf("    %s r;\n", m->name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n * m->n);
    printf("        r.v[i] = a.v[i] - b.v[i];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_mat_scale(MatDef *m) {
    printf("static inline %s %s_scale(%s a, float s) {\n",
           m->name, m->name, m->name);
    printf("    %s r;\n", m->name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n * m->n);
    printf("        r.v[i] = a.v[i] * s;\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_mat_mul(MatDef *m) {
    printf("static inline %s %s_mul(%s a, %s b) {\n",
           m->name, m->name, m->name, m->name);
    printf("    %s r = {0};\n", m->name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n);
    printf("        for (int j = 0; j < %d; j++)\n", m->n);
    printf("            for (int k = 0; k < %d; k++)\n", m->n);
    printf("                r.m[i][j] += a.m[i][k] * b.m[k][j];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_mat_transpose(MatDef *m) {
    printf("static inline %s %s_transpose(%s a) {\n",
           m->name, m->name, m->name);
    printf("    %s r;\n", m->name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n);
    printf("        for (int j = 0; j < %d; j++)\n", m->n);
    printf("            r.m[i][j] = a.m[j][i];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

static void emit_mat_mul_vec(MatDef *m) {
    printf("static inline %s %s_mul_vec(%s a, %s b) {\n",
           m->vec_name, m->name, m->name, m->vec_name);
    printf("    %s r = {0};\n", m->vec_name);
    printf("    for (int i = 0; i < %d; i++)\n", m->n);
    printf("        for (int j = 0; j < %d; j++)\n", m->n);
    printf("            r.v[i] += a.m[i][j] * b.v[j];\n");
    printf("    return r;\n");
    printf("}\n\n");
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(void) {
    printf("#ifndef LINALG_H\n");
    printf("#define LINALG_H\n\n");
    printf("#include <math.h>\n");
    printf("#include <stddef.h>\n");
    printf("#include <stdint.h>\n\n");
    printf("#if defined(__GNUC__) || defined(__clang__)\n");
    printf("#  pragma GCC diagnostic ignored \"-Wfloat-equal\"\n");
    printf("#  pragma GCC diagnostic ignored \"-Wmissing-braces\"\n");
    printf("#  pragma GCC diagnostic ignored \"-Wmissing-field-initializers\"\n");
    printf("#  ifdef __clang__\n");
    printf("#    pragma GCC diagnostic ignored \"-Wgnu-anonymous-struct\"\n");
    printf("#    pragma GCC diagnostic ignored \"-Wnested-anon-types\"\n");
    printf("#  endif\n");
    printf("#endif\n\n");

    int vec_count = ARRAY_COUNT(vecs);
    for (int i = 0; i < vec_count; i++) {
        VecDef *v = &vecs[i];
        printf("// ── %s ──\n\n", v->name);
        emit_vec_struct(v);
        emit_vec_add(v);
        emit_vec_sub(v);
        emit_vec_scale(v);
        emit_vec_dot(v);
        emit_vec_len(v);
        emit_vec_norm(v);
        emit_vec_cross(v);
    }

    int mat_count = ARRAY_COUNT(mats);
    for (int i = 0; i < mat_count; i++) {
        MatDef *m = &mats[i];
        printf("// ── %s ──\n\n", m->name);
        emit_mat_struct(m);
        emit_mat_identity(m);
        emit_mat_add(m);
        emit_mat_sub(m);
        emit_mat_scale(m);
        emit_mat_mul(m);
        emit_mat_transpose(m);
        emit_mat_mul_vec(m);
    }

    printf("#endif // LINALG_H\n");
    return 0;
}
