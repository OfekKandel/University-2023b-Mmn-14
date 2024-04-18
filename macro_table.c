/* The source file for the macro_table.h file. Both the macro table and macro lines are implemented
 * as linked lists */

#include "macro_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* [Docs in header file], dynamically allocates memory both for the node and for the name (which
 * it copies with strcpy) */
MacroLines *insert_macro(MacroTable *table, char *name) {
  MacroNode *node = malloc(sizeof(MacroNode));

  node->name = malloc(sizeof(char) * strlen(name) + 1);
  strcpy(node->name, name);

  node->next = table->head;
  table->head = node;

  node->lines.head = NULL;
  return &node->lines;
}

/* [Docs in header file] iterates over the linked list until it finds the name (or returns NULL) */
MacroLines *get_macro_lines(MacroTable *table, char *name) {
  MacroNode *node;

  for (node = table->head; node != NULL; node = node->next)
    if (strcmp(node->name, name) == 0) return &node->lines;
  return NULL;
}

/* Frees the dynamically allocated memory used by the given a MacroLineNode struct and recursively
 * frees the memory for all following nodes */
static void free_line_nodes(MacroLineNode *node) {
  if (node == NULL) return;

  free_line_nodes(node->next);
  free(node->content);
  free(node);
}

/* Frees the dynamically allocated memory used by the given a MacroNode struct and recursively
 * frees the memory for all following nodes */
static void free_table_node(MacroNode *node) {
  if (node == NULL) return;

  free_table_node(node->next);
  free_line_nodes(node->lines.head);
  free(node->name);
  free(node);
}

/* [Docs in header file] simply calls free_table_node on the first node */
void free_macro_table(MacroTable table) { free_table_node(table.head); }

/* [Docs in the header file], dynamically allocates memory both for the node and for the line's
 * content (which it copies with strcpy) */
void append_line(MacroLines *lines, char *line) {
  MacroLineNode *node = malloc(sizeof(MacroLineNode));
  node->content = malloc(sizeof(char) * strlen(line) + 1);
  strcpy(node->content, line);

  if (lines->head == NULL)
    lines->head = node;
  else
    lines->tail->next = node;

  lines->tail = node;
}
