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
    // 'data' should not be freed here
}

struct {
    const char *name;
    const char *message;
    void (*function)(char *data);
} fns[] = {
    {"tablen"    , "tablen: "                      , tablen    },
    {"linebreak" , "linebreak (LF, CR, CRLF): "    , linebreak },
    {"use-spaces", "use-spaces (0/FALSE, 1/TRUE): ", use_spaces},
    {"autotab"   , "autotab (0/FALSE, 1/TRUE): "   , autotab   },
    {"save-as"   , "save-as: "                     , save_as   }
};

void config_dialog() {
    char msg[1000];
    snprintf(msg, 1000, "Configure: ");

    char *answer = prompt(msg);

    if (answer == NULL) {
        beep();
        return;
    }
    
    bool did = 0;
    const unsigned int fnslen = sizeof fns / sizeof *fns;
    for (unsigned int i = 0; i < fnslen; i++) {
        if (strcmp(answer, fns[i].name) == 0) {
            char msg1[1000];
            strcpy(msg1, fns[i].message);
            char *answer1 = prompt(msg1);
            
            if (answer1 == NULL)
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
