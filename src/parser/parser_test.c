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

void *test_statement(Statement *stmt, ExprType type) {
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(type, stmt->expression->type);

  return stmt->expression->value;
}

void *test_single_expression_in_program(Program *p, ExprType type) {
  TEST_ASSERT_EQUAL(1, p->statements->size);

  Statement *stmt = p->statements->tail->value;
  return test_statement(stmt, type);
}

void test_int_value(IntegerLiteral *integer, long value) {
  TEST_ASSERT_EQUAL(value, integer->value);
}

void test_bool_value(BooleanLiteral *boll, bool value) {
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
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier,
                             stmt->name->token.literal);
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

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
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
  Identifier *ident = test_single_expression_in_program(program, IDENT_EXPR);

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
    BooleanLiteral *boolean = test_single_expression_in_program(program, BOOL_EXPR);

    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, boolean->token.literal);
    TEST_ASSERT_EQUAL(tests[i].expected_value, boolean->value);
  }
}

void test_integer_literal_expression(void) {
  char *input = "5;";
  Program *program = parse_and_check_errors(input);
  IntegerLiteral *literal = test_single_expression_in_program(program, INT_EXPR);

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
    PrefixExpression *exp = test_single_expression_in_program(p, PREFIX_EXPR);

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
    InfixExpression *expr = test_single_expression_in_program(p, INFIX_EXPR);

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
          "(3 + 4)((-5) * 5);\n",
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
          "((a + add((b * c))) + d);\n",
      },
      {
          "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
          "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)));\n",
      },
      {
          "add(a + b + c * d / f + g)",
          "add((((a + b) + ((c * d) / f)) + g));\n",
      },
      {
          "a * [1, 2, 3, 4][b * c] * d",
          "((a * ([1, 2, 3, 4][(b * c)])) * d);\n",
      },
      {
          "add(a * b[2], b[1], 2 * [1, 2][1])",
          "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])));\n",
      },
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Program *p = parse_and_check_errors(tests[i].input);

    ResizableBuffer actual;
    init_resizable_buffer(&actual, 100);
    program_string(&actual, p);

    TEST_ASSERT_EQUAL_STRING(tests[i].expected, actual.buf);
    free(actual.buf);
  }
}

void test_if_expression(void) {
  char *input = "if (x < y) { x } else { y }";

  Program *program = parse_and_check_errors(input);
  IfExpression *expr = test_single_expression_in_program(program, IF_EXPR);

  TEST_ASSERT_EQUAL(1, expr->consequence->statements->size);
  TEST_ASSERT_NOT_NULL(expr->alternative);
}

void test_nested_if_expression(void) {
  char *input = "if (10 > 1) { if (10 > 1) { return 10; } return 1; }";

  Program *program = parse_and_check_errors(input);
  IfExpression *parent = test_single_expression_in_program(program, IF_EXPR);

  Statement *nested_stmt = parent->consequence->statements->tail->value;

  TEST_ASSERT_EQUAL(EXPR_STATEMENT, nested_stmt->type);
  TEST_ASSERT_EQUAL(IF_EXPR, nested_stmt->expression->type);

  IfExpression *nested_if = nested_stmt->expression->value;

  TEST_ASSERT_EQUAL(2, parent->consequence->statements->size);
  TEST_ASSERT_EQUAL(1, nested_if->consequence->statements->size);
}

void test_function_literal(void) {
  char *input = "fn (x, y) { x + y; };";

  Program *program = parse_and_check_errors(input);
  FunctionLiteral *fn = test_single_expression_in_program(program, FN_EXPR);

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
    FunctionLiteral *fn = test_single_expression_in_program(p, FN_EXPR);

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
  CallExpression *expr = test_single_expression_in_program(program, CALL_EXPR);

  test_identifier(expr->function, "add");

  Node *cur_node = expr->arguments->tail;

  TEST_ASSERT_EQUAL(3, expr->arguments->size);
  test_integer_literal(cur_node->value, 1); // 1st argument
  cur_node = cur_node->next;
  test_infix_expression(cur_node->value, "*", 2, 3); // 2nd argument
  cur_node = cur_node->next;
  test_infix_expression(cur_node->value, "+", 4, 5); // 3rd argument
}

void test_string_literal_expression(void) {
  char *input = "\"hello world\"";

  Program *p = parse_and_check_errors(input);
  StringLiteral *str = test_single_expression_in_program(p, STRING_EXPR);

  TEST_ASSERT_EQUAL_STRING("hello world", str->value);
}

void test_parsing_array_literals(void) {
  char *input = "[1, 2 * 2, 3 + 3]";

  Program *p = parse_and_check_errors(input);
  ArrayLiteral *arr = test_single_expression_in_program(p, ARRAY_EXPR);

  TEST_ASSERT_EQUAL(3, arr->elements->len);

  test_integer_literal(arr->elements->arr[0], 1);
  test_infix_expression(arr->elements->arr[1], "*", 2, 2);
  test_infix_expression(arr->elements->arr[2], "+", 3, 3);
}

void test_parsing_index_expressions(void) {
  char *input = "myArray[1 + 1]";

  Program *p = parse_and_check_errors(input);
  IndexExpression *expr = test_single_expression_in_program(p, INDEX_EXPR);

  test_identifier(expr->left, "myArray");
  test_infix_expression(expr->index, "+", 1, 1);
}

int iterate_over_string_int_hashmap(void *expected, hashmap_element_t *pair) {
  hashmap_t *expected_map = expected;

  Expression *key_expr = (void *)pair->key;
  TEST_ASSERT_EQUAL(STRING_EXPR, key_expr->type);

  StringLiteral *key = key_expr->value;
  int *expected_value = hashmap_get(expected_map, key->value, strlen(key->value));
  TEST_ASSERT_NOT_NULL(expected_value);

  Expression *value = pair->data;
  test_integer_literal(value, *expected_value);

  return 0;
}

void test_parsing_hash_literals_string_keys(void) {
  char *input = "{\"one\": 1, \"two\": 2, \"three\": 3}";

  Program *p = parse_and_check_errors(input);
  HashLiteral *hash = test_single_expression_in_program(p, HASH_EXPR);

  hashmap_t expected_map;
  hashmap_create(3, &expected_map);
  int one = 1;
  int two = 2;
  int three = 3;
  hashmap_put(&expected_map, "one", strlen("one"), &one);
  hashmap_put(&expected_map, "two", strlen("two"), &two);
  hashmap_put(&expected_map, "three", strlen("three"), &three);

  hashmap_iterate_pairs(&hash->pairs, &iterate_over_string_int_hashmap, &expected_map);
}

void test_parsing_empty_hash_literal(void) {
  char *input = "{}";

  Program *p = parse_and_check_errors(input);
  HashLiteral *hash = test_single_expression_in_program(p, HASH_EXPR);

  TEST_ASSERT_EQUAL(0, hash->len);
}

void test_parsing_while_loops(void) {
  char *input = "while (b > 5) {"
                " let b = b + 1;"
                "}";

  Program *p = parse_and_check_errors(input);
  WhileLoop *loop = test_single_expression_in_program(p, WHILE_EXPR);

  TEST_ASSERT_EQUAL(1, loop->body->statements->size);
  TEST_ASSERT_EQUAL(INFIX_EXPR, loop->condition->type);
}

void test_parsing_full_for_loop(void) {
  char *input = "for (let i = 0; i < 10; let i = i + 1) {"
                "  puts(i);                             "
                "}";

  Program *p = parse_and_check_errors(input);
  ForLoop *loop = test_single_expression_in_program(p, FOR_EXPR);

  TEST_ASSERT_EQUAL(1, loop->body->statements->size);

  TEST_ASSERT_NOT_NULL(loop->initialization);
  TEST_ASSERT_EQUAL(LET_STATEMENT, loop->initialization->type);

  TEST_ASSERT_NOT_NULL(loop->condition);
  TEST_ASSERT_EQUAL(INFIX_EXPR, loop->condition->type);

  TEST_ASSERT_NOT_NULL(loop->update);
  TEST_ASSERT_EQUAL(LET_STATEMENT, loop->update->type);
}

void test_infinite_for_loop(void) {
  char *input = "for (;;) { }";

  Program *p = parse_and_check_errors(input);
  ForLoop *loop = test_single_expression_in_program(p, FOR_EXPR);
  TEST_ASSERT_EQUAL(0, loop->body->statements->size);

  TEST_ASSERT_NULL(loop->initialization);
  TEST_ASSERT_NULL(loop->condition);
  TEST_ASSERT_NULL(loop->update);
}

void test_incomplete_for_loop(void) {
  char *input = "for (let c = 0; ;) { }";

  Program *p = parse_and_check_errors(input);
  ForLoop *loop = test_single_expression_in_program(p, FOR_EXPR);

  TEST_ASSERT_EQUAL(0, loop->body->statements->size);

  TEST_ASSERT_NOT_NULL(loop->initialization);
  TEST_ASSERT_NULL(loop->condition);
  TEST_ASSERT_NULL(loop->update);
}

void test_parsing_reassignment(void) {
  char *input = "let b = 0; b = 1";

  Program *p = parse_and_check_errors(input);
  TEST_ASSERT_EQUAL(2, p->statements->size);

  Statement *stmt = p->statements->tail->next->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(REASSIGN_EXPR, stmt->expression->type);

  Reassignment *reassign = stmt->expression->value;

  TEST_ASSERT_EQUAL_STRING("b", reassign->name->value);
  TEST_ASSERT_EQUAL(INT_EXPR, reassign->value->type);
  
  IntegerLiteral *intt = reassign->value->value;
  TEST_ASSERT_EQUAL(1, intt->value);
}

void test_parsing_binary_literal(void) {
  char *input = "0b1010;";
  Program *program = parse_and_check_errors(input);
  IntegerLiteral *literal = test_single_expression_in_program(program, INT_EXPR);

  TEST_ASSERT_EQUAL_INT64(10, literal->value);
  TEST_ASSERT_EQUAL_STRING("1010", literal->token.literal);
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
  RUN_TEST(test_nested_if_expression);
  RUN_TEST(test_string_literal_expression);
  RUN_TEST(test_parsing_array_literals);
  RUN_TEST(test_parsing_index_expressions);
  RUN_TEST(test_parsing_hash_literals_string_keys);
  RUN_TEST(test_parsing_empty_hash_literal);
  RUN_TEST(test_parsing_while_loops);
  RUN_TEST(test_parsing_full_for_loop);
  RUN_TEST(test_infinite_for_loop);
  RUN_TEST(test_incomplete_for_loop);
  RUN_TEST(test_parsing_reassignment);
  RUN_TEST(test_parsing_binary_literal);
  return UNITY_END();
}
