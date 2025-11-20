#ifndef BASE_H
#define BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

typedef unsigned char byte;

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define ARENA_REGION_DEFAULT_SIZE 4096

typedef struct arena_region {
    struct arena_region *next;
    size_t len;
    size_t cap;
    uintptr_t data[];
} arena_region;

typedef struct arena {
    arena_region *start;
    arena_region *end;
} arena;

typedef struct {
    char* data;
    size_t len;
    size_t cap;
    bool owned;  // whether we own the memory
    arena *arena;
} string;


#if DEBUG
void *null_malloc(size_t size) {
    return NULL;
}
#endif

// TODO: instead of accepting specific capacity new_region() should accept the size of the object we want to fit into the region
// It should be up to new_region() to decide the actual capacity to allocate
arena_region *_arena_new_region(size_t cap) {
    size_t size_bytes = sizeof(arena_region) + sizeof(uintptr_t)*cap;
    arena_region *r = (arena_region*)malloc(size_bytes);
    assert(r);
    r->next = NULL;
    r->len = 0;
    r->cap = cap;
    return r;
}

void _arena_free_region(arena_region *r) {
    free(r);
}

void *arena_alloc(arena *a, size_t size_bytes) {
    size_t size = (size_bytes + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);

    if (a->end == NULL) {
        assert(a->start == NULL);
        size_t capacity = ARENA_REGION_DEFAULT_SIZE;
        if (capacity < size) capacity = size;
        a->end = _arena_new_region(capacity);
        a->start = a->end;
    }

    while (a->end->len + size > a->end->cap && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->len + size > a->end->cap) {
        assert(a->end->next == NULL);
        size_t capacity = ARENA_REGION_DEFAULT_SIZE;
        if (capacity < size) capacity = size;
        a->end->next = _arena_new_region(capacity);
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->len];
    a->end->len += size;
    return result;
}

void *arena_realloc(arena *a, void *oldptr, size_t oldsz, size_t newsz) {
    if (newsz <= oldsz) return oldptr;
    void *newptr = arena_alloc(a, newsz);
    char *newptr_char = (char*)newptr;
    char *oldptr_char = (char*)oldptr;
    for (size_t i = 0; i < oldsz; ++i) {
        newptr_char[i] = oldptr_char[i];
    }
    return newptr;
}

void arena_release(arena *a) {
    arena_region *r = a->start;
    while (r) {
        arena_region *r0 = r;
        r = r->next;
        _arena_free_region(r0);
    }
    a->start = NULL;
    a->end = NULL;
}

string string_arena_new(arena *a, size_t capacity) {
    char* data = arena_alloc(a, capacity + 1);
    data[0] = '\0';
    return (string){
        .data = data,
        .len = 0,
        .cap = capacity,
        .owned = true,
        .arena = a,
    };
}

// Create owned string from C string
string string_arena_from(arena *a, const char* str) {
    size_t len = strlen(str);
    string s = string_arena_new(a, len);
    strcpy(s.data, str);
    s.len = len;
    return s;
}

// Ensure capacity for owned strings
void string_arena__reserve(string* s, arena *a, size_t new_cap) {
    if (!s->owned) {
        // Convert view to owned string
        char* old_data = s->data;
        s->data = arena_alloc(a, new_cap + 1);
        strcpy(s->data, old_data);
        s->owned = true;
        s->cap = new_cap;
    } else if (new_cap > s->cap) {
        if (s->arena) {
            // Arena-allocated string: use arena_realloc
            s->data = arena_realloc(s->arena, s->data, s->cap + 1, new_cap + 1);
        } else {
            // Heap-allocated string: use realloc
            s->data = realloc(s->data, new_cap + 1);
        }
        s->cap = new_cap;
    }
}

// Append to string
void string_append(string* s, const char* str) {
    size_t add_len = strlen(str);
    size_t new_len = s->len + add_len;
    
    if (new_len > s->cap) {
        string_arena__reserve(s, s->arena, new_len * 2);  // double capacity
    }
    
    strcpy(s->data + s->len, str);
    s->len = new_len;
}

// Create string from literal (view - doesn't own memory)
string string_view(const char* str) {
    return (string){
        .data = (char*)str,
        .len = strlen(str),
        .cap = 0,
        .owned = false
    };
}

// Create owned string with capacity
string string_new(size_t capacity) {
    return string_arena_new(NULL, capacity);
}

// Create owned string from C string
string string_from(const char* str) {
    size_t len = strlen(str);
    string s = string_new(len);
    strcpy(s.data, str);
    s.len = len;
    return s;
}

// Append character
void string_push(string* s, char c) {
    if (s->len + 1 > s->cap) {
        string_arena__reserve(s, s->arena, (s->cap + 1) * 2);
    }
    s->data[s->len++] = c;
    s->data[s->len] = '\0';
}

// String manipulation functions
string string_slice(string s, int start, int end) {
    if (start < 0) start = s.len + start;
    if (end < 0) end = s.len + end;
    if (start < 0) start = 0;
    if (end > (int)s.len) end = s.len;
    if (start >= end) return string_view("");
    
    return (string){
        .data = s.data + start,
        .len = end - start,
        .cap = 0,
        .owned = false
    };
}

bool string_starts_with(string s, const char* prefix) {
    size_t prefix_len = strlen(prefix);
    return s.len >= prefix_len && strncmp(s.data, prefix, prefix_len) == 0;
}

bool string_ends_with(string s, const char* suffix) {
    size_t suffix_len = strlen(suffix);
    return s.len >= suffix_len && 
           strcmp(s.data + s.len - suffix_len, suffix) == 0;
}

int string_find(string s, const char* needle) {
    char* found = strstr(s.data, needle);
    return found ? (found - s.data) : -1;
}

string string_trim(string s) {
    char* start = s.data;
    char* end = s.data + s.len - 1;
    
    while (start <= end && isspace(*start)) start++;
    while (end > start && isspace(*end)) end--;
    
    return (string){
        .data = start,
        .len = end - start + 1,
        .cap = 0,
        .owned = false
    };
}

string string_upper(string s) {
    string result = string_from(s.data);
    for (size_t i = 0; i < result.len; i++) {
        result.data[i] = toupper(result.data[i]);
    }
    return result;
}

string string_lower(string s) {
    string result = string_from(s.data);
    for (size_t i = 0; i < result.len; i++) {
        result.data[i] = tolower(result.data[i]);
    }
    return result;
}

// Returns the string as a null-terminated C string
const char *cstr(string *s) {
    // Make sure we have capacity for the null terminator
    if (s->len > s->cap) {
        string_arena__reserve(s, s->arena, s->len * 2);  // double capacity
    }
    
    // Add null terminator
    s->data[s->len] = '\0';
    
    return s->data;
}

// Free owned string
void string_free(string* s) {
    if (s->owned && !s->arena && s->data) {
        free(s->data);
        s->data = NULL;
        s->owned = false;
    }
}


// Split string into array
typedef struct {
    string* items;
    size_t count;
} string_array;

#define string_array_arena_from(a, array) string_array_arena_from_count(a, array, ARRAY_SIZE(array))
string_array string_array_arena_from_count(arena *a, const char* array[], int count) {
    string_array sa;
    sa.items = arena_alloc(a, sizeof(*sa.items) * count);
    for (int i = 0; i < count; i++) {
        sa.items[i] = string_from(array[i]);
    }
    sa.count = count;
    return sa;
}

#define string_array_from(array) string_array_from_count(array, ARRAY_SIZE(array))
string_array string_array_from_count(const char* array[], int count) {
    return string_array_arena_from_count(NULL, array, count);
}

string_array string_array_arena_view(arena *a, char* array, int count) {
    string_array sa;
    sa.items = arena_alloc(a, sizeof(*sa.items) * count);
    for (int i = 0; i < count; i++) {
        sa.items[i] = string_view(&array[i]);
    }
    sa.count = count;
    return sa;
}

string_array string_array_view(char* array, int count) {
    return string_array_arena_view(NULL, array, count);
}

string_array string_split(string s, const char* delimiter) {
    // Count occurrences
    size_t count = 1;
    char *tmp = s.data;
    while ((tmp = strstr(tmp, delimiter)) != NULL) {
        count++;
        tmp += strlen(delimiter);
    }
    
    string *items = malloc(sizeof(string) * count);
    size_t idx = 0;
    
    char *start = s.data;
    char *end;
    
    while ((end = strstr(start, delimiter)) != NULL) {
        items[idx++] = (string){
            .data = start,
            .len = end - start,
            .cap = 0,
            .owned = false
        };
        start = end + strlen(delimiter);
    }
    
    // Last piece
    items[idx++] = string_view(start);
    
    return (string_array){.items = items, .count = count};
}

void string_array_free(string_array* arr) {
    free(arr->items);
}

// Join strings
string string_join(string_array arr, const char* separator) {
    size_t total_len = 0;
    size_t sep_len = strlen(separator);
    
    for (size_t i = 0; i < arr.count; i++) {
        total_len += arr.items[i].len;
        if (i > 0) total_len += sep_len;
    }
    
    string result = string_new(total_len);
    
    for (size_t i = 0; i < arr.count; i++) {
        if (i > 0) string_append(&result, separator);
        string_append(&result, arr.items[i].data);
    }
    
    return result;
}
// ------------ STRING END


// Platform detection
#ifdef __APPLE__
    #define PLATFORM "macos"
    #define SO_EXT "dylib"
#elif defined(_WIN32)
    #define PLATFORM "windows"
    #define SO_EXT "dll"
#else
    #define PLATFORM "linux"
    #define SO_EXT "so"
#endif

#ifndef CC
    #if _WIN32
        #if defined(__GNUC__)
            #define CC "cc"
        #elif defined(__clang__)
            #define CC "clang"
        #elif defined(_MSC_VER)
            #define CC "cl.exe"
        #endif
    #else
        #define CC "cc"
    #endif
#endif

bool cmd_v(const char *fmt, va_list args) {
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    printf("%s\n", buffer);
    return system(buffer) == 0;
}

bool cmd(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = cmd_v(fmt, args);
    va_end(args);
    return result;
}

bool cmd_string(string fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = cmd_v(fmt.data, args);
    va_end(args);
    return result;
}

#define CMD(fmt, ...) _Generic((fmt), \
    char*: cmd, \
    const char*: cmd, \
    string: cmd_string, \
    default: cmd \
)(fmt, ##__VA_ARGS__)

static bool force_rebuild = false;

// Check if target needs rebuild
// TODO: target windows
bool needs_rebuild(const char *target, const char *source) {
    if (force_rebuild) return true;

    // printf("needs_rebuild %s, source=%s\n", target, source);
    struct stat target_stat, source_stat;
    
    if (stat(target, &target_stat) != 0) {
        return true; // Target doesn't exist
    }
    
    if (stat(source, &source_stat) != 0) {
        fprintf(stderr, "Error: source file '%s' not found\n", source);
        exit(1);
    }
    
    return source_stat.st_mtime > target_stat.st_mtime;
}

// Multi-source dependency checking
bool needs_rebuild_many(const char *target, string_array sources) {
    for (int i = 0; i < sources.count; i++) {
        if (needs_rebuild(target, sources.items[i].data)) {
            return true;
        }
    }
    return false;
}

#define REBUILD_SELF(argc, argv) rebuild_self(argc, argv, __FILE__)

// Auto-rebuild the build program
void rebuild_self(int argc, char *argv[], const char *source) {
    (void)argc;
    const char *self = argv[0];
    
    if (needs_rebuild(self, source)) {
        printf("Rebuilding program...\n");
        if (!cmd(CC " -o %s %s", self, source)) {
            exit(1);
        }
        
        // Re-execute
        execvp(self, argv);
        perror("execvp");
        exit(1);
    }
}
void rebuild_deps(int argc, char *argv[], string_array sources) {
    (void)argc;
    const char *self = argv[0];
    
    if (needs_rebuild_many(self, sources)) {
        printf("Rebuilding program...\n");
        if (!cmd(CC " -o %s %s", self, sources.items[0])) {
            exit(1);
        }
        
        // Re-execute
        execvp(self, argv);
        perror("execvp");
        exit(1);
    }
}

// Join sources into a single string
void join_strings(char *buffer, size_t size, const char *sources[], size_t sources_size) {
    buffer[0] = '\0';
    for (size_t i = 0; i < sources_size; i++) {
        strlcat(buffer, sources[i], size);
        strlcat(buffer, " ", size);
    }
}

int bin2c(char *bin_file, char *c_file, char *opt_name) {
    const char *name;
    FILE *input, *output;
    unsigned int length = 0;
    unsigned char data;

    input = fopen(bin_file, "rb");
    if (!input)
        return -1;

    output = fopen(c_file, "wb");
    if (!output) {
        fclose(input);
        return -1;
    }

    if (opt_name == NULL) {
        name = c_file;
    } else {
        size_t arglen = strlen(bin_file);
        name = bin_file;

        for (size_t i = 0; i < arglen; i++) {
            if (bin_file[i] == '.')
                bin_file[i] = '_';
            else if (bin_file[i] == '/')
                name = &bin_file[i+1];
        }
    }

    fprintf(output, "const unsigned char %s_data[] = { ", name);

    while (fread(&data, 1, 1, input) > 0) {
        if (length % 12 == 0) fprintf(output, "\n\t");
        fprintf(output, "0x%02x, ", data);
        length++;
    }

    fprintf(output, "0x00\n");
    fprintf(output, "};\n");
    fprintf(output, "const unsigned int %s_len = %u;\n\n", name, length);

    fclose(output);

    if (ferror(input) || !feof(input)) {
        fclose(input);
        return -1;
    }

    fclose(input);

    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // BASE_H
