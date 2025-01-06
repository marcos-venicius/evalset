#ifndef LEXER_H_
#define LEXER_H_

typedef enum {
    TK_EOF = 0,
    TK_SYM,
    TK_EQUAL,
    TK_LBRACE,
    TK_RBRACE,
    TK_LSQUARE,
    TK_RSQUARE,
    TK_NEWLINE,
    TK_COMMENT,
    TK_STAR,
    TK_PLUS,

    TK_STRING,
    TK_INTEGER,
    TK_FLOAT,

    TK_PATH_ROOT,
    TK_PATH_CHUNK
} Token_Kind;

typedef struct {
    unsigned int line, col;
    const char *filename;
} Location;

typedef struct Token Token;

struct Token {
    char *content;
    unsigned long content_size;

    Token_Kind kind;

    Location loc;

    Token *next;
};

typedef struct {
    unsigned long content_size;

    unsigned int cursor, bot, bline, bcol;

    char *content;

    Location loc;
} Lexer;

void print_tokens();
const char *token_kind_name(Token_Kind kind);
const char *token_kind_value(Token_Kind kind);

Lexer create_lexer(const char *filename);
Token *lex(Lexer *lexer);
void lexer_free(Lexer *lexer);

#endif // LEXER_H_
