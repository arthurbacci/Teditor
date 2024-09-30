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
    static void (a)(char words[][CMD_WORD_SZ], unsigned int words_len, Node **n) { \
        Buffer *buf = &(*n)->data; \
        /* Only for suppressing warnings */ \
        USE(words); \
        USE(words_len); \
        USE(n); \
        USE(buf); \
        \
        b \
        return; \
    }



DEF_COMMAND(tablen, {
    if (words_len == 1) {
        int answer_int = atoi(words[0]);
        if (answer_int > 0)
            config.tablen = answer_int;
    }
})


DEF_COMMAND(crlf, BOOL_SET(buf->crlf))

DEF_COMMAND(use_spaces, BOOL_SET(config.use_spaces))
DEF_COMMAND(autotab, BOOL_SET(config.autotab))

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


/* TODO: reimplement this
DEF_COMMAND(eof, {
    if (words_len == 0)
        change_position(buf->lines[buf->num_lines - 1].length, buf->num_lines, buf);
})
*/

DEF_COMMAND(next, *n = (*n)->next;)
DEF_COMMAND(prev, *n = (*n)->prev;)
DEF_COMMAND(close_buffer, {
    if (buf->modified) {
        char *prt = prompt_hints("Unsaved changes: ", "", "'exit' to confirm", NULL);
        bool confirmed = prt && 0 == strcmp("exit", prt);
        free(prt);

        if (!confirmed)
            return;
    }

    *n = (*n)->prev;
    buffer_close((*n)->next);
})

struct {
    const char *name;
    void (*function)(char words[][CMD_WORD_SZ], unsigned int words_len, Node **n);
} fns[] = {
    {"tablen"           , tablen            },
    {"crlf"             , crlf              },
    {"use-spaces"       , use_spaces        },
    {"autotab"          , autotab           },
    {"save-as"          , save_as           },
    {"read-only"        , read_only         },
    //{"find"             , find              },
    //{"eof"              , eof               },
    {"next"             , next              },
    {"prev"             , prev              },
    {"close"            , close_buffer      },
    {NULL, NULL}
};

Hints hints[] = {
    {"tablen"           , "<tablen>"                     },
    {"crlf"             , "f | t"                        },
    {"use-spaces"       , "f | t"                        },
    {"autotab"          , "f | t"                        },
    {"save-as"          , "<filename>"                   },
    {"read-only"        , "f | t"                        },
    //{"find"             , "(start | cursor) <substring>" },
    //{"eof"              , ""                             },
    {"next"             , ""                             },
    {"prev"             , ""                             },
    {"close"            , ""                             },
    {NULL, NULL}
};

char last_command[1000] = "";

void calculate_base_hint(char *base_hint) {
    char *p = base_hint;
    for (size_t i = 0; hints[i].command; i++)
        p += sprintf(p, "%s ", hints[i].command);
    if (p != base_hint)
        p[-1] = '\0';
}

void config_dialog(Node **n) {
    char base_hint[1000];
    calculate_base_hint(base_hint);

    char command[MSG_SZ];
    char *_command = prompt_hints("Enter command: ", "", base_hint, hints);
    strncpy(command, _command, MSG_SZ - 1);
    command[MSG_SZ - 1] = '\0';
    free(_command);

    parse_command(command, n);
}


void parse_command(char *command, Node **n) {
    if (!command)
        return;


    char words[CMD_ARR_SZ + 1][CMD_WORD_SZ];
    size_t words_len = split_cmd_string(command, words);

    if (words_len == 1 && !strcmp(words[0], "repeat"))
        parse_command(last_command, n);
    for (unsigned int i = 0; fns[i].name; i++) {
        if (!strcmp(words[0], fns[i].name)) {
            fns[i].function(words + 1, words_len - 1, n);
            if (command != last_command)
                strcpy(last_command, command);
            break;
        }
    }
}


