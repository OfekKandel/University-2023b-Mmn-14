/* [DOCS NEEDED] */
#include "binary_table.h"
#include <stdlib.h>
#include <string.h>

void append_word(BinaryTable *table, BinaryWord word) {
  BinaryTableNode *node = malloc(sizeof(BinaryTableNode));
  node->content = word;
  table->counter++;

  if (table->head == NULL)
    table->head = table->tail = node;
  else
    table->tail->next = node;
}

static void free_binary_node(BinaryTableNode *node) {
  if (node == NULL)
    return;
  free_binary_node(node->next);
  free(node);
}

void free_binary_table(BinaryTable table) { free_binary_node(table.head); }

void append_symbol_ref(SymbolRefTable *table, int ref_idx, char *symbol_name) {
  SymbolRefTableNode *node = malloc(sizeof(SymbolRefTableNode));
  node->ref_idx = ref_idx;
  node->symbol_name = calloc(strlen(symbol_name) + 1, sizeof(char));

  if (table->head == NULL)
    table->head = table->tail = node;
  else
    table->tail->next = node;
}

void free_symbol_ref_table_node(SymbolRefTableNode *node) {
  if (node == NULL)
    return;
  free_symbol_ref_table_node(node->next);
  free(node->symbol_name);
  free(node);
}

void free_symbol_ref_table(SymbolRefTable table) {
  free_symbol_ref_table_node(table.head);
}
