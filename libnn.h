#ifndef LIBNN_H
#define LIBNN_H
#include <linux/limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define NULL_TERMINATOR_LENGTH 1
#define LOG_MAX 1024

extern const char *program_name;
extern const char *prepend_err;
extern int log_to_file;

// File Operations
int file_exists(const char *filename);
char *get_file_extension(const char *file_path);
unsigned char *read_file(const char *file_path, size_t *file_size);

// Logging
enum levels { DEBUG, INFO, WARN, ERROR, FATAL };
void log_event(int log_level, const char *msg, int log_to_file);

// String Operations
int prepend_program_data_path(char **path_buffer, const char *original_path);

#endif
