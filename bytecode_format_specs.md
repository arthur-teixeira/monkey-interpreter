# Bytecode file format
I'm taking inspiration from the [Java class file format](https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html),
but I'm going to make it much simpler, to fit the needs of my language.

## The class format of Java is described down below:
ClassFile {
    4 bytes        magic;
    2 bytes        minor_version;
    2 bytes        major_version;
    2 bytes        constant_pool_count;
    n bytes        constant_pool[constant_pool_count-1];
    2 bytes        access_flags;
    2 bytes        this_class;
    2 bytes        super_class;
    2 bytes        interfaces_count;
    2 bytes        interfaces[interfaces_count];
    2 bytes        fields_count;
    field_info     fields[fields_count];
    2 bytes        methods_count;
    method_info    methods[methods_count];
    2 bytes        attributes_count;
    attribute_info attributes[attributes_count];
}

The current plan is to make my file quite similar, but with much less features.
The initial implementation should look something like this:

MonkeyFile {
    4 bytes        magic;
    2 bytes        constant_pool_count;
    n bytes        constant_pool[constant_pool_count-1];
    2 bytes        instructions_count;
    n bytes        instructions;
}

Any multi byte value is stored in big endian.
