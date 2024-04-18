/* A util file focused on parsing raw text. Includes useful methods for parsing several syntaxes
 * used in the source code (e.g. Arguments with a separator between them or breaking down the
 * array[index] format). As well as smaller functions for example checking if a string is register
 * name or skipping to the next space */
#pragma once

#include "file_util.h"

/* Argument parsing functions ------------- */

/* The result of the scan_argument function (skip is empty unless ArgumentScanShouldSkip) */
typedef struct ScanArgumentResult {
  int skip;
  enum {
    LastArgument,           /* Formatting is OK, no next argument to skip to */
    ArgumentScanShouldSkip, /* Skip forward by skip */
    ArgumentScanNoContent,  /* No content, empty space */
    MissingFirstArg,        /* Missing first argument (e.g. cmd ,1) */
    MissingSeparator,       /* Missing separator between arguments (e.g. cmd a b) */
    DoubleSeparator,        /* Multiple consecutive separators between arguments (e.g. a ,, b) */
    TrailingSeprator        /* Trailing separator after arguments (e.g. cmd a , b ,) */
  } status;
} ScanArgumentResult;

/* The result of the scan_string function (skip is empty unless StringScanShouldSkip) */
typedef struct ScanStringResult {
  int skip;
  enum {
    StringScanShouldSkip,    /* Success */
    SrtingScanNoContent,     /* No content, empty space */
    MissingOpeningQuotation, /* Missing opening quotation mark (e.g. fhe") */
    MissingClosingQuotation, /* Missing closing quotation mark (e.g. "fhe) */
    ExtraneousText           /* Extraneous text after string */
  } status;
} ScanStringResult;

/* Helper function used to scan a list of arguments with a given separator between them. Scans
 * one argument at a time
 * Input: The raw text with the arguments and the expected separator between the arguments
 * Output: Returns A ScanArgumentResult struct, if there is another argument the skip member will
 *         include the number of chars to skip to get to the start of the next argument (on which
 *         this function can be called again) from the given 'content' pointer, else the given
 *         argument may be the last argument or there might be some syntax error based on the
 *         status. The first argument will be terminated with a \0 so its value can be used
 * Assumes: The function is called without leading whitespace */
ScanArgumentResult scan_argument(char content[], char separator);

/* Helper function used to scan a string (e.g. "hello world").
 * Input: The raw text with the quotes
 * Output: A ScanStringResult struct, if the syntax is valid the skip member will contain the
 *         number of chars to skip to the start of the content contained inside the string, else
 *         there is some error which will will be found in the status*/
ScanStringResult scan_string(char content[]);

/* Extracts the value of a number given as a string (in the format described in the assignment),
 * prints errors if the syntax is wrong (e.g. abc, 3f4)
 * Input: The text from which to extract the number, an int pointer out variable to place the
 *        result in, and the line's log-context
 * Output: Returns false on errors, else true. Also outputs the number into the out variable*/
int scan_number(char *text, int *out, LogContext context);

/* Helper function used to parse an argument of the array index type (e.g. array[index])
 * Input: The argument as a string and the line's log-context
 * Output: Returns a string containing the index/constant's name (in the brackets), the array name
 *         will be terminated with a \0 so that its value can be used
 * Assumes: The function is called without leading whitespace */
char *scan_array_index(char content[], LogContext context);

/* Smaller parsing functions --------------- */

/* Checks whether a given string represents a register name in the specified format
 * Input: the argument to check on
 * Output: true if the argument is a register name else false */
int is_register_name(char *arg);

/* Checks whether a given string is a reserved word (command name, instruction name, register etc.)
 * Input: the word to check on
 * Output: true if the word is a reserved word else false */
int is_reserved_word(char *word);

/* Return the next address that is not an alphabetical char
 * Input: The string on which to start checking
 * Output: The address of the non-alphabetical char closest to the start of the string */
char *skip_alpha(char *str);

/* Return the next address that is not a space
 * Input: The string on which to start checking
 * Output: The address of the non-whitespace char closest to the start of the string */
char *skip_space(char *str);

/* Return the next address that is a space
 * Input: The string on which to start checking
 * Output: The address of the whitespace char closest to the start of the string */
char *skip_to_space(char *str);

/* Checks if a given char is a terminator (\n, \0, EOF)
 * Input: The char to check on
 * Output: Returns true if the char is a terminator, else false */
int is_terminator(char c);
