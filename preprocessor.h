/* [DOCS NEEDED] */
#pragma once

#define PROCESSED_FILE_SUFFIX ".am"

/* [DOCS NEEDED] return -1 if preprocessor failed, 0 if no .am file was created,
 * and 1 if a .am file was created */
int process_file(char *filename);
