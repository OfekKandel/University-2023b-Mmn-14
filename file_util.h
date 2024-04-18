/* A general util file used in different parts of the program focused on easier handling of files,
 * specifically operations using a filename and an extensions (as opposed to the combined path),
 * opening/closing/removing of files with proper error-handling, and logging of errors with file and
 * line number information attached to them (log context)*/
#pragma once
#include <stdio.h>

/* File functions --------------------------------------- */

/* Given a filename (with no extension) and an extension, adds the extension to
 * the filename. Returns the new filename, memory returned by this function must
 * be freed) */

/* Creates a new string from a filename and an extension string
 * Input: The filename and the extension (format ".ob")
 * Output: Returns a new dynamically allocated string (format "file.ext") (needs to be freed) */
char *with_ext(const char *filename, const char *extension);

/* Opens a file, handles and prints errors in opening the file if they occur
 * Input: The file (as a filename and extension), the mode in which the file should be opened (fopen
 *        modes), and a description of the file to print in the error
 * Output: Returns a FILE pointer to/from which you can write/read (based on the mode), NULL if the
 *         file could not be opened */
FILE *open_with_ext(const char *filename, const char *extension, const char *mode,
                    const char *error_desc);

/* Removes (deletes) a file, handles and prints errors in removing the file if they occur
 * Input: The file (as a filename and extension), and a description of the file for the error
 * Output: Returns false on error, else true. Also removes the file of course */
int remove_file(const char *filename, const char *extension, const char *error_desc);

/* Checks if a file is empty, prints an error in case that could not be checked
 * Input: A file (as a filename and extension), and a description of the file for the error
 * Output: True if the file is empty, false if not (or if it could not be opened to check) */
int is_file_empty(const char *filename, const char *extension, const char *error_desc);

/* Logger ----------------------------------------------- */

/* The context for a given warning/error in a file (file and line number) */
typedef struct LogContext {
  char *filename;
  char *file_ext;
  int line;
} LogContext;

/* Prints a header for a warning/error with file and line information
 * Input: The log context of the line (contains file and line number), and a string to be printed as
 * the severity (e.g. "ERROR", "WARNING")
 * Output: The function returns nothing, it prints (without a new line) a header for the log in the
 * following format: "SEVERITY[FILE - LINE_NUMBER]: " (no newline, with space) */
void print_log_context(LogContext log_info, char *severity);
