/*
 
linalg.h Linear Alegbra generator.

Usage:
cc -o gen_linalg gen_linalg.c && ./gen_linalg > linalg.h
*/

#include <stdio.h>

// ─── vector table ────────────────────────────────────────────────────────────

typedef struct {
    const char *name;   // "vec3f"
    const char *type;   // "float"
    const char *tt;     // "f"
    int         n;      // 3
} VecDef;

static VecDef vecs[] = {
    { "vec2",  "uint8_t",  "",  2 },
    { "vec3",  "uint8_t",  "",  3 },
    { "vec4",  "uint8_t",  "",  4 },
    { "vec2i", "int32_t",  "i", 2 },
    { "vec3i", "int32_t",  "i", 3 },
    { "vec4i", "int32_t",  "i", 4 },
    { "vec2u", "uint32_t", "u", 2 },
    { "vec3u", "uint32_t", "u", 3 },
    { "vec4u", "uint32_t", "u", 4 },
    { "vec2f", "float",    "f", 2 },
    { "vec3f", "float",    "f", 3 },
    { "vec4f", "float",    "f", 4 },
};

// ─── field names ─────────────────────────────────────────────────────────────

static const char *fields[] = { "x", "y", "z", "w" };

// ─── helpers ─────────────────────────────────────────────────────────────────

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

static void emit_vec_fields(VecDef *v) {
    for (int i = 0; i < v->n; i++)
        printf("            %s %s;\n", v->type, fields[i]);
}

// ─── emitters ────────────────────────────────────────────────────────────────

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
    printf("static inline %s %s_len(%s a) {\n",
           v->type, v->name, v->name);
    printf("    %s r = 0;\n", v->type);
    printf("    for (int i = 0; i < %d; i++)\n", v->n);
    printf("        r += a.v[i] * a.v[i];\n");
    printf("    return sqrtf(r);\n");
    printf("}\n\n");
}

static void emit_vec_norm(VecDef *v) {
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

// ─── main ────────────────────────────────────────────────────────────────────

int main(void) {
    printf("#ifndef LINALG_H\n");
    printf("#define LINALG_H\n\n");
    printf("#include <math.h>\n");
    printf("#include <stddef.h>\n");
    printf("#include <stdint.h>\n\n");

    int count = ARRAY_COUNT(vecs);
    for (int i = 0; i < count; i++) {
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

    printf("#endif // LINALG_H\n");
    return 0;
}
