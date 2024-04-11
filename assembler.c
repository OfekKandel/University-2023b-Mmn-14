/* [DOCS NEEDED] */
#include "assembler.h"
#include "binary_table.h"
#include "parse_util.h"
#include "symbol_table.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* [DOCS NEEDED] returns whether the operation was successful */
static int add_define_symbol(DefineLine line, SymbolTable *table) {
  int value = 0;

  /* Scan value number and return on error */
  if (!scan_number(line.value, &value))
    return false;

  /* Add symbol to table */
  if (!append_symbol(table, line.name, 'm', value)) {
    printf("ERROR: Attempt to redefine a constant\n");
    return false;
  }

  return true;
}

/* [DOCS NEEDED] returns how much to skip, 0 if scan should stop, -1 if there
 * were errors */
static int add_data_argument(char *content, BinaryTable *data_table,
                             SymbolTable *symbol_table) {
  int res = scan_argument(content, ',');

  /* TODO: Better as switch? Better to extract the return? */
  if (res == -5) {
    printf("ERROR: A data instruction must contain at least one argument\n");
    return -1;
  }
  if (res == -1) {
    printf("ERROR: A data instruction must start with an argument\n");
    return -1;
  }
  if (res == -2) {
    printf("ERROR: Two arguments in a data instruction must be separated by a "
           "comma\n");
    return -1;
  }
  if (res == -3) {
    printf("ERROR: Two consecutive commas between arguments in data "
           "instruction\n");
    return -1;
  }
  if (res == -4) {
    printf("ERROR: Trailing comma in data instruction\n");
    return -1;
  }

  /* TODO: Parse argument, handle error, and add it to table */

  return res; /* Return chars to skip */
}

/* [DOCS NEEDED] return whether the data instruction was scanned successfully */
static int add_data_instruction(DotInstructionLine line,
                                BinaryTable *data_table,
                                SymbolTable *symbol_table) {
  char *content = line.args_start;
  int skip;

  while ((skip = add_data_argument(content, data_table, symbol_table)) > 0)
    content += skip;

  if (skip == -1)
    return false;

  return true;
}

/* [DOCS NEEDED] */
static int add_dot_instruction(DotInstructionLine line, BinaryTable *data_table,
                               SymbolTable *symbol_table) {
  if (strcmp(line.name, "data") == 0) {
    return add_data_instruction(line, data_table, symbol_table);
  }

  /* TODO: Implement other dot instructions */

  printf("ERROR: Invalid dot instruction name\n");
  return false;
}

/* [DOCS NEEDED] returns whether the line was parsed successfully */
static int first_pass_line(char *line, SymbolTable *symbol_table,
                           BinaryTable *instruction_table,
                           BinaryTable *data_table) {
  ParsedLine parsed_line = parse_line(line);
  switch (parsed_line.line_type) {
  case Error:
  case Empty:
  case Comment:
    break;
  case Define:
    return add_define_symbol(parsed_line.content.define, symbol_table);
    break;
  case DotInstruction:
    return add_dot_instruction(parsed_line.content.dot_instruction, data_table,
                               symbol_table);
    break;
  case Command:
    /* TODO: */
    break;
  }

  return true;
}

/* [DOCS NEEDED] returns whether the pass was successful */
static int first_pass(FILE *src_file, SymbolTable *symbol_table,
                      BinaryTable *instruction_table, BinaryTable *data_table) {
  bool succesful = true;
  char line[MAX_LINE_LEN + 1];

  while (fgets(line, sizeof(line), src_file) != NULL) {
    if (!first_pass_line(line, symbol_table, data_table, instruction_table))
      succesful = false;
  }

  return succesful;
}

static void read_file(FILE *src_file, const char *src_path, FILE *out_file,
                      const char *out_path) {
  SymbolTable symbol_table;
  BinaryTable data_table, instruction_table;
  symbol_table.head = NULL;

  /* Return if first pass failed */
  if (!first_pass(src_file, &symbol_table, &instruction_table, &data_table)) {
    printf("DEBUG: First pass failed\n");
    free_symbol_table(symbol_table);
    return;
  }

  printf("DEBUG: Finished first pass\n");
  free_symbol_table(symbol_table);
}

int assemble_file(char *filename, char *suffix) {
  char *src_file_path, *out_file_path;
  FILE *src_file, *out_file;

  /* Open source file */
  src_file_path = with_ext(filename, suffix);
  src_file = fopen(src_file_path, "r");
  if (src_file == NULL) {
    printf("ERROR: Failed to open assembly source file '%s'\n", src_file_path);
    return false;
  }

  /* Open output file */
  out_file_path = with_ext(filename, OBJECT_FILE_SUFFIX);
  out_file = fopen(out_file_path, "w");
  if (out_file == NULL) {
    printf("ERROR: Failed to open object output file '%s'\n", out_file_path);
    return false;
  }

  /* Process file */
  read_file(src_file, src_file_path, out_file, out_file_path);

  /* Close, free and return */
  fclose(src_file);
  fclose(out_file);
  free(src_file_path);
  free(out_file_path);
  return true;
}
