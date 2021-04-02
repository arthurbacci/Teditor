#ifndef SYNTAX_HEADER
#define SYNTAX_HEADER

#include "ted.h"

#define KEYWORD(name, color)    {name, color, sizeof(name) - sizeof(char), 0}
#define OPERATOR(op, color)     {op, color, sizeof(op) - sizeof(char), 1}
#define STRMATCH(name)          {name, sizeof(name) - sizeof(char)}

#define PALETTE_COLOR(fg, bg) ((fg) << 4) + (bg)

// to keep update with palette in show_lines
#define PALETTE_OFF             0
#define PALETTE_RED             1
#define PALETTE_GREEN           2
#define PALETTE_YELLOW          3
#define PALETTE_BLUE            4
#define PALETTE_MAGENTA         5
#define PALETTE_CYAN            6
#define PALETTE_GRAY            7
#define PALETTE_BRIGHT_RED      8
#define PALETTE_BRIGHT_GREEN    9
#define PALETTE_BRIGHT_YELLOW   10
#define PALETTE_BRIGHT_BLUE     11
#define PALETTE_BRIGHT_MAGENTA  12
#define PALETTE_BRIGHT_CYAN     13

extern struct SHD *syntaxes[];

void register_syntax(void);

#endif
