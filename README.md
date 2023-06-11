# Monkey Interpreter

This is an interpreter for the Monkey programming language, built while reading
[The Interpreter Book](https://interpreterbook.com) by Thorsten Ball.
While the original book implementation is written in Go, I'm writing it in C as
an exercise to learn the language.

### Quick Start

```sh
$ make
$ ./bin/program
```

## To-do list
- [X] For/while loops
- [X] Reassignment expression without let keyword
- [X] Binary numbers
- [X] Hex numbers
- [X] Bitwise operators
- [X] && and || operators
- [X] Floating point number support
- [X] Interpret source files
- [ ] Turing completeness proof (rule 110)
- [ ] Garbage collection
- [ ] Concurrent GC
