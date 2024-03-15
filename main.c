/* [DOCS NEEDED] */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "preprocessor.h"

/* [DOCS NEEDED] */
int analyze_file(char *filename) {
  /* Add .as to filename */
  char *filepath = malloc(sizeof(char) * strlen(filename) + 2);
  strcpy(filepath, filename);
  strcat(filepath, ".as");

  /* Check if file exists */
  if (access(filepath, F_OK) == -1) {
    printf("ERROR: File '%s' does not exist; skipping\n", filepath);
    free(filepath);
    return false;
  }

  /* Run preprocessor */
  if (!process_file(filename)) {
    free(filepath);
    return false;
  }

  free(filepath);
  return true;
}

/* [DOCS NEEDED] */
int main(int argc, char *argv[]) {
  int i;

  /* [FOR DEBUG] */
  argc = 2;
  argv[1] = "inputs/ps";

  if (argc <= 1) {
    printf("ERROR: No files to assemble were given");
    return EXIT_FAILURE;
  }


  /* Analyze every given file */
  for (i = 1; i < argc; i++) {
    if (!analyze_file(argv[i]))
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
