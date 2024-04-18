/* All the functions and structs needed to use a binary table, meaning an ordered list of binary
 * words of length 14, including support for storing symbols next to words in order for them to be
 * filled-in in the second pass. (note: while the assignment states this *can* be done with an array
 * I don't think it's the best practice to assume a maximum length in any situation, so I have
 * used a binary list) */
#pragma once

#define BINARY_WORD_SIZE 14

/* A 14 bit binary word  */
typedef struct BinaryWord {
  unsigned int content : BINARY_WORD_SIZE;
} BinaryWord;

/* A node in the binary table containing a single binary word and (alternatively) a symbol as a
 * string for it to be replaced by */
typedef struct BinaryTableNode {
  BinaryWord content;
  char *symbol;
  struct BinaryTableNode *next;
} BinaryTableNode;

/* A binary table, including a counter (of the number of words in the table) */
typedef struct BinaryTable {
  int counter;
  BinaryTableNode *head;
  BinaryTableNode *tail;
} BinaryTable;

/* Appends a word to the given table
 * Input: The table to add the word to and the word itself */
void append_word(BinaryTable *table, BinaryWord word);

/* Appends a symbol word to the given table, meaning an empty word with a symbol stored next to it
 * for it to be replaced by in the second pass
 * Input: The table to add the symbol word to and the name of the symbol */
void append_symbol_word(BinaryTable *table, char *symbol);

/* Takes a binary word and converts it into the encoded format as stated in the assignment
 * Input: A binary word to incode and an 8-char long string (includes \0) to output the word to
 * Output: The function has no return value, but it outputs the encoded string into out */
void get_encoded_word(BinaryWord word, char out[8]);

/* Frees the dynamically allocated memory used by the binary table */
void free_binary_table(BinaryTable table);
