/* [DOCS NEEDED] */
#pragma once

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
  char *arg1; /* Optional */
  char *arg2; /* Optional */
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
ParsedLine parse_line(char line[MAX_LINE_LEN]);

/* Given a filename (with no extension) and an extension, adds the extension to
 * the filename. Returns the new filename, memory returned by this function must
 * be freed) */
char *with_ext(const char *filename, const char *extension);

/* Smaller functions */

/* Return a pointer to the next position that isn't a letter in a string */
char *skip_alpha(char *str);
/* Return a pointer to the next position that isn't a space in a string */
char *skip_space(char *str);
/* [DOCS NEEDED] */
char *skip_to_space(char *str);
/* [DOCS NEEDED] */
int is_terminator(char c);
