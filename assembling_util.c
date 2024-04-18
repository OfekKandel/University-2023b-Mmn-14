/* The source file for assembling_util.h
 * Contains the bulk of the logic for the assembler's first pass, and used only by it, does not edit
 * any files, only updates the tables */
#include "binary_table.h"
#include "encoding_util.h"
#include "parse_util.h"
#include "parser.h"
#include "symbol_table.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

/* Gets the value of an argument (given as a string) which should be a constant, meaning either a
 * number or a constant declared via the .define instruction (e.g. 5, -7, len)
 * Input: The symbol table, the argument as a string, an int pointer used as an output variable, and
 *        the current line's log-context
 * Output: Returns false on errors, else true. On success outputs the value of the constant to the
 *         out variable.
 * Errors: Any formatting errors in the argument, or in the attempt to get its value (e.g. If the
 * constant has not been declared, or if it is not a constant but a label) will be printed */
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

/* Encodes a command argument of type array index (e.g. arr[4], dtt[len]), leaving an empty word
 * with a symbol name in place of the array label to be filled-in in the second pass
 * Input: The argument as a string, the instruction and symbol tables, and the line's log-context
 * Output: Returns false on errors, else true, also edits the instruction table
 * Errors: Any formatting or logical errors in the given code will be printed */
static int add_array_index_arg(char *arg, BinaryTable *instruction_table, SymbolTable *symbol_table,
                               LogContext context) {
  int index_value;
  char *index_str;
  ArgumentWord index_word;
  index_word.are = 0;

  index_str = scan_array_index(arg, context);
  if (index_str == NULL) return false;

  /* Add word for the array symbol */
  append_symbol_word(instruction_table, arg, context);

  /* Get value of the index and add it to the table */
  if (!get_constant_value(symbol_table, index_str, &index_value, context)) return false;
  index_word.content = index_value;
  append_word(instruction_table, arg_word_to_binary(index_word));

  return true;
}

/* Encodes and adds a single argument of a .data instruction to the data table
 * Input: The argument (and all the following arguments, as a string) the data and symbol tables,
 *        and the line's log-context
 * Output: Returns how many chars to skip to get to the next argument in the string, if there is no
 *         next argument 0, if there were errors -1
 * Errors: Any formatting or logical errors in the argument's code will be printed */
static int add_data_argument(char *content, BinaryTable *data_table, SymbolTable *symbol_table,
                             LogContext context) {
  BinaryWord word;
  ScanArgumentResult res = scan_argument(content, ',');
  int value;

  if (res.status != ArgumentScanShouldSkip && res.status != LastArgument) {
    print_log_context(context, "ERROR");
    switch (res.status) {
    case ArgumentScanNoContent:
      printf("A data instruction must contain at least one argument\n");
      break;
    case MissingFirstArg:
      printf("A data instruction must start with an argument\n");
      break;
    case MissingSeparator:
    case DoubleSeparator:
      printf("Two arguments in a data instruction are separated by a single comma\n");
      break;
    case TrailingSeprator:
      printf("Trailing comma in data instruction\n");
      break;
    default:
      break;
    }
    return -1;
  }

  /* Get value of argument, return on error */
  if (!get_constant_value(symbol_table, content, &value, context)) return -1;

  /* Add value to data table */
  word.content = value;
  append_word(data_table, word);

  /* Return 0 if scan should stop */
  if (res.status == LastArgument) return 0;

  return res.skip; /* Return chars to skip */
}

/* Encodes and adds a full a .data instruction to the data table
 * Input: The line (as a DotInstructionLine struct), the data and symbol tables, and the line's
 *        log-context
 * Output: Return false on errors, else true. Also edits the data and symbol tables
 * Errors: Any formatting or logical errors in the instruction will be printed
 * Algorithm: As a .data instruction's arguments are not parsed by the initial parser, some
 * additional parsing functions are used to iterate over the arguments */
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

/* Encodes and adds a full a .string instruction to the data table
 * Input: The line (as a DotInstructionLine struct), the data and symbol tables, and the line's
 *        log-context
 * Output: Return false on errors, else true. Also edits the data and symbol tables
 * Errors: Any formatting or logical errors in the instruction will be printed */
static int add_string_instruction(DotInstructionLine line, BinaryTable *data_table,
                                  SymbolTable *symbol_table, LogContext context) {
  char *content = line.args_start;
  ScanStringResult res;
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
  if (res.status != StringScanShouldSkip) {
    print_log_context(context, "ERROR");
    switch (res.status) {
    case SrtingScanNoContent:
      printf("No string given in .string instruction\n");
      break;
    case MissingOpeningQuotation:
      printf("Missing opening quotation mark in string instruction\n");
      break;
    case MissingClosingQuotation:
      printf("Missing closing quotation mark in string instruction\n");
      break;
    case ExtraneousText:
      printf("Extraneous text after string instruction\n");
      break;
    default:
      break;
    }
    return false;
  }
  content += res.skip;

  /* Add string to data table */
  for (; content[0] != '\0'; content++) {
    word.content = content[0];
    append_word(data_table, word);
  }
  word.content = '\0'; /* Also add the terminator */
  append_word(data_table, word);

  return true;
}

/* Encodes and adds a .entry or .extern instruction to the data table
 * Input: The line (as a DotInstructionLine struct), the symbol table, the linker flag (extern/entry
 *        based on which command it is), and the line's log-context
 * Output: Return false on errors, else true. Also edits the data and symbol tables
 * Errors: Any formatting or logical errors in the instruction will be printed
 * Assumes: the given linker flag represents the right type of instruction */
static int add_ent_ext_instruction(DotInstructionLine line, SymbolTable *symbol_table,
                                   LinkerFlag flag, LogContext context) {
  char *argument = line.args_start;
  ScanArgumentResult res = scan_argument(argument, ','); /* Separator only matters in errors here */

  /* Print error and return if there is more than one argument */
  if (res.status != LastArgument) {
    print_log_context(context, "ERROR");
    printf(".%s instruction takes one argument\n", line.name);
    return false;
  }

  /* Mark symbol */
  return mark_symbol(symbol_table, argument, flag, context);
}

/* Encodes and adds a single command argument to the instruction table
 * Input: The argument (as a string), the adressing mode of the argument, the instruction and symbol
 * tables, and the line's log-context
 * Output: Return false on errors, else true. Also edits the instruction and symbol tables
 * Errors: Any formatting or logical errors in the instruction will be printed
 * Assumes: the given argument is not of type register (these are encoded with encode_registers) */
static int add_cmd_arg_word(char *arg, int adressing_mode, BinaryTable *instruction_table,
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
    append_symbol_word(instruction_table, arg, context);
    break;
  case 2: /* 2 - Index */
    if (!add_array_index_arg(arg, instruction_table, symbol_table, context)) return false;
    break;
  case 3: /* 3 - Register, skipped */
    break;
  }

  return true;
}

int add_dot_instruction(DotInstructionLine line, BinaryTable *data_table, SymbolTable *symbol_table,
                        LogContext context) {
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

  print_log_context(context, "ERROR");
  printf("Unkown dot instruction name: '%s'\n", line.name);
  return false;
}

int add_define_symbol(DefineLine line, SymbolTable *table, LogContext context) {
  int value = 0;

  /* Scan value number and return on error */
  if (!scan_number(line.value, &value, context)) return false;

  /* Add symbol to table */
  if (!append_symbol(table, line.name, 'm', value, context)) {
    print_log_context(context, "ERROR");
    printf("Attempt to redefine a constant\n");
    return false;
  }

  return true;
}

int add_command(CommandLine line, BinaryTable *instruction_table, SymbolTable *symbol_table,
                LogContext context) {
  int successful = true, res;
  BinaryWord word;
  VerifyAdressingResult verify_res;
  CommandFirstWord first_word;
  first_word.are = 0; /* ARE is always 0 in first word */

  /* If there is a label, add to the symbol table */
  if (line.label != NULL)
    if (!append_symbol(symbol_table, line.label, 'c', instruction_table->counter + 100, context)) {
      print_log_context(context, "ERROR");
      printf("Attempt to redefine symbol\n");
      return false;
    }

  /* Get opcode for command */
  res = get_opcode(line.cmd);
  if (res == -1) {
    print_log_context(context, "ERROR");
    printf("Unknown command name '%s'\n", line.cmd);
    return false;
  }
  first_word.opcode = res;

  /* Get addressing modes and verify them */
  first_word.src_adressing = get_adressing_mode(line.src_arg);
  first_word.dest_adressing = get_adressing_mode(line.dest_arg);
  verify_res = verify_adressing_mode(first_word.opcode,
                                     (line.src_arg == NULL) ? -1 : first_word.src_adressing,
                                     (line.dest_arg == NULL) ? -1 : first_word.dest_adressing);
  if (verify_res != AdressingsOK) {
    print_log_context(context, "ERROR");
    switch (verify_res) {
    case MissingArgument:
      printf("Missing arguments for command '%s'\n", line.cmd);
      return false;
    case TakesNoArguments:
      printf("Command '%s' takes no arguments\n", line.cmd);
      return false;
    case NoSourceArgument:
      printf("Command '%s' doesn't take a source argument\n", line.cmd);
      return false;
    case DestinationAdressingWrong:
      printf("Invalid destination addressing mode for command '%s'\n", line.cmd);
      return false;
    case SourceAdressingWrong:
      printf("Invalid source addressing mode for command '%s'\n", line.cmd);
      return false;
    case BothAdressingsWrong:
      printf("Source and destination adressing modes for command '%s' are wrong\n", line.cmd);
      return false;
    default:
      break;
    }
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
      successful = successful && add_cmd_arg_word(line.src_arg, first_word.src_adressing,
                                                  instruction_table, symbol_table, context);
  }

  /* Add destination argument word to instruction table */
  if (line.dest_arg != NULL) {
    /* Special case for registers not already encoded */
    if (first_word.src_adressing != 3 && first_word.dest_adressing == 3)
      encode_registers(instruction_table, NULL, line.dest_arg);
    else
      successful = successful && add_cmd_arg_word(line.dest_arg, first_word.dest_adressing,
                                                  instruction_table, symbol_table, context);
  }

  return successful;
}
