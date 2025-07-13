#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./parser.h"
#include "./loc.h"
#include "./lexer.h"

Var_Data_Types parse_object_variable(Token **ref);
Var_Data_Types parse_array_variable(Token **ref);

static Location current_location;

void advance_token(Token **ref) {
    if (ref != NULL && *ref != NULL) *ref = (*ref)->next;
}

#define unwrap_ref(ref) *ref;

static String copy_string_as_null_terminated(String string) {
    String ret = {
        .value = malloc(string.size + 1),
        .size = string.size
    };

    memcpy(ret.value, string.value, string.size);
    ret.value[string.size] = '\0';

    return ret;
}

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

    current_location = var_rhs->loc;

    return (Var_Data_Types){
        .string = copy_string_as_null_terminated((String){
            .size = var_rhs->content_size - 2, // removing quotes
            .value = var_rhs->content + 1
        })
    };
}

Var_Data_Types parse_integer_variable(Token **tokens) {
    Token *var_rhs = expect_kind(tokens, TK_INTEGER);

    current_location = var_rhs->loc;

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

static Var create_variable_from_kind(Location loc, Var_Kind kind, Token* var_lhs, Var_Data_Types data) {
    Var var = {
        .kind = kind,
        .loc = loc,
        .name = {0},
    };

    if (var_lhs->kind == TK_STRING) {
        var.name.size = var_lhs->content_size - 2;
        var.name.value = malloc(var.name.size + 1);

        memcpy(var.name.value, var_lhs->content+1, var.name.size);
        var.name.value[var.name.size] = '\0';
    } else {
        var.name.value = malloc(var_lhs->content_size + 1);
        var.name.size = var_lhs->content_size;

        memcpy(var.name.value, var_lhs->content, var_lhs->content_size);
        var.name.value[var_lhs->content_size] = '\0';
    }

    switch (kind) {
        case VK_STRING: var.as.string = data.string; break;
        case VK_INTEGER: var.as.integer = data.integer; break;
        case VK_FLOAT: var.as.floating = data.floating; break;
        case VK_NIL: break;
        case VK_BOOLEAN: var.as.boolean = data.boolean; break;
        case VK_ARRAY: var.as.array = data.array; break;
        case VK_OBJECT: var.as.object = data.object; break;
        case VK_PATH: var.as.path = data.path; break;
        case VK_FUN_CALL: var.as.fun_call = data.fun_call; break;
        default: assert(0 && "update variable creation function");
    }

    return var;
}

static Argument create_argument_from_kind(Location loc, Var_Kind kind, Var_Data_Types data) {
    switch (kind) {
        case VK_INTEGER: {
            return (Argument){
                .kind = AK_INTEGER,
                .loc = loc,
                .as = {
                    .integer = data.integer
                }
            };
        } break;
        case VK_STRING: {
            return (Argument){
                .kind = AK_STRING,
                .loc = loc,
                .as = {
                    .string = data.string
                }
            };
        } break;
        case VK_FLOAT: {
            return (Argument){
                .kind = AK_FLOAT,
                .loc = loc,
                .as = {
                    .floating = data.floating
                }
            };
        } break;
        case VK_NIL: {
            return (Argument){
                .kind = AK_NIL,
                .loc = loc,
            };
        } break;
        case VK_BOOLEAN: {
            return (Argument){
                .kind = AK_BOOLEAN,
                .loc = loc,
                .as = {
                    .boolean = data.boolean
                }
            };
        } break;
        case VK_ARRAY: {
            return (Argument){
                .kind = AK_ARRAY,
                .loc = loc,
                .as = {
                    .array = data.array
                }
            };
        } break;
        case VK_OBJECT: {
            return (Argument){
                .kind = AK_OBJECT,
                .loc = loc,
                .as = {
                    .object = data.object
                }
            };
        } break;
        case VK_PATH: {
            return (Argument){
                .kind = AK_PATH,
                .loc = loc,
                .as = {
                    .path = data.path
                }
            };
        } break;
        case VK_FUN_CALL: {
            return (Argument){
                .kind = AK_FUN_CALL,
                .loc = loc,
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

    current_location = var_rhs->loc;

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
    current_location = (*ref)->loc;

    advance_token(ref);

    return (Var_Data_Types){
        .nil = NULL
    };
}

Var_Data_Types parse_bool_variable(bool value, Token **ref) {
    current_location = (*ref)->loc;
    advance_token(ref);

    return (Var_Data_Types){
        .boolean = {
            .value = value
        }
    };
}

Var_Data_Types_Indentified parse_path_variable(Token **ref) {
    current_location = (*ref)->loc;

    advance_token(ref);

    Var_Data_Types var = {
        .path = {
            .capacity = 0,
            .length = 0,
            .data = NULL
        }
    };

    Token *last = NULL;
    Token *current = unwrap_ref(ref);

    int chunks = 0;

    while (current->kind == TK_PATH_CHUNK) {
        chunks++;

        String string = copy_string_as_null_terminated((String){
            .value = current->content,
            .size = current->content_size
        });

        array_append(&var.path, string);

        last = current;
        current = current->next;
    }

    if (chunks != 1) {
        if (last == NULL) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Invalid syntax. Invalid path\n",
                LOC_ERROR_ARG(current->loc)
            );
        } else {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Invalid syntax. Invalid path\n",
                LOC_ERROR_ARG(last->loc)
            );
        }
        exit(1);
    }

    *ref = current;

    if ((*ref)->kind == TK_LSQUARE) {
        advance_token(ref);

        while ((*ref)->kind == TK_NEWLINE) advance_token(ref);

        Argument reference_argument = create_argument_from_kind(last->loc, VK_PATH, var);
        Argument index_argument;

        Token *index_token = *ref;

        switch (index_token->kind) {
            case TK_INTEGER: {
                index_argument = create_argument_from_kind(index_token->loc, VK_INTEGER, parse_integer_variable(ref));
            } break;
            case TK_STRING: {
                index_argument = create_argument_from_kind(index_token->loc, VK_STRING, parse_string_variable(ref));
            } break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid syntax. Invalid index type %s\n",
                    LOC_ERROR_ARG(last->loc),
                    token_kind_name((*ref)->kind)
                );
                exit(1);
            };
        }

        while ((*ref)->kind == TK_NEWLINE) advance_token(ref);

        (void)expect_kind(ref, TK_RSQUARE);

        Var_Data_Types fun = {
            .fun_call = calloc(1, sizeof(Fun_Call))
        };

        switch (index_argument.kind) {
            case AK_INTEGER: { fun.fun_call->name.value = "##get_array_index"; } break;
            case AK_STRING: { fun.fun_call->name.value = "##get_object_index"; } break;
            default: break;
        }

        fun.fun_call->name.size = strlen(fun.fun_call->name.value);
        array_append(&fun.fun_call->arguments, reference_argument);
        array_append(&fun.fun_call->arguments, index_argument);
        
        return (Var_Data_Types_Indentified){
            .kind = VK_FUN_CALL,
            .as = fun
        };
    }

    return (Var_Data_Types_Indentified){
        .kind = VK_PATH,
        .as = var
    };
}

Var_Data_Types parse_fun_call_variable(Token **ref) {
    Location fun_call_location = (*ref)->loc;

    Token *fun_name = *ref;

    advance_token(ref); // consume the function call name

    (void)expect_kind(ref, TK_LPAREN); // consume '('

    Var_Data_Types var = {
        .fun_call = calloc(1, sizeof(Fun_Call)),
    };

    var.fun_call->name = copy_string_as_null_terminated((String){
        .value = fun_name->content,
        .size = fun_name->content_size
    });

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
                    argument = create_argument_from_kind(current_location, VK_INTEGER, integer);
                } break;
                case TK_FLOAT: {
                    Var_Data_Types floating = parse_float_variable(&current);
                    argument = create_argument_from_kind(current_location, VK_FLOAT, floating);
                } break;
                case TK_STRING: {
                    Var_Data_Types string = parse_string_variable(&current);
                    argument = create_argument_from_kind(current_location, VK_STRING, string);
                } break;
                case TK_NIL: {
                    Var_Data_Types nil = parse_nil_variable(&current);
                    argument = create_argument_from_kind(current_location, VK_NIL, nil);
                } break;
                case TK_TRUE: case TK_FALSE: {
                    Var_Data_Types boolean = parse_bool_variable(current->kind == TK_TRUE, &current);
                    argument = create_argument_from_kind(current_location, VK_BOOLEAN, boolean);
                } break;
                case TK_LSQUARE: {
                    Var_Data_Types array = parse_array_variable(&current);
                    argument = create_argument_from_kind(current_location, VK_ARRAY, array);
                } break;
                case TK_LBRACE: {
                    Var_Data_Types object = parse_object_variable(&current);
                    argument = create_argument_from_kind(current_location, VK_OBJECT, object);
                } break;
                case TK_PATH_ROOT: {
                    Var_Data_Types_Indentified path = parse_path_variable(&current);

                    switch (path.kind) {
                        case VK_PATH: {
                            argument = create_argument_from_kind(current_location, VK_PATH, path.as);
                        } break;
                        case VK_FUN_CALL: {
                            argument = create_argument_from_kind(current_location, VK_FUN_CALL, path.as);
                        } break;
                        default: break;
                    }
                } break;
                case TK_SYM: {
                    Var_Data_Types fun_call = parse_fun_call_variable(&current);
                    argument = create_argument_from_kind(current_location, VK_FUN_CALL, fun_call);
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

    current_location = fun_call_location;

    expect_kind(ref, TK_RPAREN); // consume ')'

    return var;
}

Var_Data_Types parse_array_variable(Token **ref) {
    advance_token(ref);

    current_location = (*ref)->loc;

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
            case TK_STRING: array_append(&var.array, create_argument_from_kind(current_location, VK_STRING, parse_string_variable(&current))); break;
            case TK_INTEGER: array_append(&var.array, create_argument_from_kind(current_location, VK_INTEGER, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&var.array, create_argument_from_kind(current_location, VK_FLOAT, parse_float_variable(&current))); break;
            case TK_TRUE: array_append(&var.array, create_argument_from_kind(current_location, VK_BOOLEAN, parse_bool_variable(true, &current))); break;
            case TK_FALSE: array_append(&var.array, create_argument_from_kind(current_location, VK_BOOLEAN, parse_bool_variable(false, &current))); break;
            case TK_NIL: array_append(&var.array, create_argument_from_kind(current_location, VK_NIL, parse_nil_variable(&current))); break;
            case TK_LSQUARE: array_append(&var.array, create_argument_from_kind(current_location, VK_ARRAY, parse_array_variable(&current))); break;
            case TK_LBRACE: array_append(&var.array, create_argument_from_kind(current_location, VK_OBJECT, parse_object_variable(&current))); break;
            case TK_PATH_ROOT: {
                Var_Data_Types_Indentified path = parse_path_variable(&current);

                switch (path.kind) {
                    case VK_PATH: {
                        array_append(&var.array, create_argument_from_kind(current_location, VK_PATH, path.as))
                    } break;
                    case VK_FUN_CALL: {
                        array_append(&var.array, create_argument_from_kind(current_location, VK_FUN_CALL, path.as))
                    } break;
                    default: break;
                }
            } break;
            case TK_SYM: array_append(&var.array, create_argument_from_kind(current_location, VK_FUN_CALL, parse_fun_call_variable(&current))); break;
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
    current_location = (*ref)->loc;

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
            case TK_STRING: array_append(&var.object, create_variable_from_kind(current_location, VK_STRING, key_lhs, parse_string_variable(&current))); break;
            case TK_INTEGER: array_append(&var.object, create_variable_from_kind(current_location, VK_INTEGER, key_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&var.object, create_variable_from_kind(current_location, VK_FLOAT, key_lhs, parse_float_variable(&current))); break;
            case TK_TRUE: array_append(&var.object, create_variable_from_kind(current_location, VK_BOOLEAN, key_lhs, parse_bool_variable(true, &current))); break;
            case TK_FALSE: array_append(&var.object, create_variable_from_kind(current_location, VK_BOOLEAN, key_lhs, parse_bool_variable(false, &current))); break;
            case TK_NIL: array_append(&var.object, create_variable_from_kind(current_location, VK_NIL, key_lhs, parse_nil_variable(&current))); break;
            case TK_LSQUARE: array_append(&var.object, create_variable_from_kind(current_location, VK_ARRAY, key_lhs, parse_array_variable(&current))); break;
            case TK_LBRACE: array_append(&var.object, create_variable_from_kind(current_location, VK_OBJECT, key_lhs, parse_object_variable(&current))); break;
            case TK_PATH_ROOT: {
                Var_Data_Types_Indentified path = parse_path_variable(&current);

                switch (path.kind) {
                    case VK_PATH: {
                        array_append(&var.object, create_variable_from_kind(current_location, VK_PATH, key_lhs, path.as));
                    } break;
                    case VK_FUN_CALL: {
                        array_append(&var.object, create_variable_from_kind(current_location, VK_FUN_CALL, key_lhs, path.as));
                    } break;
                    default: break;
                }
            } break;
            case TK_SYM: array_append(&var.object, create_variable_from_kind(current_location, VK_FUN_CALL, key_lhs, parse_fun_call_variable(&current))); break;
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
            case TK_STRING: array_append(&parser, create_variable_from_kind(current_location, VK_STRING, var_lhs, parse_string_variable(&current))); break;
            case TK_INTEGER: array_append(&parser, create_variable_from_kind(current_location, VK_INTEGER, var_lhs, parse_integer_variable(&current))); break;
            case TK_FLOAT: array_append(&parser, create_variable_from_kind(current_location, VK_FLOAT, var_lhs, parse_float_variable(&current))); break;
            case TK_TRUE: array_append(&parser, create_variable_from_kind(current_location, VK_BOOLEAN, var_lhs, parse_bool_variable(true, &current))); break;
            case TK_FALSE: array_append(&parser, create_variable_from_kind(current_location, VK_BOOLEAN, var_lhs, parse_bool_variable(false, &current))); break;
            case TK_NIL: array_append(&parser, create_variable_from_kind(current_location, VK_NIL, var_lhs, parse_nil_variable(&current))); break;
            case TK_LSQUARE: array_append(&parser, create_variable_from_kind(current_location, VK_ARRAY, var_lhs, parse_array_variable(&current))); break;
            case TK_LBRACE: array_append(&parser, create_variable_from_kind(current_location, VK_OBJECT, var_lhs, parse_object_variable(&current))); break;
            case TK_PATH_ROOT: {
                Var_Data_Types_Indentified path = parse_path_variable(&current);

                switch (path.kind) {
                    case VK_PATH: {
                        array_append(&parser, create_variable_from_kind(current_location, VK_PATH, var_lhs, path.as));
                    } break;
                    case VK_FUN_CALL: {
                        array_append(&parser, create_variable_from_kind(current_location, VK_FUN_CALL, var_lhs, path.as));
                    } break;
                    default: break;
                }
            } break;
            case TK_SYM: array_append(&parser, create_variable_from_kind(current_location, VK_FUN_CALL, var_lhs, parse_fun_call_variable(&current))); break;

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

const char *argument_kind_name(Argument_Kind kind) {
    switch (kind) {
        case AK_NIL: return "nil";
        case AK_INTEGER: return "integer";
        case AK_STRING: return "string";
        case AK_FLOAT: return "float";
        case AK_BOOLEAN: return "boolean";
        case AK_PATH: return "path";
        case AK_OBJECT: return "object";
        case AK_ARRAY: return "array";
        case AK_FUN_CALL: return "fun_call";
        default: return "UNKNOWN";
    }
}
