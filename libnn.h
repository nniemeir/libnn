#ifndef LIBNN_H
#define LIBNN_H
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define NULL_TERMINATOR_LENGTH 1
#define LOG_MAX 1024

extern int log_to_file;

extern const char *program_name;

extern const char *malloc_err;

extern const char *prepend_err;

// File Operations
int file_exists(const char *filename);
char *get_file_extension(const char *file_path);
unsigned char *read_file(const char *program_name, const char *file_path,
                         size_t *file_size);

// Logging
enum levels { DEBUG, INFO, WARN, ERROR, FATAL };
int log_event(const char *program_name, int log_level, const char *msg,
              int log_to_file);

// String Operations
int prepend_program_data_path(const char *program_name, char **path_buffer,
                              char *original_path);

#endif