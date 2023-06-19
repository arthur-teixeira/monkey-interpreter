#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

void test_ast_as_string(void) {
  LinkedList *statements = new_list();
  Identifier myVarIdent = {
    .type = IDENT_EXPR,
    .token = {IDENT, "myVar"},
    .value = "myVar"
  };

  Identifier anotherVarIdent = {
    .type = IDENT_EXPR,
    .token = {IDENT, "anotherVar"},
    .value = "anotherVar"
  };


  Statement myVar = {
    LET_STATEMENT,
    {LET, "let"},
    &myVarIdent,
    (Expression *)&anotherVarIdent,
  };
  append(statements, &myVar);

  Program p = {
    statements
  };

  ResizableBuffer result;
  init_resizable_buffer(&result, MAX_LEN);
  program_string(&result, &p);
  TEST_ASSERT_EQUAL_STRING("let myVar = anotherVar;\n", result.buf);
  free(result.buf);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ast_as_string);
  return UNITY_END();
}
