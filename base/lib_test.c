#include "lib.h"
#include "../deps/myassert.h"

#define RUN(func) do { \
    printf("- \033[34m%s()\033[0m\n", #func); \
    func(); \
    printf("    \033[32mPASS\033[0m\n\n"); \
} while(0)

void test_arena() {
    arena a = {0};
    byte *data = arena_alloc(&a, 10);

    printf("start = %p\n", a.start);
    printf("next = %p\n", a.next);
    printf("cap = %ld\n", a.cap);

}

void test_arena_string() {
    arena a = {0};
    string s = string_arena_from(&a, "Hello world\n");
    ASSERT_EQ_STR(s.data, "Hello world\n");

    string_append(&s, "Hello world\n");
    ASSERT_EQ_STR(s.data, "Hello world\nHello world\n");

    arena_release(&a);
}

void test_string() {
    string s = string_from("Hello world\n");
    ASSERT_EQ_STR(s.data, "Hello world\n");

    string_append(&s, "Hello world\n");
    ASSERT_EQ_STR(s.data, "Hello world\nHello world\n");
}

int main(int argc, char *argv[]) {
    // REBUILD_SELF(argc, argv);
    const char *deps[] = {
        "lib_test.c",
        "lib.h",
    };
    rebuild_deps(argc, argv, string_array_from(deps));

    printf("--------------------\n");
    printf("\033[33mRunning Tests\033[0m\n");
    printf("--------------------\n");

    RUN(test_arena);
    RUN(test_arena_string);
    RUN(test_string);

    printf("--------------------\n");
    printf("All tests \033[32mPASSED\033[0m\n");
    printf("--------------------\n");

    return 0;
}
