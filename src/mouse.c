#include "ted.h"

void processMouseEvent(MEVENT ev) {
    if (ev.bstate & BUTTON1_CLICKED || ev.bstate & BUTTON1_PRESSED) {
        cursor.y = text_scroll.y + ev.y;
        cursor_in_valid_position();
    }
}
