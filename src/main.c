#include "./file_reader/file_reader.h"
#include "./repl/repl.h"
#include <stdio.h>
#include <string.h>

void usage() {
  printf("Usage: monkey [options] [file]\n");
  printf("Options:\n");
  printf("  -i\t\t\tStarts the REPL in interpret mode\n");
  printf("  -c\t\t\tStarts the REPL in compile mode\n");
  printf("  -h\t\t\tPrints this help message\n");
}

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
    return 0;
  } 

  if (argc == 2 && strncmp(argv[1], "-h", 2) == 0) {
    usage();
    return 0;
  }

  if (argc == 2 && strncmp(argv[1], "-", 1) == 0) {
    ReplMode mode = get_repl_mode(argv[1]);
    start_repl(mode);
    return 0;
  }

  if (argc == 3 && strncmp(argv[1], "-", 1) == 0) {
    eval_file(argv[2]); // TODO: compile file if -c flag is passed
    return 0;
  }

  usage();
  return 0;
}
