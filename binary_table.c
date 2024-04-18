/* The source file for the binary_table.h file */
#include "binary_table.h"
#include "file_util.h"
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

void append_symbol_word(BinaryTable *table, char *symbol, LogContext context) {
  BinaryTableNode *node = malloc(sizeof(BinaryTableNode));
  node->content.content = 0;
  node->context = context;
  table->counter++;

  /* Dynamically allocate space for the symbol name */
  node->symbol = calloc(strlen(symbol) + 1, sizeof(char));
  strcpy(node->symbol, symbol);

  if (table->head == NULL)
    table->head = table->tail = node;
  else
    table->tail = table->tail->next = node;
}

/* Converts a given two bits into their corresponding char based on the specified encoding
 * Input: A 2 bit int
 * Output: A single char representing the 2 bits
 * Assumes: The given number is not larger than 2 bits can store (returns '-' in that case) */
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

/* Frees the given binary node, and recursively frees every node after it */
static void free_binary_node(BinaryTableNode *node) {
  if (node == NULL) return;
  free_binary_node(node->next);
  free(node->symbol);
  free(node);
}

void free_binary_table(BinaryTable table) { free_binary_node(table.head); }
