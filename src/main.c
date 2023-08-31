#include "file_reader/file_reader.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "repl/repl.h"
#include "vm/file_loader.h"
#include <stdio.h>
#include <string.h>
#include "disassembler/disassembler.h"

void usage() {
  printf("Usage: monkey [options] [input-file] [output-file]\n");
  printf("Options:\n");
  printf("  -i\t\t\tStarts the REPL in interpret mode\n");
  printf("  -c\t\t\tCompiles [input-file] and writes binary to [output-file]\n");
  printf("  -d\t\t\tDisassembles [input-file]\n");
  printf("  -h\t\t\tPrints this help message\n");
}

ReplMode get_repl_mode(char *flag) {
  if (strncmp(flag, "-i", 2) == 0) {
    return MODE_INTERPRET;
  }

  if (strncmp(flag, "-c", 2) == 0) {
    return MODE_COMPILE;
  }

  if (strncmp(flag, "-l", 2) == 0) {
    return MODE_LOAD_BINARY;
  }

  if (strncmp(flag, "-d", 2) == 0) {
    return MODE_DISASSEMBLE;
  }

  return MODE_INTERPRET;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    start_repl(MODE_INTERPRET);
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
    switch (get_repl_mode(argv[1])) {
    case MODE_INTERPRET:
      eval_file(argv[2]);
      break;
    case MODE_LOAD_BINARY:
      load_file(argv[2]);
      break;
    case MODE_DISASSEMBLE:
      disassemble_file(argv[2]);
      break;
    case MODE_COMPILE:
      usage();
      break;
    }
    return 0;
  }

  if (argc == 4) {
    ReplMode mode = get_repl_mode(argv[1]);
    if (mode == MODE_INTERPRET) {
      eval_file(argv[2]);
    } else {
      char *in = argv[2];
      char *out = argv[3];
      compile_file(in, out);
    }
    return 0;
  }

  usage();
  return 0;
}
