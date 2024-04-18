/* The source file for assembler.h
 * IMPORTANT NOTE REGARDING THE ASSEMBLER ALGORITHM:
 * The algorithm written in this project is very similar to the one proposed in the assignment, with
 * one import distinction: While the proposed algorithm iterates over the file itself twice (it
 * reads the source code on each pass) this algorithm only reads the source file only once, instead
 * opting, during the first pass, to store an empty binary word next to the name of the symbol that
 * should be filled-in in its place, and then iterating over the instruction table a second time
 * (once the symbol table has been fully created) and filling-in the missing symbols (second pass).
 * This method has the advantage of saving a lot of duplicate work re-reading the file.
 * Note: the extern and entry commands are also performed in the first pass here, by marking symbols
 * as entries or external before their value is filled-in */

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

/* Frees the dynamically allocated memory used by the data, instruction, and symbol tables
 * Input: The symbol, data, and instruction tables */
static void free_tables(SymbolTable symbol_table, BinaryTable data_table,
                        BinaryTable instruction_table) {
  free_symbol_table(symbol_table);
  free_binary_table(data_table);
  free_binary_table(instruction_table);
}

/* Writes the object file as described in the assignment, (header, and the instruction and data
 * tables one after another in the encoded format with numbers)
 * Input: The .ob file to write to, and the data and instruction tables */
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

/* Writes the entries file as specified in the assignment (each entry symbol with its value)
 * Input: the symbol table and the .ent file to write to.
 * Output: Returns false if any errors occurred, else true. Also writes to the .ent file of course.
 * Errors: if a symbol marked entry hasn't been filled in (it doesn't exist), an error is printed */
static int write_ent_file(SymbolTable *symbol_table, FILE *ent_file) {
  SymbolTableNode *iter;
  int successful = true;

  for (iter = symbol_table->head; iter != NULL; iter = iter->next) {
    if (iter->linker_flag != EntryLinkerFlag) continue; /* skip symbols not marked entry */

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

/* Performs the first pass on a single source line (possibly leaving some lines to be filled-in)
 * Input: the current line to be encoded, the symbol, instruction, and data tables, and the line's
 *        log-context (file and line number)
 * Output: Returns false if any errors occurred, else true. Also appends to the given tables.
 * Errors: Prints any errors contained in the line (which can be detected in the first pass) */
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

/* Performs the first pass on a given file
 * Input: the source file to read from, the symbol, instruction, and data tables (which will be
 *        written into) and the file's log-context
 * Output: Returns false if any errors occurred during the pass, else true, writes to the tables
 * Algorithm: Encodes every line in the file, leaving spaces to be filled-in in the instruction
 *            table for the second-pass */
static int first_pass(FILE *src_file, SymbolTable *symbol_table, BinaryTable *instruction_table,
                      BinaryTable *data_table, LogContext context) {
  SymbolTableNode *iter;
  int succesful = true;
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

/* Performs the second pass on the given instruction table (filling in the symbols) and writes the
 * entry and externals files.
 * Input: The symbol, instruction, and data tables, as well as the .ent .ext files to write to.
 * Output: Returns false if any errors occurred during the pass, else true, also write the .ext and
 *         .ent files */
static int second_pass(SymbolTable *symbol_table, BinaryTable *instruction_table,
                       BinaryTable *data_table, FILE *ent_file, FILE *ext_file) {
  BinaryTableNode *iter;
  SymbolTableNode *symbol;
  int succesful = true;
  int word_num = 100;

  for (iter = instruction_table->head; iter != NULL; iter = iter->next, word_num++) {
    if (iter->symbol == NULL) continue;

    symbol = search_symbol(symbol_table, iter->symbol);
    if (symbol == NULL) {
      print_log_context(iter->context, "ERROR");
      printf("Usage of undeclared symbol '%s'\n", iter->symbol);
      succesful = false;
      continue;
    }
    iter->symbol = NULL;

    /* If symbol is external then set the ARE and add it to the ext file */
    if (symbol->linker_flag == ExternalLinkerFlag) {
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

/* Fully encodes the given file, performing both passes, and writing the object file
 * Input: The source, object, entries, and externals files, and the file log-context
 * Output: Returns false if any errors occurred, else true, also writes all files */
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
    free_tables(symbol_table, data_table, instruction_table);
    return false;
  }

  /* Perform second pass, return if failed */
  res = second_pass(&symbol_table, &instruction_table, &data_table, ent_file, ext_file);
  if (!res) {
    free_tables(symbol_table, data_table, instruction_table);
    return false;
  }

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
  if (success)
    printf("Successfully assembled file '%s%s'\n", filename, suffix);
  else
    printf("Failed to assemble file '%s%s'\n", filename, suffix);

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
