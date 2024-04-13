/* [DOCS NEEDED] */
#include "assembler.h"
#include "binary_table.h"
#include "parse_util.h"
#include "symbol_table.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* [DOCS NEEDED] returns whether the operation was successful */
static int add_define_symbol(DefineLine line, SymbolTable *table) {
  int value = 0;

  /* Scan value number and return on error */
  if (!scan_number(line.value, &value)) return false;

  /* Add symbol to table */
  if (!append_symbol(table, line.name, 'm', value)) {
    printf("ERROR: Attempt to redefine a constant\n");
    return false;
  }

  return true;
}

/* [DOCS NEEDED] returns false on failure */
static int get_constant_value(SymbolTable *symbol_table, char *str, int *out) {
  SymbolTableNode *constant;

  /* Check if this is written as a number */
  if (isdigit(str[0]) || str[0] == '+' || str[0] == '-') return scan_number(str, out);

  /* Else search for constant with the given name */
  constant = search_symbol(symbol_table, str);

  /* Print error if no symbol was found */
  if (constant == NULL) {
    printf("ERROR: No constant named '%s' found\n", str);
    return false;
  }

  /* Print error if the symbol is not a constant */
  if (constant->type != 'm') {
    printf("ERROR: Variables used in data instruction must be constants "
           "(.define)\n");
    return false;
  }

  *out = constant->value;
  return true;
}

/* [DOCS NEEDED] returns how much to skip, 0 if scan should stop, -1 if there
 * were errors */
static int add_data_argument(char *content, BinaryTable *data_table, SymbolTable *symbol_table) {
  BinaryWord word;
  int res = scan_argument(content, ',');
  int value;

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

  /* Get value of argument, return on error */
  if (!get_constant_value(symbol_table, content, &value)) return -1;

  /* Add value to data table */
  word.content = value;
  append_word(data_table, word);

  return res; /* Return chars to skip */
}

/* [DOCS NEEDED] return whether the data instruction was scanned successfully */
static int add_data_instruction(DotInstructionLine line, BinaryTable *data_table,
                                SymbolTable *symbol_table) {
  char *content = line.args_start;
  int skip;

  /* If there is a label, add to the symbol table */
  /* TODO: Handle situation where the label was already defined */
  if (line.label != NULL) append_symbol(symbol_table, line.label, 'd', data_table->counter);

  /* Add all arguments to the data table */
  while ((skip = add_data_argument(content, data_table, symbol_table)) > 0)
    content += skip;

  if (skip == -1) return false;

  return true;
}
static int add_string_instruction(DotInstructionLine line, BinaryTable *data_table,
                                  SymbolTable *symbol_table) {
  char *content = line.args_start;
  int res;
  BinaryWord word;

  /* If there is a label, add to the symbol table */
  /* TODO: Handle situation where the label was already defined */
  if (line.label != NULL) append_symbol(symbol_table, line.label, 'd', data_table->counter);

  /* Scan string and handle errors */
  res = scan_string(content);
  /* TODO: Better as switch? Better to extract the return? */
  if (res == -1) {
    printf("ERROR: No string given in .string instruction\n");
    return -1;
  }
  if (res == -2) {
    printf("ERROR: Missing starting quotation mark in string instruction\n");
    return -1;
  }
  if (res == -2) {
    printf("ERROR: Missing ending quotation mark in string instruction\n");
    return -1;
  }
  if (res == -4) {
    printf("ERROR: Extraneous text after string instruction\n");
    return -1;
  }
  content += res;

  /* Add string to data table */
  for (; content[0] != '\0'; content++) {
    word.content = content[0];
    append_word(data_table, word);
  }
  word.content = '\0'; /* Also add the terminator */
  append_word(data_table, word);

  return true;
}

/* [DOCS NEEDED] */
static int add_dot_instruction(DotInstructionLine line, BinaryTable *data_table,
                               SymbolTable *symbol_table) {
  if (strcmp(line.name, "data") == 0) {
    return add_data_instruction(line, data_table, symbol_table);
  }
  if (strcmp(line.name, "string") == 0) {
    return add_string_instruction(line, data_table, symbol_table);
  }

  /* TODO: Implement other dot instructions */

  printf("ERROR: Unkown dot instruction name: '%s'\n", line.name);
  return false;
}

typedef struct CommandFirstWord {
  unsigned int are : 2; /* Always zero */
  unsigned int dest_adressing : 2;
  unsigned int src_adressing : 2;
  unsigned int opcode : 4;
  unsigned int : 4; /* Unused */
} CommandFirstWord;

static BinaryWord first_word_to_binary(CommandFirstWord first) {
  BinaryWord word;
  word.content =
      (first.are) | (first.dest_adressing << 2) | (first.src_adressing << 4) | (first.opcode << 6);
  return word;
}

static int get_adressing_mode(char *arg) {
  if (arg == NULL || arg[0] == '#') return 0; /* 0 - Immediate */
  if (is_register_name(arg)) return 3;        /* 3 - Register */

  /* If argument includes '[' */
  if (strchr(arg, '[')) return 2; /* 2 - Index */

  return 1; /* 1 - Direct */
}

/* [DOCS NEEDED] returns -1 on invalid command */
static int get_opcode(char *command) {
  if (strcmp(command, "mov") == 0) return 0;
  if (strcmp(command, "cmp") == 0) return 1;
  if (strcmp(command, "add") == 0) return 2;
  if (strcmp(command, "sub") == 0) return 3;
  if (strcmp(command, "not") == 0) return 4;
  if (strcmp(command, "clr") == 0) return 5;
  if (strcmp(command, "lea") == 0) return 6;
  if (strcmp(command, "inc") == 0) return 7;
  if (strcmp(command, "dec") == 0) return 8;
  if (strcmp(command, "jmp") == 0) return 9;
  if (strcmp(command, "bne") == 0) return 10;
  if (strcmp(command, "red") == 0) return 11;
  if (strcmp(command, "prn") == 0) return 12;
  if (strcmp(command, "jsr") == 0) return 13;
  if (strcmp(command, "rts") == 0) return 14;
  if (strcmp(command, "hlt") == 0) return 15;
  return -1;
}

/* [DOCS NEEDED] 0 - OK | 1 - No arguments (one or more given) |
 * 2 - No source argument (was given)|
 * 3 - destination addressing wrong | 4 - source addressing wrong */
static int verify_adressing_mode(int opcode, int src_adressing, int dest_adressing) {
  /* TODO: Actually implement this */
  return 0; /* 0 - Both addressings are valid */
}

/* [DOCS NEEDED] */
static int add_command(CommandLine line, BinaryTable *instruction_table,
                       SymbolTable *symbol_table) {
  BinaryWord word;
  CommandFirstWord first_word;
  first_word.are = 0; /* ARE is always 0 in first word */

  /* If there is a label, add to the symbol table */
  /* TODO: Handle situation where the label was already defined */
  if (line.label != NULL) append_symbol(symbol_table, line.label, 'c', instruction_table->counter);

  /* Get opcode for command */
  first_word.opcode = get_opcode(line.cmd);
  if (first_word.opcode == -1) {
    printf("ERROR: Unknown command name '%s'\n", line.cmd);
    return false;
  }

  /* Get addressing modes and verify them */
  first_word.src_adressing = get_adressing_mode(line.src_arg);
  first_word.dest_adressing = get_adressing_mode(line.dest_arg);
  switch (verify_adressing_mode(first_word.opcode, first_word.src_adressing,
                                first_word.dest_adressing)) {
  case 1:
    printf("ERROR: Command '%s' takes no arguments\n", line.cmd);
    return false;
  case 2:
    printf("ERROR: Command '%s' doesn't take a source argument\n", line.cmd);
    return false;
  case 3:
    printf("ERROR: Invalid destination addressing mode for command '%s'", line.cmd);
    return false;
  case 4:
    printf("ERROR: Invalid source addressing mode for command '%s'", line.cmd);
    return false;
  }

  /* Add first word to instruction table */
  word = first_word_to_binary(first_word);
  append_word(instruction_table, word);

  return true;
}

/* [DOCS NEEDED] returns whether the line was parsed successfully */
static int first_pass_line(char *line, SymbolTable *symbol_table, BinaryTable *instruction_table,
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
    return add_dot_instruction(parsed_line.content.dot_instruction, data_table, symbol_table);
    break;
  case Command:
    return add_command(parsed_line.content.command, instruction_table, symbol_table);
    break;
  }

  return true;
}

/* TODO: REMOVE!!!! These are helper functions */
void print_bin(struct BinaryWord word) {
  int i;

  printf("DEBUG: ");
  for (i = 1 << (BINARY_WORD_SIZE - 1); i > 0; i >>= 1) {
    if (word.content & i)
      printf("1");
    else
      printf("0");
  }
  printf("\n");
}

static void print_bin_table(BinaryTable *table) {
  BinaryTableNode *iter;
  for (iter = table->head; iter != NULL; iter = iter->next) {
    print_bin(iter->content);
  }
}

static void print_sym_table(SymbolTable *table) {
  SymbolTableNode *iter;
  for (iter = table->head; iter != NULL; iter = iter->next) {
    printf("DEBUG:\t %s \t| %c | %d\n", iter->name, iter->type, iter->value);
  }
}

/* [DOCS NEEDED] returns whether the pass was successful */
static int first_pass(FILE *src_file, SymbolTable *symbol_table, BinaryTable *instruction_table,
                      BinaryTable *data_table) {
  bool succesful = true;
  char line[MAX_LINE_LEN + 1];

  while (fgets(line, sizeof(line), src_file) != NULL) {
    if (!first_pass_line(line, symbol_table, instruction_table, data_table)) succesful = false;
  }

  printf("DEBUG: Data table\n");
  print_bin_table(data_table);
  printf("DEBUG: Instruction table\n");
  print_bin_table(instruction_table);
  printf("DEBUG: Symbol table\n");
  print_sym_table(symbol_table);

  return succesful;
}

static void read_file(FILE *src_file, const char *src_path, FILE *out_file, const char *out_path) {
  SymbolTable symbol_table;
  BinaryTable data_table, instruction_table;
  symbol_table.head = NULL;
  data_table.head = NULL;
  data_table.counter = 0;
  instruction_table.head = NULL;
  instruction_table.counter = 0;

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
