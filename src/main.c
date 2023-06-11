#include "./repl/repl.h"
#include "./file_reader/file_reader.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("Hello from Monkey!\n");
    start_repl();
  } else {
    eval_file(argv[1]);
  }

  return 0;
}
