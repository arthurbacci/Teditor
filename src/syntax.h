#ifndef SYNTAX_HEADER
#define SYNTAX_HEADER

#include "ted.h"

#define KEYWORD(name, color)        {name, color, 0}
#define OPERATOR(operator, color)   {operator, color, 1}

extern struct SHD *syntaxes[];

void register_syntax(void);

#endif
