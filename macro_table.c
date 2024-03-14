#include "macro_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MacroLines *insert_macro(MacroTable *table, char *name) {
  MacroNode *node = malloc(sizeof(MacroNode));

  node->name = malloc(sizeof(char) * strlen(name) + 1);
  strcpy(node->name, name);

  node->next = table->head;
  table->head = node;

  node->lines.head = NULL;
  return &node->lines;
}

MacroLines *get_macro_lines(MacroTable *table, char *name) {
  MacroNode *node;

  for (node = table->head; node != NULL; node = node->next)
    if (strcmp(node->name, name) == 0)
      return &node->lines;
  return NULL;
}

/* [DOCS NEEDED] */
static void free_line_nodes(MacroLineNode *node) {
  if (node == NULL)
    return;
  
  free_line_nodes(node->next);
  free(node->content);
  free(node);
}

/* [DOCS NEEDED] */
static void free_table_node(MacroNode *node) {
  if (node == NULL)
    return;
  
  free_table_node(node->next);
  free_line_nodes(node->lines.head);
  free(node->name);
  free(node);
}

void free_table(MacroTable table) {
  free_table_node(table.head);
}

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

