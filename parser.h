/* The file for the parser, which includes a single function that takes a source code line
 * and breaks it down syntactically into a label, the type of line (comment, command, instruction),
 * the arguments etc. This function does perform any encoding or deciphering of the underlying
 * meaning of the line, it purely breaks down one big string into its sections so it is easier to
 * work with and prints and syntactical errors */
#pragma once

#include "file_util.h"

#define MAX_LINE_LEN 80

/* The different types of lines */
typedef enum LineType { 
  /* No content */
  Error, /* A line which contains a syntactical error */
  Empty,
  Comment,
  /* With content */
  Define,
  DotInstruction,
  Command
} LineType;

/* A line for a define statement (e.g. .define a = 5) */
typedef struct DefineLine {
  char *name;
  char *value;
} DefineLine;

/* A line for a dot instruction (.data, .string, .extern, .entry) */
typedef struct DotInstructionLine {
  char *label; /* Optional, sometimes meaningless */
  char *name;
  char *args_start; /* Pointer to the first argument, should be iterated */
} DotInstructionLine;

/* A line for a command (e.g. prn #-5) */
typedef struct CommandLine {
  char *label; /* Optional */
  char *cmd;
  char *src_arg;  /* Optional */
  char *dest_arg; /* Optional */
} CommandLine;

/* A struct containing the parsed version of every possible line */
typedef struct ParsedLine {
  LineType line_type; /* Read the content based on this */
  union {
    DefineLine define;
    DotInstructionLine dot_instruction;
    CommandLine command;
  } content;
} ParsedLine;

/* Parses a given raw line into its parts (as described at the top of the header file)
 * Input: The line to parse (as a string), and the line's log-context
 * Output: Returns a ParsedLine struct containing the parsed line, with it's line type and content,
 *         also prints any syntactical errors encountered (and sets the line type to Error) */
ParsedLine parse_line(char line[MAX_LINE_LEN], LogContext context);
