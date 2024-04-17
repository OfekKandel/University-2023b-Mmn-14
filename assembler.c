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

/* [DOCS NEEDED] returns false on failure */
static int get_constant_value(SymbolTable *symbol_table, char *str, int *out, LogContext context) {
  SymbolTableNode *constant;

  /* Check if this is written as a number */
  if (isdigit(str[0]) || str[0] == '+' || str[0] == '-') return scan_number(str, out, context);
  /* Else search for constant with the given name */
  constant = search_symbol(symbol_table, str);

  /* Print error if no symbol was found */
  if (constant == NULL) {
    print_log_context(context, "ERROR");
    printf("No constant named '%s' found\n", str);
    return false;
  }

  /* Print error if the symbol is not a constant */
  if (constant->type != 'm') {
    print_log_context(context, "ERROR");
    printf("Variable '%s' is not a constant\n", constant->name);
    return false;
  }

  *out = constant->value;
  return true;
}

/* DOT-COMMANDS --------------------------- */

/* [DOCS NEEDED] returns how much to skip, 0 if scan should stop, -1 if there
 * were errors */
static int add_data_argument(char *content, BinaryTable *data_table, SymbolTable *symbol_table,
                             LogContext context) {
  BinaryWord word;
  int res = scan_argument(content, ',');
  int value;

  if (res < 0) {
    print_log_context(context, "ERROR");
    switch (res) {
    case -5: /* No text */
      printf("A data instruction must contain at least one argument\n");
      break;
    case -1: /* Missing first argument */
      printf("A data instruction must start with an argument\n");
      break;
    case -2: /* Missing , */
    case -3: /* Double , */
      printf("Two arguments in a data instruction are separated by a single comma\n");
      break;
    case -4: /* Trailing , */
      printf("Trailing comma in data instruction\n");
      break;
    }
    return -1;
  }

  /* Get value of argument, return on error */
  if (!get_constant_value(symbol_table, content, &value, context)) return -1;

  /* Add value to data table */
  word.content = value;
  append_word(data_table, word);

  return res; /* Return chars to skip */
}

/* [DOCS NEEDED] return whether the data instruction was scanned successfully */
static int add_data_instruction(DotInstructionLine line, BinaryTable *data_table,
                                SymbolTable *symbol_table, LogContext context) {
  char *content = line.args_start;
  int skip;

  /* If there is a label, add to the symbol table, and handle error  */
  if (line.label != NULL)
    if (!append_symbol(symbol_table, line.label, 'd', data_table->counter, context)) {
      print_log_context(context, "ERROR");
      printf("Attempt to redefine symbol '%s'\n", line.label);
      return false;
    }

  /* Add all arguments to the data table */
  while ((skip = add_data_argument(content, data_table, symbol_table, context)) > 0)
    content += skip;

  if (skip == -1) return false;

  return true;
}

static int add_string_instruction(DotInstructionLine line, BinaryTable *data_table,
                                  SymbolTable *symbol_table, LogContext context) {
  char *content = line.args_start;
  int res;
  BinaryWord word;

  /* If there is a label, add to the symbol table, and handle error */
  if (line.label != NULL)
    if (!append_symbol(symbol_table, line.label, 'd', data_table->counter, context)) {
      print_log_context(context, "ERROR");
      printf("Attempt to redefine symbol '%s'\n", line.label);
      return false;
    }

  /* Scan string and handle errors */
  res = scan_string(content);
  if (res < 0) {
    print_log_context(context, "ERROR");
    switch (res) {
    case -1:
      printf("No string given in .string instruction\n");
      break;
    case -2:
      printf("Missing opening quotation mark in string instruction\n");
      break;
    case -3:
      printf("Missing closing quotation mark in string instruction\n");
      break;
    case -4:
      printf("Extraneous text after string instruction\n");
      break;
    }
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

/* [DOCS NEEDED] flag: 1 - entry 2 - extern */
static int add_ent_ext_instruction(DotInstructionLine line, SymbolTable *symbol_table, int flag,
                                   LogContext context) {
  char *argument = line.args_start;
  int res = scan_argument(argument, ','); /* Separator only matters in error cases here */

  /* Print error and return if there is more than one argument */
  if (res != 0) {
    print_log_context(context, "ERROR");
    printf(".%s instruction takes one argument\n", line.name);
    return false;
  }

  /* Mark symbol */
  return mark_symbol(symbol_table, argument, flag, context);
}

/* [DOCS NEEDED] returns whether the operation was successful */
static int add_define_symbol(DefineLine line, SymbolTable *table, LogContext context) {
  int value = 0;

  /* Scan value number and return on error */
  if (!scan_number(line.value, &value, context)) return false;

  /* Add symbol to table */ if (!append_symbol(table, line.name, 'm', value, context)) {
    print_log_context(context, "ERROR");
    printf("Attempt to redefine a constant\n");
    return false;
  }

  return true;
}

/* [DOCS NEEDED] */
static int add_dot_instruction(DotInstructionLine line, BinaryTable *data_table,
                               SymbolTable *symbol_table, LogContext context) {
  if (strcmp(line.name, "data") == 0) {
    return add_data_instruction(line, data_table, symbol_table, context);
  }
  if (strcmp(line.name, "string") == 0) {
    return add_string_instruction(line, data_table, symbol_table, context);
  }
  if (strcmp(line.name, "entry") == 0) {
    return add_ent_ext_instruction(line, symbol_table, 1, context);
  }
  if (strcmp(line.name, "extern") == 0) {
    return add_ent_ext_instruction(line, symbol_table, 2, context);
  }

  fflush(stdout);
  print_log_context(context, "ERROR");
  printf("Unkown dot instruction name: '%s'\n", line.name);
  return false;
}

/* OB & ent file ---------------------------------------- */

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

/* First word encoding ---------------------------------------  */
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

/* [DOCS NEEDED] Pass -1 for no argument, 0 - OK | 1 - No arguments (one or more given) | 2 - No
 * source argument (was given)| 3 - destination addressing wrong | 4 - source addressing wrong
 * | 5 - both adressings are wrong | 6 - missing argument */
static int verify_adressing_mode(int opcode, int src_adr, int dest_adr) {
  int src_valid = true, dest_valid = true;

  /* Check if too many arguments were given */
  if (15 <= opcode && opcode <= 16)
    if (dest_adr != -1) return 1; /* 1- Command takes no arguments but some were given */
  if ((4 <= opcode && opcode <= 5) || (7 <= opcode && opcode <= 14))
    if (src_adr != -1) return 2; /* 2 - Command takes no source argument but one was given */

  /* Check if there are missing arguments */
  if (opcode == 6 || (0 <= opcode && opcode <= 3)) {
    if (src_adr == -1 || dest_adr == -1) return 6; /* 6 - Missing argument */
  } else if (15 != opcode && opcode != 16) {
    if (dest_adr == -1) return 6; /* 6 - Missing argument */
  }

  /* Check if source argument is valid */
  if (0 <= opcode && opcode <= 3)
    src_valid = src_adr != -1; /* 0,1,2,3 (long as there is an argument)*/
  else if (opcode == 6)
    src_valid = src_adr == 1 || src_adr == 2; /* 1,2 */

  /* Check if destination argument is valid */
  if (opcode == 0 || opcode == 11 || (2 <= opcode && opcode <= 8))
    dest_valid = 1 <= dest_adr && dest_adr <= 3; /* 1,2,3 */
  else if (opcode == 1 || opcode == 12)
    dest_valid = dest_adr != -1; /* 0,1,2,3 (long as there is an argument)*/
  else if (opcode == 9 || opcode == 10 || opcode == 13)
    dest_valid = dest_adr == 1 || dest_adr == 3; /* 1,3 */

  if (!src_valid && !dest_valid)
    return 5; /* 5 - both adressing are wrong */
  else if (!src_valid)
    return 4; /* 4 - source adressing wrong */
  else if (!dest_valid)
    return 3; /* 3 - destination adressing wrong */

  return 0; /* 0 - Both addressings are valid */
}

/* Argument encoding ------------------------------------------------ */

/* [DOCS NEEDED] returns false on fail */
static int encode_array_index_arg(char *arg, BinaryTable *instruction_table,
                                  SymbolTable *symbol_table, LogContext context) {
  int index_value;
  char *index_str;
  ArgumentWord index_word;
  index_word.are = 0;

  index_str = scan_array_index(arg, context);
  if (index_str == NULL) return false;

  /* Add word for the array symbol */
  append_symbol_word(instruction_table, arg);

  /* Get value of the index and add it to the table */
  if (!get_constant_value(symbol_table, index_str, &index_value, context)) return false;
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
                        SymbolTable *symbol_table, LogContext context) {
  ArgumentWord word;
  int value;
  word.are = 0;

  switch (adressing_mode) {
  case 0: /* 0 - Immediate */
    if (!get_constant_value(symbol_table, arg + 1, &value, context)) return false;
    word.content = value;
    append_word(instruction_table, arg_word_to_binary(word));
    break;
  case 1: /* 1 - Direct */
    append_symbol_word(instruction_table, arg);
    break;
  case 2: /* 2 - Index */
    if (!encode_array_index_arg(arg, instruction_table, symbol_table, context)) return false;
    break;
  case 3: /* 3 - Register, skipped */
    break;
  }

  return true;
}

/* Main encoding ------------------------------------ */

/* [DOCS NEEDED] */
static int add_command(CommandLine line, BinaryTable *instruction_table, SymbolTable *symbol_table,
                       LogContext context) {
  int successful = true, res;
  BinaryWord word;
  CommandFirstWord first_word;
  first_word.are = 0; /* ARE is always 0 in first word */

  /* If there is a label, add to the symbol table */
  if (line.label != NULL)
    if (!append_symbol(symbol_table, line.label, 'c', instruction_table->counter + 100, context)) {
      print_log_context(context, "ERROR");
      printf("Attempt to redefine symbol '%s' (line %d)\n", line.label, context.line);
      return false;
    }

  /* Get opcode for command */
  first_word.opcode = get_opcode(line.cmd);
  if (first_word.opcode == -1) {
    print_log_context(context, "ERROR");
    printf("Unknown command name '%s'\n", line.cmd);
    return false;
  }

  /* Get addressing modes and verify them */
  first_word.src_adressing = get_adressing_mode(line.src_arg);
  first_word.dest_adressing = get_adressing_mode(line.dest_arg);
  res = verify_adressing_mode(first_word.opcode,
                              (line.src_arg == NULL) ? -1 : first_word.src_adressing,
                              (line.dest_arg == NULL) ? -1 : first_word.dest_adressing);
  if (res != 0) {
    print_log_context(context, "ERROR");
    switch (res) {
    case 1:
      printf("Command '%s' takes no arguments\n", line.cmd);
      return false;
    case 2:
      printf("Command '%s' doesn't take a source argument\n", line.cmd);
      return false;
    case 3:
      printf("Invalid destination addressing mode for command '%s'\n", line.cmd);
      return false;
    case 4:
      printf("Invalid source addressing mode for command '%s'\n", line.cmd);
      return false;
    case 5:
      printf("Source and destination adressing modes for command '%s' are wrong\n", line.cmd);
      return false;
    case 6:
      printf("Missing arguments for command '%s'\n", line.cmd);
      return false;
    }
  }

  /* TODO: Create encoding file? (only for writing all the files) */

  /* Add first word to instruction table */
  word = first_word_to_binary(first_word);
  append_word(instruction_table, word);

  /* Add source argument word to instruction table */
  if (line.src_arg != NULL) {
    if (first_word.src_adressing == 3) /* Special case for registers */
      encode_registers(instruction_table, line.src_arg,
                       (first_word.dest_adressing == 3) ? line.dest_arg : NULL);
    else
      successful = successful && add_arg_word(line.src_arg, first_word.src_adressing,
                                              instruction_table, symbol_table, context);
  }

  /* Add destination argument word to instruction table */
  if (line.dest_arg != NULL) {
    /* Special case for registers not already encoded */
    if (first_word.src_adressing != 3 && first_word.dest_adressing == 3)
      encode_registers(instruction_table, NULL, line.dest_arg);
    else
      successful = successful && add_arg_word(line.dest_arg, first_word.dest_adressing,
                                              instruction_table, symbol_table, context);
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

  /* Progress all data values */
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

/* [DOCS NEEDED] */
static void free_tables(SymbolTable symbol_table, BinaryTable data_table,
                        BinaryTable instruction_table) {
  free_symbol_table(symbol_table);
  free_binary_table(data_table);
  free_binary_table(instruction_table);
}

/* [DOCS NEEDED] return true on success */
static int read_file(FILE *src_file, FILE *ob_file, FILE *ent_file, FILE *ext_file,
                     LogContext context) {
  SymbolTable symbol_table;
  BinaryTable data_table, instruction_table;
  symbol_table.head = NULL;
  data_table.head = NULL;
  data_table.counter = 0;
  instruction_table.head = NULL;
  instruction_table.counter = 0;

  /* Perform first pass, return if failed */
  if (!first_pass(src_file, &symbol_table, &instruction_table, &data_table, context)) {
    printf("DEBUG: First pass failed\n");
    free_tables(symbol_table, data_table, instruction_table);
    return false;
  }
  printf("DEBUG: Finished first pass\n");

  /* Perform second pass, return if failed */
  if (!second_pass(&symbol_table, &instruction_table, &data_table, ent_file, ext_file)) {
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
  success = read_file(src_file, ob_file, ent_file, ext_file, context);

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
