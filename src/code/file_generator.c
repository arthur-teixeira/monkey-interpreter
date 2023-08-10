#include "file_generator.h"
#include "../big_endian/big_endian.h"
#include "code.h"
#include <assert.h>
#include <stdio.h>

#define ARRAY_LEN(x) sizeof(x) / sizeof(x[0])

static void write_byte(FILE *file, int byte) {
  if (fputc(byte, file) == EOF) {
    perror("ERROR: could not write to file");
    exit(EXIT_FAILURE);
  }
}

void dump_file(const char *filename);

void save_to_file(Bytecode bytecode, const char *filename) {
  FILE *file = fopen(filename, "wb");
  if (!file) {
    perror("ERROR: could not create file");
    exit(EXIT_FAILURE);
  }

  magic_number(file);
  write_constants(bytecode, file);
  write_instructions(bytecode, file);

  if (fclose(file) != 0) {
    perror("ERROR: could not close file");
    exit(EXIT_FAILURE);
  }

  dump_file(filename);
}

void dump_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  assert(file);

  printf("Magic number: ");
  for (size_t i = 0; i < 4; i++) {
    unsigned char c = fgetc(file);
    printf("%x", c);
  }
  printf("\n");

  char num_constants = fgetc(file);
  printf("Number of constants: %d\n", num_constants);

  char num_instructions = fgetc(file);
  printf("Number of instructions: %d\n", num_instructions);

  Instructions ins;
  int_array_init(&ins, num_instructions);

  for (size_t i = 0; i < num_instructions; i++) {
    int_array_append(&ins, fgetc(file));
  }

  ResizableBuffer buf;
  init_resizable_buffer(&buf, 100);
  instructions_to_string(&buf, &ins);

  printf("Instructions: -------------\n%s-----------\n", buf.buf);
  free(buf.buf);
  buf.buf = NULL;
  int_array_free(&ins);
}

static void magic_number(FILE *file) {
  int magic_number[] = {0x0B, 0xAD, 0xF0, 0x0D};

  for (size_t i = 0; i < ARRAY_LEN(magic_number); i++) {
    write_byte(file, magic_number[i]);
  }
}

static void write_constants(Bytecode bytecode, FILE *file) {
  uint8_t len_constants[2];
  big_endian_as_uint16(bytecode.constants.len, len_constants);

  write_byte(file, len_constants[0]);
  write_byte(file, len_constants[1]);

  // TODO: write actual constants to the file
}

static void write_instructions(Bytecode bytecode, FILE *file) {
  uint8_t len_instructions[2];
  big_endian_as_uint16(bytecode.instructions.len, len_instructions);

  write_byte(file, len_instructions[0]);
  write_byte(file, len_instructions[1]);

  for (size_t i = 0; i < bytecode.instructions.len; i++) {
    write_byte(file, bytecode.instructions.arr[i]);
  }
}
