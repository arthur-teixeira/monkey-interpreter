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

Instructions concat_instructions(size_t instruction_count, Instruction instructions[]) {
    Instructions result;
    int_array_init(&result, instruction_count);

    for (size_t i = 0; i < instruction_count; i++) {
        Instruction ins = instructions[i];
        for (size_t j = 0; j < ins.len; j++) {
            int_array_append(&result, ins.arr[j]);
        }
    }

    return result;
}

void test_instructions_string(void) {
    Instruction instructions[] = {
        make_instruction(OP_CONSTANT, (int[]){1}, 1),
        make_instruction(OP_CONSTANT, (int[]){2}, 1),
        make_instruction(OP_CONSTANT, (int[]){65535}, 1),
    };

    char *expected = ""
        "0000 OP_CONSTANT 1\n"
        "0003 OP_CONSTANT 2\n"
        "0006 OP_CONSTANT 65535\n";

    Instructions ins = concat_instructions(3, instructions);

    TEST_ASSERT_EQUAL_STRING(expected, instructions_to_string(ins));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_make);
    return UNITY_END();
}
