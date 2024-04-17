/* [DOCS NEEDED] */
#pragma once

#include "file_util.h"

#define MAX_LINE_LEN 80

typedef enum {
  /* No content */
  Error,
  Empty,
  Comment,
  /* With content */
  Define,
  DotInstruction,
  Command
} LineType;

typedef struct DefineLine {
  char *name;
  char *value;
} DefineLine;

typedef struct DotInstructionLine {
  char *label;
  char *name;
  char *args_start; /* Pointer to the first argument, should be iterated */
} DotInstructionLine;

typedef struct CommandLine {
  char *label; /* Optional */
  char *cmd;
  char *src_arg;  /* Optional */
  char *dest_arg; /* Optional */
} CommandLine;

typedef struct ParsedLine {
  LineType line_type;
  union {
    DefineLine define;
    DotInstructionLine dot_instruction;
    CommandLine command;
  } content;
} ParsedLine;

/* [DOCS NEEDED] */
ParsedLine parse_line(char line[MAX_LINE_LEN], LogContext context);
