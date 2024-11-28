#include "ted.h"


// TODO: error messages for incomplete commands (right now they're being
// completely ignored)

#define BOOL_COMMAND(a, b) \
    if (words_len == 1) { \
        int r = string_to_bool(words[0]); \
        if (r == 1) a \
        else if (r == 0) b \
    }

#define BOOL_SET(a) BOOL_COMMAND((a) = 1;, (a) = 0;);

#define DEF_COMMAND(a, b) \
    static void (a)(char words[][CMD_WORD_SZ], size_t words_len) { \
        PRETEND_TO_USE(words); \
        PRETEND_TO_USE(words_len); \
        b \
    }



DEF_COMMAND(tabwidth, {
    if (words_len == 1) {
        int answer_int = atoi(words[0]);
        if (answer_int > 0 && answer_int < 256)
            SEL_BUF.tab_width = answer_int;
    }
})
DEF_COMMAND(indentsize, {
    if (words_len == 1) {
        int answer_int = atoi(words[0]);
        if (answer_int >= 0 && answer_int < 256)
            SEL_BUF.indent_size = answer_int;
    }
})


DEF_COMMAND(crlf, BOOL_SET(SEL_BUF.crlf))

DEF_COMMAND(autotab, BOOL_SET(SEL_BUF.autotab_on))

DEF_COMMAND(save_as, {
    if (words_len == 1) {
        if (!can_write(words[0])) {
            message("can't write in this file");
            return;
        }
    
        free(SEL_BUF.filename);
        SEL_BUF.filename = printdup("%s", words[0]);
        SEL_BUF.can_write = can_write(SEL_BUF.filename);
        
        savefile(&SEL_BUF);
    }
})

DEF_COMMAND(read_only, {
    BOOL_SET(SEL_BUF.read_only);

    if (!SEL_BUF.can_write && !SEL_BUF.read_only) {
        SEL_BUF.read_only = 1;
        message("Can't unlock buffer without write permission");
    }
})


DEF_COMMAND(next, next_buffer();)
DEF_COMMAND(prev, previous_buffer();)
DEF_COMMAND(close_buffer, {
    if (SEL_BUF.modified) {
        char *prt = prompt_hints("Unsaved changes: ", "", "'exit' to confirm", NULL);
        bool confirmed = prt && 0 == strcmp("exit", prt);
        free(prt);

        if (!confirmed) return;
    }
    buffer_close();
})

struct {
    const char *name;
    void (*function)(char words[][CMD_WORD_SZ], size_t words_len);
} fns[] = {
    {"tab_width"  , tabwidth    },
    {"indent_size", indentsize  },
    {"crlf"       , crlf        },
    {"autotab"    , autotab     },
    {"save-as"    , save_as     },
    {"read-only"  , read_only   },
    {"next"       , next        },
    {"prev"       , prev        },
    {"close"      , close_buffer},
    {NULL, NULL}
};

Hints hints[] = {
    {"tab_width"  , "<tabwidth>"             },
    {"indent_size", "<indent sz, 0 for tabs>"},
    {"crlf"       , "f | t"                  },
    {"autotab"    , "f | t"                  },
    {"save-as"    , "<filename>"             },
    {"read-only"  , "f | t"                  },
    {"next"       , ""                       },
    {"prev"       , ""                       },
    {"close"      , ""                       },
    {NULL, NULL}
};


void calculate_base_hint(char *base_hint) {
    char *p = base_hint;
    for (size_t i = 0; hints[i].command; i++)
        p += sprintf(p, "%s ", hints[i].command);
    if (p != base_hint)
        p[-1] = '\0';
}

void config_dialog(void) {
    char base_hint[1000];
    calculate_base_hint(base_hint);

    char command[MSG_SZ];
    char *_command = prompt_hints("Enter command: ", "", base_hint, hints);
    if (!_command)
        return;

    strncpy(command, _command, MSG_SZ - 1);
    command[MSG_SZ - 1] = '\0';
    free(_command);

    parse_command(command);
}

size_t split_cmd_string(const char *s, char ret[CMD_ARR_SZ + 1][CMD_WORD_SZ]) {
    for (; *s == ' '; s++);

    size_t i;
    for (i = 0; *s; i++) {
        const char *n = s;
        for (; *n != ' ' && *n != '\0'; n++);

        size_t cpsz = MIN(n - s, CMD_WORD_SZ - 1);
        memcpy(ret[i], s, cpsz);
        ret[i][cpsz] = '\0';

        for (; *n == ' '; n++);
        s = n;
    }

    return i;
}

void parse_command(char *command) {
    if (!command) return;

    char words[CMD_ARR_SZ + 1][CMD_WORD_SZ];
    size_t words_len = split_cmd_string(command, words);

    for (size_t i = 0; fns[i].name; i++) {
        if (!strcmp(words[0], fns[i].name)) {
            fns[i].function(words + 1, words_len - 1);
            break;
        }
    }
}


