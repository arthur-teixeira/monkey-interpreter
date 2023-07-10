#include "../unity/src/unity.h"
#include "symbol_table.h"

void test_symbol(const Symbol *expected, const Symbol *actual) {
  TEST_ASSERT_EQUAL_STRING(expected->name, actual->name);
  TEST_ASSERT_EQUAL_STRING(expected->scope, actual->scope);
  TEST_ASSERT_EQUAL_INT(expected->index, actual->index);
}

void test_define(void) {
  Symbol expected_a = {
      .name = "a",
      .scope = GLOBAL_SCOPE,
      .index = 0,
  };

  Symbol expected_b = {
      .name = "b",
      .scope = GLOBAL_SCOPE,
      .index = 1,
  };

  SymbolTable *global = new_symbol_table();

  const Symbol *a = symbol_define(global, "a");
  test_symbol(&expected_a, a);

  const Symbol *b = symbol_define(global, "b");
  test_symbol(&expected_b, b);

  free_symbol_table(global);
}

void test_resolve_global(void) {
  Symbol expected_a = {
      .name = "a",
      .scope = GLOBAL_SCOPE,
      .index = 0,
  };

  Symbol expected_b = {
      .name = "b",
      .scope = GLOBAL_SCOPE,
      .index = 1,
  };

  SymbolTable *global = new_symbol_table();
  symbol_define(global, "a");
  symbol_define(global, "b");

  const Symbol *resolved_a = symbol_resolve(global, "a");
  TEST_ASSERT_NOT_NULL(resolved_a);
  test_symbol(resolved_a, &expected_a);

  const Symbol *resolved_b = symbol_resolve(global, "b");
  TEST_ASSERT_NOT_NULL(resolved_b);
  test_symbol(resolved_b, &expected_b);

  free_symbol_table(global);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_define);
  RUN_TEST(test_resolve_global);
  return UNITY_END();
}
