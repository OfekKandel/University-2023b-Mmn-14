/* [DOCS NEEDED] */
#pragma once

#define BINARY_WORD_SIZE 14

/* [DOCS NEEDED] */
typedef struct BinaryWord {
  unsigned int content : BINARY_WORD_SIZE;
} BinaryWord;

/* [DOCS NEEDED] */
typedef struct BinaryTableNode {
  BinaryWord content;
  char *symbol;
  struct BinaryTableNode *next;
} BinaryTableNode;

/* [DOCS NEEDED] */
typedef struct BinaryTable {
  int counter;
  BinaryTableNode *head;
  BinaryTableNode *tail;
} BinaryTable;

/* [DOCS NEEDED] */
void append_word(BinaryTable *table, BinaryWord word);

/* [DOCS NEEDED] */
void append_symbol_word(BinaryTable *table, char *symbol);

/* [DOCS NEEDED] */
void get_encoded_word(BinaryWord word, char out[8]);

/* [DOCS NEEDED] */
void free_binary_table(BinaryTable table);
