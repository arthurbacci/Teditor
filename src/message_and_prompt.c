#include "ted.h"

char *prompt(const char *msgtmp, char *def) {
    return prompt_hints(msgtmp, def, NULL, NULL);
}

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints) {
    char msg[MSG_SZ];

    strncpy(msg, msgtmp, sizeof(msg));
    msg[MSG_SZ - 1] = '\0';

    char *b = msg + strlen(msg);
    size_t bcap = MSG_SZ + msg - b;
    
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

        display_menu(msg, hint, NULL);

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
                return strdup(b);
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
