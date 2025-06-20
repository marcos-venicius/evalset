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
} Symbol_Value;

typedef struct {
    char *value;
    size_t size;
} Symbol_Name;

typedef struct {
    Symbol_Name name;
    Symbol_Kind kind;
    Symbol_Value as; // evaluated
} Symbol;

typedef Map* Symbols;

#define BUILTIN_FUN_SUM_I "sum_i"
#define BUILTIN_FUN_SUM_F "sum_f"

long __bultin_fun_call_sum_i(Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    long sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        Argument argument = fun_call->arguments.data[i];

        if (argument.kind != AK_INTEGER) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function sum_i expects only integers as parameters but received a \033[1;35m%s\033[0m\n",
                LOC_ERROR_ARG(argument.loc),
                argument_kind_name(argument.kind) // TODO: display the wrong value
            );
            exit(1);
        }

        sum += argument.as.integer.value;
    }

    return sum;
}

double __bultin_fun_call_sum_f(Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    double sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        Argument argument = fun_call->arguments.data[i];

        if (argument.kind != AK_INTEGER && argument.kind != AK_FLOAT) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function sum_i expects integers or floats as parameters but received a \033[1;35m%s\033[0m\n",
                LOC_ERROR_ARG(argument.loc),
                argument_kind_name(argument.kind) // TODO: display the wrong value
            );
            exit(1);
        }

        switch (argument.kind) {
            case AK_INTEGER: sum += argument.as.integer.value; break;
            case AK_FLOAT: sum += argument.as.floating.value; break;
            default: assert(0 && "It should never happen");
        }
    }

    return sum;
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
            case VK_NIL: { symbol.kind = SK_NIL; } break;
            case VK_INTEGER: { symbol.kind = SK_INTEGER; symbol.as.integer = var.as.integer; } break;
            case VK_STRING: { symbol.kind = SK_STRING; symbol.as.string = var.as.string; } break;
            case VK_FLOAT: { symbol.kind = SK_FLOAT; symbol.as.floating = var.as.floating; } break;
            case VK_BOOLEAN: { symbol.kind = SK_BOOLEAN; symbol.as.boolean = var.as.boolean; } break;
            case VK_FUN_CALL: {
                if (cmp_sized_strings(var.as.fun_call->name.value, var.as.fun_call->name.size, BUILTIN_FUN_SUM_I, strlen(BUILTIN_FUN_SUM_I))) {
                    symbol.kind = SK_INTEGER;
                    symbol.as.integer.value = __bultin_fun_call_sum_i(var.as.fun_call);
                } else if (cmp_sized_strings(var.as.fun_call->name.value, var.as.fun_call->name.size, BUILTIN_FUN_SUM_F, strlen(BUILTIN_FUN_SUM_F))) {
                    symbol.kind = SK_FLOAT;
                    symbol.as.floating.value = __bultin_fun_call_sum_f(var.as.fun_call);
                } else {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" Built-in function not found \033[1;35m%s\033[0m\n",
                        LOC_ERROR_ARG(var.loc),
                        var.as.fun_call->name.value
                    );
                    exit(1);
                }
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

            switch (symbol.kind) {
                case SK_NIL: {
                    printf("  %s = nil\n", symbol.name.value);
                } break;
                case SK_STRING: {
                    printf("  %s = %s\n", symbol.name.value, symbol.as.string.value);
                } break;
                case SK_INTEGER: {
                    printf("  %s = %lu\n", symbol.name.value, symbol.as.integer.value);
                } break;
                case SK_FLOAT: {
                    printf("  %s = %lf\n", symbol.name.value, symbol.as.floating.value);
                } break;
                case SK_BOOLEAN: {
                    printf("  %s = %s\n", symbol.name.value, symbol.as.boolean.value == 1 ? "true" : "false");
                } break;
                default: printf("unkonwn\n"); break;
            }
            node = node->next;
        }
    }

    map_free(symbols);
}
