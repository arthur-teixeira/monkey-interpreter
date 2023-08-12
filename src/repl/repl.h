typedef enum {
    MODE_INTERPRET,
    MODE_COMPILE,
    MODE_LOAD_BINARY,
} ReplMode;

void start_repl(ReplMode);
