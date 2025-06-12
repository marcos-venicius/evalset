#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./parser.h"
#include "./loc.h"
#include "./lexer.h"

Var_Data_Types parse_object_variable(Token **ref);

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

Var_Data_Types parse_string_variable(Token **tokens) {
    Token *var_rhs = expect_kind(tokens, TK_STRING);

    return (Var_Data_Types){
        .string = {
            .size = var_rhs->content_size,
            .value = var_rhs->content
        }
    };
}

Var_Data_Types parse_integer_variable(Token **tokens) {
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

    return (Var_Data_Types){
        .integer = {
            .value = integer
        }
    };
}

static Var create_variable_from_kind(Var_Kind kind, Token* var_lhs, Var_Data_Types data) {
    switch (kind) {
        case VK_STRING: {
            return (Var){
                .kind = VK_STRING,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .string = data.string
                }
            };
        } break;
        case VK_INTEGER: {
            return (Var){
                .kind = VK_INTEGER,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .integer = data.integer
                }
            };
        } break;
        case VK_FLOAT: {
            return (Var){
                .kind = VK_FLOAT,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .floating = data.floating
                }
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
                .as = {
                    .boolean = data.boolean
                }
            };
        } break;
        case VK_ARRAY: {
            return (Var){
                .kind = VK_ARRAY,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .array = data.array
                }
            };
        } break;
        case VK_OBJECT: {
            return (Var){
                .kind = VK_OBJECT,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .object = data.object
                }
            };
        } break;
        case VK_PATH: {
            return (Var){
                .kind = VK_PATH,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .path = data.path
                }
            };
        } break;
        case VK_FUN_CALL: {
            return (Var){
                .kind = VK_FUN_CALL,
                .name = {
                    .size = var_lhs->content_size,
                    .value = var_lhs->content
                },
                .as = {
                    .fun_call = data.fun_call
                }
            };
        } break;
        default: assert(0 && "update variable creation function");
    }
}

static Argument create_argument_from_kind(Var_Kind kind, Var_Data_Types data) {
    switch (kind) {
        case VK_INTEGER: {
            return (Argument){
                .kind = AK_INTEGER,
                .as = {
                    .integer = data.integer
                }
            };
        } break;
        case VK_STRING: {
            return (Argument){
                .kind = AK_STRING,
                .as = {
                    .string = data.string
                }
            };
        } break;
        case VK_FLOAT: {
            return (Argument){
                .kind = AK_FLOAT,
                .as = {
                    .floating = data.floating
                }
            };
        } break;
        case VK_NIL: {
            return (Argument){
                .kind = AK_NIL,
            };
        } break;
        case VK_BOOLEAN: {
            return (Argument){
                .kind = AK_BOOLEAN,
                .as = {
                    .boolean = data.boolean
                }
            };
        } break;
        case VK_ARRAY: {
            return (Argument){
                .kind = AK_ARRAY,
                .as = {
                    .array = data.array
                }
            };
        } break;
        case VK_OBJECT: {
            return (Argument){
                .kind = AK_OBJECT,
                .as = {
                    .object = data.object
                }
            };
        } break;
        case VK_PATH: {
            return (Argument){
                .kind = AK_PATH,
                .as = {
                    .path = data.path
                }
            };
        } break;
        case VK_FUN_CALL: {
            return (Argument){
                .kind = AK_FUN_CALL,
                .as = {
                    .fun_call = data.fun_call
                }
            };
        } break;
        default: assert(0 && "update variable creation function");
    }
}

Var_Data_Types parse_float_variable(Token **tokens) {
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

    return (Var_Data_Types){
        .floating = {
            .value = floating
        }
    };
}

Var_Data_Types parse_nil_variable(Token **ref) {
    advance_token(ref);

    return (Var_Data_Types){
        .nil = NULL
    };
}

Var_Data_Types parse_bool_variable(bool value, Token **tokens) {
    advance_token(tokens);

    return (Var_Data_Types){
        .boolean = {
            .value = value
        }
    };
}

Var_Data_Types parse_path_variable(Token **ref) {
    advance_token(ref);

    Var_Data_Types var = {
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

        String string = {
            .value = chunk,
            .size = current->content_size
        };

        array_append(&var.path, string);

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

Var_Data_Types parse_array_variable(Token **ref) {
    advance_token(ref);

    Var_Data_Types var = {
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
            case TK_STRING: array_append(&var.array, create_argument_from_kind(VK_STRING, parse_string_variable(&current))); break;
            case TK_INTEGER: array_append(&var.array, create_argument_from_kind(VK_INTEGER, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&var.array, create_argument_from_kind(VK_FLOAT, parse_float_variable(&current))); break;
            case TK_TRUE: array_append(&var.array, create_argument_from_kind(VK_BOOLEAN, parse_bool_variable(true, &current))); break;
            case TK_FALSE: array_append(&var.array, create_argument_from_kind(VK_BOOLEAN, parse_bool_variable(false, &current))); break;
            case TK_NIL: array_append(&var.array, create_argument_from_kind(VK_NIL, parse_nil_variable(&current))); break;
            case TK_LSQUARE: array_append(&var.array, create_argument_from_kind(VK_ARRAY, parse_array_variable(&current))); break;
            case TK_LBRACE: array_append(&var.array, create_argument_from_kind(VK_OBJECT, parse_object_variable(&current))); break;
            case TK_PATH_ROOT: array_append(&var.array, create_argument_from_kind(VK_PATH, parse_path_variable(&current))); break;
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

Var_Data_Types parse_object_variable(Token **ref) {
    advance_token(ref);

    Var_Data_Types var = {
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
            case TK_STRING: array_append(&var.object, create_variable_from_kind(VK_STRING, key_lhs, parse_string_variable(&current))); break;
            case TK_INTEGER: array_append(&var.object, create_variable_from_kind(VK_INTEGER, key_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&var.object, create_variable_from_kind(VK_FLOAT, key_lhs, parse_float_variable(&current))); break;
            case TK_TRUE: array_append(&var.object, create_variable_from_kind(VK_BOOLEAN, key_lhs, parse_bool_variable(true, &current))); break;
            case TK_FALSE: array_append(&var.object, create_variable_from_kind(VK_BOOLEAN, key_lhs, parse_bool_variable(false, &current))); break;
            case TK_NIL: array_append(&var.object, create_variable_from_kind(VK_NIL, key_lhs, parse_nil_variable(&current))); break;
            case TK_LSQUARE: array_append(&var.object, create_variable_from_kind(VK_ARRAY, key_lhs, parse_array_variable(&current))); break;
            case TK_LBRACE: array_append(&var.object, create_variable_from_kind(VK_OBJECT, key_lhs, parse_object_variable(&current))); break;
            case TK_PATH_ROOT: array_append(&var.object, create_variable_from_kind(VK_PATH, key_lhs, parse_path_variable( &current))); break;
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

Var_Data_Types parse_fun_call_variable(Token **ref) {
    Token *fun_name = *ref;

    advance_token(ref); // consume the function call name

    (void)expect_kind(ref, TK_LPAREN); // consume '('

    Var_Data_Types var = {
        .fun_call = calloc(1, sizeof(Fun_Call)),
    };

    var.fun_call->name = (String){
        .value = malloc(fun_name->content_size * sizeof(char)),
        .size = fun_name->content_size
    };

    memcpy(var.fun_call->name.value, fun_name->content, fun_name->content_size);

    if ((*ref)->kind != TK_RPAREN) {
        Token* current = *ref;

        while (current->kind != TK_RPAREN) {
            if (current->kind == TK_NEWLINE) {
                current = current->next;
                continue;
            }

            Argument argument = {0};

            switch (current->kind) {
                case TK_INTEGER: {
                    Var_Data_Types integer = parse_integer_variable(&current);
                    argument = create_argument_from_kind(VK_INTEGER, integer);
                } break;
                case TK_FLOAT: {
                    Var_Data_Types floating = parse_float_variable(&current);
                    argument = create_argument_from_kind(VK_FLOAT, floating);
                } break;
                case TK_STRING: {
                    Var_Data_Types string = parse_string_variable(&current);
                    argument = create_argument_from_kind(VK_STRING, string);
                } break;
                case TK_NIL: {
                    Var_Data_Types nil = parse_nil_variable(&current);
                    argument = create_argument_from_kind(VK_NIL, nil);
                } break;
                case TK_TRUE: case TK_FALSE: {
                    Var_Data_Types boolean = parse_bool_variable(current->kind == TK_TRUE, &current);
                    argument = create_argument_from_kind(VK_BOOLEAN, boolean);
                } break;
                case TK_LSQUARE: {
                    Var_Data_Types array = parse_array_variable(&current);
                    argument = create_argument_from_kind(VK_ARRAY, array);
                } break;
                case TK_LBRACE: {
                    Var_Data_Types object = parse_object_variable(&current);
                    argument = create_argument_from_kind(VK_OBJECT, object);
                } break;
                case TK_PATH_ROOT: {
                    Var_Data_Types path = parse_path_variable(&current);
                    argument = create_argument_from_kind(VK_PATH, path);
                } break;
                case TK_SYM: {
                    Var_Data_Types fun_call = parse_fun_call_variable(&current);
                    argument = create_argument_from_kind(VK_FUN_CALL, fun_call);
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

            array_append(&var.fun_call->arguments, argument);

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
            case TK_STRING: array_append(&parser, create_variable_from_kind(VK_STRING, var_lhs, parse_string_variable(&current))); break;
            case TK_INTEGER: array_append(&parser, create_variable_from_kind(VK_INTEGER, var_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&parser, create_variable_from_kind(VK_FLOAT, var_lhs, parse_float_variable(&current))); break;
            case TK_TRUE: array_append(&parser, create_variable_from_kind(VK_BOOLEAN, var_lhs, parse_bool_variable(true, &current))); break;
            case TK_FALSE: array_append(&parser, create_variable_from_kind(VK_BOOLEAN, var_lhs, parse_bool_variable(false, &current))); break;
            case TK_NIL: array_append(&parser, create_variable_from_kind(VK_NIL, var_lhs, parse_nil_variable(&current))); break;
            case TK_LSQUARE: array_append(&parser, create_variable_from_kind(VK_ARRAY, var_lhs, parse_array_variable(&current))); break;
            case TK_LBRACE: array_append(&parser, create_variable_from_kind(VK_OBJECT, var_lhs, parse_object_variable(&current))); break;
            case TK_PATH_ROOT: array_append(&parser, create_variable_from_kind(VK_PATH, var_lhs, parse_path_variable(&current))); break;
            case TK_SYM: array_append(&parser, create_variable_from_kind(VK_FUN_CALL, var_lhs, parse_fun_call_variable(&current))); break;

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
