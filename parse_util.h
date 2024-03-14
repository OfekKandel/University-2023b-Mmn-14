/* [DOCS NEEDED] */
#pragma once

#define MAX_LINE_LEN 80

typedef struct {
  char *label;
  char *command;
  char *arg_1;
  char *arg_2;
} Command;

/* [DOCS NEEDED] */
Command parse_line(char line[MAX_LINE_LEN]);

/* Given a filename (with no extension) and an extension, adds the extension to
 * the filename. Returns the new filename, memory returned by this function must
 * be freed) */
char *with_ext(const char *filename, const char *extension);

/* Smaller functions */

/* Return a pointer to the next position that isn't a letter in a string */
char *skip_alpha(char *str);
/* Return a pointer to the next position that isn't a space in a string */
char *skip_space(char *str);
/* [DOCS NEEDED] */
char *skip_to_space(char *str);
/* [DOCS NEEDED] */
int is_terminatior(char c);
