#include "preprocessor.h"
#include "macro_table.h"
#include "parse_util.h"
#include "parser.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MACRO_KEYWORD "mcr"
#define END_MACRO_KEYWORD "endmcr"

/* [DOCS NEEDED] */
static char *read_macro_declaration(char *line, LogContext context) {
  char *mcr_name;

  /* Check if the line's first word is 'mcr' */
  line = skip_space(line);
  if (strncmp(line, MACRO_KEYWORD, strlen(MACRO_KEYWORD)) != 0) return NULL;

  /* Check that mcr is followed by space */
  line += strlen(MACRO_KEYWORD);
  if (!isspace(line[0])) return NULL;

  /* Check that there is a macro name after mcr */
  line = skip_space(line);
  if (line[0] == '\n' || line[0] == EOF) {
    print_log_context(context, "ERROR");
    printf("Missing name after macro declaration\n");
    return NULL;
  }
  /* Scan the macro name */
  mcr_name = line;
  line = skip_to_space(line);
  line[0] = '\0';

  /* Check that there is no extraneous text */
  line = skip_space(++line);
  if (!is_terminator(line[0])) {
    print_log_context(context, "ERROR");
    printf("Extraneous text after macro declaration\n");
    return NULL;
  }

  return mcr_name;
}

/* [DOCS NEEDED] */
static int read_macro_termination(char *line, LogContext context) {
  /* Check if the line's first word is 'endmcr' */
  line = skip_space(line);
  if (strncmp(line, END_MACRO_KEYWORD, strlen(END_MACRO_KEYWORD)) != 0) return false;

  /* Check that there is no extraneous text */
  line += strlen(END_MACRO_KEYWORD);
  line = skip_space(line);
  if (!is_terminator(line[0])) {
    print_log_context(context, "ERROR");
    printf("Extraneous text after macro termination\n");
    return true; /* True but still prints an error of course */
  }

  return true;
}

/* [DOCS NEEDED] */
static MacroLines *read_macro_invocation(MacroTable *table, char *line, LogContext context) {
  char *mcr_name, original_char;
  MacroLines *lines_found;

  /* Read the line's first word */
  line = skip_space(line);
  mcr_name = line;
  line = skip_to_space(line);

  /* Check if the word is macro invocation */
  original_char = line[0];
  line[0] = '\0';
  lines_found = get_macro_lines(table, mcr_name);
  line[0] = original_char;

  if (!lines_found) return NULL;

  /* Check that there is no extraneous text */
  line = skip_space(line);
  if (!is_terminator(line[0])) {
    print_log_context(context, "ERROR");
    printf("Extraneous text after macro invocation\n");
  }

  return lines_found;
}

/* [DOCS NEEDED] */
static void write_macro_invocation(MacroLines *invoked_mcr, FILE *out_file) {
  MacroLineNode *line;
  for (line = invoked_mcr->head; line != NULL; line = line->next) {
    fputs(line->content, out_file);
  }
}

/* [DOCS NEEDED] returns whether a new file was created */
static int read_file(FILE *src_file, const char *src_path, FILE *out_file, const char *out_path,
                      LogContext context) {
  char line[MAX_LINE_LEN + 1], *mcr_name;
  int n_line = 1;
  MacroLines *active_mcr = NULL, *invoked_mcr;
  MacroTable table;
  table.head = NULL;

  while (fgets(line, sizeof(line), src_file) != NULL) {
    context.line = n_line;
    /* Check that line is under 80 chars */
    if (!is_terminator(line[strlen(line) - 1])) {
      print_log_context(context, "ERROR");
      printf("Line with over 80 characters found\n");
      return -1;
    }

    if (active_mcr != NULL) {
      /* If there is an active macro scan check if its over */
      if (read_macro_termination(line, context))
        active_mcr = NULL;
      else /* Else simply add the line to the macro */
        append_line(active_mcr, line);
    } else {
      /* Check if there is a macro starting line */
      mcr_name = read_macro_declaration(line, context);
      if (mcr_name) {
        /* If there is add it to the table (and start scanning) */
        /* TODO: Check that macro is not a saved word */
        active_mcr = insert_macro(&table, mcr_name);
      } else {
        /* Else check if a macro is being invoked */
        invoked_mcr = read_macro_invocation(&table, line, context);
        if (invoked_mcr) {
          write_macro_invocation(invoked_mcr, out_file);
        } else {
          fputs(line, out_file);
        }
      }
    }

    n_line++;
  }

  printf("Finished file %s\n", src_path);

  /* Remove .am file if no macros exist */
  if (table.head == NULL) {
    if (remove(out_path) != 0)
      printf("WARNING: Failed to delete preprocessor out file %s (not needed because no macros "
             "were found)\n",
             out_path);
    free_macro_table(table);
    return false;
  }

  free_macro_table(table);
  return true;
}

/* Global functions */

int process_file(char *filename) {
  char *src_file_path, *out_file_path;
  FILE *src_file, *out_file;
  LogContext context;
  int read_file_res;

  /* Open source file */
  src_file_path = with_ext(filename, ".as");
  src_file = fopen(src_file_path, "r");
  if (src_file == NULL) {
    printf("ERROR [preprocessor]: Failed to open assembly source file '%s'\n", src_file_path);
    return -1;
  }

  /* Open output file */
  out_file_path = with_ext(filename, PROCESSED_FILE_SUFFIX);
  out_file = fopen(out_file_path, "w");
  if (out_file == NULL) {
    printf("ERROR [preprocessor]: Failed to open assembly output file '%s'\n", out_file_path);
    return -1;
  }

  /* Process file */
  context.filename = filename;
  context.file_ext = ".as";
  read_file_res = read_file(src_file, src_file_path, out_file, out_file_path, context);

  /* Close, free and return */
  fclose(src_file);
  fclose(out_file);
  free(src_file_path);
  free(out_file_path);
  return read_file_res;
}
