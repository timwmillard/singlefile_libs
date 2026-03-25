
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "bash.h"

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
