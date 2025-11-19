#include "lib.h"
#include "../deps/myassert.h"

void test_string() {
    string s = string_from("Hello world\n");

    printf("s = %s\n", s.data);
    ASSERT_EQ_STR(s.data, "Hello world\n");
}

int main(int argc, char *argv[]) {
    REBUILD_SELF(argc, argv);

    printf("--------------------\n");
    printf("Running lib.h tests\n");
    printf("--------------------\n");

    test_string();

    return 0;
}
