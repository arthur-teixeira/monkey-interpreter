#define MAX_LEN 100

typedef enum {
    ILLEGAL,
    END_OF_FILE,
    IDENT,
    INT,
    ASSIGN,
    PLUS,
    COMMA,
    SEMICOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    FUNCTION,
    LET
} TokenType;

typedef struct {
    TokenType Type;
    char literal[MAX_LEN];
} Token;

TokenType lookup_ident(char *ident);
