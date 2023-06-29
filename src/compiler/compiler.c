#include "compiler.h"
#include <assert.h>
#include <stdlib.h>
#include "../ast/ast.h"

Compiler *new_compiler() {
    Compiler *compiler = malloc(sizeof(Compiler));
    assert(compiler != NULL);

    array_init(&compiler->constants, 8);
    int_array_init(&compiler->instructions, 8);
    return compiler;
}

void free_compiler(Compiler *compiler) {
    array_free(&compiler->constants);
    free(compiler);
}

int8_t compile(Compiler *compiler, Program *program) {
    return 0;
}

Bytecode bytecode(Compiler *compiler) {
    Bytecode bytecode = {
        .constants = compiler->constants,
        .instructions = compiler->instructions,
    };

    return bytecode;
}

