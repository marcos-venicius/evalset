#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "../symbols.h"

#define __BUILTIN_FUN_CALL_SUM "sum"
#define __S_BUILTIN_FUN_CALL_SUM 3

#define __BUILTIN_FUN_CALL_JOIN "join"
#define __S_BUILTIN_FUN_CALL_JOIN 4

Symbol_Value builtin_funcall__sum(Symbols symbols, Location loc, Fun_Call *fun_call);
Symbol_Value builtin_funcall__join(Symbols symbols, Location loc, Fun_Call *fun_call);

#endif // !FUNCTIONS_H_
