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

#define ASSERT_STR(s, cstr) ASSERT((s).len == strlen(cstr) && strncmp((s).data, cstr, (s).len) == 0)

/* string_from: len and data match the source */
void test_string_from() {
    arena a = {0};
    string s = string_from(&a, "hello");
    ASSERT(s.len == 5);
    ASSERT(strncmp(s.data, "hello", 5) == 0);
    ASSERT(s.arena == &a);
    arena_release(&a);
    PASS("string_from");
}

/* string_from_len: only takes the specified number of chars */
void test_string_from_len() {
    arena a = {0};
    string s = string_from_len(&a, "hello world", 5);
    ASSERT_STR(s, "hello");
    arena_release(&a);
    PASS("string_from_len");
}

/* string_view: points into the original, no arena needed */
void test_string_view() {
    string s = string_view("hello");
    ASSERT(s.len == 5);
    ASSERT(strncmp(s.data, "hello", 5) == 0);
    ASSERT(s.arena == NULL);
    PASS("string_view");
}

/* string_append: grows and concatenates correctly */
void test_string_append() {
    arena a = {0};
    string s = string_from(&a, "hello");
    string_append(&s, " world");
    ASSERT_STR(s, "hello world");
    string_append(&s, "!");
    ASSERT_STR(s, "hello world!");
    arena_release(&a);
    PASS("string_append");
}

/* string_push: appends a single character and stays null-terminated */
void test_string_push() {
    arena a = {0};
    string s = string_new(&a, 4);
    string_push(&s, 'a');
    string_push(&s, 'b');
    string_push(&s, 'c');
    ASSERT_STR(s, "abc");
    ASSERT(s.data[s.len] == '\0');
    arena_release(&a);
    PASS("string_push");
}

/* string_slice: positive and negative indices, clamping */
void test_string_slice() {
    arena a = {0};
    string s = string_from(&a, "hello world");
    string sl = string_slice(s, 6, 11);
    ASSERT_STR(sl, "world");
    string sl2 = string_slice(s, -5, 11);  /* -5 == len-5 == 6 */
    ASSERT_STR(sl2, "world");
    string sl3 = string_slice(s, 0, 5);
    ASSERT_STR(sl3, "hello");
    arena_release(&a);
    PASS("string_slice");
}

/* string_starts_with / string_ends_with */
void test_string_starts_ends_with() {
    string s = string_view("hello world");
    ASSERT(string_starts_with(s, "hello"));
    ASSERT(!string_starts_with(s, "world"));
    ASSERT(string_ends_with(s, "world"));
    ASSERT(!string_ends_with(s, "hello"));
    ASSERT(string_starts_with(s, "hello world"));
    ASSERT(string_ends_with(s, "hello world"));
    PASS("string_starts_ends_with");
}

/* string_find: found and not found */
void test_string_find() {
    string s = string_view("hello world");
    ASSERT(string_find(s, "world") == 6);
    ASSERT(string_find(s, "hello") == 0);
    ASSERT(string_find(s, "xyz") == -1);
    PASS("string_find");
}

/* string_trim: strips leading and trailing whitespace */
void test_string_trim() {
    string t = string_trim(string_view("  hello  "));
    ASSERT_STR(t, "hello");
    string t2 = string_trim(string_view("hello"));
    ASSERT_STR(t2, "hello");
    string t3 = string_trim(string_view("  "));
    ASSERT(t3.len == 0 || (t3.len == 1 && t3.data[0] == ' '));
    PASS("string_trim");
}

/* string_upper / string_lower */
void test_string_upper_lower() {
    arena a = {0};
    string s = string_from(&a, "Hello World");
    ASSERT_STR(string_upper(s), "HELLO WORLD");
    ASSERT_STR(string_lower(s), "hello world");
    arena_release(&a);
    PASS("string_upper_lower");
}

/* cstring: returns null-terminated pointer */
void test_cstring() {
    arena a = {0};
    string s = string_from(&a, "hello");
    const char *cs = cstring(&s);
    ASSERT(strcmp(cs, "hello") == 0);
    arena_release(&a);
    PASS("cstring");
}

/* string_split: correct count and values */
void test_string_split() {
    string_array arr = string_split(string_view("one,two,three"), ",");
    ASSERT(arr.count == 3);
    ASSERT_STR(arr.items[0], "one");
    ASSERT_STR(arr.items[1], "two");
    ASSERT_STR(arr.items[2], "three");
    string_array_free(&arr);
    /* no delimiter present: single element */
    string_array arr2 = string_split(string_view("hello"), ",");
    ASSERT(arr2.count == 1);
    ASSERT_STR(arr2.items[0], "hello");
    string_array_free(&arr2);
    PASS("string_split");
}

/* string_join: joins with separator */
void test_string_join() {
    arena a = {0};
    const char *parts[] = {"one", "two", "three"};
    string_array arr = string_array_arena_from(&a, parts);
    arr.arena = &a;
    string joined = string_join(arr, ", ");
    ASSERT_STR(joined, "one, two, three");
    arena_release(&a);
    PASS("string_join");
}

int main(void) {
    printf(COLOR_BOLD "\nstring tests\n" COLOR_RESET "\n");

    test_string_from();
    test_string_from_len();
    test_string_view();
    test_string_append();
    test_string_push();
    test_string_slice();
    test_string_starts_ends_with();
    test_string_find();
    test_string_trim();
    test_string_upper_lower();
    test_cstring();
    test_string_split();
    test_string_join();

    if (_failed == 0)
        printf("\n" COLOR_BOLD COLOR_GREEN "All tests passed." COLOR_RESET "\n\n");
    else
        printf("\n" COLOR_BOLD COLOR_RED "%d test(s) failed." COLOR_RESET "\n\n", _failed);
    return _failed != 0;
}
