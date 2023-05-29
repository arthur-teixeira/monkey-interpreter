#include "../evaluator/evaluator.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define PROMPT ">> "

typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

InputBuffer *new_input_buffer() {
  InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void close_input_buffer(InputBuffer *input_buffer) {
  free(input_buffer->buffer);
  free(input_buffer);
}

void read_input(InputBuffer *input_buffer) {
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void print_token(Token *tok) {
  printf("Type: %s\n", TOKEN_STRING[tok->Type]);
  printf("Value: %s\n", tok->literal);
  printf("---------\n");
}

void print_parser_errors(Parser *p) {
  Node *cur_error = p->errors->tail;
  assert(cur_error != NULL);

  while (cur_error != NULL) {
    printf("%s\n", (char *)cur_error->value);
    cur_error = cur_error->next;
  }
}

void start() {
  InputBuffer *input_buf = new_input_buffer();
  Environment *env = new_environment();

  while (true) {
    printf(PROMPT);
    read_input(input_buf);

    if (*(input_buf->buffer) == '\0') {
      return;
    }

    ResizableBuffer buf;
    init_resizable_buffer(&buf, 100);

    Lexer *l = new_lexer(input_buf->buffer);
    Parser *p = new_parser(l);
    Program *program = parse_program(p);
    if (p->errors->size > 0) {
      print_parser_errors(p);
      free(buf.buf);
      free_parser(p);
      free_program(program);
      continue;
    }

    Object *evaluated = eval_program(program, env);
    if (evaluated != NULL) {
      inspect_object(&buf, evaluated);
      printf("%s\n", buf.buf);
    }

    free(buf.buf);
    free_parser(p);
    free_program(program);
  }
  close_input_buffer(input_buf);
}
