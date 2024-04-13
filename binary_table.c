/* [DOCS NEEDED] */
#include "binary_table.h"
#include <stdlib.h>
#include <string.h>

void append_word(BinaryTable *table, BinaryWord word) {
  BinaryTableNode *node = malloc(sizeof(BinaryTableNode));
  node->content.content = word.content;
  node->symbol = NULL;
  table->counter++;

  if (table->head == NULL)
    table->head = table->tail = node;
  else
    table->tail = table->tail->next = node;
}
 
void append_symbol_word(BinaryTable *table, char *symbol) {
  BinaryTableNode *node = malloc(sizeof(BinaryTableNode));
  node->content.content = 0;
  table->counter++;

  /* Dynamically allocate space for the symbol name */
  node->symbol = calloc(strlen(symbol) + 1, sizeof(char));
  strcpy(node->symbol, symbol);

  if (table->head == NULL)
    table->head = table->tail = node;
  else
    table->tail = table->tail->next = node;
}

static void free_binary_node(BinaryTableNode *node) {
  if (node == NULL)
    return;
  free_binary_node(node->next);
  free(node->symbol);
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
