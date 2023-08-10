#include "../compiler/compiler.h"
#include <stdio.h>

static void magic_number(FILE *);
static void write_constants(Bytecode, FILE *);
static void write_instructions(Bytecode, FILE *);
