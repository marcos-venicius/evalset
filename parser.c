#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./parser.h"
#include "./loc.h"
#include "./lexer.h"

void advance_token(Token **ref) {
    if (ref != NULL && *ref != NULL) *ref = (*ref)->next;
}

#define unwrap_ref(ref) *ref;

Token *expect_kind(Token **ref, Token_Kind kind) {
    const char *expected_kind = token_kind_value(kind);

    if (ref == NULL || *ref == NULL) {
        fprintf(stderr, "\033[1;31merror:\033[0m something went wrong. Expected a %s but received \033[1;31mnull\033[0m.\n", expected_kind);
        exit(1);
    } else if ((*ref)->kind != kind) {
        if ((*ref)->kind == TK_EOF) {
            const char *received_kind = token_kind_value((*ref)->kind);

            fprintf(
                stderr,
                LOC_ERROR_FMT" Invalid syntax. Expected a \033[1;35m%s\033[0m but received \033[1;31m%s\033[0m.\n",
                LOC_ERROR_ARG((*ref)->loc),
                expected_kind,
                received_kind
            );
        } else {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Invalid syntax. Expected a \033[1;35m%s\033[0m but received \033[1;31m%.*s\033[0m.\n",
                LOC_ERROR_ARG((*ref)->loc),
                expected_kind,
                (int)(*ref)->content_size,
                (*ref)->content
            );
        }

        exit(1);
    }

    Token *token = *ref;

    (*ref) = token->next;

    return token;
}

Var parse_string_variable(Token *var_lhs, Token **tokens) {
    Token *var_rhs = expect_kind(tokens, TK_STRING);

    Var var = {
        .kind = VK_STRING,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .string = {
            .size = var_rhs->content_size,
            .value = var_rhs->content
        }
    };

    return var;
}

Var parse_integer_variable(Token *var_lhs, Token **tokens) {
    Token *var_rhs = expect_kind(tokens, TK_INTEGER);

    char *const number = calloc(var_rhs->content_size, sizeof(char));

    char *endptr;

    memcpy(number, var_rhs->content, var_rhs->content_size * sizeof(char));

    long integer = strtol(number, &endptr, 10);

    if (*endptr != '\0') {
        fprintf(
            stderr, 
            LOC_ERROR_FMT" Invalid integer \033[1;35m%.*s\033[0m\n",
            LOC_ERROR_ARG(var_rhs->loc),
            (int)var_rhs->content_size,
            var_rhs->content
        );
        exit(1);
    }

    free(number);

    Var var = {
        .kind = VK_INTEGER,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .integer = {
            .value = integer
        }
    };

    return var;
}

Var parse_float_variable(Token *var_lhs, Token **tokens) {
    Token *var_rhs = expect_kind(tokens, TK_FLOAT);

    char *const number = calloc(var_rhs->content_size, sizeof(char));

    char *endptr;

    memcpy(number, var_rhs->content, var_rhs->content_size * sizeof(char));

    double floating = strtod(number, &endptr);

    if (*endptr != '\0') {
        fprintf(
            stderr, 
            LOC_ERROR_FMT" Invalid float \033[1;35m%.*s\033[0m\n",
            LOC_ERROR_ARG(var_rhs->loc),
            (int)var_rhs->content_size,
            var_rhs->content
        );
        exit(1);
    }

    free(number);

    Var var = {
        .kind = VK_FLOAT,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .floating = {
            .value = floating
        }
    };

    return var;
}

Var parse_bool_variable(Token *var_lhs, bool value, Token **tokens) {
    advance_token(tokens);

    Var var = {
        .kind = VK_BOOLEAN,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .boolean = {
            .value = value
        }
    };

    return var;
}

Var parse_nil_variable(Token *var_lhs, Token **ref) {
    advance_token(ref);

    Var var = {
        .kind = VK_NIL,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
    };

    return var;
}

Var parse_array_variable(Token *var_lhs, Token **ref) {
    advance_token(ref);

    Var var = {
        .kind = VK_ARRAY,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .array = {
            .capacity = 0,
            .length = 0,
            .data = NULL
        }
    };

    Token *current = unwrap_ref(ref);

    while (current->kind != TK_EOF && current->kind != TK_RSQUARE) {
        if (current->kind == TK_NEWLINE) {
            current = current->next;
            continue;
        }

        switch (current->kind) {
            case TK_STRING: array_append(&var.array, parse_string_variable(var_lhs, &current)); break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid syntax. Unexpected token \033[1;31m%s\033[0m\n",
                    LOC_ERROR_ARG(current->loc),
                    token_kind_value(current->kind)
                );
                exit(1);
            }
        }

        if (current->kind == TK_COMMA) {
            current = current->next;
        }
    }

    (void)expect_kind(&current, TK_RSQUARE);

    advance_token(ref);

    *ref = current;

    return var;
}

Parser parse_tokens(Token *head) {
    if (head == NULL) return (Parser){0};

    Parser parser = {0};

    Token *current = head;

    while (current->kind != TK_EOF) {
        if (current->kind == TK_NEWLINE) {
            current = current->next;
            continue;
        }

        Token *var_lhs = expect_kind(&current, TK_SYM);
        (void)expect_kind(&current, TK_EQUAL);

        switch (current->kind) {
            // TODO: decouple variable creation of expression parser
            //       so we can reuse the expression parser when parsing arrays and call it
            //       recursively
            case TK_STRING: array_append(&parser, parse_string_variable(var_lhs, &current)); break;
            case TK_INTEGER: array_append(&parser, parse_integer_variable(var_lhs, &current)); break;
            case TK_FLOAT: array_append(&parser, parse_float_variable(var_lhs, &current)); break;
            case TK_TRUE: array_append(&parser, parse_bool_variable(var_lhs, true, &current)); break;
            case TK_FALSE: array_append(&parser, parse_bool_variable(var_lhs, false, &current)); break;
            case TK_NIL: array_append(&parser, parse_nil_variable(var_lhs, &current)); break;
            case TK_LSQUARE: array_append(&parser, parse_array_variable(var_lhs, &current)); break;

            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid syntax. Unexpected token \033[1;31m%s\033[0m\n",
                    LOC_ERROR_ARG(current->loc),
                    token_kind_value(current->kind)
                );
                exit(1);
            }
        }
    }

    return parser;
}

void parser_free(Parser parser) {
    array_free(&parser);
}

const char *var_kind_name(Var_Kind var_kind) {
    switch (var_kind) {
        case VK_STRING: return "STRING";
        case VK_INTEGER: return "INTEGER";
        case VK_FLOAT: return "FLOAT";
        case VK_NIL: return "NIL";
        case VK_BOOLEAN: return "BOOLEAN";
        case VK_OBJECT: return "OBJECT";
        case VK_ARRAY: return "ARRAY";
        default: return "UNKNOWN";
    }
}
