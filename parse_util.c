#include "parse_util.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_NAME_LENGTH 3

/* [DOCS NEEDED] returns false on error, NULL to out on no label */
static int scan_label(char line[MAX_LINE_LEN], char **out) {
  char *word;
  char illegal_char = '\0';

  /* Scan label word end of label or command */
  word = line;
  while (!is_terminator(line[0]) && !isspace(line[0]) && line[0] != ':') {
    if (!isalpha(line[0]) && !isdigit(line[0])) illegal_char = line[0];
    line++;
  }

  /* Return NULL & true if its not a label */
  if (line[0] != ':') {
    *out = NULL;
    return true;
  }

  /* Print illegal character error & return false if needed */
  if (illegal_char) {
    printf("ERROR: Illegal character '%c' in label name: '%.*s'\n", illegal_char,
           (int)(line - word + 1), word);
    return false;
  }

  /* Return label */
  line[0] = '\0';
  *out = word;
  return true;
}

/* [DOCS NEEDED] */
static LineType get_line_content_type(char content[]) {
  char *word;

  /* Skip space */
  content = skip_space(content);

  /* Return Command if its not a dot-command */
  if (content[0] != '.') return Command;

  /* Scan first word (without dot)*/
  word = ++content;
  content = skip_to_space(content);
  content[0] = '\0';

  /* Return Define if its a define command */
  if (strcmp(word, "define") == 0) return Define;

  /* Else its a dot-instruction */
  return DotInstruction;
}

int scan_argument(char content[], char separator) {
  char *start = content;

  /* Return errors if there is no argument */
  if (content == NULL || is_terminator(content[0])) return -5; /* -5: No argument (only space) */
  if (content[0] == separator) return -1; /* -1: Missing first argument (starts with separator) */

  /* Progress until space/separator/terminator (this is where the argument is)*/
  while (!isspace(content[0]) && content[0] != separator && !is_terminator(content[0]))
    content++;

  /* If there is space, terminate the word and skip it */
  if (isspace(content[0])) {
    content[0] = '\0';
    content = skip_space(++content);
  }

  /* Return if there is no text after the argument */
  if (is_terminator(content[0])) {
    content[0] = '\0';
    return 0; /* 0: Last argument */
  }

  /* Return error if text was found instead of a separator */
  if (content[0] != separator) return -2; /* -2: Missing separator between arguments */

  /* Terminate word where the separator is and skip following space */
  content[0] = '\0';
  content = skip_space(++content);

  /* Return error if there is an extra separator */
  if (content[0] == separator) return -3; /* -3: Two consecutive separator */

  /* Return error if there is not another argument following the separator */
  if (is_terminator(content[0])) return -4; /* -4: Trailing separator */

  /* Return number of chars to skip to next argument */
  return content - start;
}

/* [DOCS NEEDED] */
static void scan_define(char content[], ParsedLine *out) {
  char *word;
  int scan_result;

  /* Print error and return if there is a label */
  if (out->content.command.label != NULL) {
    printf("ERROR: A label is meaningless before a constant defenition and is not allowed\n");
    out->line_type = Error;
    return;
  }

  /* Skip space, ".define" word, and following space */
  content = skip_space(content);
  content += strlen(".define") + 1;
  content = skip_space(content);

  /* Scan first argument */
  word = content;
  scan_result = scan_argument(content, '=');

  /* Handle errors */
  if (scan_result <= 0) {
    switch (scan_result) {
    case 0:  /* No text after argument */
    case -1: /* No text */
    case -5: /* No argument */
    case -4: /* Trailing = */
      printf("ERROR: Missing argument in constant defenition\n");
      break;
    case -2: /* Missing = */
    case -3: /* Double = */
      printf("ERROR: In a constant defenition, the constant and value are seperated by a signle "
             "'=' sign \n");
      break;
    }
    out->line_type = Error;
    return;
  }

  /* Scan second argument */
  out->content.define.name = word;
  content += scan_result;
  word = content;
  scan_result = scan_argument(content, '=');

  if (scan_result != 0) {
    /* Note: the only case where scan_result != 0 is when there is extra text of some kind */
    printf("ERROR: Extraneous text after constant defenition\n");
    out->line_type = Error;
    return;
  }

  out->content.define.value = word;
}

static void scan_command(char content[], ParsedLine *out) {
  char *word;
  int scan_result;

  /* Scan command name */
  content = skip_space(content);
  word = content;
  content += CMD_NAME_LENGTH;

  /* Print error on bad name (too long) */
  if (!isspace(content[0]) && !is_terminator(content[0])) {
    printf("ERROR: Invalid command name\n");
    out->line_type = Error;
    return;
  }

  content[0] = '\0';
  out->content.command.cmd = word;

  /* Scan first argument */
  content = skip_space(++content);
  word = content;
  scan_result = scan_argument(content, ',');

  if (scan_result < 0) {
    switch (scan_result) {
    case -5: /* Return on no arguments (no text)*/
      return;
    case -1: /* No argument */
      printf("ERROR: Missing first argument of command invocation\n");
      break;
    case -2: /* Missing , */
    case -3: /* Double , */
      printf("ERROR: Arguments in command invocation are seperated by a signle comma\n");
      break;
    case -4: /* Trailing , */
      printf("ERROR: Missing second argument in command invocation\n");
      break;
    }
    out->line_type = Error;
    return;
  }

  out->content.command.dest_arg = word;

  /* Return if there is only one argument */
  if (scan_result == 0) return;

  /* Scan second argument */
  content += scan_result;
  word = content;
  scan_result = scan_argument(content, ',');

  /* Print error and return if there is extraneous text */
  if (scan_result != 0) {
    /* Note: the only case where scan_result != 0 is when there is extra text of some kind */
    printf("ERROR: Extraneous text after command invocation\n");
    out->line_type = Error;
    return;
  }

  /* Set first argument to be source and second to be destination */
  out->content.command.src_arg = out->content.command.dest_arg;
  out->content.command.dest_arg = word;
}

/* [DOCS NEEDED] */
static void scan_dot_instruction(char content[], ParsedLine *out) {
  char *word;

  /* Skip space and initial dot */
  content = skip_space(content);
  content += 1;

  /* Scan dot-command name */
  word = content;
  content = skip_alpha(content);

  /* Print error on bad name */
  if (!isspace(content[0]) && !is_terminator(content[0])) {
    printf("ERROR: Invalid dot-command name\n");
    out->line_type = Error;
    return;
  }

  content[0] = '\0';
  out->content.dot_instruction.name = word;
  content = skip_space(++content);

  if (is_terminator(content[0])) {
    out->content.dot_instruction.args_start = NULL;
    return;
  }

  out->content.dot_instruction.args_start = content;
}

/* Main parsing function -------------- */

ParsedLine parse_line(char line[MAX_LINE_LEN]) {
  ParsedLine out;
  char *label;

  /* Set everything to 0/NULL for convenience */
  memset(&out, 0, sizeof(ParsedLine));

  /* Skip space */
  line = skip_space(line);

  /* Return if line is empty */
  if (is_terminator(line[0])) {
    out.line_type = Empty;
    return out;
  } else if (line[0] == ';') {
    out.line_type = Comment;
    return out;
  }

  /* Read label if there is one and return on error */
  if (!scan_label(line, &label)) {
    out.line_type = Error;
    return out;
  }

  out.content.command.label = label;
  if (label) line += strlen(label) + 1;

  /* Parse rest of command by type */
  out.line_type = get_line_content_type(line);
  switch (out.line_type) {
  case Define:
    scan_define(line, &out);
    break;
  case DotInstruction:
    scan_dot_instruction(line, &out);
    break;
  case Command:
    scan_command(line, &out);
    break;
  default: /* Exit on error (comment/empty already handled) */
    return out;
  }

  /* Return */
  return out;
}

/* File functions ----------- */

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

/* General parsing functions ------------- */

int scan_string(char content[]) {
  char *init = content;
  char *str_start;

  /* Return error if there is no content */
  if (content == NULL) return -1; /* -1: No string */

  /* Skip space */
  content = skip_space(content);

  /* Return error if there is no string */
  if (is_terminator(content[0])) return -1; /* -1: No string (only space) */

  /* Return error if a quotation mark is missing */
  if (content[0] != '"') return -2; /* -2: Missing opening quotation mark */
  str_start = ++content;

  /* Progress until next quotation mark */
  while (content[0] != '"' && !is_terminator(content[0]))
    content++;

  /* Return error if there is no closing quotation mark  */
  if (is_terminator(content[0])) return -3; /* -3: Missing closing quotation mark */

  /* Terminate word where the separator is and skip following space */
  content[0] = '\0';
  content = skip_space(++content);

  /* Return error if there is text after the string */
  content = skip_space(content);
  if (!is_terminator(content[0])) return -4; /* -4: Extraneous text after string declaration */

  /* Return number of chars to skip to start of string */
  return str_start - init;
}

int scan_number(char *text, int *out) {
  int is_negative = false;
  *out = 0;
  text = skip_space(text);

  if (text[0] != '+' && text[0] != '-' && !isdigit(text[0])) {
    printf("ERROR: Number must start with '+', '-', or a digit\n");
    return false;
  }

  if (text[0] == '-') is_negative = true;

  if (!isdigit(text[0])) text++;

  /* Load all digits into out */
  for (; isdigit(text[0]); text++, *out *= 10) {
    *out += text[0] - '0';
  }
  *out /= 10;

  if (!is_terminator(text[0]) && !isspace(text[0])) {
    printf("ERROR: Number must only contain '+', '-' (at the start), or digits\n");
    return false;
  }

  if (is_negative) *out *= -1;

  return true;
}

char *scan_array_index(char content[]) {
  char *index;

  /* Scan until opening bracket */
  while (!is_terminator(content[0]) && content[0] != '[')
    content++;

  /* Print error and return if there is no opening bracket */
  if (is_terminator(content[0])) {
    printf("ERROR: To index an array a position must be provided within square brackets\n");
    return NULL;
  }

  /* Terminate symbol, and scan until closing bracket */
  content[0] = '\0';
  index = ++content;
  while (!is_terminator(content[0]) && content[0] != ']')
    content++;

  /* Print error if there is no closing bracket */
  if (is_terminator(content[0])) {
    printf("ERROR: Missing closing bracket in indexed array \n");
    return NULL;
  }
  content[0] = '\0';
  content++;

  /* Check that there is no text after the closing bracket */
  if (content[0] != '\0') {
    printf("ERROR: Extraneous text after indexed array argument\n");
    return NULL;
  }

  return index;
}

/* Smaller parsing functions --------------- */

int is_register_name(char *arg) {
  /* Matches argument of form r0 to r7 */
  return arg[0] == 'r' && isdigit(arg[1]) && arg[1] - '0' <= 7 && arg[2] == '\0';
}

char *skip_space(char *str) {
  while (isspace(str[0]))
    str++;
  return str;
}

char *skip_to_space(char *str) {
  while (!is_terminator(str[0]) && !isspace(str[0]))
    str++;
  return str;
}

char *skip_alpha(char *str) {
  while (isalpha(str[0]))
    str++;
  return str;
}

int is_terminator(char c) { return c == '\n' || c == '\0' || c == EOF; }
