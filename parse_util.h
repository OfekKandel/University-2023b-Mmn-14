/* [DOCS NEEDED] */
#pragma once

#include "file_util.h"

/* Argument parsing functions ------------- */
typedef struct ScanArgumentResult {
  int skip;
  enum {
    LastArgument, /* Formatting is OK, no next argument to skip to */
    ArgumentScanShouldSkip, /* Skip forward by skip */
    ArgumentScanNoContent, /* No content, empty space */
    MissingFirstArg, /* Missing first argument (e.g. cmd ,1) */
    MissingSeparator, /* Missing separator between arguments (e.g. cmd a b) */
    DoubleSeparator, /* Multiple consecutive separators between arguments (e.g. a ,, b) */
    TrailingSeprator /* Trailing separator after arguments (e.g. cmd a , b ,) */
  } status;
} ScanArgumentResult;

typedef struct ScanStringResult {
  int skip;
  enum {
    StringScanShouldSkip, /* Success */
    SrtingScanNoContent, /* No content, empty space */
    MissingOpeningQuotation, /* Missing opening quotation mark (e.g. .string fhe") */
    MissingClosingQuotation, /* Missing closing quotation mark (e.g. .string "fhe) */
    ExtraneousText /* Extraneous text after string */
  } status;
} ScanStringResult;

/* [DOCS NEEDED] should be called without leading space */
ScanArgumentResult scan_argument(char content[], char separator);

/* [DOCS NEEDED] */
ScanStringResult scan_string(char content[]);

/* [DOCS NEEDED] returns true if the text not a valid number */
int scan_number(char *text, int *out, LogContext context);

/* [DOCS NEEDED] returns a string containing the index/constant's name, NULL on error, the array
 * symbol will be terminated, meaning the given content argument can be used to access it */
char *scan_array_index(char content[], LogContext context);

/* Smaller parsing functions --------------- */

/* [DOCS NEEDED] return whether a given string is a register name */
int is_register_name(char *arg);

/* [DOCS NEEDED] */
int is_reserved_word(char *word);

/* Return a pointer to the next position that isn't a letter in a string */
char *skip_alpha(char *str);

/* Return a pointer to the next position that isn't a space in a string */
char *skip_space(char *str);

/* [DOCS NEEDED] */
char *skip_to_space(char *str);

/* [DOCS NEEDED] */
int is_terminator(char c);
