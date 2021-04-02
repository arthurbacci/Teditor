#ifndef SYNTAX_HEADER
#define SYNTAX_HEADER

#include "ted.h"

#define KEYWORD(name, color)    {name, color, sizeof(name) - sizeof(char), 0}
#define OPERATOR(op, color)     {op, color, sizeof(op) - sizeof(char), 1}

#define COLOR(fg, bg) ((fg) << 4) + (bg)

// to keep update with palette in show_lines
#define PALETTE_OFF         0
#define PALETTE_RED         1
#define PALETTE_BLUE        2
#define PALETTE_GREEN       3
#define PALETTE_MAGENTA     4
#define PALETTE_CYAN        5
#define PALETTE_YELLOW      6

extern struct SHD *syntaxes[];

void register_syntax(void);

#endif
