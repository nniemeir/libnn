#include "libnn.h"

const char *malloc_err = "Failed to allocate memory.";
const char *prepend_err = "Failed to prepend program data path to string.";

int file_exists(const char *filename) {
  struct stat buffer;
  return stat(filename, &buffer) == 0 ? 1 : 0;
}

char *get_file_extension(const char *file_path) {
  char *file_extension = strrchr(file_path, '.');
  if (!file_extension) {
    return NULL;
  }
  file_extension++;
  return file_extension;
}

unsigned char *read_file(const char *program_name, const char *file_path,
                         size_t *file_size) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    log_event(program_name, ERROR,
              "read_file failed to open the requested file.", log_to_file);
    return NULL;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    log_event(program_name, ERROR, "fseek failed.", log_to_file);
    fclose(file);
    return NULL;
  }

  long size = ftell(file);
  if (size == -1) {
    log_event(program_name, ERROR, "ftell failed.", log_to_file);
    fclose(file);
    return NULL;
  }
  *file_size = (size_t)size;

  rewind(file);

  unsigned char *buffer;
  buffer = (unsigned char *)malloc(*file_size);
  if (!buffer) {
    log_event(program_name, ERROR, "Failed to allocate memory for file buffer.",
              log_to_file);
    fclose(file);
    return NULL;
  }
  size_t bytes_read = fread(buffer, 1, *file_size, file);
  if (bytes_read != *file_size) {
    log_event(program_name, ERROR, "Error reading file into buffer.",
              log_to_file);
    free(buffer);
    fclose(file);
    return NULL;
  }
  fclose(file);
  return buffer;
}

int prepend_program_data_path(const char *program_name, char **path_buffer,
                              char *original_path) {
  const char *home = getenv("HOME");
  if (!home) {
    log_event(program_name, ERROR,
              "Failed to get value of HOME environment variable.", log_to_file);
    return 0;
  }
  if (!*path_buffer) {
    log_event(program_name, ERROR,
              "NULL pointer was passed to prepend_program_data_path.",
              log_to_file);
    return 0;
  }
  snprintf(*path_buffer, PATH_MAX, "%s/.local/share/%s/%s", home, program_name,
           original_path);
  return 1;
}

int construct_log_path(const char *program_name, char **path_buffer) {
  const char *home = getenv("HOME");
  if (!home) {
    log_event(program_name, ERROR,
              "Failed to get value of HOME environment variable.", log_to_file);
    return 0;
  }
  if (!*path_buffer) {
    log_event(program_name, ERROR,
              "NULL pointer was passed to construct_log_path.", log_to_file);
    return 0;
  }
  snprintf(*path_buffer, PATH_MAX, "%s/.local/state/%s", home, program_name);
  return 1;
}

int log_event(const char *program_name, int log_level, const char *msg,
              int log_to_file) {
  if (!msg) {
    fprintf(stderr, "NULL log message.\n");
    return 0;
  }
  if (msg[0] == '\0') {
    fprintf(stderr, "Empty log message.\n");
    return 0;
  }

  char *log_level_msg;
  switch (log_level) {
  case DEBUG:
    log_level_msg = "DEBUG";
    break;
  case INFO:
    log_level_msg = "INFO";
    break;
  case WARN:
    log_level_msg = "WARN";
    break;
  case ERROR:
    log_level_msg = "ERROR";
    break;
  case FATAL:
    log_level_msg = "FATAL";
    break;
  default:
    fprintf(stderr, "Invalid log level supplied.\n");
    return 0;
  }

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char formatted_msg[LOG_MAX];
  snprintf(formatted_msg, LOG_MAX, "[%d/%02d/%02d %02d:%02d:%02d] %s  %s\n",
           tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min,
           tm.tm_sec, log_level_msg, msg);

  if (log_level > 1) {
    fprintf(stderr, "%s", formatted_msg);
  } else {
    printf("%s", formatted_msg);
  }

  if (log_to_file) {
    char log_filename[NAME_MAX];
    snprintf(log_filename, NAME_MAX, "log_%d%02d%02d.txt", tm.tm_year + 1900,
             tm.tm_mon + 1, tm.tm_mday);
    char *path_buffer = malloc(PATH_MAX);
    if (!path_buffer) {
      fprintf(stderr, "Failed to allocate memory for path_buffer.\n");
      return 0;
    }

    if (!construct_log_path(program_name, &path_buffer)) {
      free(path_buffer);
      return 0;
    }

    char *log_path = malloc(PATH_MAX);
    if (!log_path) {
      fprintf(stderr, "Failed to allocate memory for log_path.\n");
      free(path_buffer);
      return 0;
    }

    if (!file_exists(path_buffer)) {
      if (mkdir(path_buffer, 0700) == -1) {
        fprintf(stderr, "Failed to make log directory.\n");
        free(path_buffer);
        return 0;
      }
    }

    snprintf(log_path, PATH_MAX, "%s/%s", path_buffer, log_filename);
    FILE *file = fopen(log_path, "a+");
    if (!file) {
      fprintf(stderr, "Failed to open log file.\n");
      free(path_buffer);
      return 0;
    }

    fprintf(file, "%s", formatted_msg);
    fclose(file);
    free(path_buffer);
  }

  return 1;
}