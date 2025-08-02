#include "./functions.h"
#include "../loc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

Symbol_Value reduce_argument(Symbols symbols, Argument arg);
const char *symbol_kind_name(Symbol_Kind kind);

static String concat_strings(Symbols symbols, Array array, Symbol_Value separator) {
    char *string = calloc(1, 1);
    size_t string_size = 1;

    for (size_t i = 0; i < array.length; ++i) {
        Argument arg = array.data[i];
        Symbol_Value value = reduce_argument(symbols, arg);

        if (value.kind != SK_STRING) {
            fprintf(
                stderr,
                LOC_ERROR_FMT" \033[1;35mjoin\033[0m has an invalid signature \033[1;35m%s\033[0m is not a string\n",
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

    return (String){
        .value = string,
        .size = string_size
    };

}

Symbol_Value builtin_funcall__join(Symbols symbols, Location loc, Fun_Call *fun_call) {
    if (fun_call->arguments.length < 2) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" invalid \033[1;35mjoin\033[0m signature\n",
            LOC_ERROR_ARG(loc)
        );
        exit(1);
    }

    Symbol_Value arg1 = reduce_argument(symbols, fun_call->arguments.data[0]);
    Symbol_Value arg2 = reduce_argument(symbols, fun_call->arguments.data[1]);

    if (fun_call->arguments.length == 2 && arg1.kind == SK_ARRAY && (arg2.kind == SK_STRING || arg2.kind == SK_NIL)) {
        String string = concat_strings(symbols, arg1.as.array, arg2);

        return (Symbol_Value){
            .kind = SK_STRING,
            .as.string = string
        };
    }
    
    if (arg1.kind == SK_ARRAY && arg2.kind == SK_ARRAY) {
        Array result = arg1.as.array;

        for (size_t i = 0; i < arg2.as.array.length; ++i) {
            array_append(&result, arg2.as.array.data[i]);
        }

        for (size_t i = 2; i < fun_call->arguments.length; ++i) {
            Argument arg = fun_call->arguments.data[i];
            Symbol_Value sym = reduce_argument(symbols, arg);

            if (sym.kind != SK_ARRAY) {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" invalid \033[1;35mjoin\033[0m signature\n",
                    LOC_ERROR_ARG(arg.loc)
                );
                exit(1);
            }

            for (size_t j = 0; j < sym.as.array.length; ++j) {
                array_append(&result, sym.as.array.data[j]);
            }
        }

        return (Symbol_Value){
            .kind = SK_ARRAY,
            .as.array = result
        };
    } else if (arg1.kind == SK_STRING) {
        String string = concat_strings(symbols, fun_call->arguments, (Symbol_Value){.kind = SK_NIL});

        return (Symbol_Value){
            .kind = SK_STRING,
            .as.string = string
        };
    }

    fprintf(
        stderr,
        LOC_ERROR_FMT" invalid \033[1;35mjoin\033[0m signature\n",
        LOC_ERROR_ARG(loc)
    );
    exit(1);
}
