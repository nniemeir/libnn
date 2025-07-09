#include "libnn.h"

const char *prepend_err = "Failed to prepend program data path to filename";

int file_exists(const char *filename) {
  struct stat buffer;
  int result = stat(filename, &buffer);
  if (result == -1) {
    fprintf(stderr, "Failed to get status of file %s: %s", filename,
            strerror(errno));
    return 0;
  }
  if (result == 0) {
    return 1;
  }
  return 0;
}

char *get_file_extension(const char *file_path) {
  char *file_extension = strrchr(file_path, '.');
  if (!file_extension) {
    return NULL;
  }
  file_extension++;
  return file_extension;
}

unsigned char *read_file(const char *file_path, size_t *file_size) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file %s:%s\n", file_path, strerror(errno));
    return NULL;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    fprintf(stderr, "Failed to set file position indicator: %s\n",
            strerror(errno));
    fclose(file);
    return NULL;
  }

  int size = ftell(file);
  if (size == -1) {
    fprintf(stderr, "Failed to get file position indicator: %s\n",
            strerror(errno));
    if (fclose(file) == -1) {
      fprintf(stderr, "Failed to close file %s:%s\n", file_path,
              strerror(errno));
      return NULL;
    }
  }

  *file_size = (size_t)size;

  rewind(file);

  unsigned char *buffer = (unsigned char *)malloc(*file_size);
  if (!buffer) {
    fprintf(stderr, "Failed to allocate memory for file buffer: %s\n",
            strerror(errno));

    if (fclose(file) == -1) {
      fprintf(stderr, "Failed to close file %s:%s\n", file_path,
              strerror(errno));
    }
    return NULL;
  }

  const size_t bytes_read = fread(buffer, 1, *file_size, file);

  if (bytes_read != *file_size) {
    log_event(ERROR, "Error reading file into buffer.", log_to_file);
    free(buffer);

    if (fclose(file) == -1) {
      fprintf(stderr, "Failed to close file %s:%s\n", file_path,
              strerror(errno));
    }

    return NULL;
  }

  if (fclose(file) == -1) {
    fprintf(stderr, "Failed to close file %s:%s\n", file_path, strerror(errno));
    return NULL;
  }

  return buffer;
}

int prepend_program_data_path(char **path_buffer, const char *original_path) {
  const char *home = getenv("HOME");
  if (!home) {
    log_event(ERROR, "Failed to get value of HOME environment variable.",
              log_to_file);
    return 0;
  }

  if (!*path_buffer) {
    log_event(ERROR, "NULL pointer was passed to prepend_program_data_path.",
              log_to_file);
    return 0;
  }

  snprintf(*path_buffer, PATH_MAX, "%s/.local/share/%s/%s", home, program_name,
           original_path);

  return 1;
}

int construct_log_path(char **path_buffer) {
  const char *home = getenv("HOME");
  if (!home) {
    log_event(ERROR, "Failed to get value of HOME environment variable.",
              log_to_file);
    return 0;
  }

  if (!*path_buffer) {
    log_event(ERROR, "NULL pointer was passed to construct_log_path.",
              log_to_file);
    return 0;
  }

  snprintf(*path_buffer, PATH_MAX, "%s/.local/state/%s", home, program_name);

  return 1;
}

const char *get_log_level_msg(const int log_level) {
  switch (log_level) {
  case DEBUG:
    return "DEBUG";
  case INFO:
    return "INFO";
  case WARN:
    return "WARN";
  case ERROR:
    return "ERROR";
  case FATAL:
    return "FATAL";
  default:
    fprintf(stderr, "Invalid log level supplied.\n");
    return NULL;
  }
}

void write_to_log_file(const char *formatted_msg, struct tm *tm) {
  char log_filename[NAME_MAX];

  snprintf(log_filename, NAME_MAX, "log_%d%02d%02d.txt", tm->tm_year + 1900,
           tm->tm_mon + 1, tm->tm_mday);

  char *path_buffer = malloc(PATH_MAX);
  if (!path_buffer) {
    fprintf(stderr, "Failed to allocate memory for path_buffer: %s\n",
            strerror(errno));
    return;
  }

  if (!construct_log_path(&path_buffer)) {
    free(path_buffer);
    return;
  }

  char *log_path = malloc(PATH_MAX);
  if (!log_path) {
    fprintf(stderr, "Failed to allocate memory for log_path: %s\n",
            strerror(errno));
    free(path_buffer);
    return;
  }

  if (!file_exists(path_buffer)) {
    if (mkdir(path_buffer, 0700) == -1) {
      fprintf(stderr, "Failed to make log directory: %s\n", strerror(errno));
      free(log_path);
      free(path_buffer);
      return;
    }
  }

  snprintf(log_path, PATH_MAX, "%s/%s", path_buffer, log_filename);
  FILE *file = fopen(log_path, "a+");
  if (!file) {
    fprintf(stderr, "Failed to open file %s:%s\n", log_path, strerror(errno));
    free(path_buffer);
    return;
  }

  fprintf(file, "%s", formatted_msg);

  if (fclose(file) == -1) {
    fprintf(stderr, "Failed to close file %s:%s\n", log_path, strerror(errno));
  }

  free(path_buffer);

  free(log_path);
}

void log_event(int log_level, const char *msg, int log_to_file) {
  if (!program_name) {
    fprintf(stderr, "NULL program name");
    return;
  }

  if (!msg) {
    fprintf(stderr, "NULL log message.\n");
    return;
  }

  if (msg[0] == '\0') {
    fprintf(stderr, "Empty log message.\n");
    return;
  }

  const char *log_level_msg = get_log_level_msg(log_level);
  if (!log_level_msg) {
    return;
  }

  const time_t t = time(NULL);
  if (t == -1) {
    fprintf(stderr, "Failed to get time: %s", strerror(errno));
  }

  struct tm tm = *localtime(&t);
  if (!tm.tm_hour) {
    fprintf(stderr, "Failed to get time: %s", strerror(errno));
    return;
  }

  char formatted_msg[LOG_MAX];
  snprintf(formatted_msg, LOG_MAX, "[%d/%02d/%02d %02d:%02d:%02d] %s  %s\n",
           tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min,
           tm.tm_sec, log_level_msg, msg);

  if (log_level > INFO) {
    fprintf(stderr, "%s", formatted_msg);
  } else {
    printf("%s", formatted_msg);
  }

  if (log_to_file) {
    write_to_log_file(formatted_msg, &tm);
  }
}
