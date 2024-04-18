/* All the functions and structs needed to use the symbol table, as described in the algorithms
 * in the assignment. These functions include adding a symbol (with a value and a type) to the
 * symbol table, marking a symbol as entry/external (can be done before its value is added), and
 * searching and editing symbol before they are in the table */
#pragma once

#include "parse_util.h"

/* Linker flags (entry/extern and none) */
typedef enum { NoLinkerFlag, EntryLinkerFlag, ExternalLinkerFlag } LinkerFlag;

/* A node in the symbol table, including name, type, value, and flag */
typedef struct SymbolTableNode {
  char *name;
  char type; /* In the assignment this is a string but it's simpler with a char */
  int value;
  LinkerFlag linker_flag;
  LogContext context; /* Stored alongside the symbol for error logging */
  struct SymbolTableNode *next;
} SymbolTableNode;

/* The symbol table itself */
typedef struct SymbolTable {
  SymbolTableNode *head;
} SymbolTable;

/* Adds a symbol to the symbol table
 * Input: The symbol table to add the symbol to, they symbol's name, type, value, and the
 *        log-context in which the symbol was defined
 * Output: Returns false if the symbol was already in the table (and with a value), else true */
int append_symbol(SymbolTable *table, char *name, char type, int value, LogContext context);

/* Marks a symbol in the table as entry/external, can be called before the symbol added
 * Input: The symbol table to edit, the name of the symbol, which flag to mark the symbol as
 *        (entry/external) and the log-context in which the symbol was marked
 * Output: Returns false if there is an error (errors are printed), else true */
int mark_symbol(SymbolTable *table, char *name, LinkerFlag flag, LogContext context);

/* Searches a symbol in the given table by
 * Input: The table to search in and the name of the symbol
 * Output: Returns the SymbolTableNode of the node if it was found else NULL */
SymbolTableNode *search_symbol(SymbolTable *table, char *name);

/* Frees the dynamically allocated memory used by the SymbolTable struct */
void free_symbol_table(SymbolTable table);
