#include "../code/code.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "../dyn_array/dyn_array.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

void test_make(void) {
    struct testCase {
       OpCode op;
       int operands[1];
       Instruction expected;
    }; 
    struct testCase tests[2];

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

    Instruction op_add_instruction;
    int_array_init(&op_add_instruction, 1);
    int_array_append(&op_add_instruction, OP_ADD);

    struct testCase op_add_test = {
        .op = OP_ADD,
        .operands = {},
        .expected = op_add_instruction,
    };
    
    tests[1] = op_add_test;

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
        make_instruction(OP_ADD, (int[]){}, 0),
        make_instruction(OP_CONSTANT, (int[]){2}, 1),
        make_instruction(OP_CONSTANT, (int[]){65535}, 1),
    };

    char *expected = ""
        "0000 OP_ADD\n"
        "0001 OP_CONSTANT 2\n"
        "0004 OP_CONSTANT 65535\n";

    Instructions ins = concat_instructions(3, instructions);

    ResizableBuffer buf;
    init_resizable_buffer(&buf, strlen(expected));

    instructions_to_string(&buf, &ins);

    TEST_ASSERT_EQUAL_STRING(expected, buf.buf);
    free(buf.buf);
}

void test_read_operands(void) {
    struct testCase {
        OpCode op;
        int operands[2];
        int bytes_read;
    };
    
    struct testCase tests[] = {
        {OP_CONSTANT, {65535}, 2},
    };

    for (size_t i = 0; i < 1; i++) {
        struct testCase test = tests[i];
        Instruction ins = make_instruction(test.op, test.operands, 1);

        Definition *def = lookup(test.op);
        TEST_ASSERT_NOT_NULL(def);

        size_t bytes_read;
        IntArray operands = read_operands(def, &ins, 1, &bytes_read);

        TEST_ASSERT_EQUAL(test.bytes_read, bytes_read);

        for (size_t j = 0; j < def->operand_count; j++) {
            TEST_ASSERT_EQUAL(test.operands[j], operands.arr[j]);
        }
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_make);
    RUN_TEST(test_instructions_string);
    RUN_TEST(test_read_operands);
    return UNITY_END();
}
