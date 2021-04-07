#include "ted.h"

static void tablen(char **words, unsigned int words_len) {
    if (words_len == 1) {
        int answer_int = atoi(words[0]);
        if (answer_int > 0)
            config.tablen = answer_int;
    }
}

static void linebreak(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "LF"))
            config.line_break_type = 0;
            
        else if (!strcasecmp(words[0], "CRLF"))
            config.line_break_type = 1;

        else if (!strcasecmp(words[0], "CR"))
            config.line_break_type = 2;
    }
}

static void insert_newline(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "TRUE") || !strcmp(words[0], "1"))
            config.insert_newline = 1;
        else if (!strcasecmp(words[0], "FALSE") || !strcmp(words[0], "0"))
            config.insert_newline = 0;
    }
}

static void use_spaces(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "TRUE") || !strcmp(words[0], "1"))
            config.use_spaces = 1;
        else if (!strcasecmp(words[0], "FALSE") || !strcmp(words[0], "0"))
            config.use_spaces = 0;
    }
}

static void autotab(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "TRUE") || !strcmp(words[0], "1"))
            config.autotab = 1;
        else if (!strcasecmp(words[0], "FALSE") || !strcmp(words[0], "0"))
            config.autotab = 0;
    }
}

static void automatch(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "TRUE") || !strcmp(words[0], "1"))
            config.automatch = 1;
        else if (!strcasecmp(words[0], "FALSE") || !strcmp(words[0], "0"))
            config.automatch = 0;
    }
}

static void save_as(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (needs_to_free_filename)
            free(filename);

        unsigned int size = (strlen(words[0]) + 1) * sizeof(char);
        filename = malloc(size);
        memcpy(filename, words[0], size);
        needs_to_free_filename = 1;
        detect_read_only(filename);

        if (config.selected_buf.read_only) message("Can't save as a read-only file.");
        else savefile();
    }
}

static void manual(char **words, unsigned int words_len) {
    if (words_len == 0) {
        openFile(home_path(".config/ted/docs/help.txt"), 1);
        config.selected_buf.read_only = 1;
    } else if (words_len == 1) {
        char fname[1000];
        snprintf(fname, 1000, ".config/ted/docs/%s.txt", words[0]);
        openFile(home_path(fname), 1);
        config.selected_buf.read_only = 1;
    }
}

static void syntax(char **words, unsigned int words_len) {
    if (words_len == 1) {
        char *str = malloc(strlen(words[0]) + 2);
        *str = '.';
        strcpy(str + 1, words[0]);

        if (!detect_extension(str))
            config.current_syntax = &default_syntax;
            
        free(str);
    } else
        config.current_syntax = &default_syntax;
    
    for (unsigned int at = 0; at < num_lines; at++) // reset to white all lines
        memset(lines[at].color, 0, (lines[at].length + 1) * sizeof(*lines[at].color));

    memset(&lines[0].state, 0, sizeof(lines[0].state)); // reset first line state
    set_syntax_change(0, 0);
}

static void read_only(char **words, unsigned int words_len) {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "TRUE") || !strcmp(words[0], "1"))
            config.selected_buf.read_only = 1;
        else if (!strcasecmp(words[0], "FALSE") || !strcmp(words[0], "0")) {
            if (config.selected_buf.can_write)
                config.selected_buf.read_only = 0;
            else
                message("Can't unlock buffer without write permission");
        }
    }
}

static void find(char **words, unsigned int words_len) {
    int from_cur = 0;
    if (words_len == 2 && !strcmp(words[0], "cursor"))
        from_cur = 1;
    if (words_len == 1 || words_len == 2) {
        unsigned int len = strlen(words[words_len - 1]);
        int index;
        for (unsigned int at = from_cur ? cy : 0; at < num_lines; ++at) {
            if (lines[at].length >= len &&
                (index = uchar32_sub(from_cur && at == cy ? &lines[at].data[cx] : lines[at].data, words[words_len - 1], lines[at].length, len)) >= 0
            ) {
                change_position(index + len + (from_cur && at == cy) * cx, at);
                return;
            }
        }
        message("Substring not found");
    }
}

static void eof(char **words, unsigned int words_len) {
    if (words_len == 0)
        change_position(lines[num_lines - 1].length, num_lines);
}

struct {
    const char *name;
    void (*function)(char **words, unsigned int words_len);
} fns[] = {
    {"tablen"           , tablen            },
    {"linebreak"        , linebreak         },
    {"insert-newline"   , insert_newline    },
    {"use-spaces"       , use_spaces        },
    {"autotab"          , autotab           },
    {"automatch"        , automatch         },
    {"save-as"          , save_as           },
    {"manual"           , manual            },
    {"syntax"           , syntax            },
    {"read-only"        , read_only         },
    {"find"             , find              },
    {"eof"              , eof               },
    {NULL, NULL}
};

struct HINTS hints[] = {
    {"tablen"           , " <tablen>"                           },
    {"linebreak"        , " {LF, CR, CRLF}"                     },
    {"insert-newline"   , " {0/FALSE, 1/TRUE}"                  },
    {"use-spaces"       , " {0/FALSE, 1/TRUE}"                  },
    {"autotab"          , " {0/FALSE, 1/TRUE}"                  },
    {"automatch"        , " {0/FALSE, 1/TRUE}"                  },
    {"save-as"          , " <filename>"                         },
    {"manual"           , " <page (nothing for index)>"         },
    {"syntax"           , " <language (nothing for disabling)>" },
    {"read-only"        , " {0/FALSE, 1/TRUE}"                  },
    {"find"             , " {start, cursor} <substring>"        },
    {NULL, NULL}
};

char *base_hint = "{repeat, tablen, linebreak, insert-newline, use-spaces, autotab, automatch, save-as, manual, syntax, read-only, find, eof}";

char last_command[1000] = "";

void config_dialog(void) {
    char *command = prompt_hints("Enter command: ", "", base_hint, hints);

    parse_command(command);

    free(command);
}


void parse_command(char *command) {
    if (!command)
        return;


    int words_len;
    char **words = split_str(command, &words_len);

    if (run_command(words, words_len))
        parse_command(last_command);
    else if (command != last_command)
        strcpy(last_command, command);

    for (int i = 0; i < words_len; i++)
        free(words[i]);
    free(words);
}


bool run_command(char **words, int words_len) {
    if (words_len == 1 && !strcmp(words[0], "repeat"))
        return 1;
    for (unsigned int i = 0; fns[i].name; i++) {
        if (!strcmp(words[0], fns[i].name)) {
            fns[i].function(words + 1, words_len - 1);
                return 0;
        }
    }
    return 0;
}
