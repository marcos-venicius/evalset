#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    TK_SYM = 0,
    TK_EQUAL,
    TK_LBRACE,
    TK_NEWLINE,
    TK_COMMENT,
    TK_STRING
} Token_Kind;

typedef struct Token Token;

struct Token {
    char *content;
    size_t content_size;

    Token_Kind kind;

    unsigned int line, col;

    const char *filename;

    Token *next;
};

typedef struct {
    unsigned long content_size;

    unsigned int cursor, bot, line, col, bline, bcol;

    char *content;
    const char *filename;
} Lexer;

char chr(Lexer *lexer);
void lexer_free(Lexer *lexer);
void print_tokens();

static Token *tokens_head = NULL;
static Token *tokens_tail = NULL;

static const char *token_kind_name(Token_Kind kind) {
    switch (kind) {
        case TK_SYM:
            return "SYM";
        case TK_EQUAL:
            return "EQUAL";
        case TK_LBRACE:
            return "LBRACE";
        case TK_NEWLINE:
            return "NEWLINE";
        case TK_COMMENT:
            return "COMMENT";
        case TK_STRING:
            return "STRING";
        default:
            assert(0 && "invalid token kind");
    }
}

static void unrecognized_char_error(Lexer *lexer) {
    print_tokens();

    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m unrecognized character '%c'\n", lexer->filename, lexer->line, lexer->col, chr(lexer));
    lexer_free(lexer);
    exit(1);
}

static void unterminated_string_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m unterminated string\n", lexer->filename, lexer->bline, lexer->bcol);
    lexer_free(lexer);
    exit(1);
}

Lexer create_lexer(const char *filename) {
    FILE *fptr = fopen(filename, "r");

    if (fptr == NULL) {
        fprintf(stderr, "could not open file %s due to: %s\n", filename, strerror(errno));
        exit(1);
    }

    fseek(fptr, 0, SEEK_END);
    const size_t stream_size = ftell(fptr);
    rewind(fptr);
    
    Lexer lexer = (Lexer){
        .filename = filename,
        .col = 1,
        .line = 1,
        .bline = 1,
        .bcol = 1,
        .cursor = 0,
        .bot = 0,
        .content_size = stream_size
    };

    lexer.content = malloc((stream_size + 1) * sizeof(char));

    const size_t read_size = fread(lexer.content, 1, stream_size, fptr);

    if (read_size != stream_size) {
        fprintf(stderr, "could not read file %s due to: %s\n", filename, strerror(errno));
        fclose(fptr);
        exit(1);
    }

    lexer.content[stream_size] = '\0';

    fclose(fptr);

    return lexer;
}

// Get the current char without moving the cursor
char chr(Lexer *lexer) {
    if (lexer->cursor < lexer->content_size) {
        return lexer->content[lexer->cursor];
    }

    return '\0';
}

// Advances the cursor to the next char
char nchr(Lexer *lexer) {
    if (lexer->cursor + 1 < lexer->content_size) {
        char c = chr(lexer);

        ++lexer->cursor;

        if (c == '\n') {
            lexer->col = 1;
            ++lexer->line;
        } else {
            ++lexer->col;
        }

        return chr(lexer);
    }

    return '\0';
}

// Lookahead of 1 char
char pchr(Lexer *lexer) {
    if (lexer->cursor + 1 < lexer->content_size) {
        return lexer->content[lexer->cursor + 1];
    }

    return '\0';
}

void save_token(Lexer *lexer, Token_Kind kind) {
    Token *token = calloc(1, sizeof(Token));
   
    token->col = lexer->bcol;
    token->line = lexer->bline;
    token->content = lexer->content + lexer->bot;
    token->content_size = lexer->cursor - lexer->bot;
    token->kind = kind;
    token->next = NULL;
    token->filename = lexer->filename;

    if (tokens_head == NULL) {
        tokens_head = token;
        tokens_tail = token;
    } else {
        tokens_tail->next = token;
        tokens_tail = tokens_tail->next;
    }
}

bool is_name(char c) {
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

void trim_whitespaces(Lexer *lexer) {
    while (is_whitespace(chr(lexer))) nchr(lexer);
}

void lex_name(Lexer *lexer) {
    while (is_name(chr(lexer))) nchr(lexer);

    save_token(lexer, TK_SYM);
}

void lex_comment(Lexer *lexer) {
    while (chr(lexer) != '\n') nchr(lexer);

    save_token(lexer, TK_COMMENT);
}

// TODO: basic string escape (\r \n \t \b \f .....)
void lex_string(Lexer *lexer) {
    nchr(lexer);

    while (chr(lexer) != '"') {
        if (chr(lexer) == '\n') {
            unterminated_string_error(lexer);
            break;
        }

        nchr(lexer);
    }

    nchr(lexer);

    save_token(lexer, TK_STRING);
}

void lex_char(Lexer *lexer, Token_Kind kind) {
    nchr(lexer);
    save_token(lexer, kind);
}

void lex(Lexer *lexer) {
    while (lexer->cursor < lexer->content_size) {
        trim_whitespaces(lexer);

        lexer->bot = lexer->cursor;
        lexer->bline = lexer->line;
        lexer->bcol = lexer->col;

        switch (chr(lexer)) {
            case 'a'...'z':
            case 'A'...'Z': lex_name(lexer); break; 
            case '=': lex_char(lexer, TK_EQUAL); break;
            case '{': lex_char(lexer, TK_LBRACE); break;
            case '#': lex_comment(lexer); break;
            case '"': lex_string(lexer); break;
            case '\n': lex_char(lexer, TK_NEWLINE); break;
            default: unrecognized_char_error(lexer); break;
        }
    }
}

void print_tokens() {
    Token *curr = tokens_head;

    while (curr != NULL) {
        if (curr->kind == TK_NEWLINE){
            printf("NOTE %s:%d:%d: \\n (%s)\n", curr->filename, curr->line, curr->col, token_kind_name(curr->kind));
        } else {
            printf("NOTE %s:%d:%d: %.*s (%s)\n", curr->filename, curr->line, curr->col, (int)curr->content_size, curr->content, token_kind_name(curr->kind));
        }

        curr = curr->next;
    }

    printf("\n");
}

void lexer_free(Lexer *lexer) {
    free(lexer->content);

    Token *curr = tokens_head;

    while (curr != NULL) {
        Token *next = curr->next;

        free(curr);

        curr = next;
    }
}

int main() {
    Lexer lexer = create_lexer("./examples/settings.es");

    lex(&lexer);

    print_tokens();
    lexer_free(&lexer);

    return 0;
}
