/* [DOCS NEEDED] */
#include "symbol_table.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int append_symbol(SymbolTable *table, char *name, char type, int value) {
  SymbolTableNode *iter;

  /* Create node */
  SymbolTableNode *node = malloc(sizeof(SymbolTableNode));
  node->name = calloc(strlen(name) + 1, sizeof(char));
  strcpy(node->name, name);
  node->type = type;
  node->value = value;

  /* Check that node isn't in the table */
  for (iter = table->head; iter != NULL; iter = iter->next)
    if (strcmp(iter->name, name) == 0)
      return false;

  /* Insert node at the start of the table */
  node->next = table->head;
  table->head = node;

  return true;
}

static void free_symbol_table_node(SymbolTableNode *node) {
  if (node == NULL)
    return;
  free_symbol_table_node(node->next);
  free(node->name);
  free(node);
}

void free_symbol_table(SymbolTable table) {
  free_symbol_table_node(table.head);
}
