#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_reader.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../evaluator/evaluator.h"
#include "../str_utils/str_utils.h"

FILE *open_source_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "ERROR: Could not open file: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return file;
}

void read_file(char *buf, size_t size, FILE *file) {
  fread(buf, sizeof(char), size, file);
  if (ferror(file) != 0) {
    fprintf(stderr, "ERROR: Could not read file");
    exit(EXIT_FAILURE);
  }
}

void eval_file(const char *filename) {
    FILE *file = open_source_file(filename);
    char buf[100];
    read_file(buf, 100, file);
    Lexer *l = new_lexer(buf);
    Parser *p = new_parser(l);
    Program *program = parse_program(p);
    if (p->errors->size > 0) {
      print_parser_errors(p);
      free_parser(p);
      free_program(program);
    }

    Environment *env = new_environment();
    Object *evaluated = eval_program(program, env);

    if (evaluated != NULL) {
      ResizableBuffer program_buffer;
      init_resizable_buffer(&program_buffer, 100);
      inspect_object(&program_buffer, evaluated);
      printf("%s\n", program_buffer.buf);
      free(program_buffer.buf);
    }
}
