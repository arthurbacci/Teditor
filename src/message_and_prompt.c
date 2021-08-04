#include "ted.h"

char *prompt(const char *msgtmp, char *def) {
    return prompt_hints(msgtmp, def, NULL, NULL);
}

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints) {
    String msg = dynamic_string(msgtmp, strlen(msgtmp));
    String b = dynamic_string(def, strlen(def));
    dynamic_string_push_str(&b, def, strlen(def));

    while (1) {
        char *hint = "";
        if (b.len == 0) {
            if (base)
                hint = base;
        } else if (hints) {
            if (b.start[b.len - 1] == ' ') {
                for (size_t i = 0; hints[i].command; i++) {
                    String s = dynamic_string(
                        hints[i].command,
                        strlen(hints[i].command)
                    );
                    String stripped_b = b;
                    stripped_b.len--;
                    if (dynamic_string_eq(s, stripped_b)) {
                        hint = hints[i].hint;
                        dynamic_string_free(&s);
                        break;
                    }
                    dynamic_string_free(&s);
                }
            } else {
                for (size_t i = 0; hints[i].command; i++) {
                    String s = dynamic_string(
                        hints[i].command,
                        strlen(hints[i].command)
                    );
                    if (dynamic_string_starts_with(s, b)) {
                        hint = hints[i].command + b.len;
                        dynamic_string_free(&s);
                        break;
                    }
                    dynamic_string_free(&s);
                }
            }
        }

        String s = dynamic_string_concat(msg, b);
        char *d = dynamic_string_to_str(s);

        display_menu(d, hint, NULL);

        free(d);
        dynamic_string_free(&s);

        refresh();

        int c = getch();
        switch (c) {
            case KEY_BACKSPACE:
            case ctrl('h'):
                if (dynamic_string_pop(&b))
                    break;
                // If length is 0, it will fall in the ctrl('c') case
            case ctrl('c'):
                dynamic_string_free(&b);
                dynamic_string_free(&msg);
                return NULL;
            case '\n':
                goto break_outer;
            default:
                if (c != ERR)
                    dynamic_string_push(&b, c);
                break;
        }
    }
    break_outer: {
        char *r = dynamic_string_to_str(b);
        dynamic_string_free(&b);
        dynamic_string_free(&msg);
        return r;
    }
}


void message(char *msg) {
    menu_message = msg;
}
