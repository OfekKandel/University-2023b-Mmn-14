/* The assembler (not including the preprocessor), contains a single function which assembles a
 * given file  */
#pragma once

#define OBJECT_FILE_SUFFIX ".ob"
#define ENTRIES_FILE_SUFFIX ".ent"
#define EXTERNALS_FILE_SUFFIX ".ext"

/* Runs the assembler on a given source file, creating all necessary output files.
 * Input: The filename to assemble, and its file extension (format: ".as").
 * Output: Returns false if any errors occurred, else true, also writes all files.
 * Errors: Any errors in the in the source file will be printed, also all file-related errors
 * Assumes: the given file has already passed the preprocessor. */
int assemble_file(char *filename, char *suffix);
