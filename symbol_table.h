/* [DOCS NEEDED] */
#pragma once

#include "parse_util.h"

typedef struct SymbolTableNode {
  char *name;
  char type;
  int value;
  int linker_flag; /* 0 - None, 1 - Entry, 2 - External */
  LogContext context;
  struct SymbolTableNode *next;
} SymbolTableNode;

typedef struct SymbolTable {
  SymbolTableNode *head;
} SymbolTable;

/* [DOCS NEEDED] returns false if the symbol was already in the table but wasn't marked
 * to-be-filled-in, else true */
int append_symbol(SymbolTable *table, char *name, char type, int value, LogContext context);

/* [DOCS NEEDED] returns false on error */
int mark_symbol(SymbolTable *table, char *name, int flag, LogContext context);

/* [DOCS NEEDED] returns NULL if no symbol was found */
SymbolTableNode *search_symbol(SymbolTable *table, char *name);

/* [DOCS NEEDED] */
void free_symbol_table(SymbolTable table);
