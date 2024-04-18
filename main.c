/* The main file of the program. Here is a brief explanation of the structure of the program:
 * On the highest level the program contains 2 parts: The preprocessor and the assembler, found in
 * files assembler.h and preprocessor.h (source files with the same name). The preprocessor
 * handles all macros, and its output file (if there is one) is passed to the assembler. The
 * preprocessor is rather small and uses the macro_table.h file to store the macros. The assembler
 * is the bulk of the program's logic and can be broken down into 3 parts: parsing, encoding, and
 * writing. The parsing step is handled by the parser.h file (though it leaves some extra parsing
 * to be performed later by the parse_util.h file), and it breaks down the raw text of a line
 * into its parts, but not encoding the underlying meaning of the line, the separation is purely
 * syntactical (syntax errors come from here). The encoding step uses the parsed line and deciphers
 * its meaning, encoding it into the instruction, data, and symbol tables, this is handled mostly
 * by the encoding_util.h file. Finally the writing part takes the tables generated in the previous
 * steps and writes the .ob, .ent, and .ext files for the program */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "assembler.h"
#include "parse_util.h"
#include "preprocessor.h"

#define SOURCE_FILE_SUFFIX ".as"

/* Checks that the file exists, runs both the preprocessor and the assembler on the file */
void analyze_file(char *filename) {
  int preprocessor_result;

  /* Add .as to filename */
  char *filepath = with_ext(filename, SOURCE_FILE_SUFFIX);

  /* Check if file exists */
  if (access(filepath, F_OK) == -1) {
    printf("ERROR: File '%s' does not exist; skipping\n", filepath);
    free(filepath);
    return;
  }

  /* Run preprocessor - fill-in macros */
  preprocessor_result = process_file(filename);
  if (preprocessor_result == -1) { /* Preprocessor failed */
    free(filepath);
    return;
  }

  /* Run assembler */
  assemble_file(filename, (preprocessor_result) ? PROCESSED_FILE_SUFFIX : SOURCE_FILE_SUFFIX);

  free(filepath);
}

/* The program starts here. Checks that enough command line arguments were given and runs
 * analyze_file on every file in argv */
int main(int argc, char *argv[]) {
  int i;

  if (argc <= 1) {
    printf("ERROR: No files to assemble were given\n");
    return EXIT_FAILURE;
  }

  /* Analyze every given file */
  for (i = 1; i < argc; i++)
    analyze_file(argv[i]);

  return EXIT_SUCCESS;
}
