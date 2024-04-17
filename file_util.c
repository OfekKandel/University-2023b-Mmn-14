#include "file_util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File functions --------------------------------------- */

char *with_ext(const char *filename, const char *extension) {
  /* Allocate correctly sized buffer for the new filename */
  int new_length = (strlen(filename) + strlen(extension) + 1);
  char *filepath = malloc(sizeof(char) * new_length);

  /* Join filename and extension */
  strcpy(filepath, filename);
  strcat(filepath, extension);

  return filepath;
}

FILE *open_with_ext(const char *filename, const char *extension, const char *mode,
                    const char *error_desc) {
  char *filepath = with_ext(filename, extension);
  FILE *file = fopen(filepath, mode);

  if (file == NULL) {
    printf("ERROR: Failed to open %s: '%s'\n", error_desc, filepath);
    free(filepath);
    return NULL;
  }

  free(filepath);
  return file;
}

int remove_file(const char *filename, const char *extension, const char *error_desc) {
  char *filepath = with_ext(filename, extension);

  if (remove(filepath) != 0) {
    printf("WARNING: Failed to delete %s: '%s'\n", error_desc, filepath);
    return false;
  };

  return true;
}

int is_file_empty(const char *filename, const char *extension, const char *error_desc) {
  FILE *file = open_with_ext(filename, extension, "r", error_desc);

  if (file == NULL) /* If a file can't be opened we assume it isn't empty */
    return false;

  /* Go to end of file and check if the position is zero */
  fseek(file, 0, SEEK_END);
  if (ftell(file) == 0) return true;
  return false;
}


/* Logger ----------------------------------------------- */
void print_log_context(LogContext log_info, char *severity) {
  printf("%s[%s%s - Line %d]: ", severity, log_info.filename, log_info.file_ext, log_info.line);
}
