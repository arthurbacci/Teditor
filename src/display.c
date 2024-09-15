#include "ted.h"

void display_menu(const char *message, const char *shadow, const Node *n) {
    const Buffer *buf = &n->data;

    int x, y;
    getyx(stdscr, y, x);

    move(LINES - 1, 0);
    for (size_t i = 0; i < COLS; i++)
        addch(' ');

    move(LINES - 1, 0);
    if (!*message && n) {
        printw(
            "%lu:%lu/%lu %s",
            buf->cursor.y + 1,
            buf->cursor.x_width,
            buf->cursor.x_bytes,
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

        mvprintw(LINES - 1, COLS - len, "%s", b);

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
    for (size_t i = buf.scroll.y; i < buf.scroll.y + LINES - 1; i++) {
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
            
        printw("%*lu ", len_line_number, i + 1);
        
        attroff(A_BOLD);

        const size_t screen_sz = COLS - len_line_number - 1;


        char *at = buf.lines[i].data;
        ssize_t padding = index_by_width_after(buf.scroll.x_width, &at);

        size_t size = 0;
        
        // Note that padding can be negative
        if (padding > 0) {
            size += padding;
            attron(A_REVERSE);
            for (size_t i = 0; i < padding; i++)
                addch('>');
            attroff(A_REVERSE);
        }

        size_t j;
        for (j = 0; *at; j++) {
            if (i == buf.cursor.y
            && buf.scroll.x_width + size == buf.cursor.x_width) {
                attron(A_REVERSE);
            }

            Grapheme g = get_next_grapheme(&at, SIZE_MAX);
            size_t gw = grapheme_width(g);

            if (size + gw > screen_sz) {
                attron(A_REVERSE);
                for (; size < screen_sz; size++)
                    addch('<');
                attroff(A_REVERSE);

                break;
            }


            // CAUTION: if anything is changed here be sure to make grapheme
            // _width follow along
            if (1 == g.sz && '\t' == *g.dt) {
                attron(A_REVERSE);

                for (int k = 0; k < config.tablen; k++)
                    addch(' ');
            } else if (is_replacement_character(g) || (1 == g.sz && !isprint(*g.dt))) {
                attron(A_REVERSE);
                addch('X');
            } else {
                printw("%.*s", (int)g.sz, g.dt);
            }
            size += grapheme_width(g);


            attroff(A_REVERSE);
        }

        while (size < screen_sz) {
            if (i == buf.cursor.y
            && buf.scroll.x_width + size == buf.cursor.x_width) {
                attron(A_REVERSE);
            }

            addch(' ');
            attroff(A_REVERSE);

            size++;
            j++;
        }
    }
}
