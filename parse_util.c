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
    if (!isalpha(line[0]) && !isdigit(line[0]))
      illegal_char = line[0];
    line++;
  }

  /* Return NULL & true if its not a label */
  if (line[0] != ':') {
    *out = NULL;
    return true;
  }

  /* Print illegal character error & return false if needed */
  if (illegal_char) {
    printf("ERROR: Illegal character '%c' in label name: '%.*s'\n",
           illegal_char, (int)(line - word + 1), word);
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

  /* Check if its not a dot-command */
  if (content[0] != '.')
    return Command;

  /* Scan first word (without dot)*/
  word = ++content;
  content = skip_to_space(content);
  content[0] = '\0';

  /* Check against if its a define command */
  if (strcmp(word, "define") == 0)
    return Define;

  return DotInstruction;
}

/* [DOCS NEEDED] scans current argument until a separator, returns the address
 * where the separator *should* be, if there is a following separator this will
 * be the separator, if there was no separator it will be a some other
 * character, if there is no text at all after the argument it will be a
 * terminator */
static char *scan_to_separator(char content[], char separator) {
  /* Scan argument */
  while (content[0] != separator && !is_terminator(content[0]) && !isspace(content[0]))
    content++;

  /* Terminate argument and continue to separator if there was space */
  if (isspace(content[0])) {
    content[0] = '\0';
    content = skip_space(++content);
  }

  /* Terminate argument properly as long as there is some terminator there */
  if (is_terminator(content[0]))
    content[0] = '\0';

  return content;
}

/* [DOCS NEEDED] returns address to next argument if there is one, NULL if there
 * is no separator and there's another argument, a terminator if there is
 * no text after the argument, or after the separator (while a little strange,
 * this is convenient for the different cases of error handling)*/
static char *scan_argument_and_separator(char content[], char separator) {
  /* Scan until the separator (as described in function docs) */
  content = scan_to_separator(content, separator);

  /* Return a terminator if there is no text after the argument */
  if (is_terminator(content[0]))
    return content;

  /* Return NULL if there is no following separator */
  if (content[0] != separator)
    return NULL;

  /* Terminate argument at separator and skip to the next one */
  content[0] = '\0';
  content = skip_space(++content);

  /* Return the next argument (could be terminator, or separator, as well) */
  return content;
}

/* [DOCS NEEDED] */
static void scan_define(char content[], ParsedLine *out) {
  char *word;

  /* Print warning if there is a label */
  if (out->content.command.label != NULL)
    printf("WARNING: A label is meaningless before a constant defenition\n");

  /* Skip space, ".define" word, and following space */
  content = skip_space(content);
  content += strlen(".define") + 1;
  content = skip_space(content);

  /* Print error if there is no argument */
  if (is_terminator(content[0]) || content[0] == '=') {
    printf("ERROR: Missing argument(s) in constant defenition\n");
    out->line_type = Error;
    return;
  }

  /* Scan first argument (and handle error) */
  word = content;
  content = scan_argument_and_separator(content, '=');

  /* Print error if there is no = sign */
  if (content == NULL) {
    printf("ERROR: Missing '=' sign in constant defenition\n");
    out->line_type = Error;
    return;
  }

  /* Print error if there is no second argument */
  if (is_terminator(content[0])) {
    printf("ERROR: Missing second argument in constant defenition\n");
    out->line_type = Error;
    return;
  }

  out->content.define.name = word;

  /* Scan second argument */
  word = content;
  content = scan_to_separator(content, '=');
  out->content.define.value = word;

  /* Print error if there is extraneous text */
  if (!is_terminator(content[0])) {
    printf("ERROR: Extraneous text after constant defenition\n");
    out->line_type = Error;
    return;
  }
}

static void scan_command(char content[], ParsedLine *out) {
  char *word;

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
  content = skip_space(++content);

  /* If there's no first argument return */
  if (is_terminator(content[0]))
    return;

  /* Print error if there is a comma but no argument */
  if (content[0] == ',') {
    printf("ERROR: Missing argument in command invocation\n");
    out->line_type = Error;
    return;
  }

  /* Scan first argument (and handle error) */
  word = content;
  content = scan_to_separator(content, ',');

  /* Return if there is only one argument */
  if (is_terminator(content[0])) {
    content[0] = '\0';
    out->content.command.arg1 = word;
    return;
  }

  /* Print error if there is no comma */
  if (content[0] != ',') {
    printf("ERROR: Missing comma in command invocation\n");
    out->line_type = Error;
    return;
  }

  /* Terminate word and skip to next argument */
  content[0] = '\0';
  out->content.command.arg1 = word;
  content = skip_space(++content);

  /* Print error if there is no second argument */
  if (is_terminator(content[0])) {
    printf("ERROR: Missing second argument in command invocation\n");
    out->line_type = Error;
    return;
  }

  /* Scan second argument */
  word = content;
  content = scan_to_separator(content, ',');
  out->content.command.arg2 = word;

  /* Print error if there is extraneous text */
  if (!is_terminator(content[0])) {
    printf("ERROR: Extraneous text after command invocation\n");
    out->line_type = Error;
    return;
  }
}

/* [DOCS NEEDED] */
static void scan_dot_instruction(char content[], ParsedLine *out) {
  char *word;
  int n_args = 1;

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

ParsedLine parse_line(char line[MAX_LINE_LEN]) {
  ParsedLine out;
  char *label;

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
  if (label)
    line += strlen(label) + 1;

  /* Parse rest of command by type */
  out.line_type = get_line_content_type(line);
  switch (out.line_type) {
  case Error: /* Exit on error */
    return out;
  case Define:
    scan_define(line, &out);
    break;
  case DotInstruction:
    scan_dot_instruction(line, &out);
    break;
  case Command:
    scan_command(line, &out);
    break;
  default: /* Can't happen, but compiler complains */
    break;
  }

  /* Return */
  return out;
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

int scan_number(char *text, int *out) {
  int is_negative = false;
  text = skip_space(text);

  if (text[0] != '+' && text[0] != '-' && !isdigit(text[0])) {
    printf("ERROR: Number must start with '+', '-', or a digit\n");
    return false;
  }

  if (text[0] == '-')
    is_negative = true;

  if (!isdigit(text[0]))
    text++;

  /* Load all digits into out */
  for (; isdigit(text[0]); text++, *out *= 10) {
    *out += text[0] - '0';
  }
  *out /= 10;

  if (!is_terminator(text[0]) && !isspace(text[0])) {
    printf(
        "ERROR: Number must only contain '+', '-' (at the start), or digits\n");
    return false;
  }

  if (is_negative)
    *out *= -1;

  return true;
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
