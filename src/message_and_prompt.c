#include "ted.h"

char *prompt(char *msg) {
    unsigned int len = strlen(msg);
    char *ret = malloc(1000 - len);

    int c;
    int i;
    for (i = 0; (c = message(msg)) != '\n' && i < 999 - (int)len; i++) {
        if (c == KEY_BACKSPACE)
            i -= 1 + (i > 0);
        else if (c == ctrl('c')) {
            free(ret);
            return NULL;
        }

        ret[i + 1] = '\0';
        msg[len + i + 1] = '\0';

        if (isprint(c)) {
            msg[len + i] = c;
            ret[i] = c;
        }
    }

    return ret;
}

int message(char *msg) {
    unsigned int len = strlen(msg);

    unsigned int lines = 0;
    for (unsigned int i = 0; i < len; i++)
        if (msg[i] == '\n')
            lines++;

    char *s = strtok(msg, "\n");

    clear();
    setcolor(COLOR_PAIR(1));
    attron(A_BOLD);
    for (unsigned int i = (config.LINES - lines) / 2; s != NULL; i++) {
        mvprintw(i, (COLS - strlen(s)) / 2, "%s", s);

        s = strtok(NULL, "\n");
    }
    attroff(A_BOLD);
    setcolor(COLOR_PAIR(2));
    refresh();
    
    return getch();
}
