#include <ted_input.h>
#include <ted_buffer.h>
#include <ted_prompt.h>
#include <ted_utils.h>
#include <ted_grapheme.h>
#include <ted_commands.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <grapheme.h>


void process_keypress(int c) {
    Buffer *buf = &SEL_BUF;

    if (c == ERR)
        return;

    message("");

    switch (c) {
    case ctrl('c'): {
        parse_command("close");
        break;
    } case ctrl('z'): {
        parse_command("prev");
        break;
    } case ctrl('x'): {
        parse_command("next");
        break;
    } case KEY_UP: case ctrl('p'): {
        buf->cursor.y -= buf->cursor.y > 0;
        recalc_restore_cur(buf);

        break;
    } case KEY_DOWN: case ctrl('n'): {
        buf->cursor.y += buf->cursor.y + 1 < buf->num_lines;
        recalc_restore_cur(buf);
        
        break;
    } case KEY_LEFT: case ctrl('b'): {
        char *s = buf->lines[buf->cursor.y].data;

        size_t x_grapheme = wi_to_gi(buf->cursor.x_width, s);

        if (x_grapheme > 0)
            buf->cursor.x_width = gi_to_wi(x_grapheme - 1, s);

        recalc_cur(buf);

        break;
    } case KEY_RIGHT: case ctrl('f'): {
        char *s = buf->lines[buf->cursor.y].data + buf->cursor.x_bytes;
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);
        size_t gw = grapheme_width(g);

        buf->cursor.x_width += gw;
        recalc_cur(buf);

        break;
    } case KEY_HOME: case ctrl('a'): {
        buf->cursor.x_width = 0;
        recalc_cur(buf);

        break;
    } case KEY_END: case ctrl('e'): {
        buf->cursor.x_width = SIZE_MAX;
        recalc_cur(buf);

        break;
    } case ctrl('s'): {
        if (!buf->read_only)
            savefile(buf);
        break;
    } case ctrl('g'): {
        config_dialog();
        break;
    } case ctrl('q'): {
        parse_command(buf->read_only ? "read-only 0" : "read-only 1");
        break;
    } case KEY_PPAGE: {
        buf->cursor.y -= MIN(buf->cursor.y, LINES - 1);
    
        recalc_restore_cur(buf);
        break;
    } case KEY_NPAGE: {
        buf->cursor.y += MIN(buf->num_lines - 1 - buf->cursor.y, LINES - 1);
    
        recalc_restore_cur(buf);
        break;
    } case ctrl('w'): {
        size_t oldcur = buf->cursor.x_width;
        
        process_keypress(CTRL_KEY_LEFT);
        
        for (size_t i = 0; i < oldcur - buf->cursor.x_width; i++)
            process_keypress(KEY_DC);

        break;
    } case ctrl('o'): {
        char d[MSG_SZ] = "open: ";
        // FIXME: add this back
        // prompt_hints(d, buf->filename, NULL, NULL);
        prompt_hints(d, NULL, NULL);
        if (*d) open_file(d);
        break;
    } case CTRL_KEY_LEFT: case ctrl('h'): {
        char *s = buf->lines[buf->cursor.y].data;

        size_t x_word = wi_to_word(buf->cursor.x_width, s);
        
        buf->cursor.x_width = word_to_wi(x_word > 0 ? x_word - 1 : 0, s);
        recalc_cur(buf);

        break;
    } case CTRL_KEY_RIGHT: case ctrl('l'): {
        char *s = buf->lines[buf->cursor.y].data;
        
        size_t x_word = wi_to_word(buf->cursor.x_width, s);
        
        buf->cursor.x_width = word_to_wi(x_word + 1, s);
        recalc_cur(buf);

        break;
    } case KEY_DC: case 127: {
        size_t oldcur = buf->cursor.x_width;
        process_keypress(KEY_RIGHT);
        if (oldcur != buf->cursor.x_width)
            process_keypress(KEY_BACKSPACE);
        
        break;
    } case KEY_BACKSPACE: {
        if (modify_buffer(buf)) {
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
                recalc_cur(buf);
                buf->cursor.lx_width = buf->cursor.x_width;

                reserve_line_cap(&buf->lines[buf->cursor.y], del_line.length);

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
    } case '\n': case KEY_ENTER: case '\r': {
        if (modify_buffer(buf)) {
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
                size_t indent_level = get_line_indent_level(*current);
                
                reserve_line_cap(new, indent_level);

                memcpy(new->data, current->data, indent_level);
                new->length = indent_level;
                new->data[new->length] = '\0';
            }


            size_t cur_x = buf->cursor.x_bytes;

            buf->cursor.y++;
            buf->cursor.x_width = SIZE_MAX;
            recalc_cur(buf);

            reserve_line_cap(new, current->length - cur_x);

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
    } case '\t': {
        if (buf->indent_size > 0) {
            for (int i = 0; i < buf->indent_size; i++)
                process_keypress(' ');
            break;
        }
        // Fallthrough to indent with tabs
    } default: {
        char cc[4] = {c};

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
            if (modify_buffer(buf)) {
                Grapheme g = {r, cc};

                add_char(g, buf->cursor.x_bytes, &buf->lines[buf->cursor.y]);
                process_keypress(KEY_RIGHT);
            }
        }
    }
    }
}

