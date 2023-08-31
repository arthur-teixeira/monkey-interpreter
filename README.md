# Monkey Interpreter

This is an interpreter and a compiler for the Monkey programming language, built while reading
[The Interpreter Book](https://interpreterbook.com) and [The Compiler Book](https://compilerbook) by Thorsten Ball.
While the original book implementation is written in Go, I wrote it in C as
an exercise to learn the language.

There is also a VM that executes all of the compiler generated instructions.
Currently, I'm working on the implementation of a bytecode file format, so that
I can separate compilation time from runtime when using the compiler.
There are more details regarding this format in the bytecode_format_specs.md file.

### Quick Start
First, clone the repository and compile the project:
```sh
$ make
```

To run the REPL in Interpreter mode, run the following command:
```sh
$ ./bin/monkey -i
```

You can also run the REPL in Compiler mode:
Here, the compiler will compile the input and then execute the bytecode in the VM.
```sh 
$ ./bin/monkey -c
```

If you want to run a Monkey source file, pass the path to the file as an argument:
```sh 
$ ./bin/monkey <engine-flag> <path-to-file>
```

You can also generate a bytecode file from a Monkey source file:
```sh 
$ ./bin/monkey -c <path-to-input-file> <path-to-output-file>
```

And to load and execute a bytecode file:
```sh
$ ./bin/monkey -l <path-to-bytecode-file>
```

To disassemble a bytecode file:
```sh 
$ ./bin/monkey -d <path-to-bytecode-file>
```

## To-do list
- [X] For/while loops
  - [X] For/while loop in compiler
    - [X] While loop
    - [X] For loop
  - [X] Compiled loop binary format
- [X] Continue statements
- [X] Break statements
- [X] Reassignment expression without let keyword
  - [X] Reassignment in compiler
- [X] Binary numbers
- [X] Hex numbers
- [X] Bitwise operators
- [X] && and || operators
- [X] Floating point number support
- [X] Turing completeness proof (rule 110)
- [X] Interpret source files
- [X] Compiler
- [X] VM
- [X] Compile to and from file
  - [X] Load file as compiler input
  - [X] Execute in the vm after compilation
  - [X] Write bytecode to file
- [X] File disassembler
- [ ] Implement bubble sort
- [ ] Garbage collection
- [ ] Concurrent GC

## Known bugs
- [X] Running with -l flag is not loading the binary to the VM.
- [X] Reassigning free variables loaded from a constant changes the value of the constant
- [ ] Cannot call nested functions
