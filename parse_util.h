/* [DOCS NEEDED] */
#pragma once

#include "file_util.h"

/* Argument parsing functions ------------- */

/* [DOCS NEEDED] should be called without leading space, return meaning: >0 - skip distance to next
 * argument, | -1 - Missing first argument (starts with the separator) | -2 - Missing separator
 * between arguments | -3 - two consecutive separators between arguments | -4 - trailing separator
 * after arguments (no next argument) | -5 - no argument at all (only empty space)*/
int scan_argument(char content[], char separator);
/* [DOCS NEEDED] */
int scan_string(char content[]);
/* [DOCS NEEDED] returns true if the text not a valid number */
int scan_number(char *text, int *out, LogContext context);
/* [DOCS NEEDED] returns a string containing the index/constant's name, NULL on error, the array
 * symbol will be terminated, meaning the given content argument can be used to access it */
char *scan_array_index(char content[], LogContext context);

/* Smaller parsing functions --------------- */

/* [DOCS NEEDED] return whether a given string is a register name */
int is_register_name(char *arg);
/* Return a pointer to the next position that isn't a letter in a string */
char *skip_alpha(char *str);
/* Return a pointer to the next position that isn't a space in a string */
char *skip_space(char *str);
/* [DOCS NEEDED] */
char *skip_to_space(char *str);
/* [DOCS NEEDED] */
int is_terminator(char c);
