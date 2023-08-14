#include "repl.h"
#include "../compiler/compiler.h"
#include "../evaluator/evaluator.h"
#include "../object/builtins.h"
#include "../vm/vm.h"
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

void interpret(ResizableBuffer *buf, Program *program, Environment *env) {
  Object *evaluated = eval_program(program, env);
  if (evaluated != NULL) {
    inspect_object(buf, evaluated);
    printf("%s\n", buf->buf);
  }
}

void start_repl(ReplMode mode) {
  InputBuffer *input_buf = new_input_buffer();
  Environment *env = new_environment();

  DynamicArray constants;
  array_init(&constants, 10);
  Object *globals[GLOBALS_SIZE];
  SymbolTable *symbol_table = new_symbol_table();
  for (size_t i = 0; i < builtin_definitions_len; i++) {
    symbol_define_builtin(symbol_table, i, builtin_definitions[i].name);
  }

  printf("Hello from Monkey!\n");
  while (true) {
    printf(PROMPT);
    read_input(input_buf);

    if (*(input_buf->buffer) == '\0') {
      continue;
    }

    Lexer *l = new_lexer(input_buf->buffer);
    Parser *p = new_parser(l);
    Program *program = parse_program(p);
    if (p->errors.len > 0) {
      print_parser_errors(p);
      free_parser(p);
      free_program(program);
      continue;
    }

    ResizableBuffer buf;
    init_resizable_buffer(&buf, 100);
    switch (mode) {
    case MODE_INTERPRET:
      interpret(&buf, program, env);
      break;
    case MODE_COMPILE: {
      Compiler *compiler = new_compiler_with_state(symbol_table, &constants);
      CompilerResult compiler_result = compile_program(compiler, program);
      if (compiler_result != COMPILER_OK) {
        char err[100];
        compiler_error(compiler_result, err, 100);
        printf("Compilation failed: %s\n", err);
        continue;
      }

      constants = *compiler->constants;

      VM *vm = new_vm_with_state(bytecode(compiler), globals);
      VMResult vm_result = run_vm(vm);
      if (vm_result != VM_OK) {
        char err[100];
        vm_error(vm_result, err, 100);
        printf("VM failed: %s\n", err);
        continue;
      }

      memcpy(globals, vm->globals, sizeof(Object *) * GLOBALS_SIZE);

      Object *top = vm_last_popped_stack_elem(vm);
      inspect_object(&buf, top);
      printf("%s\n", buf.buf);
      free(compiler);
      free(vm);
    }
    default:
      break;
    }

    free(buf.buf);
    free_parser(p);
    free_program(program);
  }
  close_input_buffer(input_buf);
}
