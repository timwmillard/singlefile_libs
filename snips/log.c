#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
} LogLevel;

typedef struct {
    LogLevel level;
    FILE *file;
} LogConfig;

static LogConfig log_config = {0};

static const char* log_level_string(LogLevel level) {
    switch(level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default:        return "UNKNOWN";
    }
}

#define log_debug(msg, ...) log_msg(LOG_DEBUG, msg, ##__VA_ARGS__)
#define log_info(msg, ...) log_msg(LOG_INFO, msg, ##__VA_ARGS__)
#define log_warn(msg, ...) log_msg(LOG_WARN, msg, ##__VA_ARGS__)
#define log_error(msg, ...) log_msg(LOG_ERROR, msg, ##__VA_ARGS__)
#define log_fatal(msg, ...) log_msg(LOG_FATAL, msg, ##__VA_ARGS__)

void log_msg(LogLevel level, char *msg, ...)
{
    if (level < log_config.level) return;
    if (log_config.file == NULL) log_config.file = stdout;

    // Get current timestamp
    time_t now;
    struct tm *time_info;
    char timestamp[64];
    
    time(&now);
    time_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);

    char log_entry[2048];
    snprintf(log_entry, sizeof(log_entry), "%s [%s] %s\n", 
            timestamp, log_level_string(level), msg);
    
    va_list args;
    va_start(args, msg);
    vfprintf(log_config.file, log_entry, args);
    va_end(args);

    fflush(log_config.file);

    if (level == LOG_FATAL)
        exit(1);
}

// 0x16b00bf90: 34 33 38 65 30 61 61 34 34 64 37 34 30 39 37 66  |438e0aa44d74097f|
void hex_dump(FILE *f, const void *buffer, size_t size, size_t bytes_per_line) {
    const unsigned char *byte_ptr = (const unsigned char *)buffer;
    
    // Default to 16 bytes per line if not specified
    if (bytes_per_line == 0) {
        bytes_per_line = 16;
    }
    
    fprintf(f, "--------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < size; i += bytes_per_line) {
        // Print the offset/address
        fprintf(f, "%p: ", (void *)(byte_ptr + i));
        
        // Print hex representation
        for (size_t j = 0; j < bytes_per_line; j++) {
            if (i + j < size) {
                fprintf(f, "%02x ", byte_ptr[i + j]);
            } else {
                // Pad with spaces if we've reached the end of the buffer
                fprintf(f, "   ");
            }
        }
        
        // Print ASCII representation
        fprintf(f, " |");
        for (size_t j = 0; j < bytes_per_line; j++) {
            if (i + j < size) {
                // Print printable characters, use '.' for non-printable
                unsigned char c = byte_ptr[i + j];
                fprintf(f, "%c", (isprint(c) ? c : '.'));
            }
        }
        fprintf(f, "|\n");
    }
    fprintf(f, "--------------------------------------------------------------------------------\n");
}

// Macro version for more convenient usage
#define log_hex(buffer, size) hex_dump(log_config.file, buffer, size, 16)

