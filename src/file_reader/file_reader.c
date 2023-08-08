#include "file_reader.h"
#include "../compiler/compiler.h"
#include "../evaluator/evaluator.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../str_utils/str_utils.h"
#include "../vm/vm.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *open_source_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("ERROR: could not open file");
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

Program *get_program_from_file(const char *filename) {
  Lexer *l = new_file_lexer(filename);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  if (p->errors.len > 0) {
    print_parser_errors(p);
    free_parser(p);
    free_program(program);
    program = NULL;
  }

  return program;
}

void eval_file(const char *filename) {
  Program *program = get_program_from_file(filename);
  if (!program)
    return;

  Environment *env = new_environment();
  Object *evaluated = eval_program(program, env);

  if (evaluated) {
    ResizableBuffer program_buffer;
    init_resizable_buffer(&program_buffer, 100);
    inspect_object(&program_buffer, evaluated);
    printf("%s\n", program_buffer.buf);
    free(program_buffer.buf);
  }
}

void compile_file(const char *filename) {
  Program *program = get_program_from_file(filename);
  if (!program)
    return;

  Compiler *compiler = new_compiler();
  CompilerResult compiler_result = compile_program(compiler, program);
  if (compiler_result != COMPILER_OK) {
    char err[100];
    compiler_error(compiler_result, err, 100);
    fprintf(stderr, "%s\n", err);
    free_compiler(compiler);
    free_program(program);
    exit(EXIT_FAILURE);
  }

  Bytecode bt = bytecode(compiler);

  VM *vm = new_vm(bt);
  VMResult vm_result = run_vm(vm);
  if (vm_result != VM_OK) {
    char err[100];
    vm_error(vm_result, err, 100);
    fprintf(stderr, "%s\n", err);
    free_vm(vm);
    free_compiler(compiler);
    free_program(program);
    exit(EXIT_FAILURE);
  }
  ResizableBuffer buf;
  init_resizable_buffer(&buf, 100);
  Object *top = vm_last_popped_stack_elem(vm);
  inspect_object(&buf, top);
  printf("%s\n", buf.buf);
  free(compiler);
  free(vm);
}
