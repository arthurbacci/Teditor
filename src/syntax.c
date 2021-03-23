#include "syntax.h"

/*
C and C++ syntax highlighting descriptor
*/

static struct KWD c_cpp_kwd[] = {
    KEYWORD("if", 0x10), KEYWORD("else", 0x10), KEYWORD("while", 0x10),
    KEYWORD("for", 0x10), KEYWORD("return", 0x10), KEYWORD("static", 0x10),

    KEYWORD("int", 0x20), KEYWORD("char", 0x20), KEYWORD("unsigned", 0x20), 
    KEYWORD("long", 0x20), KEYWORD("double", 0x20), KEYWORD("float", 0x20), 
    KEYWORD("struct", 0x20), KEYWORD("const", 0x20), KEYWORD("void", 0x20), 

    OPERATOR("*", 0x30), OPERATOR(",", 0x30), OPERATOR(";", 0x30), 
    OPERATOR("!", 0x30), OPERATOR("&&", 0x30), OPERATOR("||", 0x30),
    OPERATOR("&", 0x30), OPERATOR("|", 0x30), OPERATOR("~", 0x30),
    OPERATOR("^", 0x30), OPERATOR(">>", 0x30), OPERATOR("<<", 0x30),
    OPERATOR("+", 0x30), OPERATOR("-", 0x30), OPERATOR("/", 0x30),
};

static const char *c_cpp_exts[] = {"c", "h", "cpp", "hpp", "cc", "hh"};

static struct SHD c_cpp_syntax = {
    sizeof c_cpp_exts / sizeof *c_cpp_exts, c_cpp_exts,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?", // Characters that separates words
    sizeof c_cpp_kwd / sizeof *c_cpp_kwd, c_cpp_kwd, //Keywords
    0x40, 0x50, 0x05,
    "\"\'`", // Strings charaters
    "//", {"/*", "*/"}, // Comments
    {"{[(", ")]}"}
};

/*
Global syntaxes
*/
struct SHD *syntaxes[] = {&c_cpp_syntax};

void register_syntax(void) {
    config.syntaxes = syntaxes;
    config.syntax_len = sizeof syntaxes / sizeof *syntaxes;
}
