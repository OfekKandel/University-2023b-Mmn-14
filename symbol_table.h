/* [DOCS NEEDED] */
#pragma once

typedef struct SymbolTableNode {
  char *name;
  char type;
  int value;
  struct SymbolTableNode *next;
} SymbolTableNode ;

typedef struct SymbolTable {
  SymbolTableNode *head;
} SymbolTable;


/* [DOCS NEEDED] returns false if the symbol was already in the table, else true */
int append_symbol(SymbolTable *table, char *name, char type, int value);

/* [DOCS NEEDED] returns NULL if no symbol was found */
SymbolTableNode *search_symbol(SymbolTable *table, char *name);

/* [DOCS NEEDED] */
void free_symbol_table(SymbolTable table);
