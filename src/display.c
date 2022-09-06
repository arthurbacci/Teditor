#include "ted.h"

void display_menu(const char *message, const char *shadow, const Node *n) {
    const Buffer *buf = &n->data;

    int x, y;
    getyx(stdscr, y, x);

    move(config.lines, 0);
    for (size_t i = 0; i < COLS; i++)
        addch(' ');

    move(config.lines, 0);
    if (!*message && n) {
        int scrolled = 100 * (float)buf->cursor.y / ((float)buf->num_lines - 1);

        printw(
            "I:%u %u%% %s",
            buf->lines[buf->cursor.y].ident,
            scrolled,
            buf->filename
        );

        char b[500];
        int len = snprintf(
            b, 500, "%s%s%s %s | %s<-%s->%s",
            buf->modified ? "!" : ".",
            buf->read_only ? "o" : "c",
            buf->can_write ? "W" : "R",
            buf->crlf ? "CRLF" : "LF",
            n->prev->data.name,
            buf->name,
            n->next->data.name
        );

        mvprintw(config.lines, COLS - len, "%s", b);

    } else if (shadow != NULL) {
        printw("%s", message);
        getyx(stdscr, y, x); // save message x and y

        attron(A_BOLD);
        printw("%s", shadow);

        attroff(A_BOLD);
    } else
        printw("%s", message);

    move(y, x);
}

void display_buffer(Buffer buf, int len_line_number) {
    for (size_t i = buf.scroll.y; i < buf.scroll.y + config.lines; i++) {
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
    
        char *at = &buf.lines[i].data[buf.scroll.x];

        size_t size = 0;
        // TODO: increment j by space the char occupies
        for (size_t j = 0; size < COLS - len_line_number - 1; j++) {

            // TODO: use grapheme_x
            if (j + buf.scroll.x == buf.cursor.x && i == buf.cursor.y)
                attron(A_REVERSE);

            // Write blank space at the right of the line
            if (buf.scroll.x + j >= buf.lines[i].length) {
                addch(' ');
                goto AFTER_WRITE_CHAR;
            }

            Grapheme grapheme = get_next_grapheme(
                &at,
                SIZE_MAX
            );
            

            if (1 == grapheme.sz && '\t' == *grapheme.dt) {
                for (int k = 0; k < config.tablen; k++)
                    addch(' ');
                // - 1 because it will already be incremented at the end of the
                // loop
                size += config.tablen - 1;
            } else {
                printw("%.*s", grapheme.sz, grapheme.dt);
            }
            
AFTER_WRITE_CHAR:
            attroff(A_REVERSE);
            // TODO: some graphemes can take up more than 1 cell in the
            //       terminal, implement handling for that
            size++;
        }
    }
}
