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
    
    KEYWORD("#include", 0x40), KEYWORD("#define", 0x40), KEYWORD("#ifdef", 0x40),
    KEYWORD("#ifndef", 0x40), KEYWORD("#endif", 0x40)
};

static const char *c_cpp_exts[] = {"c", "h", "cpp", "hpp", "cc", "hh"};

static struct SHD c_cpp_syntax = {
    sizeof c_cpp_exts / sizeof *c_cpp_exts, c_cpp_exts,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?", // Characters that separates words
    sizeof c_cpp_kwd / sizeof *c_cpp_kwd, c_cpp_kwd, //Keywords
    0x40, 0x50, 0x05,
    "\"\'", // Strings charaters
    "//", {"/*", "*/"}, // Comments
    {"{[(<", ">)]}"}
};

/*
Python syntax highlighting descriptor
*/

static struct KWD python_kwd[] = {
    KEYWORD("if", 0x10), KEYWORD("else", 0x10), KEYWORD("elif", 0x10),
    KEYWORD("while", 0x10), KEYWORD("for", 0x10), KEYWORD("in", 0x10),
    KEYWORD("with", 0x10), KEYWORD("class", 0x10), KEYWORD("return", 0x10),
    KEYWORD("try", 0x10), KEYWORD("except", 0x10), KEYWORD("def", 0x10),

    KEYWORD("None", 0x20), KEYWORD("False", 0x20), KEYWORD("True", 0x20),

    OPERATOR("*", 0x30), OPERATOR(";", 0x30), OPERATOR(",", 0x30),

    KEYWORD("import", 0x40), KEYWORD("self", 0x40)

//    OPERATOR("(", 0x50), OPERATOR(")", 0x50)
//    FIXME
};

static const char *python_exts[] = {"py", "py3", "pyw", "pyd", "pyde"};

static struct SHD python_syntax = {
    sizeof python_exts / sizeof *python_exts, python_exts,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?", // Characters that separates words
    sizeof python_kwd / sizeof *python_kwd, python_kwd, //Keywords
    0x40, 0x50, 0x05,
    "\"\'`", // Strings charaters
    "#", {"\"\"\"", "\"\"\""}, // Comments
    // TODO ''' is also a multi-line string, need to allow multiple types of comments, like {{"\"\"\"", "\"\"\""}, {"'''", "'''"}}
    {"{[(", ")]}"}
};


/*
Global syntaxes
*/
struct SHD *syntaxes[] = {&c_cpp_syntax, &python_syntax};

void register_syntax(void) {
    config.syntaxes = syntaxes;
    config.syntax_len = sizeof syntaxes / sizeof *syntaxes;
}
