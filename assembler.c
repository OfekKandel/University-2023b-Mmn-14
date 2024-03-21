/* [DOCS NEEDED] */
#include "assembler.h"
#include "parse_util.h"
#include "symbol_table.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* [DOCS NEEDED] returns whether the operation was successful */
static int add_define_symbol(SymbolTable *table, ParsedLine line) {
  int value = 0;

  /* Scan value number and return on error */
  if (!scan_number(line.content.define.value, &value))
    return false;

  /* Add symbol to table */
  if (!append_symbol(table, line.content.define.name, 'm', value)) {
    printf("ERROR: Attempt to redefine a constant\n");
    return false;
  }

  return true;
}

static int first_pass_line(char *line, SymbolTable *symbol_table) {
  ParsedLine parsed_line = parse_line(line);
  switch (parsed_line.line_type) {
  case Error:
  case Empty:
  case Comment:
    break;
  case Define:
    return add_define_symbol(symbol_table, parsed_line);
    break;
  case DotInstruction:
    /* TODO: */
    break;
  case Command:
    /* TODO: */
    break;
  }

  return true;
}

/* [DOCS NEEDED] returns whether the pass was successful */
static int first_pass(FILE *src_file, SymbolTable *symbol_table) {
  bool succesful = true;
  char line[MAX_LINE_LEN + 1];

  while (fgets(line, sizeof(line), src_file) != NULL) {
    if (!first_pass_line(line, symbol_table))
      succesful = false;
  }

  return succesful;
}

static void read_file(FILE *src_file, const char *src_path, FILE *out_file,
                      const char *out_path) {
  SymbolTable symbol_table;
  symbol_table.head = NULL;
  /* BinaryTable data_table, BinaryTable instruction_table; */

  /* Return if first pass failed */
  if (!first_pass(src_file, &symbol_table)) {
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
