#define MAX_LEN 100

typedef char TokenType[MAX_LEN];

typedef struct {
    TokenType Type;
    const char literal[MAX_LEN];
} Token;

extern const char ILLEGAL;
extern const char END_OF_FILE;
extern const char IDENT;
extern const char INT;
extern const char ASSIGN;
extern const char PLUS;
extern const char COMMA;
extern const char SEMICOLON;
extern const char LPAREN;
extern const char RPAREN;
extern const char LBRACE;
extern const char RBRACE;
extern const char FUNCTION;
extern const char LET;
