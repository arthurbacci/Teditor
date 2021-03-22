#include "ted.h"

void tablen(char *data) {
    const int answer_int = atoi(data);
    
    if (answer_int > 0)
        config.tablen = answer_int;
    else
        beep();
    
    free(data);
}
void linebreak(char *data) {
    if (strcmp(data, "LF") == 0)
        config.line_break_type = 0;
    else if (strcmp(data, "CRLF") == 0)
        config.line_break_type = 1;
    else if (strcmp(data, "CR") == 0)
        config.line_break_type = 2;
    else
        beep();
    free(data);
}
void use_spaces(char *data) {
    if (strcmp(data, "TRUE") == 0 || strcmp(data, "1") == 0)
        config.use_spaces = 1;
    else if (strcmp(data, "FALSE") == 0 || strcmp(data, "0") == 0)
        config.use_spaces = 0;
    else
        beep();
    free(data);
}
void autotab(char *data) {
    if (strcmp(data, "TRUE") == 0 || strcmp(data, "1") == 0)
        config.autotab = 1;
    else if (strcmp(data, "FALSE") == 0 || strcmp(data, "0") == 0)
        config.autotab = 0;
    else
        beep();
    free(data);
}
void save_as(char *data) {
    if (needs_to_free_filename)
        free(filename);
    filename = data;
    needs_to_free_filename = 1;
    savefile();
    // 'data' should not be freed here
}
void manual(char *data) {
    if (!*data) {
        openFile(home_path(".config/ted/docs/help.txt"), 1);
    } else {
        char fname[1000];
        snprintf(fname, 1000, ".config/ted/docs/%s.txt", data);
        openFile(home_path(fname), 1);
    }
    free(data);
}
void syntax (char *data) {
    if (!*data) {
        config.syntax_on = 0;
        free(data);
        return;
    }

    unsigned int len = (unsigned int)strlen(data);
    for (unsigned int i = 0; i < config.syntax_len; ++i) {
        struct SHD *syntax = config.syntaxes[i];

        for (unsigned int j = 0; j < syntax->exts_len; ++j) {
            unsigned int ext_len = (unsigned int)strlen(syntax->extensions[j]);
            if (len == ext_len && strcmp(data, syntax->extensions[j]) == 0) {
                config.syntax_on = 1;
                config.current_syntax = syntax;
                syntaxHighlight();
                free(data);
                return;
            }
        }
    }

    beep();
    free(data);
}

struct {
    const char *name;
    const char *message;
    void (*function)(char *data);
} fns[] = {
    {"tablen"    , "tablen: "                                , tablen    },
    {"linebreak" , "linebreak (LF, CR, CRLF): "              , linebreak },
    {"use-spaces", "use-spaces (0/FALSE, 1/TRUE): "          , use_spaces},
    {"autotab"   , "autotab (0/FALSE, 1/TRUE): "             , autotab   },
    {"save-as"   , "save-as: "                               , save_as   },
    {"manual"    , "manual page (blank for index): "         , manual    },
    {"syntax"    , "syntax highlight (blank for disabling): ", syntax    }
};

void config_dialog(void) {
    char *answer = prompt("Configure: ", "");

    if (!answer) {
        beep();
        return;
    }
    
    bool did = 0;
    const unsigned int fnslen = sizeof fns / sizeof *fns;
    for (unsigned int i = 0; i < fnslen; i++) {
        if (strcmp(answer, fns[i].name) == 0) {
            char *answer1 = prompt(fns[i].message, strcmp(fns[i].name, "save-as") == 0 ? filename : "");
            
            if (!answer1)
                beep();
            else
                fns[i].function(answer1);
            
            did = 1;
            break;
        }
    }
    
    if (!did)
        beep();
    
    free(answer);
}
