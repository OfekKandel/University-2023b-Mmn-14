/* [DOCS NEEDED] */
#include "assembler.h"
#include "assembling_util.h"
#include "binary_table.h"
#include "parser.h"
#include "symbol_table.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* [DOCS NEEDED] */
static void free_tables(SymbolTable symbol_table, BinaryTable data_table,
                        BinaryTable instruction_table) {
  free_symbol_table(symbol_table);
  free_binary_table(data_table);
  free_binary_table(instruction_table);
}

static void write_ob_file(FILE *ob_file, BinaryTable data_table, BinaryTable instruction_table) {
  BinaryTableNode *iter;
  int word_num = 100;
  char encoding_buff[8];

  /* Write header */
  fprintf(ob_file, "%4d %d\n", instruction_table.counter, data_table.counter);

  /* Write instructions */
  for (iter = instruction_table.head; iter != NULL; iter = iter->next, word_num++) {
    get_encoded_word(iter->content, encoding_buff);
    fprintf(ob_file, "%04d %s\n", word_num, encoding_buff);
  }

  /* Write data */
  for (iter = data_table.head; iter != NULL; iter = iter->next, word_num++) {
    get_encoded_word(iter->content, encoding_buff);
    fprintf(ob_file, "%04d %s\n", word_num, encoding_buff);
  }
}

static int write_ent_file(SymbolTable *symbol_table, FILE *ent_file) {
  SymbolTableNode *iter;
  int successful = true;

  for (iter = symbol_table->head; iter != NULL; iter = iter->next) {
    if (iter->linker_flag != 1) continue; /* skip symbols not marked entry */

    /* Print error continue if the symbol wasn't filled in */
    if (iter->type == 'x') {
      print_log_context(iter->context, "ERROR");
      printf("Symbol marked .entry not found in file\n");
      successful = false;
      continue;
    }

    /* Add it to the .ent file */
    fprintf(ent_file, "%s\t%04d \n", iter->name, iter->value);
  }

  return successful;
}

/* [DOCS NEEDED] returns whether the line was parsed successfully */
static int first_pass_line(char *line, SymbolTable *symbol_table, BinaryTable *instruction_table,
                           BinaryTable *data_table, LogContext context) {
  ParsedLine parsed_line = parse_line(line, context);
  switch (parsed_line.line_type) {
  case Error:
    return false;
  case Empty:
  case Comment:
    break;
  case Define:
    return add_define_symbol(parsed_line.content.define, symbol_table, context);
    break;
  case DotInstruction:
    return add_dot_instruction(parsed_line.content.dot_instruction, data_table, symbol_table,
                               context);
    break;
  case Command:
    return add_command(parsed_line.content.command, instruction_table, symbol_table, context);
    break;
  }

  return true;
}

/* [DOCS NEEDED] returns whether the pass was successful */
static int first_pass(FILE *src_file, SymbolTable *symbol_table, BinaryTable *instruction_table,
                      BinaryTable *data_table, LogContext context) {
  SymbolTableNode *iter;
  bool succesful = true;
  char line[MAX_LINE_LEN + 1];

  /* Encode each line */
  context.line = 1;
  while (fgets(line, sizeof(line), src_file) != NULL) {
    if (!first_pass_line(line, symbol_table, instruction_table, data_table, context))
      succesful = false;
    context.line++;
  }

  /* Increment all data values */
  for (iter = symbol_table->head; iter != NULL; iter = iter->next)
    if (iter->type == 'd') iter->value += 100 + instruction_table->counter;

  return succesful;
}

/* [DOCS NEEDED] */
static int second_pass(SymbolTable *symbol_table, BinaryTable *instruction_table,
                       BinaryTable *data_table, FILE *ent_file, FILE *ext_file) {
  BinaryTableNode *iter;
  SymbolTableNode *symbol;
  bool succesful = true;
  int word_num = 100;

  for (iter = instruction_table->head; iter != NULL; iter = iter->next, word_num++) {
    if (iter->symbol == NULL) continue;

    symbol = search_symbol(symbol_table, iter->symbol);
    if (symbol == NULL) {
      print_log_context(symbol->context, "ERROR");
      printf("Usage of undeclared symbol '%s'\n", iter->symbol);
      succesful = false;
      continue;
    }
    iter->symbol = NULL;

    /* If symbol is external then set the ARE and add it to the ext file */
    if (symbol->linker_flag == 2) {
      fprintf(ext_file, "%s\t%04d \n", symbol->name, word_num);
      iter->content.content = 1; /* Sets ARE to 1 and rest (the value) to 0 */
      continue;
    }

    /* Else just set its value normally */
    iter->content.content = 2 | (symbol->value << 2);
  }

  /* Create the .ent file from the symbol table */
  if (!write_ent_file(symbol_table, ent_file)) succesful = false;

  return succesful;
}

/* [DOCS NEEDED] return true on success */
static int encode_file(FILE *src_file, FILE *ob_file, FILE *ent_file, FILE *ext_file,
                       LogContext context) {
  SymbolTable symbol_table;
  BinaryTable data_table, instruction_table;
  int res;
  symbol_table.head = NULL;
  data_table.head = NULL;
  data_table.counter = 0;
  instruction_table.head = NULL;
  instruction_table.counter = 0;

  /* Perform first pass, return if failed */
  res = first_pass(src_file, &symbol_table, &instruction_table, &data_table, context);
  if (!res) {
    printf("DEBUG: First pass failed\n");
    free_tables(symbol_table, data_table, instruction_table);
    return false;
  }
  printf("DEBUG: Finished first pass\n");

  /* Perform second pass, return if failed */
  res = second_pass(&symbol_table, &instruction_table, &data_table, ent_file, ext_file);
  if (!res) {
    printf("DEBUG: Second pass failed\n");
    free_tables(symbol_table, data_table, instruction_table);
    return false;
  }
  printf("DEBUG: Finished second pass\n");

  /* Write output files */
  write_ob_file(ob_file, data_table, instruction_table);

  /* Free tables */
  free_tables(symbol_table, data_table, instruction_table);

  return true;
}

int assemble_file(char *filename, char *suffix) {
  FILE *src_file, *ob_file, *ent_file, *ext_file;
  int success;
  LogContext context;
  context.filename = filename;
  context.file_ext = suffix;

  /* Open files and return on error */
  src_file = open_with_ext(filename, suffix, "r", "assembly source file");
  ob_file = open_with_ext(filename, OBJECT_FILE_SUFFIX, "w", "object output file");
  ent_file = open_with_ext(filename, ENTRIES_FILE_SUFFIX, "w", "entries output file");
  ext_file = open_with_ext(filename, EXTERNALS_FILE_SUFFIX, "w", "externals output file");
  if (!src_file || !ob_file || !ent_file || !ext_file) return false;

  /* Process file */
  success = encode_file(src_file, ob_file, ent_file, ext_file, context);

  /* Close files */
  fclose(src_file);
  fclose(ob_file);
  fclose(ent_file);
  fclose(ext_file);

  /* Delete all output files on fail, else delete only the empty ones */
  if (!success || is_file_empty(filename, OBJECT_FILE_SUFFIX, "object output file (cleanup)"))
    remove_file(filename, OBJECT_FILE_SUFFIX, "object output file (cleanup)");
  if (!success || is_file_empty(filename, ENTRIES_FILE_SUFFIX, "entries output file (cleanup)"))
    remove_file(filename, ENTRIES_FILE_SUFFIX, "entries output file (cleanup)");
  if (!success || is_file_empty(filename, EXTERNALS_FILE_SUFFIX, "externals output file (cleanup)"))
    remove_file(filename, EXTERNALS_FILE_SUFFIX, "externals output file (cleanup)");

  return true;
}
