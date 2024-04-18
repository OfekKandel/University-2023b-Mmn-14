/* The preprocessor (pre-assembler), as described in the assignment. Parses all macros to create a
 * new file (.am) with no macros, only source code */
#pragma once

#define PROCESSED_FILE_SUFFIX ".am"

/* Reads a source file and (if it includes macros) parses its macros (reads their declarations and
 * replaces their invocations)
 * Input: The filename (no extension) of the file to process
 * Output: If there were macros creates a .am file (with the given filename) with them parsed,
 *         returns 0 if no .am file was, 1 if one was create, -1 if the preprocessor failed*/
int process_file(char *filename);
