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

#define BUILTIN_FUN_SUM_I "sum_i"
#define BUILTIN_FUN_SUM_F "sum_f"
#define BUILTIN_FUN_LEN   "len"
#define BUILTIN_FUN_SUM_AI "sum_ai"
#define BUILTIN_FUN_SUM_AF "sum_af"
#define BUILTIN_FUN_CONCAT_A "concat_a"
#define BUILTIN_FUN_CONCAT_S "concat_s"
#define BUILTIN_FUN_JOIN_AS "join_as"
#define BUILTIN_FUN_KEYS "keys"
#define BUILTIN_FUN_IOTA "iota"

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
Symbol_Value reduce_argument(Symbols symbols, Argument arg);

static long __builtin_iota_current_value = 0;

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

Symbol_Value compute_indexing(Symbols symbols, Metadata metadata, Symbol_Value initial) {
    Symbol_Value value = initial;

    for (size_t i = 0; i < metadata.indexes.length; ++i) {
        Argument arg = metadata.indexes.data[i];
        Symbol_Value index = reduce_argument(symbols, metadata.indexes.data[i]);

        switch (index.kind) {
            case SK_INTEGER: {
                if (value.kind != SK_ARRAY) {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" you cannot index a \033[1;35m%s\033[0m with an integer",
                        LOC_ERROR_ARG(arg.loc),
                        symbol_kind_name(value.kind)
                    );
                    exit(1);
                }

                if (index.as.integer.value < 0 || (size_t)index.as.integer.value >= value.as.array.length) {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" index %ld out of range\n",
                        LOC_ERROR_ARG(arg.loc),
                        index.as.integer.value
                    );
                    exit(1);
                }

                Argument new_value = value.as.array.data[index.as.integer.value];

                value = reduce_argument(symbols, new_value);
            } break;
            case SK_STRING: {
                if (value.kind != SK_OBJECT) {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" You cannot index \033[1;35m%s\033[0m with \"%s\"\n",
                        LOC_ERROR_ARG(arg.loc),
                        symbol_kind_name(value.kind),
                        index.as.string.value
                    );
                    exit(1);
                }

                bool found = false;

                // TODO: change object to a map
                for (size_t i = 0; i < value.as.object.length; ++i) {
                    Var var = value.as.object.data[i];

                    if (cmp_sized_strings(var.name.value, var.name.size, index.as.string.value, index.as.string.size)) {
                        found = true;
                        value = interpret_var(symbols, var).value;
                        break;
                    }
                }

                if (!found) {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" key \"\033[1;35m%s\033[0m\" not found\n",
                        LOC_ERROR_ARG(arg.loc),
                        index.as.string.value
                    );

                    exit(1);
                }
            } break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Invalid index value of kind \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(arg.loc),
                    symbol_kind_name(value.kind)
                );
                exit(1);
            } break;
        }
    }

    return value;
}

Symbol_Value compute_variable_reference(Symbols symbols, Location loc, Path path, Metadata metadata) {
    if (path.length != 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" invalid path reference",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    String chunk = path.data[0];

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
    
    return compute_indexing(symbols, metadata, symbol->value);
}

Symbol_Value reduce_argument(Symbols symbols, Argument arg) {
    switch (arg.kind) {
        case AK_NIL: return (Symbol_Value){.kind = SK_NIL};
        case AK_INTEGER: return (Symbol_Value){.kind = SK_INTEGER, .as.integer = arg.as.integer};
        case AK_STRING: return (Symbol_Value){.kind = SK_STRING, .as.string = arg.as.string};
        case AK_FLOAT: return (Symbol_Value){.kind = SK_FLOAT, .as.floating = arg.as.floating};
        case AK_BOOLEAN: return (Symbol_Value){.kind = SK_BOOLEAN, .as.boolean = arg.as.boolean};
        case AK_OBJECT: return compute_indexing(symbols, arg.metadata, (Symbol_Value){.kind = SK_OBJECT, .as.object = arg.as.object});
        case AK_ARRAY: return compute_indexing(symbols, arg.metadata, (Symbol_Value){.kind = SK_ARRAY, .as.array = arg.as.array});
        case AK_PATH: return compute_variable_reference(symbols, arg.loc, arg.as.path, arg.metadata);
        case AK_FUN_CALL: return compute_indexing(symbols, arg.metadata, eval_builtin_fun_call(symbols, arg.loc, arg.as.fun_call));
        default: assertf(false, "unreacheable");
    }
}

long __bultin_fun_call_sum_ai(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AI" expects an array of integers as argument\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    if (fun_call->arguments.length > 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AI" expects only one argument which is an array of integers\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    Symbol_Value sym = reduce_argument(symbols, fun_call->arguments.data[0]);

    if (sym.kind != SK_ARRAY) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AI" expects an array of integer as argument but received \033[1;35m%s\033[0m\n",
            LOC_ERROR_ARG(loc),
            symbol_kind_name(sym.kind)
        );
        exit(1);
    }

    long sum = 0;

    for (size_t i = 0; i < sym.as.array.length; ++i) {
        Argument arg = sym.as.array.data[i];
        Symbol_Value value = reduce_argument(symbols, arg);

        if (value.kind != SK_INTEGER) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AI" expects an array of integer as argument but received \033[1;35m%s\033[0m at index %ld\n",
                LOC_ERROR_ARG(arg.loc),
                symbol_kind_name(value.kind),
                i
            );
            exit(1);
        }

        sum += value.as.integer.value;
    }

    return sum;
}

double __bultin_fun_call_sum_af(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AF" expects an array of floats as argument\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    if (fun_call->arguments.length > 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AF" expects only one argument which is an array of floats\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    Symbol_Value sym = reduce_argument(symbols, fun_call->arguments.data[0]);

    if (sym.kind != SK_ARRAY) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AF" expects an array of float as argument but received \033[1;35m%s\033[0m\n",
            LOC_ERROR_ARG(loc),
            symbol_kind_name(sym.kind)
        );
        exit(1);
    }

    double sum = 0;

    for (size_t i = 0; i < sym.as.array.length; ++i) {
        Argument arg = sym.as.array.data[i];
        Symbol_Value value = reduce_argument(symbols, arg);

        switch (value.kind) {
            case SK_FLOAT: sum += value.as.floating.value; break;
            case SK_INTEGER: sum += value.as.integer.value; break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_AF" expects an array of float as argument but received \033[1;35m%s\033[0m at index %ld\n",
                    LOC_ERROR_ARG(arg.loc),
                    symbol_kind_name(value.kind),
                    i
                );
                exit(1);
            }
        }
    }

    return sum;
}

Array __bultin_fun_call_concat_a(Symbols symbols, Location loc, Fun_Call *fun_call) {
    (void)loc;

    Array result = {0};

    for (size_t argument_index = 0; argument_index < fun_call->arguments.length; ++argument_index) {
        Argument arg_a = fun_call->arguments.data[argument_index];
        Symbol_Value arr = reduce_argument(symbols, arg_a);

        if (arr.kind != SK_ARRAY) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function "BUILTIN_FUN_CONCAT_A" \033[1;35m%s\033[0m is not an array\n",
                LOC_ERROR_ARG(arg_a.loc),
                symbol_kind_name(arr.kind)
            );
            exit(1);
        }


        for (size_t i = 0; i < arr.as.array.length; ++i) array_append(&result, arr.as.array.data[i]);
    }

    return result;
}

String __bultin_fun_call_concat_s(Symbols symbols, Location loc, Fun_Call *fun_call) {
    (void)loc;

    char *string = calloc(1, 1);
    size_t string_size = 1;

    for (size_t argument_index = 0; argument_index < fun_call->arguments.length; ++argument_index) {
        Argument arg = fun_call->arguments.data[argument_index];
        Symbol_Value value = reduce_argument(symbols, arg);

        if (value.kind != SK_STRING) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function "BUILTIN_FUN_CONCAT_S" \033[1;35m%s\033[0m is not a string\n",
                LOC_ERROR_ARG(arg.loc),
                symbol_kind_name(value.kind)
            );
            exit(1);
        }

        string_size += value.as.string.size;

        string = realloc(string, string_size);
        strncat(string, value.as.string.value, value.as.string.size);
    }

    string[string_size] = '\0';

    return (String){.value = string, .size = string_size};
}

String __bultin_fun_call_join_as(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length != 2) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_LEN" expects 2 arguments but received %ld\n",
            LOC_ERROR_ARG(loc),
            fun_call->arguments.length
        );
        exit(1);
    }

    char *string = calloc(1, 1);
    size_t string_size = 1;

    Argument arg1 = fun_call->arguments.data[0];
    Symbol_Value strings = reduce_argument(symbols, arg1);

    if (strings.kind != SK_ARRAY) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_JOIN_AS" \033[1;35m%s\033[0m is not an array\n",
            LOC_ERROR_ARG(arg1.loc),
            symbol_kind_name(strings.kind)
        );
        exit(1);
    }

    Argument arg2 = fun_call->arguments.data[1];
    Symbol_Value separator = reduce_argument(symbols, arg2);

    if (separator.kind != SK_STRING && separator.kind != SK_NIL) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_JOIN_AS" \033[1;35m%s\033[0m is not of type string?\n",
            LOC_ERROR_ARG(arg2.loc),
            symbol_kind_name(separator.kind)
        );
        exit(1);
    }

    for (size_t i = 0; i < strings.as.array.length; ++i) {
        Argument arg = strings.as.array.data[i];
        Symbol_Value value = reduce_argument(symbols, arg);

        if (value.kind != SK_STRING) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function "BUILTIN_FUN_JOIN_AS" \033[1;35m%s\033[0m is not a string\n",
                LOC_ERROR_ARG(arg.loc),
                symbol_kind_name(value.kind)
            );
            exit(1);
        }

        if (i > 0 && separator.kind == SK_STRING) {
            string_size += separator.as.string.size;

            string = realloc(string, string_size);
            strncat(string, separator.as.string.value, separator.as.string.size);
        }

        string_size += value.as.string.size;

        string = realloc(string, string_size);
        strncat(string, value.as.string.value, value.as.string.size);
    }

    string[string_size] = '\0';

    return (String){.value = string, .size = string_size};
}

Array __bultin_fun_call_keys(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length != 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_KEYS" expects 1 arguments but received %ld\n",
            LOC_ERROR_ARG(loc),
            fun_call->arguments.length
        );
        exit(1);
    }

    Argument arg = fun_call->arguments.data[0];
    Symbol_Value value = reduce_argument(symbols, arg);

    if (value.kind != SK_OBJECT) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_KEYS" \033[1;35m%s\033[0m is not an object\n",
            LOC_ERROR_ARG(arg.loc),
            symbol_kind_name(value.kind)
        );
        exit(1);
    }

    Array result = {0};

    for (size_t i = 0; i < value.as.object.length; ++i) {
        Var var = value.as.object.data[i];
        Argument argument = {
            .kind = AK_STRING,
            .loc = var.loc, // this is the wrong location
            .as.string = var.name
        };

        array_append(&result, argument);
    }

    return result;
}

long __bultin_fun_call_len(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length != 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_LEN" expects 1 argument but received %ld\n",
            LOC_ERROR_ARG(loc),
            fun_call->arguments.length
        );
        exit(1);
    }

    Symbol_Value sym = reduce_argument(symbols, fun_call->arguments.data[0]);

    switch (sym.kind) {
        case SK_ARRAY: return sym.as.array.length;
        case SK_STRING: return sym.as.string.size;
        default: {
            fprintf(
                stderr,
                LOC_ERROR_FMT" Function "BUILTIN_FUN_LEN" expects an array as argument but received \033[1;35m%s\033[0m\n",
                LOC_ERROR_ARG(loc),
                symbol_kind_name(sym.kind)
            );
            exit(1);
        }
    }

    return 0;
}

long __bultin_fun_call_sum_i(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length == 0) return 0;

    long sum = 0;

    for (size_t i = 0; i < fun_call->arguments.length; ++i) {
        bool computing = true;

        while (computing) {
            computing = false;

            Argument argument = fun_call->arguments.data[i];

            Symbol_Value value = reduce_argument(symbols, argument);

            switch (value.kind) {
                case SK_INTEGER: sum += value.as.integer.value; break;
                default: {
                    fprintf(
                        stderr,
                        LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_I" expects only integers as arguments but received a \033[1;35m%s\033[0m\n",
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
        Symbol_Value value = reduce_argument(symbols, argument);

        switch (value.kind) {
            case SK_INTEGER: sum += value.as.integer.value; break;
            case SK_FLOAT: sum += value.as.floating.value; break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" Function "BUILTIN_FUN_SUM_F" expects integers or floats as arguments but received a \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(argument.loc),
                    argument_kind_name(argument.kind) // TODO: display the wrong value
                );
                exit(1);
            };
        }
    }

    return sum;
}

long __bultin_fun_call_iota(Symbols symbols, Location loc, Fun_Call *fun_call) {
    (void)symbols;
    (void)loc;

    if (fun_call->arguments.length != 0) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" Function "BUILTIN_FUN_IOTA" expects 0 arguments but received %ld\n",
            LOC_ERROR_ARG(loc),
            fun_call->arguments.length
        );
        exit(1);
    }

    return __builtin_iota_current_value++;
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
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_SUM_AI, strlen(BUILTIN_FUN_SUM_AI))) {
        return (Symbol_Value){
            .kind = SK_INTEGER,
            .as.integer.value = __bultin_fun_call_sum_ai(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_SUM_AF, strlen(BUILTIN_FUN_SUM_AF))) {
        return (Symbol_Value){
            .kind = SK_FLOAT,
            .as.floating.value = __bultin_fun_call_sum_af(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_CONCAT_A, strlen(BUILTIN_FUN_CONCAT_A))) {
        return (Symbol_Value){
            .kind = SK_ARRAY,
            .as.array = __bultin_fun_call_concat_a(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_CONCAT_S, strlen(BUILTIN_FUN_CONCAT_S))) {
        return (Symbol_Value){
            .kind = SK_STRING,
            .as.string = __bultin_fun_call_concat_s(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_JOIN_AS, strlen(BUILTIN_FUN_JOIN_AS))) {
        return (Symbol_Value){
            .kind = SK_STRING,
            .as.string = __bultin_fun_call_join_as(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_KEYS, strlen(BUILTIN_FUN_KEYS))) {
        return (Symbol_Value){
            .kind = SK_ARRAY,
            .as.array = __bultin_fun_call_keys(symbols, loc, fun_call),
        };
    } else if (cmp_sized_strings(fun_call->name.value, fun_call->name.size, BUILTIN_FUN_IOTA, strlen(BUILTIN_FUN_IOTA))) {
        return (Symbol_Value){
            .kind = SK_INTEGER,
            .as.integer.value = __bultin_fun_call_iota(symbols, loc, fun_call),
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
                Symbol_Value reduced_value = reduce_argument(*symbols, arg);

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
            symbol.value = compute_variable_reference(symbols, var.loc, var.as.path, var.metadata);
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

    switch (var.kind) {
        case VK_FUN_CALL:
        case VK_ARRAY:
        case VK_OBJECT: {
            symbol.value = compute_indexing(symbols, var.metadata, symbol.value);
        } break;
        default: break;
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
