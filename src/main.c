#include "./file_reader/file_reader.h"
#include "./repl/repl.h"
#include <stdio.h>
#include <string.h>

ReplMode get_repl_mode(char *flag) {
  if (strncmp(flag, "-i", 2) == 0) {
    return REPL_INTERPRET;
  }

  if (strncmp(flag, "-c", 2) == 0) {
    return REPL_COMPILE;
  }

  return REPL_INTERPRET;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    start_repl(REPL_INTERPRET);
  } else if (argv[1][0] == '-') {
    ReplMode mode = get_repl_mode(argv[1]);
    start_repl(mode);
  } else {
    //TODO: add support for compiling files
    eval_file(argv[1]);
  }

  return 0;
}
