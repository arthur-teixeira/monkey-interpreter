#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void check_parser_errors(Parser *p) {
  if (p->errors->size == 0) {
    return;
  }

  printf("ERROR: parser has %zu errors\n", p->errors->size);

  Node *n = p->errors->tail;
  while (n != NULL) {
    char *value = n->value;
    printf("Parser error: %s\n", value);

    n = n->next;
  }
  TEST_FAIL();
}

Program *parse_and_check_errors(char *input) {
  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);

  Program *program = parse_program(p);
  check_parser_errors(p);

  return program;
}

void test_int_value(IntegerLiteral *integer, long value) {
  TEST_ASSERT_EQUAL(value, integer->value);
}

void test_bool_value(Boolean *boll, bool value) {
  TEST_ASSERT_EQUAL(value, boll->value);
}

void test_ident_value(Identifier *ident, char *value) {
  TEST_ASSERT_EQUAL_STRING(value, ident->value);
}

void test_expr_value(Expression *expr, void *expected_value) {
  switch (expr->type) {
    case INT_EXPR:
      return test_int_value(expr->value, *(long *)expected_value);
    case BOOL_EXPR:
      return test_bool_value(expr->value, *(bool *)expected_value);
    case IDENT_EXPR:
      return test_ident_value(expr->value, expected_value);
    default:
      TEST_FAIL();
  }
}

void test_let_statements() {
  typedef struct {
    char *input;
    char *expected_identifier;
    void *expected_value;
  } TestCase;

  long x_value = 5;
  bool y_value = true;
  char *foobar_value = "y";

  TestCase tests[] = {{"let x = 5;", "x", &x_value},
                      {"let y = true;", "y", &y_value},
                      {"let foobar = y;", "foobar", foobar_value}};

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(TestCase); i++) {
    Program *program = parse_and_check_errors(tests[i].input);
    Node *stmt_node = program->statements->tail;

    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->statements->size);

    Statement *stmt = stmt_node->value;

    TEST_ASSERT_EQUAL_STRING("let", stmt->token.literal);
    TEST_ASSERT_EQUAL(LET_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier, stmt->name->value);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier, stmt->name->token.literal);
    test_expr_value(stmt->expression, tests[i].expected_value);

    free(program);
  }
}

void test_return_statements(void) {
  struct testCase {
    char *input;
    void *expected_value;
  };

  long value1 = 5;
  bool value2 = true;

  struct testCase tests[] = {
    {"return 5;", &value1},
    {"return true;", &value2},
    {"return y;", "y"},
  };

  for (uint32_t i = 0; i < sizeof(tests)/sizeof(struct testCase); i++) {
    Program *program = parse_and_check_errors(tests[i].input);
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->statements->size);

    Node *stmt_node = program->statements->tail;
    Statement *stmt = stmt_node->value;
    TEST_ASSERT_EQUAL_STRING("return", stmt->token.literal);
    test_expr_value(stmt->expression, tests[i].expected_value);
    free(program);
  }
}

void test_identifier_expression(void) {
  char *input = "foobar;";

  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(IDENT_EXPR, stmt->expression->type);

  Identifier *ident = stmt->expression->value;
  TEST_ASSERT_EQUAL_STRING("foobar", ident->token.literal);
  TEST_ASSERT_EQUAL_STRING("foobar", ident->value);
}

void test_boolean_expressions(void) {

  struct test_case {
    char *input;
    bool expected_value;
    char *expected_literal;
  };

  struct test_case tests[] = {{"false;", false, "false"},
                              {"true;", true, "true"}};

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct test_case); i++) {
    Program *program = parse_and_check_errors(tests[i].input);

    TEST_ASSERT_EQUAL(1, program->statements->size);

    Statement *stmt = program->statements->tail->value;
    TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(BOOL_EXPR, stmt->expression->type);

    Boolean *boolean = stmt->expression->value;
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, boolean->token.literal);
    TEST_ASSERT_EQUAL(tests[i].expected_value, boolean->value);
  }
}

void test_integer_literal_expression(void) {
  char *input = "5;";
  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(INT_EXPR, stmt->expression->type);

  IntegerLiteral *literal = stmt->expression->value;
  TEST_ASSERT_EQUAL_INT64(5, literal->value);
  TEST_ASSERT_EQUAL_STRING("5", literal->token.literal);
}

void test_integer_literal(Expression *expr, long value) {
  TEST_ASSERT_EQUAL(INT_EXPR, expr->type);
  IntegerLiteral *lit = expr->value;

  TEST_ASSERT_EQUAL_INT64(value, lit->value);

  char as_string[10];
  sprintf(as_string, "%zu", value);
  TEST_ASSERT_EQUAL_STRING(as_string, lit->token.literal);
}

void test_parsing_prefix_expressions(void) {
  struct prefix_test_case {
    char *input;
    char *operator;
    long integer_value;
  };

  struct prefix_test_case prefix_tests[] = {{"!5;", "!", 5}, {"-15;", "-", 15}};

  for (uint32_t i = 0;
       i < sizeof(prefix_tests) / sizeof(struct prefix_test_case); i++) {
    Program *p = parse_and_check_errors(prefix_tests[i].input);

    TEST_ASSERT_EQUAL(1, p->statements->size);

    Statement *stmt = p->statements->tail->value;
    TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(PREFIX_EXPR, stmt->expression->type);

    PrefixExpression *exp = stmt->expression->value;
    TEST_ASSERT_EQUAL_STRING(prefix_tests[i].operator, exp->operator);

    test_integer_literal(exp->right, prefix_tests[i].integer_value);

    free_program(p);
  }
}

void test_parsing_infix_expressions(void) {
  struct infix_test_case {
    char *input;
    long left_value;
    char *operator;
    long right_value;
  };

  struct infix_test_case infix_tests[] = {
      {"5 + 5;", 5, "+", 5},   {"5 - 5;", 5, "-", 5},   {"5 * 1;", 5, "*", 1},
      {"5 / 5;", 5, "/", 5},   {"5 > 5;", 5, ">", 5},   {"5 < 5;", 5, "<", 5},
      {"5 == 5;", 5, "==", 5}, {"5 != 5;", 5, "!=", 5},
  };

  for (uint32_t i = 0; i < sizeof(infix_tests) / sizeof(struct infix_test_case);
       i++) {
    Program *p = parse_and_check_errors(infix_tests[i].input);
    TEST_ASSERT_EQUAL(1, p->statements->size);

    Statement *stmt = p->statements->tail->value;
    TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);

    TEST_ASSERT_EQUAL(INFIX_EXPR, stmt->expression->type);
    InfixExpression *expr = stmt->expression->value;

    test_integer_literal(expr->left, infix_tests[i].left_value);
    TEST_ASSERT_EQUAL_STRING(infix_tests[i].operator, expr->operator);
    test_integer_literal(expr->right, infix_tests[i].right_value);
  }
}

void test_operator_precedence_parsing(void) {
  struct testCase {
    char *input;
    char *expected;
  };

  struct testCase tests[] = {
      {
          "-a * b",
          "((-a) * b);\n",
      },
      {
          "!-a",
          "(!(-a));\n",
      },
      {
          "a + b + c",
          "((a + b) + c);\n",
      },
      {
          "a + b - c",
          "((a + b) - c);\n",
      },
      {
          "a * b * c",
          "((a * b) * c);\n",
      },
      {
          "a * b / c",
          "((a * b) / c);\n",
      },
      {
          "a + b / c",
          "(a + (b / c));\n",
      },
      {
          "a + b * c + d / e - f",
          "(((a + (b * c)) + (d / e)) - f);\n",
      },
      {
          "3 + 4; -5 * 5",
          "(3 + 4);\n((-5) * 5);\n",
      },
      {
          "5 > 4 == 3 < 4",
          "((5 > 4) == (3 < 4));\n",
      },
      {
          "5 < 4 != 3 > 4",
          "((5 < 4) != (3 > 4));\n",
      },
      {
          "3 + 4 * 5 == 3 * 1 + 4 * 5",
          "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)));\n",
      },
      {
          "3 + 4 * 5 == 3 * 1 + 4 * 5",
          "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)));\n",
      },
      {
          "(5 + 5) * 2",
          "((5 + 5) * 2);\n",
      },
      {
          "2 / (5 + 5)",
          "(2 / (5 + 5));\n",
      },
      {
          "-(5 + 5)",
          "(-(5 + 5));\n",
      },
      {
          "!(true == true)",
          "(!(true == true));\n",
      },
      {
          "a + add(b * c) + d",
          "((a + add((b * c)) ) + d);\n",
      },
      {
          "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
          "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)) ) ;\n",
      },
      {
          "add(a + b + c * d / f + g)",
          "add((((a + b) + ((c * d) / f)) + g)) ;\n",
      }};

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Program *p = parse_and_check_errors(tests[i].input);

    char *actual = malloc(1000);
    *actual = '\0';
    program_string(actual, p);

    TEST_ASSERT_EQUAL_STRING(tests[i].expected, actual);
    free(actual);
  }
}

void test_if_expression(void) {
  char *input = "if (x < y) { x } else { y }";

  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(IF_EXPR, stmt->expression->type);

  IfExpression *expr = stmt->expression->value;

  TEST_ASSERT_EQUAL(1, expr->consequence->statements->size);
  TEST_ASSERT_NOT_NULL(expr->alternative);
}

void test_function_literal(void) {
  char *input = "fn (x, y) { x + y; };";

  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(FN_EXPR, stmt->expression->type);

  FunctionLiteral *fn = stmt->expression->value;
  TEST_ASSERT_EQUAL(2, fn->parameters->size);
  TEST_ASSERT_NOT_NULL(fn->body);
}

void test_function_parameter_parsing(void) {
  struct test_case {
    char *input;
    char expected_params[10][50];
  };

  struct test_case tests[] = {
      {"fn() {};", {}},
      {"fn(x) {};", {"x"}},
      {"fn(x, y) {};", {"x", "y"}},
      {"fn(x, y, z) {};", {"x", "y", "z"}},
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct test_case); i++) {
    Program *p = parse_and_check_errors(tests[i].input);

    Statement *stmt = p->statements->tail->value;
    TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(FN_EXPR, stmt->expression->type);

    FunctionLiteral *fn = stmt->expression->value;

    TEST_ASSERT_EQUAL(i, fn->parameters->size);

    Node *cur_node = fn->parameters->tail;
    for (uint32_t j = 0; j < i && cur_node != NULL;
         j++, cur_node = cur_node->next) {
      Identifier *ident = cur_node->value;
      TEST_ASSERT_EQUAL_STRING(tests[i].expected_params[j],
                               ident->token.literal);
      TEST_ASSERT_EQUAL_STRING(tests[i].expected_params[j], ident->value);
    }

    free(p);
  }
}

void test_identifier(Expression *expr, char *expected_identifier) {
  TEST_ASSERT_EQUAL(IDENT_EXPR, expr->type);

  Identifier *ident = expr->value;

  TEST_ASSERT_EQUAL_STRING(expected_identifier, ident->value);
  TEST_ASSERT_EQUAL_STRING(expected_identifier, ident->token.literal);
}

void test_infix_expression(Expression *expr, char *expected_operator, long left,
                           long right) {
  TEST_ASSERT_EQUAL(INFIX_EXPR, expr->type);

  InfixExpression *expression = expr->value;

  TEST_ASSERT_EQUAL_STRING(expected_operator, expression->operator);

  test_integer_literal(expression->right, right);
  test_integer_literal(expression->left, left);
}

void test_call_expression_parsing(void) {
  char *input = "add(1, 2 * 3, 4 + 5);";

  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(CALL_EXPR, stmt->expression->type);

  CallExpression *expr = stmt->expression->value;
  test_identifier(expr->function, "add");

  Node *cur_node = expr->arguments->tail;

  TEST_ASSERT_EQUAL(3, expr->arguments->size);
  test_integer_literal(cur_node->value, 1); // 1st argument
  cur_node = cur_node->next;
  test_infix_expression(cur_node->value, "*", 2, 3); // 2nd argument
  cur_node = cur_node->next;
  test_infix_expression(cur_node->value, "+", 4, 5); // 3rd argument
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_let_statements);
  RUN_TEST(test_return_statements);
  RUN_TEST(test_identifier_expression);
  RUN_TEST(test_integer_literal_expression);
  RUN_TEST(test_parsing_prefix_expressions);
  RUN_TEST(test_parsing_infix_expressions);
  RUN_TEST(test_operator_precedence_parsing);
  RUN_TEST(test_boolean_expressions);
  RUN_TEST(test_if_expression);
  RUN_TEST(test_function_literal);
  RUN_TEST(test_function_parameter_parsing);
  RUN_TEST(test_call_expression_parsing);
  UNITY_END();
}
