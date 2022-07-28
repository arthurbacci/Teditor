#include "ted.h"

char *prompt(const char *msgtmp, char *def) {
    return prompt_hints(msgtmp, def, NULL, NULL);
}

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints) {
    char msg[MSG_SZ];
    {
        size_t msglen = strlen(msgtmp);
        if (msglen > MSG_SZ)
            msglen = MSG_SZ;
        memcpy(msg, msgtmp, msglen);
        msg[msglen - 1] = '\0';
    }

    char *b = msg + strlen(msg);
    size_t bcap = msg + MSG_SZ - b;
    
    {
        size_t deflen = strlen(def) + 1;
        if (deflen >= bcap)
            deflen = bcap;
        memcpy(b, def, deflen);
        b[deflen - 1] = '\0';
    }

    size_t blen = strlen(b);

    while (1) {
        const char *hint = "";
        if (blen == 0) {
            if (base)
                hint = base;
        } else if (hints) {
            if (b[blen - 1] == ' ') {
                for (size_t i = 0; hints[i].command; i++) {
                    b[blen - 1] = '\0';
                    int cmp = strcmp(hints[i].command, b);
                    b[blen - 1] = ' ';

                    if (cmp == 0) {
                        hint = hints[i].hint;
                        break;
                    }
                }
            } else {
                for (size_t i = 0; hints[i].command; i++) {
                    if (strlen(hints[i].command) < blen)
                        continue;

                    if (memcmp(hints[i].command, b, blen)) {
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
            case '\n': {
                char *r = malloc(blen + 1);
                memcpy(r, b, blen + 1);
                return r;
            }
            default:
                if (c != ERR) {
                    if (blen + 1 < bcap) {
                        b[blen++] = c;
                        b[blen] = '\0';
                    }
                }
                break;
        }
    }
}


void message(char *msg) {
    menu_message = msg;
}
