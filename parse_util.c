#include "parse_util.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Command parse_line(char line[MAX_LINE_LEN]) {
  /* TODO: This function */
}

char *with_ext(const char *filename, const char *extension) {
  /* Allocate correctly sized buffer for the new filename */
  int new_length = (strlen(filename) + strlen(extension) + 1);
  char *filepath = malloc(sizeof(char) * new_length);

  /* Join filename and extension */
  strcpy(filepath, filename);
  strcat(filepath, extension);

  return filepath;
}

char *skip_space(char *str) {
  while (isspace(str[0]))
    str++;
  return str;
}

char *skip_to_space(char *str) {
  while (!is_terminatior(str[0]) && !isspace(str[0]))
    str++;
  return str;
}

char *skip_alpha(char *str) {
  while (isalpha(str[0]))
    str++;
  return str;
}

int is_terminatior(char c) {
  return c == '\n' || c == '\0' || c == EOF;
}
