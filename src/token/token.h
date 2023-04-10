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
  LET,
  BANG,
  LT,
  GT,
  MINUS,
  SLASH,
  ASTERISK,
} TokenType;

typedef struct {
  TokenType Type;
  char literal[MAX_LEN];
} Token;

TokenType lookup_ident(char *ident);
