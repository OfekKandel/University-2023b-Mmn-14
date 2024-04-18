/* The source file for the encoding_util.h file */

#include "encoding_util.h"
#include <stdbool.h>
#include <string.h>

BinaryWord first_word_to_binary(CommandFirstWord first) {
  BinaryWord word;
  word.content =
      (first.are) | (first.dest_adressing << 2) | (first.src_adressing << 4) | (first.opcode << 6);
  return word;
}

BinaryWord arg_word_to_binary(ArgumentWord first) {
  BinaryWord word;
  word.content = (first.are) | (first.content << 2);
  return word;
}

/* [Docs in header file] the adressing mode is decided purely based on syntax, no check of the
 * validity of the source code is performed here */
int get_adressing_mode(char *arg) {
  if (arg == NULL || arg[0] == '#') return 0; /* 0 - Immediate */
  if (is_register_name(arg)) return 3;        /* 3 - Register */

  /* If argument includes '[' */
  if (strchr(arg, '[')) return 2; /* 2 - Index */

  return 1; /* 1 - Direct */
}

/* [Docs in header file] implements the opcode table (page 18) using a list of ifs and strcmp(s) */
int get_opcode(char *command) {
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

/* [Docs in header file] while this function looks daunting it simply implements the different
 * errors that arise from the adressing modes table (page 33) using a list of if statements */
VerifyAdressingResult verify_adressing_mode(int opcode, int src_adr, int dest_adr) {
  int src_valid = true, dest_valid = true;

  /* Check if too many arguments were given */
  if (15 <= opcode && opcode <= 16)
    if (dest_adr != -1) return TakesNoArguments;
  if ((4 <= opcode && opcode <= 5) || (7 <= opcode && opcode <= 14))
    if (src_adr != -1) return NoSourceArgument;

  /* Check if there are missing arguments */
  if (opcode == 6 || (0 <= opcode && opcode <= 3)) {
    if (src_adr == -1 || dest_adr == -1) return MissingArgument;
  } else if (15 != opcode && opcode != 16) {
    if (dest_adr == -1) return MissingArgument;
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
    return BothAdressingsWrong;
  else if (!src_valid)
    return SourceAdressingWrong;
  else if (!dest_valid)
    return DestinationAdressingWrong;

  return AdressingsOK;
}

void encode_registers(BinaryTable *instruction_table, char *src_reg, char *dest_reg) {
  ArgumentWord word;
  word.are = 0;
  word.content = 0;

  if (dest_reg != NULL) /* Extract value from name and place it in bits 2-4 */
    word.content |= (dest_reg[1] - '0') << 0;
  if (src_reg != NULL) /* Extract value from name and place it in bits 5-7 */
    word.content |= (src_reg[1] - '0') << 3;

  append_word(instruction_table, arg_word_to_binary(word));
}
