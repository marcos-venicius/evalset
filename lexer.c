#include "./lexer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// This do-while(0) is a hack to avoid some issues. https://www.geeksforgeeks.org/multiline-macros-in-c/
// As the article says, we can wrap with parenthesis, but we're using -pedantic and
// "ISO C forbids braced-groups within expressions".
#define throw_error(call, lexer) \
    do { \
        error(); \
        call(lexer); \
        lexer_free(lexer); \
        exit(1); \
    } while(0); \

// I don't think we need to put this inside the lexer
// I'm not sure if would make sense, so, I prefer to create this in a static segment and "private"
// So, we can just return the pointer to it and that's it.
static Token *tokens_head = NULL;
static Token *tokens_tail = NULL;

static char chr(Lexer *lexer);

void print_tokens() {
    Token *curr = tokens_head;

    while (curr != NULL) {
        switch (curr->kind) {
            case TK_NEWLINE:
                printf("NOTE %s:%d:%d: \\n (%s)\n", curr->loc.filename, curr->loc.line, curr->loc.col, token_kind_name(curr->kind));
                break;
            case TK_EOF:
                printf("NOTE %s:%d:%d: <eof> (%s)\n", curr->loc.filename, curr->loc.line, curr->loc.col, token_kind_name(curr->kind));
                break;
            default:
                printf("NOTE %s:%d:%d: %.*s (%s)\n", curr->loc.filename, curr->loc.line, curr->loc.col, (int)curr->content_size, curr->content, token_kind_name(curr->kind));
                break;
        }

        curr = curr->next;
    }
}

// Later, We plan to have some intelligent error reporting at parsing and evaluating level
// So, if we want to say that is missing a RBRACE to close the context, we just use the token kind
// So, if we change the "rendering style" we don't have to modify in many places but only here.
const char *token_kind_value(Token_Kind kind) {
    switch (kind) {
        case TK_EOF:
            return "<eof>";
        case TK_SYM:
            return "<symbol>";
        case TK_EQUAL:
            return "=";
        case TK_LBRACE:
            return "{";
        case TK_RBRACE:
            return "}";
        case TK_LSQUARE:
            return "[";
        case TK_RSQUARE:
            return "]";
        case TK_NEWLINE:
            return "<newline>";
        case TK_COMMENT:
            return "#";
        case TK_STRING:
            return "\"";
        case TK_INTEGER:
            return "<integer>";
        case TK_FLOAT:
            return "<float>";
        case TK_PATH_ROOT:
            return "$";
        case TK_PATH_CHUNK:
            return "/...";
        case TK_STAR:
            return "*";
        case TK_PLUS:
            return "+";
        default:
            assert(0 && "invalid token kind");
    }
}

const char *token_kind_name(Token_Kind kind) {
    switch (kind) {
        case TK_EOF:
            return "EOF";
        case TK_SYM:
            return "SYM";
        case TK_EQUAL:
            return "EQUAL";
        case TK_LBRACE:
            return "LBRACE";
        case TK_RBRACE:
            return "RBRACE";
        case TK_LSQUARE:
            return "LSQUARE";
        case TK_RSQUARE:
            return "RSQUARE";
        case TK_NEWLINE:
            return "NEWLINE";
        case TK_COMMENT:
            return "COMMENT";
        case TK_STRING:
            return "STRING";
        case TK_INTEGER:
            return "INTEGER";
        case TK_FLOAT:
            return "FLOAT";
        case TK_PATH_ROOT:
            return "TK_PATH_ROOT";
        case TK_PATH_CHUNK:
            return "TK_PATH_CHUNK";
        case TK_STAR:
            return "TK_STAR";
        case TK_PLUS:
            return "TK_PLUS";
        default:
            assert(0 && "invalid token kind");
    }
}

static void error() {
    #if DEBUG
    print_tokens();
    #endif
}

static void unrecognized_char_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m unrecognized character '%c'\n", lexer->loc.filename, lexer->loc.line, lexer->loc.col, chr(lexer));
}

static void unexpected_char_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m unexpected character '%c'\n", lexer->loc.filename, lexer->loc.line, lexer->loc.col, chr(lexer));
}

static void invalid_number_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m invalid number '%.*s'\n", lexer->loc.filename, lexer->loc.line, lexer->loc.col, lexer->cursor - lexer->bot, lexer->content + lexer->bot);
}

static void invalid_path_chunk_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m invalid path chunk '%.*s'\n", lexer->loc.filename, lexer->loc.line, lexer->loc.col, lexer->cursor - lexer->bot, lexer->content + lexer->bot);
}

static void unterminated_string_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m unterminated string\n", lexer->loc.filename, lexer->bline, lexer->bcol);
}

// For now, reading the file here is OK.
// But, in the future we'll need to abstract this to the own function, so we can deal with
// multiple imports
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
        .loc = (Location){
            .filename = filename,
            .col = 1,
            .line = 1,
        },
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
static char chr(Lexer *lexer) {
    if (lexer->cursor < lexer->content_size) {
        return lexer->content[lexer->cursor];
    }

    return '\0';
}

// Advance the cursor to the next char
static char nchr(Lexer *lexer) {
    if (lexer->cursor < lexer->content_size) {
        char c = chr(lexer);

        ++lexer->cursor;

        if (c == '\n') {
            lexer->loc.col = 1;
            ++lexer->loc.line;
        } else {
            ++lexer->loc.col;
        }

        return chr(lexer);
    } else if (lexer->cursor < lexer->content_size) {
        ++lexer->cursor;
    }

    return '\0';
}

static void save_token(Lexer *lexer, Token_Kind kind) {
    Token *token = calloc(1, sizeof(Token));
   
    token->loc.filename = lexer->loc.filename;
    token->loc.col = lexer->bcol;
    token->loc.line = lexer->bline;

    token->content = lexer->content + lexer->bot;
    token->content_size = lexer->cursor - lexer->bot;

    token->kind = kind;
    token->next = NULL;

    if (tokens_head == NULL) {
        tokens_head = token;
        tokens_tail = token;
    } else {
        tokens_tail->next = token;
        tokens_tail = tokens_tail->next;
    }
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_symbol(char c) {
    return c == '_' || is_alpha(c);
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

// lexing

static void lex_symbol(Lexer *lexer) {
    while (is_symbol(chr(lexer))) nchr(lexer);

    if (is_digit(chr(lexer))) throw_error(unexpected_char_error, lexer);

    save_token(lexer, TK_SYM);
}

// Path chunk format: ^\/(([a-z]*)([A-Z]*)(_*))*
static void lex_path_chunk(Lexer *lexer) {
    if (chr(lexer) != '/') throw_error(invalid_path_chunk_error, lexer);

    nchr(lexer);

    int path_size = 0;

    while (is_symbol(chr(lexer))) {
        ++path_size;
        nchr(lexer);
    }

    if (path_size == 0) throw_error(invalid_path_chunk_error, lexer);

    save_token(lexer, TK_PATH_CHUNK);
}

static void lex_comment(Lexer *lexer) {
    while (chr(lexer) != '\n') nchr(lexer);

    save_token(lexer, TK_COMMENT);
}

// TODO: basic string escape (\r \n \t \b \f .....)
static void lex_string(Lexer *lexer) {
    nchr(lexer);

    while (chr(lexer) != '"') {
        if (chr(lexer) == '\n') {
            throw_error(unterminated_string_error, lexer);
            break;
        }

        nchr(lexer);
    }

    nchr(lexer);

    save_token(lexer, TK_STRING);
}

static void lex_number(Lexer *lexer) {
    int digits_count = 0;

    if (chr(lexer) == '-') {
        nchr(lexer);
    }

    while (is_digit(chr(lexer))) {
        nchr(lexer);
        ++digits_count;
    }

    if (digits_count == 0) throw_error(invalid_number_error, lexer);

    digits_count = 0;
    bool is_floating = false;

    if (chr(lexer) == '.') {
        is_floating = true;

        nchr(lexer);

        while (is_digit(chr(lexer))) {
            nchr(lexer);
            ++digits_count;
        }

        if (digits_count == 0) throw_error(invalid_number_error, lexer);
    }

    if (is_symbol(chr(lexer))) throw_error(unexpected_char_error, lexer);

    save_token(lexer, is_floating ? TK_FLOAT : TK_INTEGER);
}

static void lex_char(Lexer *lexer, Token_Kind kind) {
    nchr(lexer);
    save_token(lexer, kind);
}

static void lex_eof(Lexer *lexer) {
    save_token(lexer, TK_EOF);
    nchr(lexer);
}

Token *lex(Lexer *lexer) {
    bool lexing = true;

    while (lexing) {
        // trim whitespaces
        while (is_whitespace(chr(lexer))) nchr(lexer);

        lexer->bot = lexer->cursor;
        lexer->bline = lexer->loc.line;
        lexer->bcol = lexer->loc.col;

        switch (chr(lexer)) {
            case '-': lex_number(lexer); break;
            case '=': lex_char(lexer, TK_EQUAL); break;
            case '*': lex_char(lexer, TK_STAR); break;
            case '+': lex_char(lexer, TK_PLUS); break;
            case '$': lex_char(lexer, TK_PATH_ROOT); break;
            case '/': lex_path_chunk(lexer); break;
            case '{': lex_char(lexer, TK_LBRACE); break;
            case '}': lex_char(lexer, TK_RBRACE); break;
            case '[': lex_char(lexer, TK_LSQUARE); break;
            case ']': lex_char(lexer, TK_RSQUARE); break;
            case '#': lex_comment(lexer); break;
            case '"': lex_string(lexer); break;
            case '\n': lex_char(lexer, TK_NEWLINE); break;
            case '\0': {
                lex_eof(lexer);
                lexing = false;
            } break;
            default: {
                if (is_alpha(chr(lexer))) {
                    lex_symbol(lexer);
                } else if (is_digit(chr(lexer))) {
                    lex_number(lexer);
                } else {
                    throw_error(unrecognized_char_error, lexer);
                }
            } break;
        }
    }

    return tokens_head;
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
