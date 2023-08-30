# Bytecode file format
I'm taking inspiration from the [Java class file format](https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html),
but I'm going to make it much simpler, to fit the needs of my language.

## The class format of Java is described down below:
ClassFile { \
    4 bytes        magic; \
    2 bytes        minor_version; \
    2 bytes        major_version; \
    2 bytes        constant_pool_count; \
    n bytes        constant_pool[constant_pool_count-1]; \
    2 bytes        access_flags; \
    2 bytes        this_class; \
    2 bytes        super_class; \
    2 bytes        interfaces_count; \
    2 bytes        interfaces[interfaces_count]; \
    2 bytes        fields_count; \
    field_info     fields[fields_count]; \
    2 bytes        methods_count; \
    method_info    methods[methods_count]; \
    2 bytes        attributes_count; \
    attribute_info attributes[attributes_count]; \
}

The current plan is to make my file quite similar, but with much less features.
The initial implementation should look something like this:

MonkeyFile { \
    4 bytes        magic; \
    2 bytes        constant_pool_count; \
    constant_info  constant_pool[constant_pool_count-1]; \
    2 bytes        instructions_count; \
    n bytes        instructions; \
}

Any multi byte value is stored in big endian format.

## The magic number
The magic number is used to identify the file as a Monkey bytecode file.
It is used to check if the file is valid, and to check if the file is a Monkey bytecode file.
The magic number is 6 bytes long, and is the following bytes: 0x4D 0x4F 0x4E 0x4B 0x45 0x59 (MONKEY in ASCII).

## The constant pool
The constant pool is used to store constants. In the compiler, constants are Object values allocated in memory.
In the object file, we have to store these values in a specific layout to be able to load them into memory again.
Each constant has a different structure in the object file, described below.

### The constant_info structure
The constant_info structure is used to store a constant in the constant pool.
It has the following structure:

constant_info { \
    1 byte         constant_type;\
    constant_value value;\
}

The constant_type field stores the enum value defining the type of the constant.

| value | definition            |
|-------|-----------------------|
| 0     | NUMBER_OBJ            |
| 6     | STRING_OBJ            |
| 12    | COMPILED_FUNCTION_OBJ |
| 14    | COMPILED_LOOP_OBJ     |

## Number constant
A number object contains a double storing the value of the number.

number_constant { \
  8 bytes  value; \
}

## String constant
A string constant is defined as the following structure:

string_constant { \
  2 bytes  string_length; \
  [string_length] bytes  string; \
}

where each character of the string is saved as an ASCII value.

## Compiled function constant
A compiled function constant is defined as the following structure:

function_constant { \
      2 bytes                     local_variables_count; \
      1 byte                      parameters_count; \
      2 bytes                     instructions_length; \
      [instructions_length] bytes instructions; \
}

## Compiled loop constant
A compiled loop constant is defined as the following structure:

loop_constant { \
      2 bytes                     local_variables_count; \
      2 bytes                     instructions_length; \
      [instructions_length] bytes instructions; \
}
