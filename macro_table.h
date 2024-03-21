/* [DOCS NEEDED] */
#pragma once

/* [DOCS NEEDED] */
typedef struct MacroLineNode {
  char *content;
  struct MacroLineNode *next;
} MacroLineNode;

/* [DOCS NEEDED] */
typedef struct {
  MacroLineNode *head;
  MacroLineNode *tail;
} MacroLines;

/* [DOCS NEEDED] */
typedef struct MacroNode {
  char *name;
  MacroLines lines;
  struct MacroNode *next;
} MacroNode;

/* [DOCS NEEDED] */
typedef struct {
  MacroNode *head;
} MacroTable;

/* Table functions */
/* [DOCS NEEDED] */
MacroLines *insert_macro(MacroTable *table, char *name);
/* [DOCS NEEDED] */
MacroLines *get_macro_lines(MacroTable *table, char *name);
/* [DOCS NEEDED] */
void free_macro_table(MacroTable table);

/* Lines functions */
/* [DOCS NEEDED] */
void append_line(MacroLines *lines, char *line);
