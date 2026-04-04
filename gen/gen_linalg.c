/*

linalg.h Linear Algebra generator.

Usage:
  cc -o gen_linalg gen_linalg.c && ./gen_linalg > linalg.h
  cc -o gen_linalg gen_linalg.c && ./gen_linalg --test > linalg_test.c
*/

#include <stdio.h>
#include <string.h>

// ─── vector table ────────────────────────────────────────────────────────────

typedef struct {
    const char *name;      // "vec3"
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
    const char *vec_name;  // "vec3"
    int         n;         // 3
} MatDef;

static MatDef mats[] = {
    { "mat2", "vec2", 2 },
    { "mat3", "vec3", 3 },
    { "mat4", "vec4", 4 },
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

// ─── test data ────────────────────────────────────────────────────────────────

static int unsigned_a[4] = {3, 4, 5, 6};
static int unsigned_b[4] = {1, 2, 3, 4};
static int signed_a[4]   = {-3, 4, -5, 6};
static int signed_b[4]   = {1, -2, 3, -4};

// pythagorean triple vectors giving len=5 for each n
static int len_vals[3][4] = {
    {3, 4, 0, 0},    // n=2
    {0, 3, 4, 0},    // n=3
    {0, 0, 3, 4},    // n=4
};

// ─── test emitters ────────────────────────────────────────────────────────────

static void emit_test_header(void) {
    printf("/* generated — do not edit; run: ./gen_linalg --test > linalg_test.c */\n\n");
    printf("#include <math.h>\n");
    printf("#include <stdio.h>\n");
    printf("#include \"linalg.h\"\n\n");
    printf("#define COLOR_GREEN  \"\\033[32m\"\n");
    printf("#define COLOR_RED    \"\\033[31m\"\n");
    printf("#define COLOR_BOLD   \"\\033[1m\"\n");
    printf("#define COLOR_RESET  \"\\033[0m\"\n\n");
    printf("#define PASS(name) printf(COLOR_GREEN \"  PASS\" COLOR_RESET \"  %%s\\n\", name)\n\n");
    printf("static int _failed = 0;\n\n");
    printf("#define ASSERT(expr) do { \\\n");
    printf("    if (!(expr)) { \\\n");
    printf("        printf(COLOR_RED \"  FAIL\" COLOR_RESET \"  %%s:%%d  \" #expr \"\\n\", __FILE__, __LINE__); \\\n");
    printf("        _failed++; \\\n");
    printf("    } \\\n");
    printf("} while(0)\n\n");
    printf("#define EPS 1e-5f\n");
    printf("#define ASSERT_FEQ(a, b) ASSERT(fabsf((a) - (b)) < EPS)\n\n");
}

static void emit_test_vec(VecDef *v) {
    int is_signed = (v->type[0] == 'i');
    int *a = is_signed ? signed_a : unsigned_a;
    int *b = is_signed ? signed_b : unsigned_b;
    int n = v->n;
    const char *nm = v->name;
    int *lv = len_vals[n - 2];

    printf("// ─── %s (%s) ", nm, v->type);
    printf("─────────────────────────────────────────────────────────────\n\n");
    printf("static void test_%s(void) {\n", nm);

    // declare a and b
    if (v->is_float) {
        printf("    %s a = {", nm);
        for (int i = 0; i < n; i++) printf("%s%.1ff", i ? ", " : "", (float)a[i]);
        printf("};\n");
        printf("    %s b = {", nm);
        for (int i = 0; i < n; i++) printf("%s%.1ff", i ? ", " : "", (float)b[i]);
        printf("};\n\n");
    } else {
        printf("    %s a = {", nm);
        for (int i = 0; i < n; i++) printf("%s%d", i ? ", " : "", a[i]);
        printf("};\n");
        printf("    %s b = {", nm);
        for (int i = 0; i < n; i++) printf("%s%d", i ? ", " : "", b[i]);
        printf("};\n\n");
    }

    // add
    if (v->is_float) {
        printf("    %s r = %s_add(a, b);\n", nm, nm);
        for (int i = 0; i < n; i++)
            printf("    ASSERT_FEQ(r.%s, %.1ff);\n", fields[i], (float)(a[i] + b[i]));
    } else {
        printf("    %s r = %s_add(a, b);\n", nm, nm);
        printf("    ASSERT(");
        for (int i = 0; i < n; i++) {
            if (i) printf(" && ");
            printf("r.%s == %d", fields[i], a[i] + b[i]);
        }
        printf(");\n");
    }
    printf("\n");

    // sub
    if (v->is_float) {
        printf("    r = %s_sub(a, b);\n", nm);
        for (int i = 0; i < n; i++)
            printf("    ASSERT_FEQ(r.%s, %.1ff);\n", fields[i], (float)(a[i] - b[i]));
    } else {
        printf("    r = %s_sub(a, b);\n", nm);
        printf("    ASSERT(");
        for (int i = 0; i < n; i++) {
            if (i) printf(" && ");
            printf("r.%s == %d", fields[i], a[i] - b[i]);
        }
        printf(");\n");
    }
    printf("\n");

    // scale
    if (v->is_float) {
        printf("    r = %s_scale(a, 2.0f);\n", nm);
        for (int i = 0; i < n; i++)
            printf("    ASSERT_FEQ(r.%s, %.1ff);\n", fields[i], (float)(a[i] * 2));
    } else {
        printf("    r = %s_scale(a, 2);\n", nm);
        printf("    ASSERT(");
        for (int i = 0; i < n; i++) {
            if (i) printf(" && ");
            printf("r.%s == %d", fields[i], a[i] * 2);
        }
        printf(");\n");
    }
    printf("\n");

    // dot
    int dot = 0;
    for (int i = 0; i < n; i++) dot += a[i] * b[i];
    if (v->is_float) {
        printf("    ASSERT_FEQ(%s_dot(a, b), %.1ff);\n", nm, (float)dot);
    } else {
        printf("    ASSERT(%s_dot(a, b) == %d);\n", nm, dot);
    }
    printf("\n");

    // len — pythagorean triple giving 5
    if (v->is_float) {
        printf("    ASSERT_FEQ(%s_len((%s){", nm, nm);
        for (int i = 0; i < n; i++) printf("%s%.1ff", i ? ", " : "", (float)lv[i]);
        printf("}), 5.0f);\n");
    } else {
        printf("    ASSERT_FEQ(%s_len((%s){", nm, nm);
        for (int i = 0; i < n; i++) printf("%s%d", i ? ", " : "", lv[i]);
        printf("}), 5.0f);\n");
    }

    // norm (float only)
    if (v->is_float) {
        printf("\n    %s nv = %s_norm((%s){", nm, nm, nm);
        for (int i = 0; i < n; i++) printf("%s%.1ff", i ? ", " : "", (float)lv[i]);
        printf("});\n");
        printf("    ASSERT_FEQ(%s_len(nv), 1.0f);\n", nm);
        for (int i = 0; i < n; i++) {
            if (lv[i] != 0)
                printf("    ASSERT_FEQ(nv.%s, %.1ff / 5.0f);\n", fields[i], (float)lv[i]);
        }
    }

    // cross (n==3 only)
    if (n == 3) {
        printf("\n");
        if (v->is_float) {
            printf("    %s cx = %s_cross((%s){1.0f,0.0f,0.0f}, (%s){0.0f,1.0f,0.0f});\n",
                   nm, nm, nm, nm);
            printf("    ASSERT_FEQ(cx.x, 0.0f); ASSERT_FEQ(cx.y, 0.0f); ASSERT_FEQ(cx.z, 1.0f);\n");
        } else {
            printf("    %s cx = %s_cross((%s){1,0,0}, (%s){0,1,0});\n", nm, nm, nm, nm);
            printf("    ASSERT(cx.x == 0 && cx.y == 0 && cx.z == 1);\n");
        }
    }

    printf("}\n\n");
}

static void emit_test_mat(MatDef *m) {
    int n = m->n;
    int nn = n * n;
    const char *mn = m->name;
    const char *vn = m->vec_name;

    printf("// ─── %s ", mn);
    printf("─────────────────────────────────────────────────────────────────────\n\n");
    printf("static void test_%s(void) {\n", mn);

    // identity check
    printf("    %s I = %s_identity();\n", mn, mn);
    printf("    for (int i = 0; i < %d; i++)\n", n);
    printf("        for (int j = 0; j < %d; j++)\n", n);
    printf("            ASSERT_FEQ(I.m[i][j], i == j ? 1.0f : 0.0f);\n\n");

    // sequential a
    printf("    %s a = {.v = {", mn);
    for (int i = 0; i < nn; i++) printf("%s%d", i ? "," : "", i + 1);
    printf("}};\n\n");

    // scale to zero
    printf("    %s r = %s_scale(a, 0.0f);\n", mn, mn);
    printf("    for (int i = 0; i < %d; i++) ASSERT_FEQ(r.v[i], 0.0f);\n\n", nn);

    // add/sub roundtrip
    printf("    %s b = %s_scale(I, 2.0f);\n", mn, mn);
    printf("    r = %s_sub(%s_add(a, b), b);\n", mn, mn);
    printf("    for (int i = 0; i < %d; i++) ASSERT_FEQ(r.v[i], a.v[i]);\n\n", nn);

    // identity * a = a
    printf("    r = %s_mul(I, a);\n", mn);
    printf("    for (int i = 0; i < %d; i++) ASSERT_FEQ(r.v[i], a.v[i]);\n\n", nn);

    // transpose
    printf("    r = %s_transpose(a);\n", mn);
    printf("    for (int i = 0; i < %d; i++)\n", n);
    printf("        for (int j = 0; j < %d; j++)\n", n);
    printf("            ASSERT_FEQ(r.m[i][j], a.m[j][i]);\n\n");

    // identity * v = v
    printf("    %s v = {", vn);
    for (int i = 0; i < n; i++) printf("%s%.1ff", i ? ", " : "", (float)(i + 1));
    printf("};\n");
    printf("    %s rv = %s_mul_vec(I, v);\n", vn, mn);
    for (int i = 0; i < n; i++)
        printf("    ASSERT_FEQ(rv.%s, %.1ff);\n", fields[i], (float)(i + 1));
    printf("\n");

    // diagonal scale: diag[i] = i+2, v[i] = i+1, result[i] = (i+1)*(i+2)
    printf("    %s ds = {.m = {", mn);
    for (int i = 0; i < n; i++) {
        printf("{");
        for (int j = 0; j < n; j++) {
            if (j) printf(",");
            printf("%d", i == j ? i + 2 : 0);
        }
        printf("}");
        if (i < n - 1) printf(",");
    }
    printf("}};\n");
    printf("    rv = %s_mul_vec(ds, v);\n", mn);
    for (int i = 0; i < n; i++)
        printf("    ASSERT_FEQ(rv.%s, %.1ff);\n", fields[i], (float)((i + 1) * (i + 2)));

    printf("}\n\n");
}

static void emit_test_main(void) {
    int vec_count = ARRAY_COUNT(vecs);
    int mat_count = ARRAY_COUNT(mats);

    printf("// ─── main ");
    printf("────────────────────────────────────────────────────────────────────\n\n");
    printf("int main(void) {\n");
    printf("    printf(COLOR_BOLD \"\\nlinalg.h tests\\n\" COLOR_RESET \"\\n\");\n\n");
    for (int i = 0; i < vec_count; i++)
        printf("    test_%s(); PASS(\"%s\");\n", vecs[i].name, vecs[i].name);
    printf("\n");
    for (int i = 0; i < mat_count; i++)
        printf("    test_%s(); PASS(\"%s\");\n", mats[i].name, mats[i].name);
    printf("\n");
    printf("    if (_failed == 0)\n");
    printf("        printf(\"\\n\" COLOR_BOLD COLOR_GREEN \"All tests passed.\" COLOR_RESET \"\\n\\n\");\n");
    printf("    else\n");
    printf("        printf(\"\\n\" COLOR_BOLD COLOR_RED \"%%d test(s) failed.\" COLOR_RESET \"\\n\\n\", _failed);\n");
    printf("    return _failed != 0;\n");
    printf("}\n");
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char **argv) {
    int gen_test = argc > 1 && strcmp(argv[1], "--test") == 0;

    if (gen_test) {
        emit_test_header();
        int vec_count = ARRAY_COUNT(vecs);
        for (int i = 0; i < vec_count; i++)
            emit_test_vec(&vecs[i]);
        int mat_count = ARRAY_COUNT(mats);
        for (int i = 0; i < mat_count; i++)
            emit_test_mat(&mats[i]);
        emit_test_main();
        return 0;
    }

    // ── generate linalg.h ────────────────────────────────────────────────────
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
