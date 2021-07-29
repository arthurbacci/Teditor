#include "ted.h"

void show_menu(char *message, char *shadow) {
    int x, y;
    getyx(stdscr, y, x);

    move(config.lines, 0);
    for (unsigned int i = 0; i < (unsigned int)COLS; i++)
        addch(' ');

    move(config.lines, 0);
    if (!*message) {
        unsigned int scrolled = ((double)cy / ((double)num_lines - 1)) * 100;

        printw("I:%u %u%% %s", lines[cy].ident, scrolled, filename);

        char buf[500];
        int len = snprintf(buf, 500, "%s%s%s %s",
                           config.selected_buf.modified ? "!" : ".",
                           config.selected_buf.read_only ? "o" : "c",
                           config.selected_buf.can_write ? "W" : "R",
                           config.line_break_type == 0 ? "LF" : (config.line_break_type == 1 ? "CRLF" : "CR"));

        mvprintw(config.lines, COLS - len, "%s", buf);

    } else if (shadow != NULL) {
        printw("%s", message);
        getyx(stdscr, y, x); //save message x and y

        attron(A_BOLD);
        printw("%s", shadow);

        attroff(A_BOLD);
    } else
        printw("%s", message);

    move(y, x);
}

void show_lines(void) {
    for (unsigned int i = text_scroll.y; i < text_scroll.y + config.lines; i++) {
        move(i - text_scroll.y, 0);
        if (i >= num_lines) {
            for (unsigned int j = 0; j < len_line_number - 1; j++)
                addch(' ');
            addch('~');
            for (unsigned int j = 0; j < (unsigned int)COLS - len_line_number; j++)
                addch(' ');
            continue;
        }
        
        if (i == cy)
            attron(A_BOLD);
            
        printw("%*d ", len_line_number + 1, i + 1);
        
        if (i == cy)
            attroff(A_BOLD);
    
        unsigned int size = 0;
        for (unsigned int j = 0; size < (unsigned int)COLS - len_line_number - 1; j++) {

            if (j + text_scroll.x == cursor.x && i == cursor.y)
                attron(A_REVERSE);

            // Write blank space at the right of the line
            if (text_scroll.x + j >= lines[i].length) {
                addch(' ');
                goto AFTER_WRITE_CHAR;
            }
            uchar32_t el = lines[i].data[text_scroll.x + j];
            

            if (el == '\t') {
                for (unsigned int k = 0; k < config.tablen; k++)
                    addch(' ');
                size += config.tablen - 1;
            } else {
                unsigned char b[4];
                int len = utf8ToMultibyte(el, b, 1);

                if (len == -1) {
                    b[0] = substitute_string[0];
                    b[1] = substitute_string[1];
                    b[2] = substitute_string[2];
                    len = 3;
                }
                printw("%.*s", len, b);
            }
            
AFTER_WRITE_CHAR:
            attroff(A_REVERSE);
            size++;
        }
    }
}
