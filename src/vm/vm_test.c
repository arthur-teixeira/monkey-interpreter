#include "../ast/ast.h"
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/constants.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define VM_RUN_TESTS(tests) run_vm_tests(tests, ARRAY_LEN(tests));

#define EXPECTED_NULL_SENTINEL 98534921

typedef struct {
  char *input;
  Object *expected;
} vmTestCase;

Program *parse(vmTestCase test) {
  Lexer *l = new_lexer(test.input);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  free_parser(p);
  return program;
}

void test_number_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(NUMBER_OBJ, actual->type);
  TEST_ASSERT_EQUAL(NUMBER_OBJ, expected->type);
  TEST_ASSERT_EQUAL_INT64(((Number *)expected)->value,
                          ((Number *)actual)->value);
}

void test_boolean_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(BOOLEAN_OBJ, actual->type);
  TEST_ASSERT_EQUAL(BOOLEAN_OBJ, expected->type);
  TEST_ASSERT_EQUAL(((Boolean *)expected)->value, ((Boolean *)actual)->value);
}

void test_string_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(STRING_OBJ, actual->type);
  TEST_ASSERT_EQUAL(STRING_OBJ, expected->type);
  TEST_ASSERT_EQUAL_STRING(((String *)expected)->value,
                           ((String *)actual)->value);
}

void test_array_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(ARRAY_OBJ, expected->type);
  TEST_ASSERT_EQUAL(ARRAY_OBJ, actual->type);
  TEST_ASSERT_EQUAL_INT64(((Array *)expected)->elements.len,
                          ((Array *)actual)->elements.len);

  for (size_t i = 0; i < ((Array *)expected)->elements.len; i++) {
    // might need to expend to accept other types
    test_number_object(((Array *)expected)->elements.arr[i],
                       ((Array *)actual)->elements.arr[i]);
  }
}

void test_error_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(ERROR_OBJ, actual->type);
  TEST_ASSERT_EQUAL(ERROR_OBJ, expected->type);

  TEST_ASSERT_EQUAL_STRING(((Error *)expected)->message,
                           ((Error *)actual)->message);
}

void test_expected_object(Object *expected, Object *actual) {
  switch (actual->type) {
  case NUMBER_OBJ:
    test_number_object(expected, actual);
    break;
  case BOOLEAN_OBJ:
    test_boolean_object(expected, actual);
    break;
  case NULL_OBJ:
    TEST_ASSERT_EQUAL(NULL_OBJ, actual->type);
    break;
  case STRING_OBJ:
    test_string_object(expected, actual);
    break;
  case ARRAY_OBJ:
    test_array_object(expected, actual);
    break;
  case ERROR_OBJ:
    test_error_object(expected, actual);
    break;
  default:
    TEST_FAIL_MESSAGE("Unknown or unimplemented assertion for object type");
  }

  free_object(expected);
}

void run_vm_tests(vmTestCase *tests, size_t tests_count) {
  for (size_t i = 0; i < tests_count; i++) {
    vmTestCase test = tests[i];
    Program *program = parse(test);
    Compiler *compiler = new_compiler();
    CompilerResult result = compile_program(compiler, program);
    if (result != COMPILER_OK) {
      char msg[100];
      compiler_error(result, msg, 100);
      TEST_FAIL_MESSAGE(msg);
    }

    VM *vm = new_vm(bytecode(compiler));
    VMResult vm_result = run_vm(vm);
    if (vm_result != VM_OK) {
      char msg[100];
      vm_error(vm_result, msg, 100);
      TEST_FAIL_MESSAGE(msg);
    }

    Object *top = vm_last_popped_stack_elem(vm);
    test_expected_object(test.expected, top);

    free_program(program);
    free_compiler(compiler);
    free_vm(vm);
  }
}

void test_integer_arithmetic(void) {
  vmTestCase tests[] = {
      {"1", new_number(1)},
      {"2", new_number(2)},
      {"1 + 2", new_number(3)},
      {"1 - 2", new_number(-1)},
      {"1 * 2", new_number(2)},
      {"4 / 2", new_number(2)},
      {"50 / 2 * 2 + 10 - 5", new_number(55)},
      {"5 + 5 + 5 + 5 - 10", new_number(10)},
      {"2 * 2 * 2 * 2 * 2", new_number(32)},
      {"5 * 2 + 10", new_number(20)},
      {"5 + 2 * 10", new_number(25)},
      {"5 * (2 + 10)", new_number(60)},
      {"4 | 1", new_number(5)},
      {"2 & 1", new_number(0)},
      {"2 ^ 1", new_number(3)},
      {"2 << 1", new_number(4)},
      {"2 >> 1", new_number(1)},
      {"3 % 2", new_number(1)},
      {"3 % 3", new_number(0)},
      {"-5", new_number(-5)},
      {"-10", new_number(-10)},
      {"-50 + 100 + -50", new_number(0)},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", new_number(50)},
  };

  VM_RUN_TESTS(tests);
}

void test_boolean_expressions(void) {
  vmTestCase tests[] = {
      {"true", (Object *)&obj_true},
      {"1 < 2", (Object *)&obj_true},
      {"1 == 1", (Object *)&obj_true},
      {"1 != 2", (Object *)&obj_true},
      {"true == true", (Object *)&obj_true},
      {"false == false", (Object *)&obj_true},
      {"true != false", (Object *)&obj_true},
      {"false != true", (Object *)&obj_true},
      {"(1 < 2) == true", (Object *)&obj_true},
      {"(1 > 2) == false", (Object *)&obj_true},
      {"!false", (Object *)&obj_true},
      {"!!true", (Object *)&obj_true},
      {"!!5", (Object *)&obj_true},
      {"false", (Object *)&obj_false},
      {"1 > 2", (Object *)&obj_false},
      {"1 < 1", (Object *)&obj_false},
      {"1 > 1", (Object *)&obj_false},
      {"1 != 1", (Object *)&obj_false},
      {"1 == 2", (Object *)&obj_false},
      {"true == false", (Object *)&obj_false},
      {"(1 < 2) == false", (Object *)&obj_false},
      {"(1 > 2) == true", (Object *)&obj_false},
      {"!true", (Object *)&obj_false},
      {"!5", (Object *)&obj_false},
      {"!!false", (Object *)&obj_false},
  };

  VM_RUN_TESTS(tests);
}

void test_conditionals(void) {
  vmTestCase tests[] = {
      {"if (true) { 10 }", new_number(10)},
      {"if (true) { 10 } else { 20 }", new_number(10)},
      {"if (false) { 10 } else { 20 } ", new_number(20)},
      {"if (1) { 10 }", new_number(10)},
      {"if (1 < 2) { 10 }", new_number(10)},
      {"if (1 < 2) { 10 } else { 20 }", new_number(10)},
      {"if (1 > 2) { 10 } else { 20 }", new_number(20)},
      {"if ((if (false) { 10 })) { 10 } else { 20 }", new_number(20)},
      {"if (false) { 10; }", (Object *)&obj_null},
      {"if (1 > 2) { 10; }", (Object *)&obj_null},
      {"!(if (false) { 10; })", (Object *)&obj_true},
  };

  VM_RUN_TESTS(tests);
}

void test_global_let_statements(void) {
  vmTestCase tests[] = {
      {"let one = 1; one", new_number(1)},
      {"let one = 1; let two = 2; one + two", new_number(3)},
      {"let one = 1; let two = one + one; one + two", new_number(3)},
  };

  VM_RUN_TESTS(tests);
}

void test_string_expressions(void) {
  vmTestCase tests[] = {
      {"\"monkey\"", new_string("monkey")},
      {"\"mon\" + \"key\"", new_string("monkey")},
      {"\"mon\" + \"key\" + \"banana\"", new_string("monkeybanana")},
  };

  VM_RUN_TESTS(tests);
}

void test_array_literals(void) {
  vmTestCase tests[] = {
      {"[]", new_array((Object *[]){}, 0)},
      {
          "[1, 2, 3]",
          new_array(
              (Object *[]){
                  new_number(1),
                  new_number(2),
                  new_number(3),
              },
              3),
      },
      {
          "[1 + 2, 3 * 4, 5 + 6]",
          new_array(
              (Object *[]){
                  new_number(3),
                  new_number(12),
                  new_number(11),
              },
              3),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_index_expressions(void) {
  vmTestCase tests[] = {
      {"[1, 2, 3][1]", new_number(2)},
      {"[1, 2, 3][0 + 2]", new_number(3)},
      {"[[1, 1, 1]][0][0]", new_number(1)},
      {"[][0]", new_number(-1)},
      {"[1, 2, 3][99]", (Object *)&obj_null},
      {"[1][-1]", (Object *)&obj_null},
      {"{1: 1, 2: 2}[1]", new_number(1)},
      {"{1: 1, 2: 2}[2]", new_number(2)},
      {"{1: 1}[0]", (Object *)&obj_null},
      {"{}[0]", (Object *)&obj_null},
  };

  VM_RUN_TESTS(tests);
}

void test_calling_functions(void) {
  vmTestCase tests[] = {
      {
          .input = "let fivePlusTen = fn() { 5 + 10; }; fivePlusTen();",
          .expected = new_number(15),
      },
      {
          .input = "let one = fn() { 1; };"
                   "let two = fn() { 2; };"
                   "one() + two()",
          .expected = new_number(3),
      },
      {
          .input = "let a = fn() { 1 };"
                   "let b = fn() { a() + 1 };"
                   "let c = fn() { b() + 1 };"
                   "c(); ",
          .expected = new_number(3),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_functions_with_return_statement(void) {
  vmTestCase tests[] = {
      {
          .input = "let earlyExit = fn() { return 99; 100; };"
                   "earlyExit();",
          .expected = new_number(99),
      },
      {
          .input = "let earlyExit = fn() { return 99; return 100; };"
                   "earlyExit();",
          .expected = new_number(99),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_functions_without_return_value(void) {
  vmTestCase tests[] = {
      {
          .input = "let noReturn = fn() { }; noReturn();",
          .expected = (Object *)&obj_null,
      },
      {
          .input = "let noReturn = fn() { };"
                   "let noReturnTwo = fn() { noReturn(); };"
                   "noReturn(); noReturnTwo();",
          .expected = (Object *)&obj_null,
      },
  };

  VM_RUN_TESTS(tests);
}

void test_calling_functions_with_bindings(void) {
  vmTestCase tests[] = {
      {
          .input = "let one = fn() { let one = 1; one; }; one();",
          .expected = new_number(1),
      },
      {
          .input =
              "let oneAndTwo = fn() { let one = 1; let two = 2; one + two; };"
              "oneAndTwo();",
          .expected = new_number(3),
      },
      {
          .input =
              "let oneAndTwo = fn() { let one = 1; let two = 2; one + two; };"
              "let threeAndFour = fn() { let three = 3; let four = 4; three + "
              "four; };"
              "oneAndTwo() + threeAndFour();",
          .expected = new_number(10),
      },
      {
          .input = "let firstFoobar = fn() { let foobar = 50; foobar; };"
                   "let secondFoobar = fn() { let foobar = 100; foobar; };"
                   "firstFoobar() + secondFoobar();",
          .expected = new_number(150),
      },
      {
          .input = "let globalSeed = 50;"
                   "let minusOne = fn() {"
                   "  let num = 1;"
                   "  globalSeed - num;"
                   "};"
                   "let minusTwo = fn() {"
                   "  let num = 2;"
                   "  globalSeed - num;"
                   "};"
                   "minusOne() + minusTwo();",
          .expected = new_number(97),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_functions_with_arguments_and_bindings(void) {
  vmTestCase tests[] = {
      {
          .input = "let identity = fn(a) { a; };"
                   "identity(4);",
          .expected = new_number(4),
      },
      {
          .input = "let sum = fn(a, b) { a + b; };"
                   "sum(1, 2);",
          .expected = new_number(3),
      },
      {
          .input = "let sum = fn(a, b) {"
                   "let c = a + b;"
                   "c;"
                   "};"
                   "sum(1, 2);",
          .expected = new_number(3),
      },
      {
          .input = "let sum = fn(a, b) {"
                   "let c = a + b;"
                   "c;"
                   "};"
                   "sum(1, 2) + sum(3, 4)",
          .expected = new_number(10),
      },
      {
          .input = "let sum = fn(a, b) {"
                   "let c = a + b;"
                   "c;"
                   "};"
                   ""
                   "let outer = fn() {"
                   "  sum(1, 2) + sum(3, 4);"
                   "};"
                   "outer();",
          .expected = new_number(10),
      },
      {
          .input = "let globalNum = 10;"
                   "let sum = fn(a, b) {"
                   "let c = a + b;"
                   "c + globalNum;"
                   "};"
                   "let outer = fn() {"
                   "sum(1, 2) + sum(3, 4) + globalNum;"
                   "};"
                   "outer() + globalNum;",
          .expected = new_number(50),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_calling_functions_with_wrong_arguments(void) {
  vmTestCase tests[] = {
      {
          .input = "fn() { 1; }(1);",
          .expected = new_string("wrong number of arguments"),
      },
      {
          .input = "fn(a) { a; }();",
          .expected = new_string("wrong number of arguments"),
      },
      {
          .input = "fn(a, b) { a + b; }(1);",
          .expected = new_string("wrong number of arguments"),
      },
  };

  for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
    Program *program = parse(tests[i]);

    Compiler *compiler = new_compiler();
    CompilerResult compiler_error = compile_program(compiler, program);
    TEST_ASSERT_EQUAL(COMPILER_OK, compiler_error);

    VM *vm = new_vm(bytecode(compiler));
    VMResult vm_result = run_vm(vm);

    TEST_ASSERT_NOT_EQUAL(VM_OK, vm_result);

    char msg[100];
    memset(msg, 0, 100);
    vm_error(vm_result, msg, 100);

    TEST_ASSERT_EQUAL(STRING_OBJ, tests[i].expected->type);
    String *str = (String *)tests[i].expected;
    TEST_ASSERT_EQUAL_STRING(str->value, msg);

    free_object(tests[i].expected);
  }
}

void test_builtin_functions(void) {
  vmTestCase tests[] = {
      {"push([], 1)", new_array((Object *[]){new_number(1)}, 1)},
      {"len(\"\")", new_number(0)},
      {"len(\"four\")", new_number(4)},
      {"len(\"hello world\")", new_number(11)},
      {"len(1)", new_error("argument to 'len' not supported, got NUMBER_OBJ")},
      {"len(\"one\", \"two\")",
       new_error("wrong number of arguments: Expected 1 got 2")},
      {"len([1, 2, 3])", new_number(3)},
      {"len([])", new_number(0)},
      {"first([1, 2, 3])", new_number(1)},
      {"first([])", (Object *)&obj_null},
      {"first(1)",
       new_error("argument to 'first' not supported, got NUMBER_OBJ")},
      {"last([1, 2, 3])", new_number(3)},
      {"last([])", (Object *)&obj_null},
      {"last(1)",
       new_error("argument to 'last' not supported, got NUMBER_OBJ")},
      {"rest([1, 2, 3])", new_array(
                              (Object *[]){
                                  new_number(2),
                                  new_number(3),
                              },
                              2)},
      {"rest([])", (Object *)&obj_null},
      {"puts(\"hello\", \"world!\")", (Object *)&obj_null},
      {"push(1, 1)",
       new_error("argument to 'push' not supported, got NUMBER_OBJ")},
  };

  VM_RUN_TESTS(tests);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  RUN_TEST(test_boolean_expressions);
  RUN_TEST(test_conditionals);
  RUN_TEST(test_global_let_statements);
  RUN_TEST(test_string_expressions);
  RUN_TEST(test_array_literals);
  RUN_TEST(test_calling_functions);
  RUN_TEST(test_functions_with_return_statement);
  RUN_TEST(test_functions_without_return_value);
  RUN_TEST(test_calling_functions_with_bindings);
  RUN_TEST(test_functions_with_arguments_and_bindings);
  RUN_TEST(test_calling_functions_with_wrong_arguments);
  RUN_TEST(test_builtin_functions);
  return UNITY_END();
}
