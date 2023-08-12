#include "file_loader.h"
#include "../big_endian/big_endian.h"
#include "../code/code.h"
#include "../object/object.h"
#include "vm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(x) sizeof(x) / sizeof(x[0])

static bool test_magic_number(uint8_t *);
static Object *read_number_constant(FILE *);
static Object *read_string_constant(FILE *);
static Object *read_function_constant(FILE *);
static Object *read_constant(FILE *);
static DynamicArray read_constants(FILE *, size_t);

void load_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("ERROR: could not read file:");
    exit(EXIT_FAILURE);
  }

  uint8_t magic_number[6];
  for (size_t i = 0; i < 6; i++) {
    magic_number[i] = fgetc(file);
  }

  if (!test_magic_number(magic_number)) {
    fprintf(stderr, "ERROR: file is not a monkey binary\n");
    exit(EXIT_FAILURE);
  }

  int8_t num_constants_buf[2];
  num_constants_buf[0] = fgetc(file);
  num_constants_buf[1] = fgetc(file);

  uint16_t num_constants;
  big_endian_to_uint16(&num_constants, num_constants_buf);

  DynamicArray constants = read_constants(file, num_constants);

  int8_t num_instructions_buf[2];
  num_instructions_buf[0] = fgetc(file);
  num_instructions_buf[1] = fgetc(file);

  uint16_t num_instructions;
  big_endian_to_uint16(&num_instructions, num_instructions_buf);

  Instructions ins;
  int_array_init(&ins, 10);

  int8_t c;
  while ((c = fgetc(file)) != EOF) {
    int_array_append(&ins, c);
  }

  Bytecode bt = {
      .instructions = ins,
      .constants = constants,
  };

  VM *vm = new_vm(bt);
  VMResult result = run_vm(vm);
  if (result != VM_OK) {
    char buf[100];
    vm_error(result, buf, 100);
    fprintf(stderr, "ERROR: Error running the program: %s\n ", buf);
  }
  ResizableBuffer buf;
  init_resizable_buffer(&buf, 10);
  Object *top = vm_last_popped_stack_elem(vm);
  inspect_object(&buf, top);
  printf("%s\n", buf.buf);
}

static bool test_magic_number(uint8_t *value) {
  char magic_number[] = "MONKEY";

  for (size_t i = 0; i < strlen(magic_number); i++) {
    if (magic_number[i] != value[i]) {
      return false;
    }
  }

  return true;
}

static DynamicArray read_constants(FILE *file, size_t num_constants) {
  DynamicArray constants;
  array_init(&constants, num_constants);

  for (size_t i = 0; i < num_constants; i++) {
    array_append(&constants, read_constant(file));
  }

  return constants;
}

static Object *read_constant(FILE *file) {
  ObjectType type = fgetc(file);
  switch (type) {
  case NUMBER_OBJ:
    return read_number_constant(file);
  case STRING_OBJ:
    return read_string_constant(file);
  case COMPILED_FUNCTION_OBJ:
    return read_function_constant(file);
  default:
    assert(0 && "unknown constant value");
  }
}

static Object *read_number_constant(FILE *file) {
  Number *num = malloc(sizeof(Number));
  assert(num);
  num->type = NUMBER_OBJ;

  double value;
  fread(&value, 1, sizeof(num->value), file);

  num->value = value;

  return (Object *)num;
}

static Object *read_string_constant(FILE *file) {
  String *str = malloc(sizeof(String));
  assert(str);
  str->type = STRING_OBJ;

  int8_t len_buf[2];
  len_buf[0] = fgetc(file);
  len_buf[1] = fgetc(file);

  uint16_t len;
  big_endian_to_uint16(&len, len_buf);

  str->len = len;

  char str_value[len + 1];

  size_t i = 0;
  for (; i < len; i++) {
    str_value[i] = fgetc(file);
  }
  str_value[i] = '\0';

  str->value = str_value;

  return (Object *)str;
}

static Object *read_function_constant(FILE *file) {
  CompiledFunction *fn = malloc(sizeof(CompiledFunction));
  assert(fn);
  fn->type = COMPILED_FUNCTION_OBJ;

  int8_t local_variables_count_buf[2];
  local_variables_count_buf[0] = fgetc(file);
  local_variables_count_buf[1] = fgetc(file);

  uint16_t local_variables_count;
  big_endian_to_uint16(&local_variables_count, local_variables_count_buf);

  fn->num_locals = local_variables_count;

  int8_t num_parameters = fgetc(file);
  fn->num_parameters = num_parameters;

  int8_t instructions_len_buf[2];
  instructions_len_buf[0] = fgetc(file);
  instructions_len_buf[1] = fgetc(file);

  uint16_t instructions_len;
  big_endian_to_uint16(&instructions_len, instructions_len_buf);

  int_array_init(&fn->instructions, instructions_len);

  for (size_t i = 0; i < instructions_len; i++) {
    int_array_append(&fn->instructions, fgetc(file));
  }

  return (Object *)fn;
}
