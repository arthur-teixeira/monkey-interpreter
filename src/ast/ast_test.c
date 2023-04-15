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
    {IDENT, "myVar"},
    "myVar"
  };

  Identifier anotherVarIdent = {
    {IDENT, "anotherVar"},
    "anotherVar"
  };

  Expression anotherVarExpr = {
    IDENT_EXPR,
    &anotherVarIdent
  };

  Statement myVar = {
    LET_STATEMENT,
    {LET, "let"},
    &myVarIdent,
    &anotherVarExpr,
  };
  append(statements, &myVar);

  Program p = {
    statements
  };
  char *result = program_string(&p);
  TEST_ASSERT_EQUAL_STRING("let myVar = anotherVar;\n", result);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ast_as_string);
  UNITY_END();
}
