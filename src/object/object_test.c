#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "object.h"
#include <string.h>

void assert_keys(Object *a, Object *b) {
  int32_t hash_a = get_hash_key(a);
  int32_t hash_b = get_hash_key(b);

  TEST_ASSERT_EQUAL(hash_a, hash_b);
}

void test_string_hash_key(void) {
  size_t hello_len = strlen("Hello World");
  String hello1 = {
    .type = STRING_OBJ,
    .value = "Hello World",
    .len = hello_len
  };

  String hello2 = {
      .type = STRING_OBJ,
      .value = "Hello World",
      .len = hello_len
  };

  size_t jeff_len = strlen("My name Jeff");
  String jeff1 = {
      .type = STRING_OBJ,
      .value = "My name Jeff",
      .len = jeff_len
  };

  String jeff2 = {
      .type = STRING_OBJ,
      .value = "My name Jeff",
      .len = jeff_len
  };

  assert_keys((Object*)&hello1, (Object*)&hello2);
  assert_keys((Object*)&jeff1, (Object*)&jeff2);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_string_hash_key);
  return UNITY_END();
}
