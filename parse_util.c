#include "parse_util.h"
#include "file_util.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ScanArgumentResult scan_argument(char content[], char separator) {
  char *start = content;
  ScanArgumentResult res;
  res.status = ArgumentScanShouldSkip;

  /* Return errors if there is no argument */
  if (content == NULL || is_terminator(content[0])) {
    res.status = ArgumentScanNoContent;
    return res;
  }
  if (content[0] == separator) {
    res.status = MissingFirstArg;
    return res;
  }

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
      res.status = LastArgument;
      return res;
  }

  /* Return error if text was found instead of a separator */
  if (content[0] != separator) {
    res.status = MissingSeparator;
    return res;
  }

  /* Terminate word where the separator is and skip following space */
  content[0] = '\0';
  content = skip_space(++content);

  /* Return error if there is an extra separator */
  if (content[0] == separator) {
    res.status = DoubleSeparator;
    return res;
  }

  /* Return error if there is not another argument following the separator */
  if (is_terminator(content[0])) {
    res.status = TrailingSeprator;
    return res;
  }

  /* Return number of chars to skip to next argument */
  res.skip = content - start;
  return res;
}

ScanStringResult scan_string(char content[]) {
  char *init = content;
  char *str_start;
  ScanStringResult res;
  res.status = StringScanShouldSkip;

  /* Return error if there is no content */
  if (content == NULL) {
    res.status = SrtingScanNoContent;
    return res;
  }

  /* Skip space */
  content = skip_space(content);

  /* Return error if there is no string */
  if (is_terminator(content[0])) {
    res.status = SrtingScanNoContent;
    return res;
  }

  /* Return error if a quotation mark is missing */
  if (content[0] != '"') {
    res.status = MissingOpeningQuotation;
    return res;
  }
  str_start = ++content;

  /* Progress until next quotation mark */
  while (content[0] != '"' && !is_terminator(content[0]))
    content++;

  /* Return error if there is no closing quotation mark  */
  if (is_terminator(content[0])) {
    res.status = MissingClosingQuotation;
    return res;
  }

  /* Terminate word where the separator is and skip following space */
  content[0] = '\0';
  content = skip_space(++content);

  /* Return error if there is text after the string */
  content = skip_space(content);
  if (!is_terminator(content[0])) {
    res.status = ExtraneousText;
    return res;
  }

  /* Return number of chars to skip to start of string */
  res.skip = str_start - init;
  return res;
}

int scan_number(char *text, int *out, LogContext context) {
  int is_negative = false;
  *out = 0;
  text = skip_space(text);

  if (text[0] != '+' && text[0] != '-' && !isdigit(text[0])) {
    print_log_context(context, "ERROR");
    printf("Number must start with '+', '-', or a digit\n");
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
    print_log_context(context, "ERROR");
    printf("Number must only contain '+', '-' (at the start), or digits\n");
    return false;
  }

  if (is_negative) *out *= -1;

  return true;
}

char *scan_array_index(char content[], LogContext context) {
  char *index;

  /* Scan until opening bracket */
  while (!is_terminator(content[0]) && content[0] != '[')
    content++;

  /* Print error and return if there is no opening bracket */
  if (is_terminator(content[0])) {
    print_log_context(context, "ERROR");
    printf("To index an array a position must be provided within square brackets\n");
    return NULL;
  }

  /* Terminate symbol, and scan until closing bracket */
  content[0] = '\0';
  index = ++content;
  while (!is_terminator(content[0]) && content[0] != ']')
    content++;

  /* Print error if there is no closing bracket */
  if (is_terminator(content[0])) {
    print_log_context(context, "ERROR");
    printf("Missing closing bracket in indexed array \n");
    return NULL;
  }
  content[0] = '\0';
  content++;

  /* Check that there is no text after the closing bracket */
  if (content[0] != '\0') {
    print_log_context(context, "ERROR");
    printf("Extraneous text after indexed array argument\n");
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
