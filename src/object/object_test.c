#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "object.h"
#include <string.h>

Object *wrap_string(String *str) {
  Object *obj = malloc(sizeof(Object));
  obj->type = STRING_OBJ;
  obj->object = str;

  return obj;
}

void assert_and_free(Object *a, Object *b) {
  int32_t hash_a = get_hash_key(a);
  int32_t hash_b = get_hash_key(b);

  TEST_ASSERT_EQUAL(hash_a, hash_b);

  free(a);
  free(b);
}

void test_string_hash_key(void) {
  size_t hello_len = strlen("Hello World");
  String hello1 = {"Hello World", hello_len};
  String hello2 = {"Hello World", hello_len};

  size_t jeff_len = strlen("My name Jeff");
  String jeff1 = {"My name Jeff", jeff_len};
  String jeff2 = {"My name Jeff", jeff_len};

  assert_and_free(wrap_string(&hello1), wrap_string(&hello2));
  assert_and_free(wrap_string(&jeff1), wrap_string(&jeff2));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_string_hash_key);
  return UNITY_END();
}
