#include "./parser.h"
#include "./interpreter.h"
#include "./map.h"
#include "./utils.h"
#include "./loc.h"
#include "./assertf.h"
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
    SK_ARRAY,
    SK_OBJECT,
} Symbol_Kind;

typedef union {
    Nil nil;
    Integer integer;
    String string;
    Float floating;
    Boolean boolean;
    Array array;
    Object object;
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

Symbol_Value eval_builtin_fun_call(Symbols symbols, Location loc, Fun_Call *fun_call);
void print_symbol(Symbols *symbols, Symbol symbol, bool is_inside_array);
Symbol interpret_var(Symbols symbols, Var var);

#define BUILTIN_FUN_SUM_I "sum_i"
#define BUILTIN_FUN_SUM_F "sum_f"
#define BUILTIN_FUN_LEN   "len"

static const char *symbol_kind_name(Symbol_Kind kind) {
    switch (kind) {
        case SK_NIL: return "nil";
        case SK_INTEGER: return "integer";
        case SK_STRING: return "string";
        case SK_FLOAT: return "float";
        case SK_BOOLEAN: return "boolean";
        case SK_ARRAY: return "array";
        default: return "unknown";
    }
}

Symbol_Value compute_variable_reference(Symbols symbols, Location loc, Path path) {
    assertf(path.length == 1, "For now, path should only reference root items");

    for (size_t i = 0; i < path.length; ++i) {
        String chunk = path.data[i];

        Symbol *symbol = map_get(symbols, chunk.value);

        if (symbol == NULL) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" variable reference \033[1;35m%s\033[0m not found\n",
                LOC_ERROR_ARG(loc),
                chunk.value
            );
            exit(1);
        }

        return symbol->value;
    }
    
    assertf(false, "unreacheable");
}

Symbol_Value reduce_argument(Symbols symbols, Location loc, Argument arg) {
    switch (arg.kind) {
        case AK_NIL: return (Symbol_Value){.kind = SK_NIL};
        case AK_INTEGER: return (Symbol_Value){.kind = SK_INTEGER, .as.integer = arg.as.integer};
        case AK_STRING: return (Symbol_Value){.kind = SK_STRING, .as.string = arg.as.string};
        case AK_FLOAT: return (Symbol_Value){.kind = SK_FLOAT, .as.floating = arg.as.floating};
        case AK_BOOLEAN: return (Symbol_Value){.kind = SK_BOOLEAN, .as.boolean = arg.as.boolean};
        case AK_OBJECT: assertf(false, "object is not handled yet");
        case AK_ARRAY: assertf(false, "array is not handled yet");
        case AK_PATH: return compute_variable_reference(symbols, loc, arg.as.path);
        case AK_FUN_CALL: return eval_builtin_fun_call(symbols, loc, arg.as.fun_call);
        default: assertf(false, "unreacheable");
    }
}

long __bultin_fun_call_len(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_LEN" expects an array as parameter\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    if (fun_call->arguments.length > 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_LEN" expects only one argument which is an array\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    Symbol_Value sym = reduce_argument(symbols, fun_call->arguments.data[0].loc, fun_call->arguments.data[0]);

    if (sym.kind != SK_ARRAY) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_LEN" expects an array as parameter but received \033[1;35m%s\033[0m\n",
            LOC_ERROR_ARG(loc),
            symbol_kind_name(sym.kind)
        );
        exit(1);
    }

    return sym.as.array.length;
}

long __bultin_fun_call_sum_i(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    long sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        bool computing = true;

        while (computing) {
            computing = false;

            Argument argument = fun_call->arguments.data[i];

            Symbol_Value value = reduce_argument(symbols, loc, argument);

            switch (value.kind) {
                case SK_INTEGER: sum += value.as.integer.value; break;
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
    }

    return sum;
}

double __bultin_fun_call_sum_f(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    double sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        Argument argument = fun_call->arguments.data[i];
        Symbol_Value value = reduce_argument(symbols, loc, argument);

        switch (value.kind) {
            case SK_INTEGER: sum += value.as.integer.value; break;
            case SK_FLOAT: sum += value.as.floating.value; break;
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

Symbol_Value eval_builtin_fun_call(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_SUM_I, strlen(BUILTIN_FUN_SUM_I))) {
        return (Symbol_Value){
            .kind = SK_INTEGER,
            .as.integer.value = __bultin_fun_call_sum_i(symbols, loc, fun_call)
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_SUM_F, strlen(BUILTIN_FUN_SUM_F))) {
        return (Symbol_Value){
            .kind = SK_FLOAT,
            .as.floating.value = __bultin_fun_call_sum_f(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_LEN, strlen(BUILTIN_FUN_LEN))) {
        return (Symbol_Value){
            .kind = SK_INTEGER,
            .as.integer.value = __bultin_fun_call_len(symbols, loc, fun_call),
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

void print_symbol(Symbols *symbols, Symbol symbol, bool is_inside_array) {
    if (!is_inside_array) {
        printf("  %s = ", symbol.name.value);
    }

    switch (symbol.value.kind) {
        case SK_NIL: {
            printf("nil");
        } break;
        case SK_STRING: {
            printf("\"%s\"", symbol.value.as.string.value);
        } break;
        case SK_INTEGER: {
            printf("%lu", symbol.value.as.integer.value);
        } break;
        case SK_FLOAT: {
            printf("%lf", symbol.value.as.floating.value);
        } break;
        case SK_BOOLEAN: {
            printf("%s", symbol.value.as.boolean.value == 1 ? "true" : "false");
        } break;
        case SK_ARRAY: {
            printf("[");
            for (size_t i = 0; i < symbol.value.as.array.length; ++i) {
                Argument arg = symbol.value.as.array.data[i];
                Symbol_Value reduced_value = reduce_argument(*symbols, arg.loc, arg);

                if (i > 0) printf(", ");
                print_symbol(symbols, (Symbol){.value = reduced_value}, true);
            }
            printf("]");
        } break;
        case SK_OBJECT: {
            printf("{");
            for (size_t i = 0; i < symbol.value.as.object.length; ++i) {
                Var var = symbol.value.as.object.data[i];

                if (i > 0) printf(", ");

                Symbol symbol = interpret_var(*symbols, var);

                printf("%s = ", var.name.value);

                print_symbol(symbols, symbol, true);
            }
            printf("}");
        } break;
        default: printf("unkonwn\n"); break;
    }

    if (!is_inside_array) printf("\n");
}

Symbol interpret_var(Symbols symbols, Var var) {
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
        case VK_PATH: {
            symbol.value = compute_variable_reference(symbols, var.loc, var.as.path);
        } break;
        case VK_FUN_CALL: {
            symbol.value = eval_builtin_fun_call(symbols, var.loc, var.as.fun_call);
        } break;
        case VK_ARRAY: {
            symbol.value.kind = SK_ARRAY;
            symbol.value.as.array = var.as.array;
        } break;
        case VK_OBJECT: {
            symbol.value.kind = SK_OBJECT;
            symbol.value.as.object = var.as.object;
        } break;
        default: assert(0 && "kind not evaluated yet");
    }

    return symbol;
}

void interpret(const Var *vars, size_t length) {
    Symbols symbols = map_new();

    for (size_t i = 0; i < length; i++) {
        Var var = vars[i];

        Symbol symbol = interpret_var(symbols, var);

        map_set(symbols, symbol.name.value, &symbol, sizeof(Symbol));
    }

    printf("Symbols table (%ld)\n", symbols->length);
    for (size_t i = 0; i < MAP_BUCKET_SIZE; ++i) {
        MapNode *node = symbols->nodes[i];

        while (node != NULL) {
            Symbol symbol = *(Symbol*)node->data;

            print_symbol(&symbols, symbol, false);

            node = node->next;
        }
    }

    map_free(symbols);
}
