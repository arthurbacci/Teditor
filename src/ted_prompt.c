#include <ted_prompt.h>
#include <ted_utils.h>
#include <ted_grapheme.h>
#include <string.h>
#include <ncurses.h>

const char *menu_message = "";

// FIXME: this is a very bad practise
void display_menu(const char *message, const char *shadow);

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints) {
    char msg[1000];

    strncpy(msg, msgtmp, sizeof(msg));
    msg[sizeof(msg) - 1] = '\0';

    char *b = msg + strlen(msg);
    size_t bcap = sizeof(msg) + msg - b;
    
    strncpy(b, def, bcap);
    b[bcap - 1] = '\0';

    size_t blen = strlen(b);

    while (1) {
        const char *hint = "";
        if (blen == 0) {
            if (base)
                hint = base;
        } else if (hints) {
            if (b[blen - 1] == ' ') {
                for (size_t i = 0; hints[i].command; i++) {
                    if (0 == strncmp(hints[i].command, b, strlen(hints[i].command))) {
                        hint = hints[i].hint;
                        break;
                    }
                }
            } else {
                for (size_t i = 0; hints[i].command; i++) {
                    if (0 == strncmp(hints[i].command, b, blen)) {
                        hint = hints[i].command + blen;
                        break;
                    }
                }
            }
        }

        display_menu(msg, hint);

        refresh();

        int c = getch();
        switch (c) {
            case KEY_BACKSPACE:
            case ctrl('h'):
                if (blen > 0) {
                    b[--blen] = '\0';
                    break;
                }
                // If length is 0, it will fall in the ctrl('c') case
            case ctrl('c'):
                return NULL;
            case '\n':
                return printdup("%s", b);
            default:
                if (c != ERR) {
                    if (blen + 1 < bcap) {
                        b[blen++] = c;
                        b[blen] = '\0';
                    }
                }
        }
    }
}

void message(char *msg) {
    menu_message = msg;
}
