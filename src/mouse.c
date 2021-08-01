#include "ted.h"

bool process_mouse_event(MEVENT ev, Node **n) {
    if (ev.bstate & BUTTON1_CLICKED || ev.bstate & BUTTON1_PRESSED) {
        // FIXME: It is not working
        /*
        cursor.y = buf.scroll.y + ev.y;
        if ((unsigned int)ev.x >= len_line_number + 1)
            cursor.x = buf.scroll.x + ev.x - len_line_number - 1;
        else
            cursor.x = 0;
        cursor_in_valid_position();
        last_cursor_x = cx;
        */
    } else if (ev.bstate == BUTTON4_PRESSED)
        return process_keypress(KEY_UP, n);
    else if (ev.bstate == 0x200000)
        return process_keypress(KEY_DOWN, n);
    else if (ev.bstate == 0x4200000)
        return process_keypress(KEY_RIGHT, n);
    else if (ev.bstate == 0x4010000)
        return process_keypress(KEY_LEFT, n);
    return false;
}
