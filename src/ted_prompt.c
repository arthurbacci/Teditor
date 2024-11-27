#include <ted_prompt.h>
#include <ted_utils.h>
#include <ted_grapheme.h>
#include <string.h>
#include <ncurses.h>


const char *menu_message = "";

// FIXME: this is a very bad practise
void display_menu(const char *message, const char *shadow);

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints) {
    return NULL;
}


void message(char *msg) {
    menu_message = msg;
}
