#include "./parser.h"
#include "./interpreter.h"
#include "./map.h"
#include "./utils.h"
#include "./loc.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
    SK_NIL = 0,
    SK_INTEGER,
    SK_STRING,
    SK_FLOAT,
    SK_BOOLEAN,
} Symbol_Kind;

typedef union {
    Nil nil;
    Integer integer;
    String string;
    Float floating;
    Boolean boolean;
} Symbol_Data_Types;

typedef struct {
    char *value;
    size_t size;
} Symbol_Name;

typedef struct {
    Symbol_Kind kind;
    Symbol_Data_Types as; // evaluated
} Symbol_Value;

typedef struct {
    Symbol_Name name;
    Symbol_Value value;
} Symbol;

typedef Map* Symbols;

Symbol_Value eval_builtin_fun_call(Location loc, Fun_Call *fun_call);

#define BUILTIN_FUN_SUM_I "sum_i"
#define BUILTIN_FUN_SUM_F "sum_f"

static const char *symbol_kind_name(Symbol_Kind kind) {
    switch (kind) {
        case SK_NIL: return "nil";
        case SK_INTEGER: return "integer";
        case SK_STRING: return "string";
        case SK_FLOAT: return "float";
        case SK_BOOLEAN: return "boolean";
        default: return "unkonwn";
    }
}

long __bultin_fun_call_sum_i(Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    long sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        Argument argument = fun_call->arguments.data[i];

        switch (argument.kind) {
            case AK_INTEGER: sum += argument.as.integer.value; break;
            case AK_FUN_CALL: {
                Symbol_Value value = eval_builtin_fun_call(loc, argument.as.fun_call);

                switch (value.kind) {
                    case SK_INTEGER: sum += value.as.integer.value; break;
                    default: {
                        fprintf(
                            stderr,
                            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_I" expects integers or floats as parameters but received a \033[1;35m%s\033[0m\n",
                            LOC_ERROR_ARG(argument.loc),
                            symbol_kind_name(value.kind) // TODO: display the wrong value
                        );
                        exit(1);
                    }
                }
            } break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_I" expects only integers as parameters but received a \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(argument.loc),
                    argument_kind_name(argument.kind) // TODO: display the wrong value
                );
                exit(1);
            }
        }
    }

    return sum;
}

double __bultin_fun_call_sum_f(Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    double sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        Argument argument = fun_call->arguments.data[i];

        switch (argument.kind) {
            case AK_INTEGER: sum += argument.as.integer.value; break;
            case AK_FLOAT: sum += argument.as.floating.value; break;
            case AK_FUN_CALL: {
                Symbol_Value value = eval_builtin_fun_call(loc, argument.as.fun_call);

                switch (value.kind) {
                    case SK_INTEGER: sum += value.as.integer.value; break;
                    case SK_FLOAT: sum += value.as.floating.value; break;
                    default: {
                        fprintf(
                            stderr,
                            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_F" expects integers or floats as parameters but received a \033[1;35m%s\033[0m\n",
                            LOC_ERROR_ARG(argument.loc),
                            symbol_kind_name(value.kind) // TODO: display the wrong value
                        );
                        exit(1);
                    }
                }

            } break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_F" expects integers or floats as parameters but received a \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(argument.loc),
                    argument_kind_name(argument.kind) // TODO: display the wrong value
                );
                exit(1);
            };
        }
    }

    return sum;
}

Symbol_Value eval_builtin_fun_call(Location loc, Fun_Call *fun_call) {
    if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_SUM_I, strlen(BUILTIN_FUN_SUM_I))) {
        return (Symbol_Value){
            .kind = SK_INTEGER,
            .as.integer.value = __bultin_fun_call_sum_i(loc, fun_call)
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_SUM_F, strlen(BUILTIN_FUN_SUM_F))) {
        return (Symbol_Value){
            .kind = SK_FLOAT,
            .as.floating.value = __bultin_fun_call_sum_f(loc, fun_call),
        };
    } else {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Built-in function not found \033[1;35m%s\033[0m\n",
            LOC_ERROR_ARG(loc),
            fun_call->name.value
        );
        exit(1);
    }
}

void interpret(const Var *vars, size_t length) {
    Symbols symbols = map_new();

    for (size_t i = 0; i < length; i++) {
        Var var = vars[i];

        Symbol symbol = {
            .name = {
                .value = var.name.value,
                .size = var.name.size
            },
        };

        switch (var.kind) {
            case VK_NIL: { symbol.value.kind = SK_NIL; } break;
            case VK_INTEGER: { symbol.value.kind = SK_INTEGER; symbol.value.as.integer = var.as.integer; } break;
            case VK_STRING: { symbol.value.kind = SK_STRING; symbol.value.as.string = var.as.string; } break;
            case VK_FLOAT: { symbol.value.kind = SK_FLOAT; symbol.value.as.floating = var.as.floating; } break;
            case VK_BOOLEAN: { symbol.value.kind = SK_BOOLEAN; symbol.value.as.boolean = var.as.boolean; } break;
            case VK_FUN_CALL: {
                symbol.value = eval_builtin_fun_call(var.loc, var.as.fun_call);
            } break;
            default: assert(0 && "kind not evaluated yet");
        }

        map_set(symbols, symbol.name.value, &symbol, sizeof(Symbol));
    }

    printf("Symbols table (%ld)\n", symbols->length);
    for (size_t i = 0; i < MAP_BUCKET_SIZE; ++i) {
        MapNode *node = symbols->nodes[i];

        while (node != NULL) {
            Symbol symbol = *(Symbol*)node->data;

            switch (symbol.value.kind) {
                case SK_NIL: {
                    printf("  %s = nil\n", symbol.name.value);
                } break;
                case SK_STRING: {
                    printf("  %s = %s\n", symbol.name.value, symbol.value.as.string.value);
                } break;
                case SK_INTEGER: {
                    printf("  %s = %lu\n", symbol.name.value, symbol.value.as.integer.value);
                } break;
                case SK_FLOAT: {
                    printf("  %s = %lf\n", symbol.name.value, symbol.value.as.floating.value);
                } break;
                case SK_BOOLEAN: {
                    printf("  %s = %s\n", symbol.name.value, symbol.value.as.boolean.value == 1 ? "true" : "false");
                } break;
                default: printf("unkonwn\n"); break;
            }
            node = node->next;
        }
    }

    map_free(symbols);
}
