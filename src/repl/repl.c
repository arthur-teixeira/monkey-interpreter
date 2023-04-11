#include "../lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

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

void print_prompt() { printf(PROMPT); }

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

void start() {
    InputBuffer *buf = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(buf);

        if (*(buf->buffer) == '\0') {
            return;
        }

        Lexer *l = new_lexer(buf->buffer);

        for (Token tok = next_token(l); tok.Type != END_OF_FILE; tok = next_token(l)) {
            print_token(&tok);
        }

        free_lexer(l);
    }

    assert(0 && "unreachable");
    close_input_buffer(buf);
}
