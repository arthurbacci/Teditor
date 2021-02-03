#include "ted.h"

void process_keypress(int c)
{
    switch (c)
    {
        case KEY_UP:
            cursor.y -= (cursor.y > 0);

            if (cursor.x < last_cursor_x)
            {
                cursor.x = last_cursor_x;
                last_cursor_x = 0;
            }

            if (cursor.x > lines[cursor.y].length)
            {
                last_cursor_x = cursor.x;
                cursor.x = lines[cursor.y].length;
            }
            if (cursor.y < text_scroll.y)
            {
                text_scroll.y = cursor.y;
            }

                

            if (cursor.x < text_scroll.x)
            {
                text_scroll.x = cursor.x;
            }
            else if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
            {
                text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
            }
            break;
        case KEY_DOWN:
            cursor.y += (cursor.y < num_lines - 1);

            if (cursor.x < last_cursor_x)
            {
                cursor.x = last_cursor_x;
                last_cursor_x = 0;
            }

            if (cursor.x > lines[cursor.y].length)
            {
                last_cursor_x = cursor.x;
                cursor.x = lines[cursor.y].length;
            }
            if (cursor.y > text_scroll.y + config.LINES - 1)
            {
                text_scroll.y = cursor.y + 1 - config.LINES;
            }

            if (cursor.x < text_scroll.x)
            {
                text_scroll.x = cursor.x;
            }
            else if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
            {
                text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
            }
            break;
        case KEY_LEFT:
            cursor.x -= (cursor.x > 0);
            if (cursor.x < text_scroll.x)
            {
                text_scroll.x = cursor.x;
            }
            break;
        case KEY_RIGHT:
            cursor.x += (cursor.x < lines[cursor.y].length);
            if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
            {
                text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
            }
            break;
        case KEY_HOME:
            cursor.x = 0;
            if (cursor.x < text_scroll.x)
            {
                text_scroll.x = cursor.x;
            }
            break;
        case KEY_END:
            cursor.x = lines[cursor.y].length;
            if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
            {
                text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
            }
            break;
        case ctrl('s'):
            savefile();
            break;
        case '\t':
            if (config.use_spaces == 1)
            {
                for (unsigned int i = 0; i < config.tablen; i++)
                {
                    process_keypress(' ');
                }
                return;
            }
            // else, it will pass though and be add to the buffer
            break;
        case ctrl('h'):
            config_dialog();
            break;
        case KEY_PPAGE:
        {
            unsigned int ccy = cy;
            for (
                unsigned int i = 0;
                i < (unsigned int)(ccy % config.LINES + config.LINES);
                i++
            )
            {
                process_keypress(KEY_UP);
            }
            break;
        }
        case KEY_NPAGE:
        {
            unsigned int ccy = cy;
            for (
                unsigned int i = 0;
                i < (unsigned int)(config.LINES - (ccy % config.LINES) - 1 + config.LINES);
                i++
            )
            {
                process_keypress(KEY_DOWN);
            }
            break;
        }
        case CTRL_KEY_RIGHT:
        {
            unsigned int trash;
            char passed_spaces = 0;
            while (
                lines[cy].data[calculate_real_cx(&trash)] != '\0' &&
                !(lines[cy].data[calculate_real_cx(&trash)] == ' ' && passed_spaces)
            )
            {
                if (lines[cy].data[calculate_real_cx(&trash)] != ' ')
                {
                    passed_spaces = 1;
                }
                process_keypress(KEY_RIGHT);
            }
            break;
        }
        case CTRL_KEY_LEFT:
        {
            unsigned int trash;
            char passed_spaces = 0;
            while (
                calculate_real_cx(&trash) > 0 &&
                !(lines[cy].data[calculate_real_cx(&trash) - 1] == ' ' && passed_spaces)
            )
            {
                if (lines[cy].data[calculate_real_cx(&trash) - 1] != ' ')
                {
                    passed_spaces = 1;
                }
                process_keypress(KEY_LEFT);
            }
            break;
        }
    }

    unsigned int last_one_size;
    unsigned int real_cx = calculate_real_cx(&last_one_size);

    if (isprint(c) || c == '\t')
    {
        if (c == ' ' && cx <= lines[cy].ident)
        {
            lines[cy].ident++;
        }

        if (lines[cy].len <= lines[cy].real_length + 1)
        {
            lines[cy].len += READ_BLOCKSIZE;
            lines[cy].data = realloc(lines[cy].data, lines[cy].len);
        }

        memmove(&lines[cy].data[real_cx + 1], &lines[cy].data[real_cx], lines[cy].real_length - real_cx);

        lines[cy].data[real_cx] = c;
        lines[cy].data[lines[cy].real_length + 1] = '\0';

        lines[cy].real_length++;
        lines[cy].length++;

        process_keypress(KEY_RIGHT);
    }
    else if (c >= 0xC0 && c <= 0xDF)
    {
        while (lines[cy].len <= lines[cy].real_length + 2)
        {
            lines[cy].len += READ_BLOCKSIZE;
            lines[cy].data = realloc(lines[cy].data, lines[cy].len);
        }

        memmove(&lines[cy].data[real_cx + 2], &lines[cy].data[real_cx], lines[cy].real_length - real_cx);

        lines[cy].data[real_cx] = c;
        lines[cy].data[real_cx + 1] = getch();
        lines[cy].data[lines[cy].real_length + 2] = '\0';

        lines[cy].real_length += 2;

        lines[cy].length++;

        process_keypress(KEY_RIGHT);
    }
    else if (c >= 0xE0 && c <= 0xEF)
    {
        while (lines[cy].len <= lines[cy].real_length + 3)
        {
            lines[cy].len += READ_BLOCKSIZE;
            lines[cy].data = realloc(lines[cy].data, lines[cy].len);
        }

        memmove(&lines[cy].data[real_cx + 3], &lines[cy].data[real_cx], lines[cy].real_length - real_cx);

        lines[cy].data[real_cx] = c;
        lines[cy].data[real_cx + 1] = getch();
        lines[cy].data[real_cx + 2] = getch();
        lines[cy].data[lines[cy].real_length + 3] = '\0';

        lines[cy].real_length += 3;

        lines[cy].length++;

        process_keypress(KEY_RIGHT);
    }
    else if (c >= 0xF0 && c <= 0xF7)
    {
        while (lines[cy].len <= lines[cy].real_length + 4)
        {
            lines[cy].len += READ_BLOCKSIZE;
            lines[cy].data = realloc(lines[cy].data, lines[cy].len);
        }

        memmove(&lines[cy].data[real_cx + 4], &lines[cy].data[real_cx], lines[cy].real_length - real_cx);

        lines[cy].data[real_cx] = c;
        lines[cy].data[real_cx + 1] = getch();
        lines[cy].data[real_cx + 2] = getch();
        lines[cy].data[real_cx + 3] = getch();
        lines[cy].data[lines[cy].real_length + 4] = '\0';

        lines[cy].real_length += 4;

        lines[cy].length++;

        process_keypress(KEY_RIGHT);
    }
    else if (c == KEY_BACKSPACE)
    {
        if (cx <= lines[cy].ident && cx > 0)
        {
            lines[cy].ident--;
        }

        if (real_cx >= last_one_size)
        {
            memmove(
                &lines[cy].data[real_cx - last_one_size],
                &lines[cy].data[real_cx],
                lines[cy].real_length - real_cx
            );
            lines[cy].real_length -= last_one_size;
            lines[cy].length--;
            lines[cy].data[lines[cy].real_length] = '\0';

            process_keypress(KEY_LEFT);
        }
        else if (cy > 0)
        {
            unsigned char *del_line = lines[cy].data;
            unsigned int del_line_len = lines[cy].real_length;

            memmove(
                &lines[cy],
                &lines[cy + 1],
                (num_lines - cy - 1) * sizeof(struct LINE)
            );

            num_lines--;
            lines = realloc(lines, num_lines * sizeof(struct LINE));

    
            process_keypress(KEY_UP);

            cursor.x = lines[cy].length;
            
            process_keypress(KEY_RIGHT);

            while (lines[cy].len <= lines[cy].real_length + del_line_len)
            {
                lines[cy].len += READ_BLOCKSIZE;
                lines[cy].data = realloc(lines[cy].data, lines[cy].len);
            }

            
            for (unsigned int i = 0; i < del_line_len; i++)
            {
                lines[cy].data[lines[cy].real_length + i] = del_line[i];

                lines[cy].length++;

                unsigned char cc = del_line[i];
                if (cc >= 0xC0 && cc <= 0xDF)
                {
                    lines[cy].length--;
                }
                else if (cc >= 0xE0 && cc <= 0xEF)
                {
                    lines[cy].length -= 2;
                }
                else if (cc >= 0xF0 && cc <= 0xF7)
                {
                    lines[cy].length -= 3;
                }

            }

            lines[cy].real_length += del_line_len;
            lines[cy].data[lines[cy].real_length] = '\0';

            free(del_line);
        }

        lines[cy].ident = 0;
        for (unsigned int i = 0; lines[cy].data[i] != '\0'; i++)
        {
            if (lines[cy].data[i] != ' ')
            {
                break;
            }
            lines[cy].ident++;
        }
    }
    else if (c == '\n' || c == KEY_ENTER || c == '\r')
    {
        lines = realloc(lines, (num_lines + 1) * sizeof(struct LINE));
    
        memmove(&lines[cy + 2], &lines[cy + 1], (num_lines - cy - 1) * sizeof(struct LINE));

        num_lines++;

        cursor.x = 0;
        last_cursor_x = 0;
        process_keypress(KEY_DOWN);

        lines[cy].len = READ_BLOCKSIZE;
        lines[cy].data = malloc(lines[cy].len);

        lines[cy].length = 0;
        lines[cy].real_length = 0;
        

        for (unsigned int i = 0; i < lines[cy - 1].real_length - real_cx; i++)
        {
            int cc = lines[cy - 1].data[i + real_cx];

            lines[cy].real_length++;
            lines[cy].length++;
            while (lines[cy].real_length >= lines[cy].len)
            {
                lines[cy].len += READ_BLOCKSIZE;
                lines[cy].data = realloc(lines[cy].data, lines[cy].len * sizeof(struct LINE));
            }
            lines[cy].data[i] = lines[cy - 1].data[i + real_cx];
            
            if (cc >= 0xC0 && cc <= 0xDF)
            {
                lines[cy].length--;
            }
            else if (cc >= 0xE0 && cc <= 0xEF)
            {
                lines[cy].length -= 2;
            }
            else if (cc >= 0xF0 && cc <= 0xF7)
            {
                lines[cy].length -= 3;
            }
        }
        lines[cy].data[lines[cy].real_length] = '\0';

        lines[cy - 1].length = real_cx;
        lines[cy - 1].real_length = real_cx;

        for (unsigned int i = 0; i < lines[cy - 1].real_length; i++)
        {
            int cc = lines[cy - 1].data[i];

            if (cc >= 0xC0 && cc <= 0xDF)
            {
                lines[cy - 1].length--;
            }
            else if (cc >= 0xE0 && cc <= 0xEF)
            {
                lines[cy - 1].length -= 2;
            }
            else if (cc >= 0xF0 && cc <= 0xF7)
            {
                lines[cy - 1].length -= 3;
            }
        }

        lines[cy - 1].data[lines[cy - 1].real_length] = '\0';

        char tmp[50];
        len_line_number = snprintf(tmp, 50, "%u", num_lines + 1);

        
        const unsigned int ident = lines[cy - 1].ident;
        lines[cy].ident = ident;

        lines[cy].len += ident;
        lines[cy].data = realloc(lines[cy].data, lines[cy].len);

        memmove(&lines[cy].data[ident], lines[cy].data, lines[cy].real_length + 1);

        for (unsigned int i = 0; i < ident; i++)
        {
            lines[cy].data[i] = ' ';
        }

        lines[cy].length += ident;
        lines[cy].real_length += ident;
        
        for (unsigned int i = 0; i < ident; i++)
        {
            process_keypress(KEY_RIGHT);
        }

    }
}
