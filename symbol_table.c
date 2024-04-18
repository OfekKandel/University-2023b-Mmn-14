/* The source file for symbol_table.h */
#include "symbol_table.h"
#include "parse_util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int append_symbol(SymbolTable *table, char *name, char type, int value, LogContext context) {
  SymbolTableNode *node;

  /* Edit the symbol if it's already in the table (and marked to-be-filled-in) */
  node = search_symbol(table, name);
  if (node != NULL) {
    /* Print return false if symbol isn't marked appropriately */
    if (node->type != 'x') return false;
    /* Else edit it */
    node->type = type;
    node->value = value;
    node->context = context;
    return true;
  }

  /* If needed create new node */
  node = malloc(sizeof(SymbolTableNode));
  node->name = calloc(strlen(name) + 1, sizeof(char));
  strcpy(node->name, name);
  node->type = type;
  node->value = value;
  node->linker_flag = NoLinkerFlag;

  /* Insert it at the start of the table */
  node->next = table->head;
  table->head = node;

  return true;
}

int mark_symbol(SymbolTable *table, char *name, LinkerFlag flag, LogContext context) {
  SymbolTableNode *existing;
  existing = search_symbol(table, name);

  /* If the symbol doesn't exist yet we create an empty one */
  if (existing == NULL) {
    append_symbol(table, name, 'x', 0, context); /* x type is for to-be filled-in */
    table->head->linker_flag = flag;
    return true;
  }

  /* If it exists we make sure it isn't marked */
  if (existing->linker_flag != NoLinkerFlag) {
    if (existing->linker_flag == flag) return true; /* If its just a re-marking then return */
    print_log_context(context, "ERROR");
    printf("Symbol cannot be not marked as both entry and external\n");
    return false;
  }

  /* Mark the symbol */
  existing->linker_flag = flag;
  return true;
}

SymbolTableNode *search_symbol(SymbolTable *table, char *name) {
  SymbolTableNode *iter;

  /* Find symbol */
  for (iter = table->head; iter != NULL; iter = iter->next)
    if (strcmp(iter->name, name) == 0) return iter;

  return NULL;
}

static void free_symbol_table_node(SymbolTableNode *node) {
  if (node == NULL) return;
  free_symbol_table_node(node->next);
  free(node->name);
  free(node);
}

void free_symbol_table(SymbolTable table) { free_symbol_table_node(table.head); }
