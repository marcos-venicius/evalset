#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./parser.h"
#include "./loc.h"
#include "./lexer.h"

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

Parser parse_tokens(Token *head) {
    if (head == NULL) return (Parser){0};

    Parser parser = {0};

    Token *current = head;

    while (current->kind != TK_EOF) {
        Token *var_lhs = expect_kind(&current, TK_SYM);
        (void)expect_kind(&current, TK_EQUAL);
        Token *var_rhs = expect_kind(&current, TK_STRING);

        Var var1 = {
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

        array_append(&parser, var1);

        (void)expect_kind(&current, TK_NEWLINE);
    }

    return parser;
}

void parser_free(Parser parser) {
    array_free(&parser);
}

