#include "../unity/src/unity.h"
#include "symbol_table.h"

#define ARRAY_LEN(a) sizeof(a) / sizeof(a[0])

void test_symbol(const Symbol *expected, const Symbol *actual) {
  TEST_ASSERT_EQUAL_STRING(expected->name, actual->name);
  TEST_ASSERT_EQUAL(expected->scope, actual->scope);
  TEST_ASSERT_EQUAL_INT(expected->index, actual->index);
}

void test_define(void) {
  Symbol expected_a = {"a", SYMBOL_GLOBAL_SCOPE, 0};
  Symbol expected_b = {"b", SYMBOL_GLOBAL_SCOPE, 1};
  Symbol expected_c = {"c", SYMBOL_LOCAL_SCOPE, 0};
  Symbol expected_d = {"d", SYMBOL_LOCAL_SCOPE, 1};
  Symbol expected_e = {"e", SYMBOL_LOCAL_SCOPE, 0};
  Symbol expected_f = {"f", SYMBOL_LOCAL_SCOPE, 1};

  SymbolTable *global = new_symbol_table();

  const Symbol *a = symbol_define(global, "a");
  test_symbol(&expected_a, a);

  const Symbol *b = symbol_define(global, "b");
  test_symbol(&expected_b, b);

  SymbolTable *first_local = new_enclosed_symbol_table(global);

  const Symbol *c = symbol_define(first_local, "c");
  test_symbol(&expected_c, c);

  const Symbol *d = symbol_define(first_local, "d");
  test_symbol(&expected_d, d);

  SymbolTable *second_local = new_enclosed_symbol_table(first_local);

  const Symbol *e = symbol_define(second_local, "e");
  test_symbol(&expected_e, e);

  const Symbol *f = symbol_define(second_local, "f");
  test_symbol(&expected_f, f);

  free_symbol_table(global);
  free_symbol_table(first_local);
  free_symbol_table(second_local);
}

void test_resolve_global(void) {
  Symbol expected_a = {
      .name = "a",
      .scope = SYMBOL_GLOBAL_SCOPE,
      .index = 0,
  };

  Symbol expected_b = {
      .name = "b",
      .scope = SYMBOL_GLOBAL_SCOPE,
      .index = 1,
  };

  SymbolTable *global = new_symbol_table();
  symbol_define(global, "a");
  symbol_define(global, "b");

  const Symbol *resolved_a = symbol_resolve(global, "a");
  TEST_ASSERT_NOT_NULL(resolved_a);
  test_symbol(&expected_a, resolved_a);

  const Symbol *resolved_b = symbol_resolve(global, "b");
  TEST_ASSERT_NOT_NULL(resolved_b);
  test_symbol(&expected_b, resolved_b);

  free_symbol_table(global);
}

void test_resolve_local(void) {
  SymbolTable *global = new_symbol_table();
  symbol_define(global, "a");
  symbol_define(global, "b");

  SymbolTable *local = new_enclosed_symbol_table(global);
  symbol_define(local, "c");
  symbol_define(local, "d");

  Symbol expected[] = {
      {"a", SYMBOL_GLOBAL_SCOPE, 0},
      {"b", SYMBOL_GLOBAL_SCOPE, 1},
      {"c", SYMBOL_LOCAL_SCOPE, 0},
      {"d", SYMBOL_LOCAL_SCOPE, 1},
  };

  for (size_t i = 0; i < ARRAY_LEN(expected); i++) {
    const Symbol *resolved = symbol_resolve(local, expected[i].name);
    TEST_ASSERT_NOT_NULL(resolved);
    test_symbol(&expected[i], resolved);
  }

  free_symbol_table(global);
  free_symbol_table(local);
}

void test_resolve_nested_local(void) {
  SymbolTable *global = new_symbol_table();
  symbol_define(global, "a");
  symbol_define(global, "b");

  SymbolTable *local = new_enclosed_symbol_table(global);
  symbol_define(local, "c");
  symbol_define(local, "d");

  SymbolTable *second_local = new_enclosed_symbol_table(local);
  symbol_define(second_local, "e");
  symbol_define(second_local, "f");

  struct test_case {
    SymbolTable *table;
    size_t len;
    Symbol *expected;
  } tests[] = {
      {
          .table = local,
          .len = 4,
          .expected =
              (Symbol[]){
                  {"a", SYMBOL_GLOBAL_SCOPE, 0},
                  {"b", SYMBOL_GLOBAL_SCOPE, 1},
                  {"c", SYMBOL_LOCAL_SCOPE, 0},
                  {"d", SYMBOL_LOCAL_SCOPE, 1},
              },
      },
      {
          .table = second_local,
          .len = 4,
          .expected =
              (Symbol[]){
                  {"a", SYMBOL_GLOBAL_SCOPE, 0},
                  {"b", SYMBOL_GLOBAL_SCOPE, 1},
                  {"e", SYMBOL_LOCAL_SCOPE, 0},
                  {"f", SYMBOL_LOCAL_SCOPE, 1},
              },
      },
  };

  for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
    struct test_case test = tests[i];
    for (size_t j = 0; j < test.len; j++) {
      Symbol expected = test.expected[j];
      const Symbol *actual = symbol_resolve(test.table, expected.name);
      TEST_ASSERT_NOT_NULL(actual);

      test_symbol(&expected, actual);
    }
  }

  free_symbol_table(global);
  free_symbol_table(local);
  free_symbol_table(second_local);
}

void test_define_resolve_builtins(void) {
  SymbolTable *global = new_symbol_table();
  SymbolTable *first_local = new_enclosed_symbol_table(global);
  SymbolTable *second_local = new_enclosed_symbol_table(first_local);

  Symbol expected[] = {
      {"a", SYMBOL_BUILTIN_SCOPE, 0},
      {"c", SYMBOL_BUILTIN_SCOPE, 1},
      {"e", SYMBOL_BUILTIN_SCOPE, 2},
      {"f", SYMBOL_BUILTIN_SCOPE, 3},
  };

  for (size_t i = 0; i < ARRAY_LEN(expected); i++) {
    symbol_define_builtin(global, i, expected[i].name);
  }

  SymbolTable *tables[] = {
      global,
      first_local,
      second_local,
  };

  for (size_t i = 0; i < ARRAY_LEN(tables); i++) {
    for (size_t j = 0; j < ARRAY_LEN(expected); j++) {
      const Symbol *result = symbol_resolve(tables[i], expected[j].name);

      TEST_ASSERT_NOT_NULL(result);

      test_symbol(&expected[j], result);
    }
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_define);
  RUN_TEST(test_resolve_global);
  RUN_TEST(test_resolve_local);
  RUN_TEST(test_resolve_nested_local);
  RUN_TEST(test_define_resolve_builtins);
  return UNITY_END();
}
