#include "ted.h"

// TODO: error messages for incomplete commands (right now they're being
// completely ignored)

#define BOOL_COMMAND(a, b) \
    if (words_len == 1) { \
        int r = process_as_bool(words[0]); \
        if (r == 1) \
            a \
        else if (r == 0) \
            b \
    }

#define BOOL_SET(a) BOOL_COMMAND((a) = 1;, (a) = 0;);

#define DEF_COMMAND(a, b) \
    static void (a)(char words[][CMD_WORD_SZ], size_t words_len) { \
        Buffer *buf = &SEL_BUF; \
        /* Only for suppressing warnings */ \
        USE(words); \
        USE(words_len); \
        USE(buf); \
        \
        b \
        return; \
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


DEF_COMMAND(crlf, BOOL_SET(buf->crlf))

DEF_COMMAND(autotab, BOOL_SET(SEL_BUF.autotab_on))

DEF_COMMAND(save_as, {
    if (words_len == 1) {
        free(buf->filename);

        size_t size = strlen(words[0]) + 1;
        buf->filename = malloc(size);
        memcpy(buf->filename, words[0], size);

        // Permissions may change since the last time it was detected
        buf->can_write = can_write(buf->filename);

        if (buf->can_write)
            savefile(buf);
        else
            message("Can't save, no permission to write");
    }
})

DEF_COMMAND(read_only, {
    BOOL_SET(buf->read_only);

    if (!buf->can_write && !buf->read_only) {
        buf->read_only = 1;
        message("Can't unlock buffer without write permission");
    }
})


DEF_COMMAND(next, next_buffer();)
DEF_COMMAND(prev, previous_buffer();)
DEF_COMMAND(close_buffer, {
    if (buf->modified) {
        char *prt = prompt_hints("Unsaved changes: ", "", "'exit' to confirm", NULL);
        bool confirmed = prt && 0 == strcmp("exit", prt);
        free(prt);

        if (!confirmed)
            return;
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

// FIXME: dont hardcode 1000
char last_command[1000] = "";

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


void parse_command(char *command) {
    if (!command)
        return;

    char words[CMD_ARR_SZ + 1][CMD_WORD_SZ];
    size_t words_len = split_cmd_string(command, words);

    if (words_len == 1 && !strcmp(words[0], "repeat"))
        parse_command(last_command);
    for (unsigned int i = 0; fns[i].name; i++) {
        if (!strcmp(words[0], fns[i].name)) {
            fns[i].function(words + 1, words_len - 1);
            if (command != last_command)
                strcpy(last_command, command);
            break;
        }
    }
}


