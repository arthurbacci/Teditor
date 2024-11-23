#include "ted.h"

// garants that the capacity is (x + 1) bytes greater than the length
void expand_line(Line *ln, size_t x) {
    if (ln->cap <= ln->length + x + 1) {
        while (ln->cap <= ln->length + x + 1)
            ln->cap *= 2;

        ln->data = realloc(ln->data, ln->cap);
    }
}


void process_keypress(int c) {
    Buffer *buf = &SEL_BUF;

    if (c == ERR)
        return;

    message("");

    switch (c) {
    case ctrl('c'):
        parse_command("close");
    case ctrl('z'):
        parse_command("prev");
    case ctrl('x'):
        parse_command("next");
    case KEY_UP:
    case ctrl('p'):
        // Decrements `y` if it is greater than 0
        buf->cursor.y -= buf->cursor.y > 0;

        recalc_cur(buf);

        break;
    case KEY_DOWN:
    case ctrl('n'):
        // Increments `y` if it doesn't gets greater or equal than `num_lines`
        buf->cursor.y += buf->cursor.y + 1 < buf->num_lines;

        recalc_cur(buf);

        break;
    case KEY_LEFT:
    case ctrl('b'): {
        char *s = buf->lines[buf->cursor.y].data;

        size_t x_grapheme = wi_to_gi(buf->cursor.x_width, s);

        if (x_grapheme > 0) {
            buf->cursor.x_width = gi_to_wi(x_grapheme - 1, s);
            truncate_cur(buf);
        }

        buf->cursor.lx_width = buf->cursor.x_width;

        break;
    }
    case KEY_RIGHT:
    case ctrl('f'): {
        char *s = buf->lines[buf->cursor.y].data + buf->cursor.x_bytes;
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);
        size_t gw = grapheme_width(g);

        buf->cursor.x_width += gw;
        truncate_cur(buf);
        buf->cursor.lx_width = buf->cursor.x_width;

        break;
    }
    case KEY_HOME:
    case ctrl('a'):
        buf->cursor.x_width = 0;
        truncate_cur(buf);
        buf->cursor.lx_width = buf->cursor.x_width;

        break;
    case KEY_END:
    case ctrl('e'):
        buf->cursor.x_width = SIZE_MAX;
        truncate_cur(buf);
        buf->cursor.lx_width = buf->cursor.x_width;

        break;
    case ctrl('s'):
        if (!buf->read_only)
            savefile(buf);
        break;
    case '\t':
        if (SEL_BUF.indent_size > 0) {
            for (int i = 0; i < SEL_BUF.indent_size; i++)
                process_keypress(' ');
            return;
        } // else, it will pass though and be added to the buffer
        break;
    case ctrl('g'):
        config_dialog();
        break;
    case ctrl('q'):
        parse_command(
            buf->read_only ? "read-only 0" : "read-only 1"
        );
        break;
/*    case KEY_PPAGE: {
        size_t dec = SROW + buf->cursor.y % SROW;

        if (buf->cursor.y > dec)
            buf->cursor.y -= dec;
        else
            buf->cursor.y = 0;
        
        buf->scroll.y = buf->cursor.y;

        recalc_cur(buf);
        break;
    }
    case KEY_NPAGE: {
        size_t inc = SROW - buf->cursor.y % SROW;

        buf->cursor.y += inc;
        if (buf->cursor.y >= buf->num_lines)
            buf->cursor.y = buf->num_lines - 1;

        buf->scroll.y = buf->cursor.y;

        recalc_cur(buf);
        break;
    }*/
    /*
    case ctrl('w'): {
        Line *ln = &buf->lines[buf->cursor.y];

        while (buf->cursor.x_bytes > 0) {
            process_keypress(KEY_LEFT);
            bool w = is_whitespace(ln->data[buf->cursor.x_bytes]);
            process_keypress(KEY_RIGHT);
            if (w)
                break;

            process_keypress(KEY_BACKSPACE);
        }
        
        while (buf->cursor.x_bytes > 0) {
            process_keypress(KEY_LEFT);
            bool w = is_whitespace(ln->data[buf->cursor.x_bytes]);
            process_keypress(KEY_RIGHT);
            if (!w)
                break;

            process_keypress(KEY_BACKSPACE);
        }

        break;
    }
    */
    case ctrl('o'): {
        char *d = prompt_hints("open: ", buf->filename, NULL, NULL);
        if (d) open_file(d);
        break;
    }
    /*
    case CTRL_KEY_LEFT:
    case ctrl('h'): {
        char *s = buf->lines[buf->cursor.y].data;

        while (
            buf->cursor.x_bytes > 0 && !is_whitespace(s[buf->cursor.x_bytes])
        ) {
            process_keypress(KEY_LEFT);
        }
        
        while (buf->cursor.x_bytes > 0 && is_whitespace(s[buf->cursor.x_bytes]))
            process_keypress(KEY_LEFT);

        break;
    }
    case CTRL_KEY_RIGHT:
    case ctrl('l'): {
        Line ln = buf->lines[buf->cursor.y];

        while (
            buf->cursor.x_bytes < ln.length
            && !is_whitespace(ln.data[buf->cursor.x_bytes])
        ) {
            process_keypress(KEY_RIGHT);
        }

        while (
            buf->cursor.x_bytes < ln.length
            && is_whitespace(ln.data[buf->cursor.x_bytes])
        ) {
            process_keypress(KEY_RIGHT);
        }

        break;
    }
    */
    case KEY_BACKSPACE: case KEY_DC: case 127: {
        if (modify(buf)) {
            if (buf->cursor.x_bytes > 0) {
                process_keypress(KEY_LEFT);
                remove_char(buf->cursor.x_bytes, &buf->lines[buf->cursor.y]);
            } else if (buf->cursor.y > 0) {
                Line del_line = buf->lines[buf->cursor.y];

                memmove(
                    &buf->lines[buf->cursor.y],
                    &buf->lines[buf->cursor.y + 1],
                    (buf->num_lines - buf->cursor.y - 1) * sizeof(Line)
                );
                buf->num_lines--;

                buf->cursor.y--;
                buf->cursor.x_width = SIZE_MAX;
                truncate_cur(buf);
                buf->cursor.lx_width = buf->cursor.x_width;

                expand_line(&buf->lines[buf->cursor.y], del_line.length);

                Line *to_append = &buf->lines[buf->cursor.y];

                memmove(
                    &to_append->data[to_append->length],
                    del_line.data,
                    del_line.length
                );

                to_append->length += del_line.length;
                to_append->data[to_append->length] = '\0';

                free(del_line.data);
            }
        }
        break;
    } case '\n': case KEY_ENTER: case '\r':
    {
        if (modify(buf)) {
            buf->lines = realloc(buf->lines, ++buf->num_lines * sizeof(Line));
            memmove(
                &buf->lines[buf->cursor.y + 2],
                &buf->lines[buf->cursor.y + 1],
                (buf->num_lines - buf->cursor.y - 2) * sizeof(Line)
            );

            buf->lines[buf->cursor.y + 1] = blank_line();

            Line *current = &buf->lines[buf->cursor.y];
            Line *new = &buf->lines[buf->cursor.y + 1];

            if (SEL_BUF.autotab_on) {
                size_t ident_sz = get_ident_sz(current->data);
                
                expand_line(new, ident_sz);

                memcpy(new->data, current->data, ident_sz);
                new->length = ident_sz;
                new->data[new->length] = '\0';
            }


            size_t cur_x = buf->cursor.x_bytes;

            buf->cursor.y++;
            buf->cursor.x_width = SIZE_MAX;
            truncate_cur(buf);
            buf->cursor.lx_width = buf->cursor.x_width;

            expand_line(new, current->length - cur_x);

            memcpy(
                &new->data[new->length],
                &current->data[cur_x],
                // + 1 so that it also copies the null byte
                current->length + 1 - cur_x
            );

            new->length += current->length - cur_x;

            current->length = cur_x;
            current->data[current->length] = '\0';
        }

        break;
    }
    }

    // TODO: move this all to `default`

    char cc[4];
    cc[0] = c;

    uint32_t codepoint;
    size_t r = grapheme_decode_utf8(cc, 1, &codepoint);

    if (r > 1) {
        for (size_t i = 1; i < r; i++)
            cc[i] = getch();

        grapheme_decode_utf8(cc, r, &codepoint);
    }

    if (GRAPHEME_INVALID_CODEPOINT == codepoint)
        // TODO: maybe I can print a message?
        return;


    if (r > 1 || isprint(c) || '\t' == c) {
        if (modify(buf)) {
            Grapheme g = {r, cc};

            add_char(g, buf->cursor.x_bytes, &buf->lines[buf->cursor.y]);
            process_keypress(KEY_RIGHT);
        }
    }
    

    return;
}

