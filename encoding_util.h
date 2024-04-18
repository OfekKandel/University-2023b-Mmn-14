/* A helper file for the assembler focused on the editing the bits of the instruction and data
 * tables, but not handling any file editing */

#pragma once
#include "binary_table.h"
#include "symbol_table.h"

/* A command encoding's first binary word as specified in the assignment */
typedef struct CommandFirstWord {
  unsigned int are : 2; /* Always zero */
  unsigned int dest_adressing : 2;
  unsigned int src_adressing : 2;
  unsigned int opcode : 4;
  unsigned int : 4; /* Unused */
} CommandFirstWord;

/* A command argument's word as specified in the assignment */
typedef struct ArgumentWord {
  unsigned int are : 2;
  unsigned int content : 12;
} ArgumentWord;

/* Converts the CommandFirstWord struct to an BinaryWord struct
 * Input: The CommandFirstWord struct to convert
 * Output: The BinaryWord struct result */
BinaryWord first_word_to_binary(CommandFirstWord first);

/* Converts the ArgumentWord struct to an BinaryWord struct
 * Input: The ArgumentWord struct to convert
 * Output: The BinaryWord struct result */
BinaryWord arg_word_to_binary(ArgumentWord first);

/* Returns the opcode for a given command name as specified in the assignment
 * Input: A command name as a string
 * Output: The commands opcode, or if the command doesn't exist -1 */
int get_opcode(char *command);

/* Returns the adressing mode of a given argument (given as a string)
 * Input: The argument (as written in the source code, as a string)
 * Output: The adressing mode of the argument, values as specified in the assignment */
int get_adressing_mode(char *arg);

/* Adds the register argument word for the two given registers to the instruction table
 * Input: The table to add the word to, and the source and destination registers, if there is no
 * source or destination register pass NULL
 * Output: the function doesn't return anything but does edit the instruction table */
void encode_registers(BinaryTable *instruction_table, char *src_reg, char *dest_reg);

/* TODO: Convert the return value here to an enum */

/* [DOCS NEEDED] Pass -1 for no argument, 0 - OK | 1 - No arguments (one or more given) | 2 - No
 * source argument (was given)| 3 - destination addressing wrong | 4 - source addressing wrong
 * | 5 - both adressings are wrong | 6 - missing argument */

/* Verifies that the adressing modes for the two given arguments of a command or compatible with the
 * command (based on the given table in the assignment)
 * Input: the command's opcode as specified in the assignment, the source and destination adressing,
 *        modes for both arguments (with -1 for no argument)
 * Output: Based on the VerifyAdressingMode enum, the result of the check */
int verify_adressing_mode(int opcode, int src_adr, int dest_adr);
