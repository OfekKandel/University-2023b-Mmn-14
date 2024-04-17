/* [DOCS NEEDED] */
#pragma once
#include <stdio.h>

/* File functions --------------------------------------- */

/* Given a filename (with no extension) and an extension, adds the extension to
 * the filename. Returns the new filename, memory returned by this function must
 * be freed) */
char *with_ext(const char *filename, const char *extension);

/* [DOCS NEEDED] */
FILE *open_with_ext(const char *filename, const char *extension, const char *mode,
                    const char *error_desc);

/* [DOCS NEEDED] */
int remove_file(const char *filename, const char *extension, const char *error_desc);

/* [DOCS NEEDED] */
int is_file_empty(const char *filename, const char *extension, const char *error_desc);

/* Logger ----------------------------------------------- */

/* [DOCS NEEDED] */
typedef struct LogContext {
  char *filename;
  char *file_ext;
  int line;
} LogContext;

/* [DOCS NEEDED] */
void print_log_context(LogContext log_info, char *severity);
