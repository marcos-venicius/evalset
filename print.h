#ifndef PRINT_H_
#define PRINT_H_

#include <stdbool.h>
#include "./parser.h"

void print_var(Var var, bool is_inside_array, int level);

#endif // !PRINT_H_
