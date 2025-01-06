#ifndef LEXER_H_
#define LEXER_H_

typedef enum {
    TK_EOF = 0,

    TK_SYM,

    // KEYWORDS
    TK_NIL,
    TK_TRUE,
    TK_FALSE,

    // Braces
    TK_LBRACE,
    TK_RBRACE,
    TK_LSQUARE,
    TK_RSQUARE,

    // Line terminator (the same behavior as ; in C)
    // This languages is simillar to python in the way we handle the end of line.
    // Some specific "expressions" need to have a new line at the end of the expression to close it.
    // It's not valid for everything, for example: you can have "root = {}", with no problem,
    // but for the properties inside, it's required.
    TK_NEWLINE,

    TK_COMMENT,

    // Signs
    TK_EQUAL,
    TK_STAR,
    TK_PLUS,

    // Data types
    TK_STRING,
    TK_INTEGER,
    TK_FLOAT,

    // Path
    TK_PATH_ROOT,
    TK_PATH_CHUNK
} Token_Kind;

typedef struct {
    unsigned int line, col;
    const char *filename;
} Location;

typedef struct Token Token;

struct Token {
    // The content of the token is not a null-terminated string, it's a pointer to where it starts
    // and the size of it, so, we can grab later the real value without wasting memory to every new
    // token we found. We just use the same file data to all of the tokens without allocating memory.
    char *content;
    unsigned long content_size;

    Token_Kind kind;

    // It's important to have the `filename` in the token struct because, maybe later, we implement
    // some kind of "import" stuff, then we need to know in which file we encountered an error, warning, etc.
    Location loc;

    Token *next;
};

typedef struct {
    // This is the reading cursor. It moves over the file during the lexing.
    unsigned int cursor;
    // This is where a token starts (Beginning of the token). So, everytime we check for
    // some sort of token, we update it to the current cursor value, to indicate that this
    // is the beginning of the token we are capturing right now.
    unsigned int bot;
    // This is the line where the token begins. So, if, for some reason, the token need
    // multiple lines, we keep track in which line it begins.
    unsigned int bline;
    // As in the `bline`, this keep track of the color in which the tokens started to be captured
    // So, if it's a symbol like "root", it'll have the column of the first symbol character 
    unsigned int bcol;

    char *content;
    unsigned long content_size;

    // Here, we keep track of the current line and column in which the cursor are
    // So, if we need to show an erro in the current cursor position we know the exact position in the file
    Location loc;
} Lexer;

void print_tokens();
const char *token_kind_name(Token_Kind kind);
const char *token_kind_value(Token_Kind kind);

Lexer create_lexer(const char *filename);
Token *lex(Lexer *lexer);
void lexer_free(Lexer *lexer);

#endif // LEXER_H_
