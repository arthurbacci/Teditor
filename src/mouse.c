#include "ted.h"

void processMouseEvent(MEVENT ev) {
    if (ev.bstate & BUTTON1_CLICKED || ev.bstate & BUTTON1_PRESSED) {
        cursor.y = text_scroll.y + ev.y;
        if ((unsigned int)ev.x >= len_line_number + 1)
            cursor.x = text_scroll.x + ev.x - len_line_number - 1;
        else
            cursor.x = 0;
        cursor_in_valid_position();
    }
}
