#include "../ast/ast.h"
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define VM_RUN_TESTS(tests) run_vm_tests(tests, ARRAY_LEN(tests));

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
  switch (expected->type) {
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

void dump_bytecode(Bytecode bt) {
  for (size_t i = 0; i < bt.constants.len; i++) {
    Object *obj = bt.constants.arr[i];
    printf("CONSTANT %ld %p (%s) \n", i, obj, ObjectTypeString[obj->type]);

    switch (obj->type) {
    case NUMBER_OBJ:
      printf(" Value: %.1f\n", ((Number *)obj)->value);
      break;
    case COMPILED_FUNCTION_OBJ: {
      ResizableBuffer buf;
      init_resizable_buffer(&buf, 50);
      instructions_to_string(&buf, &((CompiledFunction *)obj)->instructions);
      printf(" Instructions:\n%s\n", buf.buf);

      free(buf.buf);
      break;
    }
    default:
      break;
    }
  }
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

    Bytecode bt = bytecode(compiler);
    VM *vm = new_vm(bt);
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
      {"true", new_boolean(true)},
      {"1 < 2", new_boolean(true)},
      {"1 == 1", new_boolean(true)},
      {"1 != 2", new_boolean(true)},
      {"true == true", new_boolean(true)},
      {"false == false", new_boolean(true)},
      {"true != false", new_boolean(true)},
      {"false != true", new_boolean(true)},
      {"(1 < 2) == true", new_boolean(true)},
      {"(1 > 2) == false", new_boolean(true)},
      {"!false", new_boolean(true)},
      {"!!true", new_boolean(true)},
      {"!!5", new_boolean(true)},
      {"false", new_boolean(false)},
      {"1 > 2", new_boolean(false)},
      {"1 < 1", new_boolean(false)},
      {"1 > 1", new_boolean(false)},
      {"1 != 1", new_boolean(false)},
      {"1 == 2", new_boolean(false)},
      {"true == false", new_boolean(false)},
      {"(1 < 2) == false", new_boolean(false)},
      {"(1 > 2) == true", new_boolean(false)},
      {"!true", new_boolean(false)},
      {"!5", new_boolean(false)},
      {"!!false", new_boolean(false)},
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
      {"if (false) { 10; }", new_null()},
      {"if (1 > 2) { 10; }", new_null()},
      {"!(if (false) { 10; })", new_boolean(true)},
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
      {"[1, 2, 3][99]", new_null()},
      {"[1][-1]", new_null()},
      {"{1: 1, 2: 2}[1]", new_number(1)},
      {"{1: 1, 2: 2}[2]", new_number(2)},
      {"{1: 1}[0]", new_null()},
      {"{}[0]", new_null()},
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
          .expected = new_null(),
      },
      {
          .input = "let noReturn = fn() { };"
                   "let noReturnTwo = fn() { noReturn(); };"
                   "noReturn(); noReturnTwo();",
          .expected = new_null(),
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
      {"first([])", new_null()},
      {"first(1)",
       new_error("argument to 'first' not supported, got NUMBER_OBJ")},
      {"last([1, 2, 3])", new_number(3)},
      {"last([])", new_null()},
      {"last(1)",
       new_error("argument to 'last' not supported, got NUMBER_OBJ")},
      {"rest([1, 2, 3])", new_array(
                              (Object *[]){
                                  new_number(2),
                                  new_number(3),
                              },
                              2)},
      {"rest([])", new_null()},
      {"puts(\"hello\", \"world!\")", new_null()},
      {"push(1, 1)",
       new_error("argument to 'push' not supported, got NUMBER_OBJ")},
  };

  VM_RUN_TESTS(tests);
}

void test_closures(void) {
  vmTestCase tests[] = {
      {
          .input = "let newClosure = fn(a) {"
                   "  fn() { a; };"
                   "};"
                   "let closure = newClosure(99);"
                   "closure();",
          .expected = new_number(99),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_recursive_functions(void) {
  vmTestCase tests[] = {
      {
          .input = "let countDown = fn(x) {"
                   "  if (x == 0) {"
                   "    return 0;"
                   "  } else { "
                   "    countDown(x - 1);"
                   "  }"
                   "};"
                   "countDown(1);",
          .expected = new_number(0),
      },
      {
          .input = "let countDown = fn(x) {"
                   "  if (x == 0) {"
                   "    return 0;"
                   "  } else { "
                   "    countDown(x - 1);"
                   "  }"
                   "};"
                   "let wrapper = fn() {"
                   "  countDown(1);"
                   "};"
                   "wrapper();",
          .expected = new_number(0),
      },
      {
          .input = "let wrapper = fn() {"
                   "  let countDown = fn(x) {"
                   "    if (x == 0) {"
                   "      return 0;"
                   "    } else { "
                   "      countDown(x - 1);"
                   "    }"
                   "  };"
                   "  let wrapper = fn() {"
                   "    countDown(1);"
                   "  };"
                   "  countDown(1);"
                   "};"
                   "wrapper();",
          .expected = new_number(0),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_reassignments(void) {
  vmTestCase tests[] = {
      {
          .input = "let a = 1; "
                   " a = 2; a; ",
          .expected = new_number(2),
      },
      {
          .input = "let a = fn(x) { x + 2 };"
                   "a = fn(x) { x * 2 };"
                   "a(10);",
          .expected = new_number(20),
      },
      {
          .input = "let a = 10;"
                   "a = 20;",
          .expected = new_number(20),
      },
      {
          .input = "let a = 10;"
                   "let b = fn(x) { a = x; }"
                   "b(20);",
          .expected = new_number(20),
      },
      {
          .input = "let wrapper = fn() {"
                   " let a = 10;"
                   " let nestedFn = fn(x) {"
                   "   a = x; "
                   " };"
                   " nestedFn(2);"
                   "return a;"
                   "};"
                   "wrapper();",
          .expected = new_number(2),
      },
      {
          .input = "let wrapper = fn() {"
                   " let a = 10;"
                   " let nestedFn = fn(x) {"
                   "   a = x; "
                   " };"
                   " nestedFn(\"hello world\");"
                   "return a;"
                   "};"
                   "wrapper();",
          .expected = new_string("hello world"),
      },
      {
          .input = "let a = 1; a = a + 1; a;",
          .expected = new_number(2),
      },
      {
          .input = "let a = [1, 2, 3]; a[1] = 7; a;",
          .expected = new_array(
              (Object *[]){
                  new_number(1),
                  new_number(7),
                  new_number(3),
              },
              3),
      },
      {
          .input = "let a = { \"key\": 10 }; a[\"key\"] = 5; a[\"key\"]",
          .expected = new_number(5),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_while_loop(void) {
  vmTestCase tests[] = {
      {
          .input = "let a = 0;"
                   "while (a < 10) {"
                   "  a = a + 1;"
                   "};"
                   "a;",
          .expected = new_number(10),
      },
      {
          .input = "let a = 0;"
                   "while (a < 10) {"
                   "  a = a + 1;    "
                   "  if (a == 5) { "
                   "    break;      "
                   " }              "
                   "};"
                   "a;",
          .expected = new_number(5),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_while_loop_closures(void) {
  vmTestCase tests[] = {
      {
          .input = "let fun = fn(x) {   "
                   "  let acc = 0;      "
                   "  while (acc < x) { "
                   "    acc = acc + 1;  "
                   "  };                "
                   "  return acc;       "
                   "};                  "
                   "fun(10);            "
                   "fun(10);            ",
          .expected = new_number(10),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_for_loop(void) {
  vmTestCase tests[] = {
      {
          .input = "let a = 0;"
                   "for (let b = 0; b < 11; b = b + 1) {"
                   "  a = a + b;                        "
                   "}"
                   "a; ",
          .expected = new_number(55),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_nested_loops(void) {
  vmTestCase tests[] = {
      {
          .input = "let a = 0;         "
                   "while (a < 2) {    "
                   "  let b = 0;       "
                   "  while (b < 10) { "
                   "    a = a + b;     "
                   "    b = b + 1;     "
                   "  }                "
                   "  a = a + 1;       "
                   "}                  "
                   "a;                 ",
          .expected = new_number(46),
      },
  };

  VM_RUN_TESTS(tests);
}

void test_nested_closures(void) {
  vmTestCase tests[] = {
      {
          .input = "let foo = fn (x) { "
                   "  let buffer = \"\";                 "
                   "  let i = 0;                         "
                   "  while (i < x) {                    "
                   "    buffer = buffer + \"a\";         "
                   "    i = i + 1;                       "
                   "  };                                 "
                   "  return buffer;                     "
                   "};                                   "
                   "foo(1);                              "
                   "foo(2);                              ",
          .expected = new_string("aa"),
      },
      {
          .input = "let foo = fn (x) { "
                   "  let buffer = \"\";                 "
                   "  for (let i = 0; i < x; i = i + 1) {"
                   "    buffer = buffer + \"a\";         "
                   "  };                                 "
                   "  return buffer;                     "
                   "};                                   "
                   "foo(1);                              "
                   "foo(2);                              ",
          .expected = new_string("aa"),
      },
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
  RUN_TEST(test_closures);
  RUN_TEST(test_recursive_functions);
  RUN_TEST(test_reassignments);
  RUN_TEST(test_while_loop);
  RUN_TEST(test_while_loop_closures);
  RUN_TEST(test_for_loop);
  RUN_TEST(test_nested_loops);
  RUN_TEST(test_nested_closures);
  return UNITY_END();
}
