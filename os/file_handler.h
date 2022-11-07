#ifndef FILE_HANDLER
#define FILE_HANDLER

#include <stdio.h>

void init_formatted_drive(char* dirname);
int create_file(FILE* drive, char* filename, char* directory, int sectors);

#endif
