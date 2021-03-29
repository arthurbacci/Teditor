#ifndef SYNTAX_HEADER
#define SYNTAX_HEADER

#include "ted.h"

#define KEYWORD(name, color)    {name, color, sizeof(name) - sizeof(char), 0}
#define OPERATOR(op, color)     {op, color, sizeof(op) - sizeof(char), 1}

extern struct SHD *syntaxes[];

void register_syntax(void);

#endif
