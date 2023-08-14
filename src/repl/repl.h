typedef enum {
    MODE_INTERPRET,
    MODE_COMPILE,
    MODE_LOAD_BINARY,
    MODE_DISASSEMBLE,
} ReplMode;

void start_repl(ReplMode);
