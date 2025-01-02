#include <ted_commands.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ted_utils.h>
#include <ted_buffer.h>
#include <ted_prompt.h>
#include <ted_config.h>

#define BOOL_COMMAND(a, b) \
        int r = string_to_bool(next_word(&words)); \
        if (r == 1) a \
        else if (r == 0) b

#define BOOL_SET(a) BOOL_COMMAND((a) = 1;, (a) = 0;);

#define DEF_COMMAND(a, b) \
    static void (a)(char *words) { \
        PRETEND_TO_USE(words); \
        b \
    }


DEF_COMMAND(tabwidth, {
    int answer_int = atoi(next_word(&words));
    if (IN_RANGE(1, 255, answer_int))
        SEL_BUF.tab_width = answer_int;
})
DEF_COMMAND(indentsize, {
    int answer_int = atoi(next_word(&words));
    if (IN_RANGE(0, 255, answer_int))
        SEL_BUF.indent_size = answer_int;
})

DEF_COMMAND(crlf, BOOL_SET(SEL_BUF.crlf))

DEF_COMMAND(autotab, BOOL_SET(SEL_BUF.autotab_on))

DEF_COMMAND(save_as, {
    const char *filename = next_word(&words);
    if (!can_write(filename)) {
        message("can't write in this file");
        return;
     }
    
    free(SEL_BUF.filename);
    SEL_BUF.filename = printdup("%s", filename);
    SEL_BUF.can_write = can_write(SEL_BUF.filename);
        
    savefile(&SEL_BUF);
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
        char msg[MSG_SZ] = "Unsaved changes: ";
        prompt_hints(msg, "'exit' to confirm", NULL);
        
        if (0 != strcmp("exit", msg)) return;
    }
    buffer_close();
})

struct {
    const char *name;
    void (*function)(char *words);
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

void parse_command(char *words) {
    char *fstword = next_word(&words);
    
    for (size_t i = 0; fns[i].name; i++) {
        if (0 == strcmp(fstword, fns[i].name)) {
            fns[i].function(words);
            return;
        }
    }
}

void config_dialog(void) {
    char base_hint[MSG_SZ];
    // FIXME: may overrun buffer
    calculate_base_hint(base_hint);

    char words[MSG_SZ] = "Enter command: ";
    prompt_hints(words, base_hint, hints);
    
    parse_command(words);
}

void calculate_base_hint(char *base_hint) {
    for (size_t i = 0; hints[i].command; i++)
        base_hint += sprintf(base_hint, "%s ", hints[i].command);
    base_hint[hints[0].command ? -1 : 0] = '\0';
}
