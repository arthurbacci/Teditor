#include "ted.h"

#define BOOL_COMMAND(a, b) \
    if (words_len == 1) { \
        if (!strcmp(words[0], "t")) \
            a \
        else if (!strcmp(words[0], "f")) \
            b \
    }

#define BOOL_SET(a) BOOL_COMMAND((a) = 1;, (a) = 0;);

#define DEF_COMMAND(a, b) \
    static void (a)(char **words, unsigned int words_len, Node **n) { \
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
    if (words_len == 1) {
        if (!strcasecmp(words[0], "TRUE") || !strcmp(words[0], "1"))
            buf->read_only = 1;
        else if (!strcasecmp(words[0], "FALSE") || !strcmp(words[0], "0")) {
            if (buf->can_write)
                buf->read_only = 0;
            else
                message("Can't unlock buffer without write permission");
        }
    }
})

/*
// FIXME: this code is horrible
DEF_COMMAND(find, {
    int from_cur = 0;
    if (words_len == 2 && !strcmp(words[0], "cursor"))
        from_cur = 1;
    if (words_len == 1 || words_len == 2) {
        unsigned int len = strlen(words[words_len - 1]);
        int index;
        for (unsigned int at = from_cur ? buf->cursor.y : 0; at < buf->num_lines; ++at) {
            if (buf->lines[at].length >= len &&
                (index = uchar32_sub(
                    from_cur && at == buf->cursor.y ? &buf->lines[at].data[buf->cursor.x] : buf->lines[at].data,
                    words[words_len - 1], buf->lines[at].length,
                    len
                )) >= 0
            ) {
                change_position(index + len + (from_cur && at == buf->cursor.y) * buf->cursor.x, at, buf);
                return;
            }
        }
        message("Substring not found");
    }
})
*/

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
    void (*function)(char **words, unsigned int words_len, Node **n);
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
    char *command = prompt_hints("Enter command: ", "", base_hint, hints);

    parse_command(command, n);

    free(command);
}


void parse_command(char *command, Node **n) {
    if (!command)
        return;

    int words_len;
    char **words = split_str(command, &words_len);

    switch (run_command(words, words_len, n)) {
        // repeat command
        case 0:
            parse_command(last_command, n);
            break;

        // normal command
        case 1:
            if (command != last_command)
                strcpy(last_command, command);
            break;
    }

    for (int i = 0; i < words_len; i++)
        free(words[i]);
    free(words);
}


int run_command(char **words, int words_len, Node **n) {
    if (words_len == 1 && !strcmp(words[0], "repeat"))
        return 0;
    for (unsigned int i = 0; fns[i].name; i++) {
        if (!strcmp(words[0], fns[i].name)) {
            fns[i].function(words + 1, words_len - 1, n);
            break;
        }
    }

    return 1;
}
