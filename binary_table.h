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


/* TODO: Remove this it is not needed */

/* [DOCS NEEDED] */
typedef struct SymbolRefTableNode {
  int ref_idx;
  char *symbol_name;
  struct SymbolRefTableNode *next;
} SymbolRefTableNode;

/* [DOCS NEEDED] */
typedef struct SymbolRefTable {
  SymbolRefTableNode *head;
  SymbolRefTableNode *tail;
} SymbolRefTable;

/* [DOCS NEEDED] */
void append_symbol_ref(SymbolRefTable *table, int ref_idx, char *symbol_name);

/* [DOCS NEEDED] */
void free_symbol_ref_table(SymbolRefTable table);
