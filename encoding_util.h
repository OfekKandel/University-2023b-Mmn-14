#include "binary_table.h"
#include "symbol_table.h"

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

BinaryWord first_word_to_binary(CommandFirstWord first);
BinaryWord arg_word_to_binary(ArgumentWord first);

/* [DOCS NEEDED] returns -1 on invalid command */
int get_opcode(char *command);

/* [DOCS NEEDED] */
int get_adressing_mode(char *arg);

/* [DOCS NEEDED] */
void encode_registers(BinaryTable *instruction_table, char *src_reg, char *dest_reg);

/* [DOCS NEEDED] Pass -1 for no argument, 0 - OK | 1 - No arguments (one or more given) | 2 - No
 * source argument (was given)| 3 - destination addressing wrong | 4 - source addressing wrong
 * | 5 - both adressings are wrong | 6 - missing argument */
int verify_adressing_mode(int opcode, int src_adr, int dest_adr);
