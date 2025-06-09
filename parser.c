#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./parser.h"
#include "./loc.h"
#include "./lexer.h"

Var parse_object_variable(Token *var_lhs, Token **ref);

void advance_token(Token **ref) {
    if (ref != NULL && *ref != NULL) *ref = (*ref)->next;
}

#define unwrap_ref(ref) *ref;

// TODO: do not advance the token. Let this job the an outside called
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

    *ref = token->next;

    return token;
}

Token *expect_two_kinds(Token **ref, Token_Kind a, Token_Kind b) {
    const char *expected_kind_a = token_kind_name(a);
    const char *expected_kind_b = token_kind_name(b);
    const char *received_kind = token_kind_value((*ref)->kind);
    const char *received_name = token_kind_name((*ref)->kind);

    if (ref == NULL || *ref == NULL) {
        fprintf(
            stderr,
            "\033[1;31merror:\033[0m something went wrong. Expected a \033[1;35m%s\033[0m or \033[1;35m%s\033[0m but received \033[1;31mnull\033[0m which is \033[1;31m%s\033[0m.\n",
            expected_kind_a,
            expected_kind_b,
            received_name
        );
        exit(1);
    } else if ((*ref)->kind != a && (*ref)->kind != b) {
        if ((*ref)->kind == TK_EOF) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Invalid syntax. Expected a \033[1;35m%s\033[0m or \033[1;35m%s\033[0m but received \033[1;31m%s\033[0m which is \033[1;31m%s\033[0m.\n",
                LOC_ERROR_ARG((*ref)->loc),
                expected_kind_a,
                expected_kind_b,
                received_kind,
                received_name
            );
        } else {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Invalid syntax. Expected a \033[1;35m%s\033[0m or \033[1;35m%s\033[0m but received \033[1;31m%.*s\033[0m which is \033[1;35m%s\033[0m.\n",
                LOC_ERROR_ARG((*ref)->loc),
                expected_kind_a,
                expected_kind_b,
                (int)(*ref)->content_size,
                (*ref)->content,
                received_name
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

Data_Type parse_integer_variable(Token **tokens) {
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

    return (Data_Type){
        .integer = {
            .value = integer
        }
    };
}

static Var create_variable_from_kind(Var_Kind kind, Token* var_lhs, Data_Type data) {
    switch (kind) {
        case VK_STRING: {
            return (Var){
                .kind = VK_STRING,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .string = data.string
            };
        } break;
        case VK_INTEGER: {
            return (Var){
                .kind = VK_INTEGER,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .integer = data.integer
            };
        } break;
        case VK_FLOAT: {
            return (Var){
                .kind = VK_FLOAT,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .floating = data.floating
            };
        } break;
        case VK_NIL: {
            return (Var){
                .kind = VK_NIL,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
            };
        } break;
        case VK_BOOLEAN: {
            return (Var){
                .kind = VK_BOOLEAN,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .boolean = data.boolean
            };
        } break;
        case VK_ARRAY: {
            return (Var){
                .kind = VK_ARRAY,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .array = data.array
            };
        } break;
        case VK_OBJECT: {
            return (Var){
                .kind = VK_OBJECT,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .object = data.object
            };
        } break;
        case VK_PATH: {
            return (Var){
                .kind = VK_PATH,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .path = data.path
            };
        } break;
        case VK_FUN_CALL: {
            return (Var){
                .kind = VK_FUN_CALL,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .fun_call = data.fun_call
            };
        } break;
        default: assert(0 && "update variable creation function");
    }
}

static Argument create_argument_from_kind(Var_Kind kind, Data_Type data) {
    switch (kind) {
        case VK_INTEGER: {
            return (Argument){
                .kind = VK_INTEGER,
                .integer = data.integer
            };
        } break;
        /* case VK_STRING: {
            return (Var){
                .kind = VK_STRING,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .string = data.string
            };
        } break;
        case VK_FLOAT: {
            return (Var){
                .kind = VK_FLOAT,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .floating = data.floating
            };
        } break;
        case VK_NIL: {
            return (Var){
                .kind = VK_NIL,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
            };
        } break;
        case VK_BOOLEAN: {
            return (Var){
                .kind = VK_BOOLEAN,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .boolean = data.boolean
            };
        } break;
        case VK_ARRAY: {
            return (Var){
                .kind = VK_ARRAY,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .array = data.array
            };
        } break;
        case VK_OBJECT: {
            return (Var){
                .kind = VK_OBJECT,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .object = data.object
            };
        } break;
        case VK_PATH: {
            return (Var){
                .kind = VK_PATH,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .path = data.path
            };
        } break;
        case VK_FUN_CALL: {
            return (Var){
                .kind = VK_FUN_CALL,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .fun_call = data.fun_call
            };
        } break; */
        default: assert(0 && "update variable creation function");
    }
}

/* Var parse_integer_variable(Token *var_lhs, Token **tokens) {
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
} */

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

Var parse_path_variable(Token *var_lhs, Token **ref) {
    advance_token(ref);

    Var var = {
        .kind = VK_PATH,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .path = {
            .capacity = 0,
            .length = 0,
            .data = NULL
        }
    };

    Token *current = unwrap_ref(ref);

    int chunks = 0;

    while (current->kind == TK_PATH_CHUNK) {
        chunks++;

        char *chunk = malloc(current->content_size);

        memcpy(chunk, current->content, current->content_size);

        array_append(&var.path, chunk);

        current = current->next;
    }

    if (chunks == 0) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Invalid syntax. Invalid path\n",
            LOC_ERROR_ARG(current->loc)
        );
        exit(1);
    }

    *ref = current;

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
            case TK_INTEGER: array_append(&var.array, create_variable_from_kind(VK_INTEGER, var_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&var.array, parse_float_variable(var_lhs, &current)); break;
            case TK_TRUE: array_append(&var.array, parse_bool_variable(var_lhs, true, &current)); break;
            case TK_FALSE: array_append(&var.array, parse_bool_variable(var_lhs, false, &current)); break;
            case TK_NIL: array_append(&var.array, parse_nil_variable(var_lhs, &current)); break;
            case TK_LSQUARE: array_append(&var.array, parse_array_variable(var_lhs, &current)); break;
            case TK_LBRACE: array_append(&var.array, parse_object_variable(var_lhs, &current)); break;
            case TK_PATH_ROOT: array_append(&var.array, parse_path_variable(var_lhs, &current)); break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid syntax. Unexpected token \033[1;31m%.*s\033[0m which is \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(current->loc),
                    (int)current->content_size,
                    current->content,
                    token_kind_name(current->kind)
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

Var parse_object_variable(Token *var_lhs, Token **ref) {
    advance_token(ref);

    Var var = {
        .kind = VK_OBJECT,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .object = {
            .capacity = 0,
            .length = 0,
            .data = NULL
        }
    };

    Token *current = unwrap_ref(ref);

    while (current->kind != TK_EOF && current->kind != TK_RBRACE) {
        if (current->kind == TK_NEWLINE) {
            current = current->next;
            continue;
        }

        Token *key_lhs = expect_two_kinds(&current, TK_SYM, TK_STRING);
        (void)expect_kind(&current, TK_EQUAL);

        switch (current->kind) {
            case TK_STRING: array_append(&var.object, parse_string_variable(key_lhs, &current)); break;
            case TK_INTEGER: array_append(&var.object, create_variable_from_kind(VK_INTEGER, key_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&var.object, parse_float_variable(key_lhs, &current)); break;
            case TK_TRUE: array_append(&var.object, parse_bool_variable(key_lhs, true, &current)); break;
            case TK_FALSE: array_append(&var.object, parse_bool_variable(key_lhs, false, &current)); break;
            case TK_NIL: array_append(&var.object, parse_nil_variable(key_lhs, &current)); break;
            case TK_LSQUARE: array_append(&var.object, parse_array_variable(key_lhs, &current)); break;
            case TK_LBRACE: array_append(&var.object, parse_object_variable(key_lhs, &current)); break;
            case TK_PATH_ROOT: array_append(&var.object, parse_path_variable(key_lhs, &current)); break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid syntax. Unexpected token \033[1;31m%.*s\033[0m which is \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(current->loc),
                    (int)current->content_size,
                    current->content,
                    token_kind_name(current->kind)
                );
                exit(1);
            }
        }

        if (current->kind == TK_COMMA) {
            current = current->next;
        }
    }

    (void)expect_kind(&current, TK_RBRACE);

    advance_token(ref);

    *ref = current;

    return var;
}

Var parse_fun_call_variable(Token *var_lhs, Token **ref) {
    Token *fun_name = *ref;

    advance_token(ref); // consume the function call name

    (void)expect_kind(ref, TK_LPAREN); // consume '('

    Var var = {
        .kind = VK_FUN_CALL,
        .name = {
            .size = var_lhs->content_size,
            .value = var_lhs->content
        },
        .fun_call = {
            .name = {
                .value = malloc(fun_name->content_size * sizeof(char)),
                .size = fun_name->content_size
            },
            .arguments = {0}
        }
    };

    memcpy(var.fun_call.name.value, fun_name->content, fun_name->content_size);

    if ((*ref)->kind != TK_RPAREN) {
        Token* current = *ref;

        while (current->kind != TK_RPAREN) {
            if (current->kind == TK_NEWLINE) {
                current = current->next;
                continue;
            }

            switch (current->kind) {
                case TK_INTEGER: {
                    Data_Type integer = parse_integer_variable(&current);
                    Argument argument = create_argument_from_kind(VK_INTEGER, integer);

                    array_append(&var.fun_call.arguments, argument);
                } break;
                default: {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" Invalid syntax. Unexpected token \033[1;31m%.*s\033[0m which is \033[1;35m%s\033[0m\n",
                        LOC_ERROR_ARG(current->loc),
                        (int)current->content_size,
                        current->content,
                        token_kind_name(current->kind)
                    );
                    exit(1);
                }
            }

            if (current->kind == TK_COMMA) current = current->next;
        }

        *ref = current;
    }

    expect_kind(ref, TK_RPAREN); // consume ')'

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

        Token *var_lhs = expect_two_kinds(&current, TK_SYM, TK_STRING);
        (void)expect_kind(&current, TK_EQUAL);

        switch (current->kind) {
            case TK_STRING: array_append(&parser, parse_string_variable(var_lhs, &current)); break;
            case TK_INTEGER: array_append(&parser, create_variable_from_kind(VK_INTEGER, var_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&parser, parse_float_variable(var_lhs, &current)); break;
            case TK_TRUE: array_append(&parser, parse_bool_variable(var_lhs, true, &current)); break;
            case TK_FALSE: array_append(&parser, parse_bool_variable(var_lhs, false, &current)); break;
            case TK_NIL: array_append(&parser, parse_nil_variable(var_lhs, &current)); break;
            case TK_LSQUARE: array_append(&parser, parse_array_variable(var_lhs, &current)); break;
            case TK_LBRACE: array_append(&parser, parse_object_variable(var_lhs, &current)); break;
            case TK_PATH_ROOT: array_append(&parser, parse_path_variable(var_lhs, &current)); break;
            case TK_SYM: array_append(&parser, parse_fun_call_variable(var_lhs, &current)); break;

            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid syntax. Unexpected token \033[1;31m%.*s\033[0m which is \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(current->loc),
                    (int)current->content_size,
                    current->content,
                    token_kind_name(current->kind)
                );
                exit(1);
            }
        }
    }

    return parser;
}

void parser_free(Parser parser) {
    // TODO: implement a better parser free
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
        case VK_PATH: return "PATH";
        case VK_FUN_CALL: return "FUN_CALL";
        default: return "UNKNOWN";
    }
}
