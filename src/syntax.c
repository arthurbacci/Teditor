#include "syntax.h"

/*
C and C++ syntax highlighting descriptor
*/

static struct KWD c_cpp_kwd[] = {
    KEYWORD("if", 0x10), KEYWORD("else", 0x10), KEYWORD("while", 0x10),
    KEYWORD("for", 0x10), KEYWORD("return", 0x10), KEYWORD("static", 0x10),
    KEYWORD("goto", 0x10), KEYWORD("switch", 0x10), KEYWORD("case", 0x10),
    KEYWORD("break", 0x10), KEYWORD("continue", 0x10),
    
    KEYWORD("#include", 0x10), KEYWORD("#define", 0x10), KEYWORD("#ifdef", 0x10),
    KEYWORD("#ifndef" , 0x10), KEYWORD("#endif" , 0x10),

    KEYWORD("int", 0x20), KEYWORD("char", 0x20), KEYWORD("unsigned", 0x20), 
    KEYWORD("long", 0x20), KEYWORD("double", 0x20), KEYWORD("float", 0x20), 
    KEYWORD("struct", 0x20), KEYWORD("const", 0x20), KEYWORD("void", 0x20), 
    
    OPERATOR("*" , 0x40), OPERATOR("," , 0x40), OPERATOR(";" , 0x40), 
    OPERATOR("!" , 0x40), OPERATOR("&&", 0x40), OPERATOR("||", 0x40),
    OPERATOR("&" , 0x40), OPERATOR("|" , 0x40), OPERATOR("~" , 0x40),
    OPERATOR("^" , 0x40), OPERATOR(">>", 0x40), OPERATOR("<<", 0x40),
    OPERATOR("+" , 0x40), OPERATOR("-" , 0x40), OPERATOR("/" , 0x40),
    OPERATOR("->", 0x40), OPERATOR("=" , 0x40), OPERATOR("!=", 0x40),
    OPERATOR("<" , 0x40), OPERATOR(">" , 0x40),
    
    OPERATOR("(", 0x50), OPERATOR(")", 0x50),
    OPERATOR("{", 0x50), OPERATOR("}", 0x50),
    OPERATOR("[", 0x50), OPERATOR("]", 0x50),
};

static const char *c_cpp_exts[] = {"c", "h", "cpp", "hpp", "cc", "hh"};

static struct SHD c_cpp_syntax = {
    "C/C++",
    sizeof c_cpp_exts / sizeof *c_cpp_exts, c_cpp_exts,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?", // Characters that separates words
    sizeof c_cpp_kwd / sizeof *c_cpp_kwd, c_cpp_kwd, //Keywords
    0x60, 0x50, 0x05,
    "\"\'", // Strings charaters
    "//", {"/*", "*/"}, // Comments
    {"{[(", ")]}"}
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

    KEYWORD("import", 0x40), KEYWORD("self", 0x40),
    
    OPERATOR("(", 0x50), OPERATOR(")", 0x50),
    OPERATOR("{", 0x50), OPERATOR("}", 0x50),
    OPERATOR("[", 0x50), OPERATOR("]", 0x50)
};

static const char *python_exts[] = {"py", "py3", "pyw", "pyd", "pyde"};

static struct SHD python_syntax = {
    "Python",
    sizeof python_exts / sizeof *python_exts, python_exts,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/", // Characters that separates words
    sizeof python_kwd / sizeof *python_kwd, python_kwd, //Keywords
    0x40, 0x50, 0x05,
    "\"\'`", // Strings charaters
    "#", {"", ""}, // Comments
    {"{[(", ")]}"}
};

/*
Shell syntax highlighting descriptor
*/

static struct KWD sh_kwd[] = {
    KEYWORD("if", 0x10), KEYWORD("then", 0x10), KEYWORD("else", 0x10),
    KEYWORD("fi", 0x10), KEYWORD("while", 0x10), KEYWORD("for", 0x10),
    KEYWORD("do", 0x10), KEYWORD("done", 0x10), KEYWORD("break", 0x10),
    KEYWORD("continue", 0x10), KEYWORD("return", 0x10), KEYWORD("in", 0x10),
    KEYWORD("case", 0x10), KEYWORD("esac", 0x10), KEYWORD("until", 0x10),
    KEYWORD("function", 0x10), KEYWORD("elif", 0x10),

    KEYWORD("$?", 0x20), KEYWORD("..", 0x20),  KEYWORD(".", 0x20),
    KEYWORD("alias", 0x30), KEYWORD("bg", 0x30), KEYWORD("bind", 0x30), //builtins
    KEYWORD("builtin", 0x30), KEYWORD("cd", 0x30), KEYWORD("command", 0x30),
    KEYWORD("continue", 0x30), KEYWORD("declare", 0x30), KEYWORD("dirs", 0x30),
    KEYWORD("disown", 0x30), KEYWORD("echo", 0x30), KEYWORD("enable", 0x30),
    KEYWORD("eval", 0x30), KEYWORD("exec", 0x30), KEYWORD("exit", 0x30),
    KEYWORD("export", 0x30), KEYWORD("false", 0x30), KEYWORD("fc", 0x30),
    KEYWORD("fg", 0x30), KEYWORD("getopts", 0x30), KEYWORD("hash", 0x30),
    KEYWORD("help", 0x30), KEYWORD("history", 0x30), KEYWORD("jobs", 0x30),
    KEYWORD("kill", 0x30), KEYWORD("let", 0x30), KEYWORD("local", 0x30),
    KEYWORD("logout", 0x30), KEYWORD("popd", 0x30), KEYWORD("pushd", 0x30),
    KEYWORD("printf", 0x30), KEYWORD("pwd", 0x30), KEYWORD("read", 0x30),
    KEYWORD("readarray", 0x30), KEYWORD("readonly", 0x30), KEYWORD("set", 0x30),
    KEYWORD("shift", 0x30), KEYWORD("source", 0x30), KEYWORD("suspend", 0x30),
    KEYWORD("test", 0x30), KEYWORD("times", 0x30), KEYWORD("time", 0x30),
    KEYWORD("trap", 0x30), KEYWORD("true", 0x30), KEYWORD("type", 0x30),
    KEYWORD("ulimit", 0x30), KEYWORD("umask", 0x30), KEYWORD("unalias", 0x30),
    KEYWORD("unset", 0x30), KEYWORD("wait", 0x30), KEYWORD("shopt", 0x30),
    
    OPERATOR("$", 0x30), OPERATOR(";", 0x30), OPERATOR("=", 0x30),
    OPERATOR("<<-", 0x30), OPERATOR("!", 0x30), OPERATOR("&", 0x30),
    OPERATOR("&&", 0x30), OPERATOR("|", 0x30), OPERATOR("||", 0x30),
    OPERATOR("<", 0x30), OPERATOR(">", 0x30), OPERATOR("2>", 0x30),

    OPERATOR("(", 0x50), OPERATOR(")", 0x50),
    OPERATOR("{", 0x50), OPERATOR("}", 0x50),
    OPERATOR("[", 0x50), OPERATOR("]", 0x50),
};

static const char *sh_exts[] = {"sh", "zsh"};

static struct SHD sh_syntax = {
    "Shell",
    sizeof sh_exts / sizeof *sh_exts, sh_exts,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?", // Characters that separates words
    sizeof sh_kwd / sizeof *sh_kwd, sh_kwd, //Keywords
    0x40, 0x50, 0x05,
    "\"\'`", // Strings charaters
    "#", {": '", "'"}, // Comments
    {"{[(", ")]}"}
};

/*
Default syntax
*/

struct SHD default_syntax = {
    "Default",
    0, NULL,
    " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?",
    0, NULL,
    0, 0, 0,
    "",
    "", {"", ""},
    {"", ""}
};

/*
Global syntaxes
*/

struct SHD *syntaxes[] = {&c_cpp_syntax, &python_syntax, &sh_syntax};

void register_syntax(void) {
    config.syntaxes = syntaxes;
    config.syntax_len = sizeof syntaxes / sizeof *syntaxes;
}
