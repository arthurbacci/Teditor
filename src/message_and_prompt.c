#include "ted.h"

char *prompt(const char *msgtmp, char *def) {
    return prompt_hints(msgtmp, def, NULL, NULL);
}

static char *show_hint(Hints *hints, unsigned int len, int i, char *msg) {
    if (hints) {
        Hints *hint = hints;
        while (hint->command) {
            unsigned int word_len = (unsigned int)strlen(hint->command);
            if (len + i >= word_len &&
                !strncmp(msg + (len + i - word_len) + 1, hint->command, word_len))
                return (char *)hint->hint;
            ++hint;
        }
    }
    return NULL;
}

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints) {
    char msg[1000];
    strcpy(msg, msgtmp);
    unsigned int len = strlen(msg);
    char *ret = malloc(1000 - len);
    strcpy(ret, def);
    strcpy(msg + len, def);
    unsigned int deflen = strlen(def);

    int c, i;
    char *shadow = base;

    message(msg);
    display_menu(menu_message, shadow, NULL);

    refresh();
    for (i = deflen; (c = getch()) != '\n' && i < 999 - (int)len; i++) {
        message(msg);
        shadow = NULL;

        if (c == KEY_BACKSPACE || c == ctrl('h') || c == '\b') {
            i -= 1 + (i > 0);
            shadow = show_hint(hints, len, i, msg);
            
        } else if (c == ctrl('c')) {
            free(ret);
            menu_message = "";
            return NULL;
        }

        ret[i + 1] = '\0';
        msg[len + i + 1] = '\0';

        if (isprint(c)) {
            msg[len + i] = c;
            ret[i] = c;
            shadow = show_hint(hints, len, i, msg);
        } else if (c != KEY_BACKSPACE && c != ctrl('h') && c != '\b')
            i--;
            
        shadow = i + 1 == 0 ? base : shadow;
        display_menu(menu_message, shadow, NULL);
        refresh();
        
    }
    ret[i] = '\0';
    menu_message = "";
    return ret;
}

void message(char *msg) {
    menu_message = msg;
}
