#include "../compiler/compiler.h"
#include "../object/object.h"
#include "code.h"
#include <stdio.h>

static void magic_number(FILE *);
static void write_constants(Bytecode, FILE *);
static void write_instructions(Instructions, FILE *);
static void write_number_constant(FILE *, Number *);
static void write_string_constant(FILE *, String *);
static void write_function_constant(FILE *, CompiledFunction *);
static void write_loop_constant(FILE *, CompiledLoop *);
void dump_file(const char *);
