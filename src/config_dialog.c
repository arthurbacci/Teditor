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
    static void (a)(char **words, unsigned int words_len, Buffer *buf) { \
        /* Only for suppressing warnings */ \
        USE(words); \
        USE(words_len); \
        USE(buf); \
        \
        b \
    }

DEF_COMMAND(tablen, {
    if (words_len == 1) {
        int answer_int = atoi(words[0]);
        if (answer_int > 0)
            config.tablen = answer_int;
    }
})

DEF_COMMAND(linebreak, {
    if (words_len == 1) {
        if (!strcasecmp(words[0], "LF"))
            buf->line_break_type = 0;
        else if (!strcasecmp(words[0], "CRLF"))
            buf->line_break_type = 1;
        else if (!strcasecmp(words[0], "CR"))
            buf->line_break_type = 2;
    }
})


DEF_COMMAND(insert_newline, BOOL_SET(config.insert_newline))
DEF_COMMAND(use_spaces, BOOL_SET(config.use_spaces))
DEF_COMMAND(autotab, BOOL_SET(config.autotab))
DEF_COMMAND(automatch, BOOL_SET(config.automatch))

DEF_COMMAND(save_as, {
    if (words_len == 1) {
        if (needs_to_free_filename)
            free(filename);

        unsigned int size = (strlen(words[0]) + 1) * sizeof(char);
        filename = malloc(size);
        memcpy(filename, words[0], size);
        needs_to_free_filename = 1;

        // Permissions may change since the last time it was detected
        buf->can_write = can_write(filename);

        if (buf->can_write)
            savefile(*buf);
        else
            message("Can't save, no permission to write");
    }
})

DEF_COMMAND(manual, {
    if (words_len == 0) {
        open_file(home_path(".config/ted/docs/help.txt"), 1, buf);
        buf->read_only = 1;
    } else if (words_len > 0) {
        char fname[1000];
        char *n = fname;
        n += sprintf(n, ".config/ted/docs");
        for (size_t i = 0; i < words_len; i++)
            n += sprintf(n, "/%s", words[i]);
        n += sprintf(n, ".txt");
        open_file(home_path(fname), 1, buf);
        buf->read_only = 1;
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

DEF_COMMAND(eof, {
    if (words_len == 0)
        change_position(buf->lines[buf->num_lines - 1].length, buf->num_lines, buf);
})

struct {
    const char *name;
    void (*function)(char **words, unsigned int words_len, Buffer *buf);
} fns[] = {
    {"tablen"           , tablen            },
    {"linebreak"        , linebreak         },
    {"insert-newline"   , insert_newline    },
    {"use-spaces"       , use_spaces        },
    {"autotab"          , autotab           },
    {"automatch"        , automatch         },
    {"save-as"          , save_as           },
    {"manual"           , manual            },
    {"read-only"        , read_only         },
    {"find"             , find              },
    {"eof"              , eof               },
    {NULL, NULL}
};

Hints hints[] = {
    {"tablen"           , " <tablen>"                     },
    {"linebreak"        , " LF | CR | CRLF"               },
    {"insert-newline"   , " f | t"                        },
    {"use-spaces"       , " f | t"                        },
    {"autotab"          , " f | t"                        },
    {"automatch"        , " f | t"                        },
    {"save-as"          , " <filename>"                   },
    {"manual"           , " <page (nothing for index)>"   },
    {"read-only"        , " f | t"                        },
    {"find"             , " (start | cursor) <substring>" },
    {NULL, NULL}
};

char last_command[1000] = "";

void calculate_base_hint(Hints *hints, char *base_hint) {
    char *p = base_hint;
    for (size_t i = 0; hints[i].command; i++)
        p += sprintf(p, "%s ", hints[i].command);
    if (p != base_hint)
        p[-1] = '\0';
}

void config_dialog(Buffer *buf) {
    char base_hint[1000];
    calculate_base_hint(hints, base_hint);
    char *command = prompt_hints("Enter command: ", "", base_hint, hints);

    parse_command(command, buf);

    free(command);
}


void parse_command(char *command, Buffer *buf) {
    if (!command)
        return;

    int words_len;
    char **words = split_str(command, &words_len);

    if (run_command(words, words_len, buf))
        parse_command(last_command, buf);
    else if (command != last_command)
        strcpy(last_command, command);

    for (int i = 0; i < words_len; i++)
        free(words[i]);
    free(words);
}


bool run_command(char **words, int words_len, Buffer *buf) {
    if (words_len == 1 && !strcmp(words[0], "repeat"))
        return 1;
    for (unsigned int i = 0; fns[i].name; i++) {
        if (!strcmp(words[0], fns[i].name)) {
            fns[i].function(words + 1, words_len - 1, buf);
                return 0;
        }
    }
    return 0;
}
