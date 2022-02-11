#ifndef READING_FILE_H_INCLUDED
#define READING_FILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *Open_File       (const char *const file_name, const char *const mode);
int  Close_File       (FILE *file_ptr, const char *file_name);
long Define_File_Size (FILE * file_ptr);
char *Make_Buffer     (FILE *file_ptr, const long n_symbs);

#endif // READING_FILE_H_INCLUDED
