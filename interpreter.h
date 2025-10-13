#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "./symbols.h"
#include "./parser.h"

Symbols interpret(const Var *vars, size_t length);

#endif // !INTERPRETER_H_
