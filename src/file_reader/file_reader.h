#ifndef FILE_READER_H
#define FILE_READER_H
#include <stdio.h>

FILE *open_source_file(const char *filename);
void read_file(char *buf, size_t size, FILE *file);
void eval_file(const char *filename);
void compile_file(const char *filename);

#endif // FILE_READER_H
