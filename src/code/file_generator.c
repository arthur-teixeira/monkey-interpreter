#include "file_generator.h"
#include "../big_endian/big_endian.h"
#include "code.h"
#include <assert.h>
#include <stdio.h>

#define ARRAY_LEN(x) sizeof(x) / sizeof(x[0])

#define DEBUG true

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

#ifdef DEBUG
    printf("Bytecode as numbers: ");
    for (size_t i = 0; i < bytecode.instructions.len; i++) {
      printf("%d ", bytecode.instructions.arr[i]);
    }
    printf("\n");
#endif

  magic_number(file);
  write_constants(bytecode, file);
  write_instructions(bytecode, file);

  if (fclose(file) != 0) {
    perror("ERROR: could not close file");
    exit(EXIT_FAILURE);
  }

#ifdef DEBUG
  dump_file(filename);
#endif
}

void dump_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  assert(file);

  printf("Magic number: ");
  for (size_t i = 0; i < 6; i++) {
    unsigned char c = fgetc(file);
    printf("%c", c);
  }
  printf("\n");

  int8_t num_constants_buf[2];
  num_constants_buf[0] = fgetc(file);
  num_constants_buf[1] = fgetc(file);

  uint16_t num_constants;
  big_endian_to_uint16(&num_constants, num_constants_buf);
  printf("Number of constants: %d\n", num_constants);

  int8_t num_instructions_buf[2];
  num_instructions_buf[0] = fgetc(file);
  num_instructions_buf[1] = fgetc(file);

  uint16_t num_instructions;
  big_endian_to_uint16(&num_instructions, num_instructions_buf);
  printf("Number of instructions: %d\n", num_instructions);

  Instructions ins;
  int_array_init(&ins, 10);

  int8_t c;
  while ((c = fgetc(file)) != EOF) {
    int_array_append(&ins, c);
  }

  printf("Instructions in file: ");
  for (size_t i = 0; i < ins.len; i++) {
    printf("%d ", ins.arr[i]);
  }
  printf("\n");

  ResizableBuffer buf;
  init_resizable_buffer(&buf, 100);
  instructions_to_string(&buf, &ins);

  printf("Instructions: -------------\n%s-----------\n", buf.buf);
  free(buf.buf);
  buf.buf = NULL;
  int_array_free(&ins);
}

static void magic_number(FILE *file) {
  int magic_number[] = {0x4D, 0x4F, 0x4E, 0x4B, 0x45, 0x59};

  for (size_t i = 0; i < ARRAY_LEN(magic_number); i++) {
    write_byte(file, magic_number[i]);
  }
}

static void write_constants(Bytecode bytecode, FILE *file) {
  uint8_t len_constants[2];
  uint16_to_big_endian(bytecode.constants.len, len_constants);

  write_byte(file, len_constants[0]);
  write_byte(file, len_constants[1]);

  // TODO: write actual constants to the file
}

static void write_instructions(Bytecode bytecode, FILE *file) {
  uint8_t len_instructions[2];
  uint16_to_big_endian(bytecode.instructions.len, len_instructions);

  write_byte(file, len_instructions[0]);
  write_byte(file, len_instructions[1]);

  for (size_t i = 0; i < bytecode.instructions.len; i++) {
    write_byte(file, bytecode.instructions.arr[i]);
  }
}
