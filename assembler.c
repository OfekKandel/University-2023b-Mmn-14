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

  /* Add symbol to table */ if (!append_symbol(table, line.name, 'm', value)) {
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
    printf("ERROR: Variable '%s' is not a constant\n", constant->name);
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

typedef struct ArgumentWord {
  unsigned int are : 2;
  unsigned int content : 12;
} ArgumentWord;

static BinaryWord first_word_to_binary(CommandFirstWord first) {
  BinaryWord word;
  word.content =
      (first.are) | (first.dest_adressing << 2) | (first.src_adressing << 4) | (first.opcode << 6);
  return word;
}

static BinaryWord arg_word_to_binary(ArgumentWord first) {
  BinaryWord word;
  word.content = (first.are) | (first.content << 2);
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

/* [DOCS NEEDED] returns false on fail */
static int encode_array_index_arg(char *arg, BinaryTable *instruction_table,
                                  SymbolTable *symbol_table) {
  int index_value;
  char *index_str;
  ArgumentWord index_word;
  index_word.are = 0;

  index_str = scan_array_index(arg);
  if (index_str == NULL) return -1;

  /* Add word for the array symbol */
  append_symbol_word(instruction_table, arg);

  /* Get value of the index and add it to the table */
  if (!get_constant_value(symbol_table, index_str, &index_value)) return false;
  index_word.content = index_value;
  append_word(instruction_table, arg_word_to_binary(index_word));

  return true;
}

/* [DOCS NEEDED] */
static void encode_registers(BinaryTable *instruction_table, char *src_reg, char *dest_reg) {
  ArgumentWord word;
  word.are = 0;
  word.content = 0;

  if (dest_reg != NULL) /* Extract value from name and place it in bits 2-4 */
    word.content |= (dest_reg[1] - '0') << 0;
  if (src_reg != NULL) /* Extract value from name and place it in bits 5-7 */
    word.content |= (src_reg[1] - '0') << 3;

  append_word(instruction_table, arg_word_to_binary(word));
}

/* [DOCS NEEDED] returns false if the argument could not be added, registers are not encoded with
 * this function, instead with encode_registers */
static int add_arg_word(char *arg, int adressing_mode, BinaryTable *instruction_table,
                        SymbolTable *symbol_table) {
  ArgumentWord word;
  int value;
  word.are = 0;

  switch (adressing_mode) {
  case 0: /* 0 - Immediate */
    if (!get_constant_value(symbol_table, arg + 1, &value)) return false;
    word.content = value;
    append_word(instruction_table, arg_word_to_binary(word));
    break;
  case 1: /* 1 - Direct */
    append_symbol_word(instruction_table, arg);
    break;
  case 2: /* 2 - Index */
    if (!encode_array_index_arg(arg, instruction_table, symbol_table)) return false;
    break;
  case 3: /* 3 - Register, skipped */
    break;
  }

  return true;
}

/* [DOCS NEEDED] */
static int add_command(CommandLine line, BinaryTable *instruction_table,
                       SymbolTable *symbol_table) {
  BinaryWord word;
  CommandFirstWord first_word;
  first_word.are = 0; /* ARE is always 0 in first word */

  /* If there is a label, add to the symbol table */
  /* TODO: Handle situation where the label was already defined */
  if (line.label != NULL)
    append_symbol(symbol_table, line.label, 'c', instruction_table->counter + 100);

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

  /* Add source argument word to instruction table */
  if (line.src_arg != NULL) {
    if (first_word.src_adressing == 3) /* Special case for registers */
      encode_registers(instruction_table, line.src_arg,
                       (first_word.dest_adressing == 3) ? line.dest_arg : NULL);
    else
      add_arg_word(line.src_arg, first_word.src_adressing, instruction_table, symbol_table);
  }

  /* Add destination argument word to instruction table */
  if (line.dest_arg != NULL) {
    /* Special case for registers not already encoded */
    if (first_word.src_adressing != 3 && first_word.dest_adressing == 3)
      encode_registers(instruction_table, NULL, line.dest_arg);
    else
      add_arg_word(line.dest_arg, first_word.dest_adressing, instruction_table, symbol_table);
  }

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
}

static void print_bin_table(BinaryTable *table) {
  BinaryTableNode *iter;
  for (iter = table->head; iter != NULL; iter = iter->next) {
    print_bin(iter->content);
    printf(" | %s\n", iter->symbol);
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
  SymbolTableNode *iter;
  bool succesful = true;
  char line[MAX_LINE_LEN + 1];

  /* Encode each line */
  while (fgets(line, sizeof(line), src_file) != NULL) {
    if (!first_pass_line(line, symbol_table, instruction_table, data_table)) succesful = false;
  }

  /* Progress all data values */
  for (iter = symbol_table->head; iter != NULL; iter = iter->next)
    if (iter->type == 'd') iter->value += 100 + instruction_table->counter;

  return succesful;
}

static int second_pass(SymbolTable *symbol_table, BinaryTable *instruction_table,
                       BinaryTable *data_table) {
  BinaryTableNode *iter;
  SymbolTableNode *symbol;
  bool succesful = true;

  for (iter = instruction_table->head; iter != NULL; iter = iter->next) {
    if (iter->symbol == NULL) continue;
    symbol = search_symbol(symbol_table, iter->symbol);
    if (symbol == NULL) {
      printf("ERROR: Usage of undeclared symbol '%s'", iter->symbol);
      succesful = false;
    }
    iter->symbol = NULL;
    /* TODO: Add support for external here, as this sets ARE to 2 (10) */
    iter->content.content = 2 | (symbol->value << 2);
  }

  return succesful;
}

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

  printf("DEBUG: End of write_ob_file\n");
}

/* [DOCS NEEDED] */
static void read_file(FILE *src_file, FILE *ob_file, FILE *ent_file, FILE *ext_file) {
  SymbolTable symbol_table;
  BinaryTable data_table, instruction_table;
  symbol_table.head = NULL;
  data_table.head = NULL;
  data_table.counter = 0;
  instruction_table.head = NULL;
  instruction_table.counter = 0;

  /* Perform first pass, return if failed */
  if (!first_pass(src_file, &symbol_table, &instruction_table, &data_table)) {
    printf("DEBUG: First pass failed\n");
    free_tables(symbol_table, data_table, instruction_table);
    return;
  }
  printf("DEBUG: Finished first pass\n");

  /* Perform second pass, return if failed */
  if (!second_pass(&symbol_table, &instruction_table, &data_table)) {
    printf("DEBUG: Second pass failed");
    free_tables(symbol_table, data_table, instruction_table);
    return;
  }
  printf("DEBUG: Finished second pass\n");

  printf("DEBUG: Instruction table\n");
  print_bin_table(&instruction_table);
  printf("DEBUG: Data table\n");
  print_bin_table(&data_table);
  printf("DEBUG: Symbol table\n");
  print_sym_table(&symbol_table);

  /* Write output files */
  write_ob_file(ob_file, data_table, instruction_table);

  /* Free tables and close files */
  free_tables(symbol_table, data_table, instruction_table);
  fclose(ob_file);
}

int assemble_file(char *filename, char *suffix) {
  FILE *src_file, *ob_file, *ent_file, *ext_file;

  /* Open files and return on error */
  src_file = open_with_ext(filename, suffix, "r", "assembly source file");
  ob_file = open_with_ext(filename, OBJECT_FILE_SUFFIX, "w", "object output file");
  ent_file = open_with_ext(filename, ENTRIES_FILE_SUFFIX, "w", "entries output file");
  ext_file = open_with_ext(filename, EXTERNALS_FILE_SUFFIX, "w", "externals output file");
  if (!src_file || !ob_file || !ent_file || !ext_file) return false;

  /* Process file */
  read_file(src_file, ob_file, ent_file, ext_file);

  /* Close files */
  fclose(src_file);
  fclose(ob_file);
  fclose(ent_file);
  fclose(ext_file);
  return true;
}
