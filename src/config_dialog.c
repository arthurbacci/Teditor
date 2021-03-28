#include "ted.h"
#include <strings.h>

static void tablen(char **words, unsigned int words_len) {
    if (words_len == 0) {
        beep();
        return;
    } else if (words_len == 1) {
        int answer_int = atoi(words[0]);

        if (answer_int > 0) {
            config.tablen = answer_int;
            return;
        }
    }
    beep();
}

static void linebreak(char **words, unsigned int words_len) {
    if (words_len == 0) {
        beep();
        return;
    } else if (words_len == 1) {
        if (strcasecmp(words[0], "LF") == 0)
            config.line_break_type = 0;

        else if (strcasecmp(words[0], "CRLF") == 0)
            config.line_break_type = 1;

        else if (strcasecmp(words[0], "CR") == 0)
            config.line_break_type = 2;

        return;
    }
    beep();
}

static void use_spaces(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (strcasecmp(words[0], "TRUE") == 0 || strcmp(words[0], "1") == 0)
            config.use_spaces = 1;

        else if (strcasecmp(words[0], "FALSE") == 0 || strcmp(words[0], "0") == 0)
            config.use_spaces = 0;

        return;
    }
    beep();
}

static void autotab(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (strcasecmp(words[0], "TRUE") == 0 || strcmp(words[0], "1") == 0)
            config.autotab = 1;

        else if (strcasecmp(words[0], "FALSE") == 0 || strcmp(words[0], "0") == 0)
            config.autotab = 0;

        printf("\n%d\n", config.autotab);
        return;
    }
    beep();
}

static void save_as(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (needs_to_free_filename)
            free(filename);

        unsigned int size = (strlen(words[0]) + 1) * sizeof(char);
        filename = malloc(size);
        memcpy(filename, words[0], size);
        filename[size - 1] = '\0';

        needs_to_free_filename = 1;
        read_only = 0; // todo: check if the new filename is read only
        savefile();
    } else
        beep();
}

static void manual(char **words, unsigned int words_len) {
    if (words_len == 0) {
        openFile(home_path(".config/ted/docs/help.txt"), 1);
        read_only = 1;

    } else if (words_len == 1) {
        char fname[1000];
        snprintf(fname, 1000, ".config/ted/docs/%s.txt", words[0]);
        openFile(home_path(fname), 1);
        read_only = 1;
    } else
        beep();
}

static void syntax(char **words, unsigned int words_len) {
    if (words_len == 0) {
        beep();
        return;
    }

    if (strcmp(words[0], "off") == 0 && words_len == 1) {
        config.syntax_on = 0;
    } else if (strcmp(words[0], "on") == 0 && words_len == 1) {
        if (config.current_syntax == NULL)
            beep();
        else
            config.syntax_on = 1;

    } else if (strcmp(words[0], "set") == 0 && words_len == 2) {
        char *str = malloc(strlen(words[1]) + 2);
        *str = '.';
        strcpy(str + 1, words[1]);
        struct SHD *current = config.current_syntax;

        if (!detect_extension(str)) {
            // dont reset syntax if syntax name doesn't exist
            config.current_syntax = current;
            beep();
        }

        free(str);
    } else
        beep();
}

struct {
    const char *name;
    void (*function)(char **words, unsigned int words_len);
} fns[] = {
    {"tablen"    , tablen    },
    {"linebreak" , linebreak },
    {"use-spaces", use_spaces},
    {"autotab"   , autotab   },
    {"save-as"   , save_as   },
    {"manual"    , manual    },
    {"syntax"    , syntax    }
};

struct HINTS hints[] = {
    {"tablen"    ,  6, " <tablen>"                   },
    {"linebreak" ,  9, " <LF, CR, CRLF>"             },
    {"use-spaces", 10, " <0/FALSE, 1/TRUE "          },
    {"autotab"   ,  7, " <0/FALSE, 1/TRUE>"          },
    {"save-as"   ,  7, " <filename>"                 },
    {"manual"    ,  6, " <page (nothing for index)>" },
    {"syntax"    ,  6, " {set <language>, off, on}"  },
    {NULL        ,  0, NULL                          }
};

void config_dialog(void) {
    char *base_hint = "{tablen, linebreak, use-spaces, autotab, save-as, manual, syntax}";

    char *command = prompt_hints("Enter command: ", "", base_hint, hints);
    char *command_ptr = command;
    
    if (!command) {
        beep();
        return;
    }

    while (isspace(*command_ptr)) ++command_ptr;
    char *save = NULL;
    char *word = split_spaces(command_ptr, &save);

    if (word == NULL) {
        beep();
        free(command);
        return;
    }

    // initially allocate a buffer of size 10
    char **words = malloc(sizeof(char*) * 10);
    char **words_ptr = words;
    unsigned int words_len = 0;

    while (word != NULL) {
        if (++words_len > (words_ptr - words)) {
            words = realloc(words, words_len * sizeof(char*));
            words_ptr = words + words_len - 1;
        }

        *words_ptr++ = word;
        word = split_spaces(NULL, &save);
    }

    const unsigned int fnslen = sizeof fns / sizeof *fns;
    for (unsigned int i = 0; i < fnslen; i++) {
        if (strcmp(words[0], fns[i].name) == 0) {
            fns[i].function(words + 1, words_len - 1);
            goto out;
        }
    }
    beep();

out:;
    free(command);
    free(words);
}
