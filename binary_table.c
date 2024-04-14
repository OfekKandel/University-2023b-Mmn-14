/* [DOCS NEEDED] */
#include "binary_table.h"
#include "parse_util.h"
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

/* [DOCS NEEDED] returns '-' if more than two bits were given */
static char get_encoded_two_bits(int bits) {
  switch (bits) {
  case 0:
    return '*';
  case 1:
    return '#';
  case 2:
    return '%';
  case 3:
    return '!';
  }
  return '-';
}

void get_encoded_word(BinaryWord word, char out[8]) {
  int i, content = word.content;
  for (i = 6; i >= 0; i--) {
    out[i] = get_encoded_two_bits(content % 4); /* Extracts two last bits */
    content >>= 2;                              /* Truncates two last bits */
  }
  out[7] = '\0';
}

static void free_binary_node(BinaryTableNode *node) {
  if (node == NULL) return;
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
  if (node == NULL) return;
  free_symbol_ref_table_node(node->next);
  free(node->symbol_name);
  free(node);
}

void free_symbol_ref_table(SymbolRefTable table) { free_symbol_ref_table_node(table.head); }
