#include "file_generator.h"
#include "../big_endian/big_endian.h"
#include "../object/object.h"
#include "code.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_LEN(x) sizeof(x) / sizeof(x[0])

static void write_byte(FILE *file, int byte) {
  if (fputc(byte, file) == EOF) {
    perror("ERROR: could not write to file");
    exit(EXIT_FAILURE);
  }
}

void save_to_file(Bytecode bytecode, const char *filename) {
  FILE *file = fopen(filename, "wb");
  if (!file) {
    perror("ERROR: could not create file");
    exit(EXIT_FAILURE);
  }

  magic_number(file);
  write_constants(bytecode, file);
  write_instructions(bytecode.instructions, file);

  if (fclose(file) != 0) {
    perror("ERROR: could not close file");
    exit(EXIT_FAILURE);
  }
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

  for (size_t i = 0; i < bytecode.constants.len; i++) {
    Object *constant = bytecode.constants.arr[i];
    switch (constant->type) {
    case NUMBER_OBJ:
      write_number_constant(file, (Number *)constant);
      break;
    case STRING_OBJ:
      write_string_constant(file, (String *)constant);
      break;
    case COMPILED_FUNCTION_OBJ:
      write_function_constant(file, (CompiledFunction *)constant);
      break;
    default:
      assert(0 && "unknown constant type");
    }
  }
}

static void write_number_constant(FILE *file, Number *num) {
  write_byte(file, num->type);
  fwrite(&num->value, 1, sizeof(num->value), file);
}

static void write_string_constant(FILE *file, String *str) {
  write_byte(file, str->type);

  uint8_t len[2];
  uint16_to_big_endian(str->len, len);

  write_byte(file, len[0]);
  write_byte(file, len[1]);

  for (size_t i = 0; i < str->len; i++) {
    write_byte(file, str->value[i]);
  }
}

static void write_function_constant(FILE *file, CompiledFunction *fn) {
  write_byte(file, fn->type);

  uint8_t local_variables_count[2];
  uint16_to_big_endian(fn->num_locals, local_variables_count);

  write_byte(file, local_variables_count[0]);
  write_byte(file, local_variables_count[1]);

  write_byte(file, (uint8_t)fn->num_parameters);

  write_instructions(fn->instructions, file);
}

static void write_instructions(Instructions instructions, FILE *file) {
  uint8_t len_instructions[2];
  uint16_to_big_endian(instructions.len, len_instructions);

  write_byte(file, len_instructions[0]);
  write_byte(file, len_instructions[1]);

  for (size_t i = 0; i < instructions.len; i++) {
    write_byte(file, instructions.arr[i]);
  }
}
