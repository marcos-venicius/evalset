#include "./lexer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./utils.h"

// This do-while(0) is a hack to avoid some issues. https://www.geeksforgeeks.org/multiline-macros-in-c/
// As the article says, we can wrap with parenthesis, but we're using -pedantic and
// "ISO C forbids braced-groups within expressions".
#define throw_error(call, lexer) \
    do { \
        error(); \
        call(lexer); \
    } while (0); \

// I don't think we need to put this inside the lexer
// I'm not sure if would make sense, so, I prefer to create this in a static segment and "private"
// So, we can just return the pointer to it and that's it.
static Token *tokens_head = NULL;
static Token *tokens_tail = NULL;

static unsigned int errors = 0;

static char chr(Lexer *lexer);

void print_tokens(Token *head) {
    return;
    Token *curr = head;

    while (curr != NULL) {
        const char *kind_name = token_kind_name(curr->kind);

        switch (curr->kind) {
            case TK_NEWLINE:
                printf("NOTE %s:%d:%d: \\n (%s)\n", curr->loc.filename, curr->loc.line, curr->loc.col, kind_name);
                break;
            case TK_EOF:
                printf("NOTE %s:%d:%d: <eof> (%s)\n", curr->loc.filename, curr->loc.line, curr->loc.col, kind_name);
                break;
            default:
                printf("NOTE %s:%d:%d: %.*s (%s)\n", curr->loc.filename, curr->loc.line, curr->loc.col, (int)curr->content_size, curr->content, kind_name);
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
        case TK_COMMA:
            return ",";
        case TK_EOF:
            return "<eof>";
        case TK_SYM:
            return "<symbol>";
        case TK_TRUE:
            return "true";
        case TK_FALSE:
            return "false";
        case TK_NIL:
            return "nil";
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
        case TK_MOD:
            return "%";
        case TK_MINUS:
            return "-";
        case TK_SLASH:
            return "/";
        case TK_LPAREN:
            return "(";
        case TK_RPAREN:
            return ")";
        default:
            assert(0 && "invalid token kind");
    }
}

const char *token_kind_name(Token_Kind kind) {
    switch (kind) {
        case TK_EOF:
            return "<eof>";
        case TK_SYM:
            return "<symbol>";
        case TK_TRUE:
            return "<true>";
        case TK_FALSE:
            return "<false>";
        case TK_NIL:
            return "<nil>";
        case TK_EQUAL:
            return "<equal>";
        case TK_SLASH:
            return "<slash>";
        case TK_MOD:
            return "<mod>";
        case TK_MINUS:
            return "<minus>";
        case TK_LBRACE:
            return "<lbrace>";
        case TK_RBRACE:
            return "<rbrace>";
        case TK_LPAREN:
            return "<lparen>";
        case TK_RPAREN:
            return "<rparen>";
        case TK_LSQUARE:
            return "<lsquare>";
        case TK_RSQUARE:
            return "<rsquare>";
        case TK_NEWLINE:
            return "<newline>";
        case TK_COMMENT:
            return "<comment>";
        case TK_STRING:
            return "<string>";
        case TK_INTEGER:
            return "<integer>";
        case TK_FLOAT:
            return "<float>";
        case TK_PATH_ROOT:
            return "<tk_path_root>";
        case TK_PATH_CHUNK:
            return "<tk_path_chunk>";
        case TK_STAR:
            return "<tk_star>";
        case TK_PLUS:
            return "<tk_plus>";
        case TK_COMMA:
            return "<tk_comma>";
        default:
            assert(0 && "invalid token kind");
    }
}

static void error(void) {
    ++errors;

    #if DEBUG
    print_tokens(tokens_head);
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

static void invalid_escape_character_error(Lexer *lexer) {
    fprintf(stderr, "%s:%d:%d: \033[1;31merror\033[0m invalid escape character '\\%c'\n", lexer->loc.filename, lexer->loc.line, lexer->loc.col, lexer->content[lexer->cursor + 1]);
}

// For now, reading the file here is OK.
// But, in the future we'll need to abstract this to the own function, so we can deal with
// multiple imports
Lexer create_lexer(const char *filename, char *data, size_t data_size) {
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
        .content_size = data_size,
        .content = data
    };

    return lexer;
}

// Get the current char without moving the cursor
static char chr(Lexer *lexer) {
    if (lexer->cursor < lexer->content_size) {
        return lexer->content[lexer->cursor];
    }

    return '\0';
}

static char pchr(Lexer *lexer) {
    if (lexer->cursor + 1 < lexer->content_size) {
        return lexer->content[lexer->cursor + 1];
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

static Token_Kind get_kind_keyword(Token *token, Token_Kind fallback) {
    if (cmp_sized_strings(token->content, token->content_size, "true", 4)) return TK_TRUE;

    if (cmp_sized_strings(token->content, token->content_size, "false", 5)) return TK_FALSE;

    if (cmp_sized_strings(token->content, token->content_size, "nil", 3)) return TK_NIL;

    return fallback;
}

static void save_token(Lexer *lexer, Token_Kind kind) {
    Token *token = calloc(1, sizeof(Token));
   
    token->loc.filename = lexer->loc.filename;
    token->loc.col = lexer->bcol;
    token->loc.line = lexer->bline;

    token->content = lexer->content + lexer->bot;
    token->content_size = lexer->cursor - lexer->bot;

    token->next = NULL;

    if (kind == TK_SYM) {
        token->kind = get_kind_keyword(token, TK_SYM);
    } else {
        token->kind = kind;
    }

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

    if (path_size == 0) {
        // throw_error(invalid_path_chunk_error, lexer);
        save_token(lexer, TK_SLASH);
    } else {
        save_token(lexer, TK_PATH_CHUNK);
    }
}

static void lex_comment(Lexer *lexer) {
    while (chr(lexer) != '\n') nchr(lexer);

    nchr(lexer);
    // maybe I'll save it to implement some kind of "prettier" later
    // save_token(lexer, TK_COMMENT);
}

// TODO: basic string escape (\r \n \t \b \f .....)
static void lex_string(Lexer *lexer) {
    nchr(lexer);

    bool lexing = true;

    while (chr(lexer) != '"' && lexing) {
        if (chr(lexer) == '\\') {
            switch (pchr(lexer)) {
                case '"':
                case '\\':
                case 'n':
                case 't':
                case 'b':
                case 'r':
                case 'f':
                    nchr(lexer);
                    break;
                default:
                    lexing = false;
                    throw_error(invalid_escape_character_error, lexer);
                    break;
            }
        } else if (chr(lexer) == '\n') {
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

    if (digits_count == 0) {
        // throw_error(invalid_number_error, lexer);
        save_token(lexer, TK_MINUS);
    } else {
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
    if (lexer->content == NULL) return NULL;

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
            case '%': lex_char(lexer, TK_MOD); break;
            case ',': lex_char(lexer, TK_COMMA); break;
            case '/': lex_path_chunk(lexer); break;
            case '{': lex_char(lexer, TK_LBRACE); break;
            case '}': lex_char(lexer, TK_RBRACE); break;
            case '[': lex_char(lexer, TK_LSQUARE); break;
            case ']': lex_char(lexer, TK_RSQUARE); break;
            case '(': lex_char(lexer, TK_LPAREN); break;
            case ')': lex_char(lexer, TK_RPAREN); break;
            case '#': lex_comment(lexer); break;
            case '"': lex_string(lexer); break;
            case '\n': lex_char(lexer, TK_NEWLINE); break;
            case '\0': {
                lex_eof(lexer);
                lexing = false;
            } break;
            default: {
                if (is_symbol(chr(lexer))) {
                    lex_symbol(lexer);
                } else if (is_digit(chr(lexer))) {
                    lex_number(lexer);
                } else {
                    throw_error(unrecognized_char_error, lexer);
                }
            } break;
        }
    }

    if (errors == 0) return tokens_head;

    return NULL;
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
