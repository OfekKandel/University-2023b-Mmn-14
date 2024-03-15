/* [DOCS NEEDED] */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse_util.h"
#include "preprocessor.h"

#define SOURCE_FILE_SUFFIX ".as"

/* [DOCS NEEDED] */
int analyze_file(char *filename) {
  int preprocessor_result;

  /* Add .as to filename */
  char *filepath = with_ext(filename, SOURCE_FILE_SUFFIX);

  /* Check if file exists */
  if (access(filepath, F_OK) == -1) {
    printf("ERROR: File '%s' does not exist; skipping\n", filepath);
    free(filepath);
    return false;
  }

  /* Run preprocessor */
  if ((preprocessor_result = process_file(filename)) == -1) {
    free(filepath);
    return false;
  }

  /* Select next file to run assembler on */
  if (preprocessor_result == 1) {
    free(filepath);
    filepath = with_ext(filename, PROCESSED_FILE_SUFFIX);
  }

  /* Run assembler */

  free(filepath);
  return true;
}

/* [DOCS NEEDED] */
int main(int argc, char *argv[]) {
  int i;

  /* [FOR DEBUG] */
  char *base_name = "inputs/no_mcr";
  if (argc <= 1) {
    argc = 2;
    argv[1] = base_name;
  }

  if (argc <= 1) {
    printf("ERROR: No files to assemble were given\n");
    return EXIT_FAILURE;
  }

  /* Analyze every given file */
  for (i = 1; i < argc; i++)
    analyze_file(argv[i]);

  return EXIT_SUCCESS;
}
