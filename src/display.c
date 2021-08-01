#include "ted.h"

// Buffer* is used instead of Buffer for being able to pass NULL
void display_menu(char *message, char *shadow, Node *n) {
    const Buffer *buf = &n->data;
    int x, y;
    getyx(stdscr, y, x);

    move(config.lines, 0);
    for (unsigned int i = 0; i < (unsigned int)COLS; i++)
        addch(' ');

    move(config.lines, 0);
    if (!*message) {
        unsigned int scrolled = ((double)buf->cursor.y / ((double)buf->num_lines - 1)) * 100;

        printw("I:%u %u%% %s", buf->lines[buf->cursor.x].ident, scrolled, buf->filename);

        char b[500];
        int len = snprintf(
            b, 500, "%s%s%s %s | %s<-%s->%s",
            buf->modified ? "!" : ".",
            buf->read_only ? "o" : "c",
            buf->can_write ? "W" : "R",
            buf->line_break_type == 0 ? "LF" : (buf->line_break_type == 1 ? "CRLF" : "CR"),
            n->prev->data.name,
            buf->name,
            n->next->data.name
        );

        mvprintw(config.lines, COLS - len, "%s", b);

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

void display_buffer(Buffer buf, int len_line_number) {
    for (unsigned int i = buf.scroll.y; i < buf.scroll.y + config.lines; i++) {
        move(i - buf.scroll.y, 0);
        if (i >= buf.num_lines) {
            for (int j = 0; j < len_line_number - 1; j++)
                addch(' ');
            addch('~');
            for (int j = 0; j < COLS - len_line_number; j++)
                addch(' ');
            continue;
        }
        
        if (i == buf.cursor.y)
            attron(A_BOLD);
            
        printw("%*d ", len_line_number, i + 1);
        
        attroff(A_BOLD);
    
        unsigned int size = 0;
        for (unsigned int j = 0; size < (unsigned int)COLS - len_line_number - 1; j++) {

            if (j + buf.scroll.x == buf.cursor.x && i == buf.cursor.y)
                attron(A_REVERSE);

            // Write blank space at the right of the line
            if (buf.scroll.x + j >= buf.lines[i].length) {
                addch(' ');
                goto AFTER_WRITE_CHAR;
            }
            uchar32_t el = buf.lines[i].data[buf.scroll.x + j];
            

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
