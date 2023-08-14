#include "../code/code.h"
#include "../file_reader/file_reader.h"
#include "../str_utils/str_utils.h"
#include "../vm/file_loader.h"
#include <stdlib.h>

static void append_index(ResizableBuffer *buf, size_t index) {
  append_to_buf(buf, "Constant [");
  char index_str[2];
  sprintf(index_str, "%zu", index);
  append_to_buf(buf, index_str);
  append_to_buf(buf, "] ");
}

static void disassemble_function_constant(CompiledFunction *fn,
                                          ResizableBuffer *buf) {
  append_to_buf(buf, "Function:\n-------------\n");

  ResizableBuffer instructions_buf;
  init_resizable_buffer(&instructions_buf, 100);

  instructions_to_string(&instructions_buf, &fn->instructions);
  append_to_buf(buf, instructions_buf.buf);

  free(instructions_buf.buf);
}

static void disassemble_constant(Object *constant, ResizableBuffer *buf) {
  switch (constant->type) {
  case STRING_OBJ:
    append_to_buf(buf, "String: ");
    append_to_buf(buf, ((String *)constant)->value);
    append_to_buf(buf, "\n");
    break;
  case NUMBER_OBJ:
    append_to_buf(buf, "Number: ");
    char num_str[20];
    sprintf(num_str, "%f", ((Number *)constant)->value);
    append_to_buf(buf, num_str);
    append_to_buf(buf, "\n");
    break;
  case COMPILED_FUNCTION_OBJ:
    disassemble_function_constant((CompiledFunction *)constant, buf);
    break;
  default:
    fprintf(stderr, "Unknown constant type: %d\n", constant->type);
    exit(EXIT_FAILURE);
  }
}

static void constants_to_string(DynamicArray *constants, ResizableBuffer *buf) {
  for (size_t i = 0; i < constants->len; i++) {
    append_index(buf, i);
    disassemble_constant(constants->arr[i], buf);
    append_to_buf(buf, "-------------\n");
  }
}

void disassemble_file(const char *filename) {
  Bytecode bt = get_bytecode_from_file(filename);

  ResizableBuffer instructions_buf;
  init_resizable_buffer(&instructions_buf, 100);
  instructions_to_string(&instructions_buf, &bt.instructions);

  ResizableBuffer constants_buf;
  init_resizable_buffer(&constants_buf, 100);
  constants_to_string(&bt.constants, &constants_buf);

  printf("Constants:\n-------------\n%s-------------\n", constants_buf.buf);
  printf("Instructions:\n-------------\n%s-------------\n",
         instructions_buf.buf);

  free(instructions_buf.buf);
  free(constants_buf.buf);
  int_array_free(&bt.instructions);
  array_free(&bt.constants);
}
