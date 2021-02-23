#include "ted.h"

char *prompt(const char *msg_) {
    char msg[1000];
    strcpy(msg, msg_);
    unsigned int len = strlen(msg);
    char *ret = malloc(1000 - len);

    int c;
    int i;
    message(msg);
    show_menu(menu_message);
    refresh();
    for (i = 0; (c = getch()) != '\n' && i < 999 - (int)len; i++) {
        message(msg);
        if (c == KEY_BACKSPACE)
            i -= 1 + (i > 0);
        else if (c == ctrl('c')) {
            free(ret);
            menu_message = "";
            return NULL;
        }

        ret[i + 1] = '\0';
        msg[len + i + 1] = '\0';

        if (isprint(c)) {
            msg[len + i] = c;
            ret[i] = c;
        }
        
        show_menu(menu_message);
        refresh();
    }
    ret[i] = '\0';
    menu_message = "";
    return ret;
}

void message(char *msg) {
    menu_message = msg;
}

