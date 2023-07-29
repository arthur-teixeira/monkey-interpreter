# Monkey Interpreter

This is an interpreter for the Monkey programming language, built while reading
[The Interpreter Book](https://interpreterbook.com) by Thorsten Ball.
While the original book implementation is written in Go, I wrote it in C as
an exercise to learn the language.

Currently, I'm working on adding a compiler for the Monkey language alongside 
[The Compiler Book](https://compilerbook.com/). The compiler
will compile Monkey source code to bytecode, which will be interpreted by the
Monkey Virtual Machine (MVM).

### Quick Start
First, clone the repository and compile the project:
```sh
$ make
```

To run the REPL in Interpreter mode, run the following command:
```sh
$ ./bin/monkey
```

You can also run the REPL in Compiler mode:
```sh 
$ ./bin/monkey -c
```

If you want to run a Monkey source file, pass the path to the file as an argument:
(currently this only works in Interpreter mode)
```sh 
$ ./bin/monkey <path-to-file>
```

## To-do list
- [X] For/while loops
- [X] Reassignment expression without let keyword
- [X] Binary numbers
- [X] Hex numbers
- [X] Bitwise operators
- [X] && and || operators
- [X] Floating point number support
- [X] Turing completeness proof (rule 110)
- [X] Interpret source files
- [ ] Compiler
- [ ] Compile to file and remove compiler from REPL
- [ ] File disassembler
- [ ] VM
- [ ] Convert objects to tagged unions
- [ ] Garbage collection
- [ ] Concurrent GC
