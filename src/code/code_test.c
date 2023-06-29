#include "../code/code.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "../dyn_array/dyn_array.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

void test_make(void) {
    struct testCase {
       OpCode op;
       int operands[1];
       Instruction expected;
    }; 
    struct testCase tests[1];

    Instruction op_constant_instructions;
    int_array_init(&op_constant_instructions, 3);
    int_array_append(&op_constant_instructions, OP_CONSTANT);
    int_array_append(&op_constant_instructions, 0xFF);
    int_array_append(&op_constant_instructions, 0xFE);

    struct testCase op_constant_test = {
        .op = OP_CONSTANT,
        .operands = {0xFFFE},
        .expected = op_constant_instructions,
    };

    tests[0] = op_constant_test;

    for (uint32_t i = 0; i < ARRAY_LEN(tests); i++) {
        struct testCase test = tests[i];
        Instruction ins = make_instruction(test.op, test.operands, 1);

        TEST_ASSERT_EQUAL(test.expected.len, ins.len);

        for (uint32_t j = 0; j < test.expected.len; j++) {
            TEST_ASSERT_EQUAL(test.expected.arr[j], ins.arr[j]);
        }

        int_array_free(&ins);
    }

    int_array_free(&op_constant_instructions);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_make);
    return UNITY_END();
}
