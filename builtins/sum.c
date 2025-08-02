#include "./functions.h"
#include "../loc.h"
#include <stdio.h>
#include <stdlib.h>

Symbol_Value reduce_argument(Symbols symbols, Argument arg);
const char *symbol_kind_name(Symbol_Kind kind);

Symbol_Value builtin_funcall__sum(Symbols symbols, Location loc, Fun_Call *fun_call) {
    (void)loc;

    if (fun_call->arguments.length == 0) {
        return (Symbol_Value){
            .kind = SK_INTEGER,
            .as.integer.value = 0
        };
    }

    double result = 0;

    Symbol_Value arg1 = reduce_argument(symbols, fun_call->arguments.data[0]);

    bool has_float = false;
    bool is_array = arg1.kind == SK_ARRAY;

    if (is_array && fun_call->arguments.length > 1) {
        fprintf(
            stderr,
            LOC_ERROR_FMT" when using sum with arrays, you cannot specify more than one argument\n",
            LOC_ERROR_ARG(fun_call->arguments.data[1].loc)
        );
        exit(1);
    }

    Array array = fun_call->arguments;

    if (is_array) array = arg1.as.array;

    for (size_t i = 0; i < array.length; ++i) {
        Argument argument = array.data[i];
        Symbol_Value arg = reduce_argument(symbols, argument);

        switch (arg.kind) {
            case SK_INTEGER: result += arg.as.integer.value; break;
            case SK_FLOAT: {
                has_float = true;
                result += arg.as.floating.value;
            } break;
            default: {
                fprintf(
                    stderr,
                    LOC_ERROR_FMT" you cannot sum the type \033[1;35m%s\033[0m\n",
                    LOC_ERROR_ARG(argument.loc),
                    symbol_kind_name(arg.kind)
                );
                exit(1);
            } break;
        }
    }

    if (has_float) {
        return (Symbol_Value){
            .kind = SK_FLOAT,
            .as.floating.value = result
        };
    }

    return (Symbol_Value){
        .kind = SK_INTEGER,
        .as.integer.value = result
    };
}
