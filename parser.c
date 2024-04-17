#include "parser.h"
#include "parse_util.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define CMD_NAME_LENGTH 3

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

/* [DOCS NEEDED] returns false on error, NULL to out on no label */
static int scan_label(char line[MAX_LINE_LEN], char **out, LogContext context) {
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
    print_log_context(context, "ERROR");
    printf("Illegal character '%c' in label name: '%.*s'\n", illegal_char, (int)(line - word + 1),
           word);
    return false;
  }

  /* Print error if a macro is a reserved word */
  line[0] = '\0';
  if (is_reserved_word(word)) {
    print_log_context(context, "ERROR");
    printf("'%s' is an illegal label as it is a reserved word\n", word);
  }

  *out = word;
  return true;
}

/* [DOCS NEEDED] */
static void scan_define(char content[], ParsedLine *out, LogContext context) {
  char *word;
  ScanArgumentResult scan_result;

  /* Print error and return if there is a label */
  if (out->content.command.label != NULL) {
    print_log_context(context, "ERROR");
    printf("A label is meaningless before a constant defenition and is not allowed\n");
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
  if (scan_result.status != ArgumentScanShouldSkip) {
    switch (scan_result.status) {
    case LastArgument:
    case ArgumentScanNoContent:
    case MissingFirstArg:
    case TrailingSeprator:
      print_log_context(context, "ERROR");
      printf("Missing argument in constant defenition\n");
      break;
    case MissingSeparator:
    case DoubleSeparator:
      print_log_context(context, "ERROR");
      printf("In a constant defenition, the constant and value are seperated by a signle "
             "'=' sign \n");
      break;
    default:
      break;
    }
    out->line_type = Error;
    return;
  }

  /* Scan second argument */
  out->content.define.name = word;
  content += scan_result.skip;
  word = content;
  scan_result = scan_argument(content, '=');

  if (scan_result.status != LastArgument) {
    print_log_context(context, "ERROR");
    printf("Extraneous text after constant defenition\n");
    out->line_type = Error;
    return;
  }

  out->content.define.value = word;
}

static void scan_command(char content[], ParsedLine *out, LogContext context) {
  char *word;
  ScanArgumentResult scan_result;

  content = skip_space(content);

  /* Print if there is no command */
  if (is_terminator(content[0])) {
    print_log_context(context, "ERROR");
    printf("Expected a command, but found nothing\n");
    out->line_type = Error;
    return;
  }

  /* Scan command name */
  word = content;
  content += CMD_NAME_LENGTH;

  /* Print error on bad name (too long) */
  if (!isspace(content[0]) && !is_terminator(content[0])) {
    print_log_context(context, "ERROR");
    printf("Invalid command name\n");
    out->line_type = Error;
    return;
  }

  content[0] = '\0';
  out->content.command.cmd = word;

  /* Scan first argument */
  content = skip_space(++content);
  word = content;
  scan_result = scan_argument(content, ',');

  if (scan_result.status != ArgumentScanShouldSkip && scan_result.status != LastArgument) {
    if (scan_result.status == ArgumentScanNoContent) return; /* Return on no arguments (no text)*/
    print_log_context(context, "ERROR");
    switch (scan_result.status) {
    case MissingFirstArg:
      printf("Missing first argument of command invocation\n");
      break;
    case MissingSeparator:
    case DoubleSeparator:
      printf("Arguments in command invocation are seperated by a signle comma\n");
      break;
    case TrailingSeprator:
      printf("Missing second argument in command invocation\n");
      break;
    default:
      break;
    }
    out->line_type = Error;
    return;
  }

  out->content.command.dest_arg = word;

  /* Return if there is only one argument */
  if (scan_result.status == LastArgument) return;

  /* Scan second argument */
  content += scan_result.skip;
  word = content;
  scan_result = scan_argument(content, ',');

  /* Print error and return if there is extraneous text */
  if (scan_result.status != LastArgument) {
    print_log_context(context, "ERROR");
    printf("Extraneous text after command invocation\n");
    out->line_type = Error;
    return;
  }

  /* Set first argument to be source and second to be destination */
  out->content.command.src_arg = out->content.command.dest_arg;
  out->content.command.dest_arg = word;
}

/* [DOCS NEEDED] */
static void scan_dot_instruction(char content[], ParsedLine *out, LogContext context) {
  char *word;

  /* Skip space and initial dot */
  content = skip_space(content);
  content += 1;

  /* Scan dot-command name */
  word = content;
  content = skip_alpha(content);

  /* Print error on bad name */
  if (!isspace(content[0]) && !is_terminator(content[0])) {
    print_log_context(context, "ERROR");
    printf("Invalid dot-command name\n");
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

ParsedLine parse_line(char line[MAX_LINE_LEN], LogContext context) {
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
  }
  if (line[0] == ';') {
    out.line_type = Comment;
    return out;
  }

  /* Read label if there is one and return on error */
  if (!scan_label(line, &label, context)) {
    out.line_type = Error;
    return out;
  }

  out.content.command.label = label;
  if (label) line += strlen(label) + 1;

  /* Parse rest of command by type */
  out.line_type = get_line_content_type(line);
  switch (out.line_type) {
  case Define:
    scan_define(line, &out, context);
    break;
  case DotInstruction:
    scan_dot_instruction(line, &out, context);
    break;
  case Command:
    scan_command(line, &out, context);
    break;
  default: /* Exit on error (comment/empty already handled) */
    return out;
  }

  /* Return */
  return out;
}
