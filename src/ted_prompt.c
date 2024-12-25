#include <ted_prompt.h>
#include <ted_utils.h>
#include <ted_grapheme.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>


char menu_message[MSG_SZ] = "";

// FIXME: this is a very bad practise
void display_menu(const char *message, const char *shadow);

size_t prompt_hints(char message[MSG_SZ], char *base_hint, Hints *hints) {
    char *afterlabel = &message[strlen(message)];
    size_t afterlabel_sz = MSG_SZ - (size_t)(afterlabel - message);
    size_t i = 0;
    
    while (1) {
        // TODO: show hints
        const char *hint = NULL;
        
        display_menu(message, hint);
        
        refresh();
        
        int c = getch();
        switch (c) {
            case KEY_BACKSPACE:
            case ctrl('h'):
                if (i > 0) {
                    afterlabel[--i] = '\0';
                    break;
                }
                // FALLTHROUGH to ^C if length is zero
            case ctrl('c'):
                message[0] = '\0';
                return 0;
            case '\n':
                afterlabel[i] = '\0';
                memmove(message, afterlabel, i + 1);
                return i;
            default:
                if (c != ERR && isprint(c))
                    afterlabel[i++] = c;
        }
    }
    
    return afterlabel_sz;
}

void message(const char *msg) {
    strncpy(menu_message, msg, MSG_SZ);
    menu_message[MSG_SZ - 1] = '\0';
}

