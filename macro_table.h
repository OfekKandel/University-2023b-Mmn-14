/* All the functions and structs needed to use the macro table. That is the table used by the
 * preprocessor to scan-in and store macro names along with the lines they are equivalent to and
 * then search the macro by name as it appears and get the appropriate lines to replace it with */
#pragma once

/* A node in the list of lines a macro is equivalent to */
typedef struct MacroLineNode {
  char *content; /* The line itself */
  struct MacroLineNode *next;
} MacroLineNode;

/* A list of lines that a macro is equivalent to */
typedef struct MacroLines {
  MacroLineNode *head;
  MacroLineNode *tail;
} MacroLines;

/* A node in the macro table containing the name of the macro and the list of lines it is equivalent
 * to (as well as the next node) */
typedef struct MacroNode {
  char *name;
  MacroLines lines;
  struct MacroNode *next;
} MacroNode;

/* The macro table (as described abstractly in the assignment), containing each macro's name and
 * then lines it is equivalent to */
typedef struct MacroTable {
  MacroNode *head;
} MacroTable;

/* MacroTable functions */

/* Inserts a macro (with no lines) into the macro table
 * Input: the table to add the macro to and the macro's name
 * Output: A pointer to an (on return) empty MacroLines struct that is stored with the macro in the
 *         table, it contains the lines the macro is equivalent to and can be appended to */
MacroLines *insert_macro(MacroTable *table, char *name);

/* Gets the MacroLines struct for a given macro name in the table, allowing to retrieve the lines
 * a macro is equivalent to by name when it is used in the program
 * Input: The macro table from which to get the lines, and the macro's name
 * Output: A pointer to the macro's MacroLines struct, NULL if no macro with the name was found */
MacroLines *get_macro_lines(MacroTable *table, char *name);

/* Frees all dynamically allocated memory used by the MacroTable struct */
void free_macro_table(MacroTable table);

/* MacroLines functions */

/* Appends a line to the end of a MacroLines struct (to the end of the list of lines)
 * Input: The MacroLines struct to add the line to, and the line itself
 * Output: There is no return value, but the MacroLines struct is appended to */
void append_line(MacroLines *lines, char *line);
