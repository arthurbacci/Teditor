#include "ted.h"

void expand_line(unsigned int at, int x, Buffer *buf) {
    while (buf->lines[at].len <= buf->lines[at].length + x) {
        buf->lines[at].len += READ_BLOCKSIZE;
        buf->lines[at].data = realloc(
            buf->lines[buf->cursor.y].data,
            buf->lines[buf->cursor.y].len * sizeof(uchar32_t)
        );
    }
}

void new_line(unsigned int at, int x, Buffer *buf) {
    buf->lines[at].len = READ_BLOCKSIZE;
    buf->lines[at].data = malloc(buf->lines[at].len * sizeof(uchar32_t));
    buf->lines[at].length = 0;

    expand_line(at, buf->lines[at - 1].length - x + 1, buf);
    memcpy(
        buf->lines[at].data,
        &buf->lines[at - 1].data[x],
        (buf->lines[at - 1].length - x) * sizeof(uchar32_t)
    );

    buf->lines[at].length += buf->lines[at - 1].length - x;
    buf->lines[at].data[buf->lines[at].length] = '\0';

    buf->lines[at - 1].length = x;
    buf->lines[at - 1].data[buf->lines[at - 1].length] = '\0';
}

void process_keypress(int c, Buffer *buf) {
    if (c != ERR)
        message("");

    switch (c) {
    case KEY_UP:
    case ctrl('p'):
        change_position(buf->cursor.last_x, buf->cursor.y - (buf->cursor.y > 0), buf);
        break;
    case KEY_DOWN:
    case ctrl('n'):
        change_position(buf->cursor.last_x, buf->cursor.y + 1, buf);
        break;
    case KEY_LEFT:
    case ctrl('b'):
        buf->cursor.x -= (buf->cursor.x > 0);
        cursor_in_valid_position(buf);
        buf->cursor.last_x = buf->cursor.x;
        break;
    case KEY_RIGHT:
    case ctrl('f'):
        buf->cursor.x++;
        cursor_in_valid_position(buf);
        buf->cursor.last_x = buf->cursor.x;
        break;
    case KEY_HOME:
    case ctrl('a'):
        buf->cursor.x = 0;
        buf->cursor.last_x = buf->cursor.x;
        cursor_in_valid_position(buf);
        break;
    case KEY_END:
    case ctrl('e'):
        buf->cursor.x = buf->lines[buf->cursor.y].length;
        buf->cursor.last_x = buf->cursor.x;
        cursor_in_valid_position(buf);
        break;
    case ctrl('s'):
        if (!buf->read_only)
            savefile(*buf);
        break;
    case '\t':
        if (config.use_spaces == 1) {
            for (unsigned int i = 0; i < config.tablen; i++)
                process_keypress(' ', buf);
            return;
        } // else, it will pass though and be added to the buffer
        break;
    case ctrl('g'):
        config_dialog(buf);
        break;
    case ctrl('q'):
        parse_command(
            buf->read_only ? "read-only 0" : "read-only 1",
            buf
        );
        break;
    case KEY_PPAGE:
    {
        unsigned int ccy = buf->cursor.y;
        for (unsigned int i = 0; i < (unsigned int)(ccy % config.lines + config.lines) && buf->cursor.y > 0; i++)
            buf->cursor.y--;
        cursor_in_valid_position(buf);
        break;
    } case KEY_NPAGE:
    {
        unsigned int ccy = buf->cursor.y;
        for (unsigned int i = 0; i < (unsigned int)(config.lines - (ccy % config.lines) - 1 + config.lines) && buf->cursor.y < buf->num_lines - 1; i++)
            buf->cursor.y++;
        cursor_in_valid_position(buf);
        break;
    } case KEY_MOUSE:
    {
        MEVENT event;
        if (getmouse(&event) == OK)
            process_mouse_event(event, buf);

        break;
    } case 0x209:
    {
        if (modify(buf)) {
            if (buf->num_lines > 1) {
                free(buf->lines[buf->cursor.y].data);
                memmove(
                    &buf->lines[buf->cursor.y],
                    &buf->lines[buf->cursor.y + 1],
                    (buf->num_lines - buf->cursor.y - 1) * sizeof(*buf->lines)
                );
                buf->lines = realloc(buf->lines, --buf->num_lines * sizeof(*buf->lines));
            } else {
                buf->lines[buf->cursor.y].data[0] = '\0';
                buf->lines[buf->cursor.y].length = 0;
            }
            cursor_in_valid_position(buf);
        }
        break;
    } case ctrl('w'):
    {
        bool passed_spaces = 0;
        while (buf->cursor.x > 0 && (!strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x - 1]) || !passed_spaces)) {
            if (!remove_char(buf->cursor.x - 1, buf->cursor.y, buf))
                break;
            process_keypress(KEY_LEFT, buf);
            if (buf->cursor.x > 0 && !strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x - 1]))
                passed_spaces = 1;
        }
        break;
    } case ctrl('o'):
    {
        char *d = prompt("open: ", filename);
        if (d)
            open_file(d, 1, buf);
        break;
    } case CTRL_KEY_LEFT:
    {
        char passed_spaces = 0;
        while (buf->cursor.x > 0) {
            process_keypress(KEY_LEFT, buf);
            if (!strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]))
                passed_spaces = 1;
            if (strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]) && passed_spaces) {
                process_keypress(KEY_RIGHT, buf);
                break;
            }
        }
        break;
    }
    case CTRL_KEY_RIGHT:
    {
        char passed_spaces = 0;
        while (buf->lines[buf->cursor.y].data[buf->cursor.x] != '\0' && !(strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]) && passed_spaces)) {
            if (!strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]))
                passed_spaces = 1;
            process_keypress(KEY_RIGHT, buf);
        }
        break;
    } case KEY_BACKSPACE: case KEY_DC: case 127:
    {
        if (modify(buf)) {
            buf->lines[buf->cursor.y].ident -= buf->cursor.x <= buf->lines[buf->cursor.y].ident && buf->cursor.x > 0;

            if (buf->cursor.x >= 1) {
                if (remove_char(buf->cursor.x - 1, buf->cursor.y, buf))
                    process_keypress(KEY_LEFT, buf);
            } else if (buf->cursor.y > 0) {
                Line del_line = buf->lines[buf->cursor.y];
                memmove(
                    &buf->lines[buf->cursor.y],
                    &buf->lines[buf->cursor.y + 1],
                    (buf->num_lines - buf->cursor.y - 1) * sizeof(*buf->lines)
                );
                buf->lines = realloc(buf->lines, --buf->num_lines * sizeof(*buf->lines));

                buf->cursor.y -= buf->cursor.y > 0;
                buf->cursor.x = buf->lines[buf->cursor.y].length;
                cursor_in_valid_position(buf);

                process_keypress(KEY_RIGHT, buf);
                expand_line(buf->cursor.y, del_line.length, buf);

                memmove(
                    &buf->lines[buf->cursor.y].data[buf->lines[buf->cursor.y].length],
                    del_line.data,
                    del_line.length * sizeof(uchar32_t)
                );
                buf->lines[buf->cursor.y].length += del_line.length;

                buf->lines[buf->cursor.y].data[buf->lines[buf->cursor.y].length] = '\0';

                free(del_line.data);
            }

            buf->lines[buf->cursor.y].ident = 0;
            for (unsigned int i = 0; buf->lines[buf->cursor.y].data[i] != '\0'; i++) {
                if (buf->lines[buf->cursor.y].data[i] != ' ')
                    break;
                buf->lines[buf->cursor.y].ident++;
            }
        }
        break;
    } case '\n': case KEY_ENTER: case '\r':
    {
        if (modify(buf)) {
            buf->lines = realloc(buf->lines, (buf->num_lines + 1) * sizeof(*buf->lines));
            memmove(
                &buf->lines[buf->cursor.y + 2],
                &buf->lines[buf->cursor.y + 1],
                (buf->num_lines - buf->cursor.y - 1) * sizeof(*buf->lines)
            );
            buf->num_lines++;

            unsigned int lcx = buf->cursor.x;
            buf->cursor.last_x = 0;
            buf->cursor.y++;
            buf->cursor.x = 0;
            cursor_in_valid_position(buf);
            new_line(buf->cursor.y, lcx, buf);

            if (config.autotab == 1) {
                const unsigned int ident = buf->lines[buf->cursor.y - 1].ident;
                buf->lines[buf->cursor.y].ident = ident;
                buf->lines[buf->cursor.y].len += ident + 1;
                buf->lines[buf->cursor.y].data = realloc(buf->lines[buf->cursor.y].data, buf->lines[buf->cursor.y].len * sizeof(uchar32_t));
                memmove(&buf->lines[buf->cursor.y].data[ident], buf->lines[buf->cursor.y].data, (buf->lines[buf->cursor.y].length + 1) * sizeof(uchar32_t));

                for (unsigned int i = 0; i < ident; i++)
                    buf->lines[buf->cursor.y].data[i] = ' ';
                buf->lines[buf->cursor.y].length += ident;

                for (unsigned int i = 0; i < ident; i++)
                    process_keypress(KEY_RIGHT, buf);
            } else
                buf->lines[buf->cursor.y].ident = 0;
        }
        break;
    }
    }

    if (isprint(c) || c == '\t' || (c >= 0xC0 && c <= 0xDF) || (c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF7)) {

        if (modify(buf)) {
            if (c == ' ' && buf->cursor.x <= buf->lines[buf->cursor.y].ident)
                buf->lines[buf->cursor.y].ident++;

            unsigned char ucs[4] = {c, 0, 0, 0};
            int len = 1;

            if ((c >= 0xC2 && c <= 0xDF) || (c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF4)) {
                ucs[1] = getch(), len++;
            }
            if ((c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF4)) {
                ucs[2] = getch(), len++;
            }
            if (c >= 0xF0 && c <= 0xF4) {
                ucs[3] = getch(), len++;
            }

            if (validate_utf8(ucs)) {
                uchar32_t ec = c;
                for (int i = 1, off = 8; i < len; i++, off += 8)
                    ec += ucs[i] << off;

                if (add_char(buf->cursor.x, buf->cursor.y, ec, buf))
                    process_keypress(KEY_RIGHT, buf);
            } else {
                for (int i = 0; i < len; i++) {
                    if (add_char(buf->cursor.x, buf->cursor.y, substitute_char, buf))
                        process_keypress(KEY_RIGHT, buf);
                    else
                        break;
                }
            }
        }
    }
}

