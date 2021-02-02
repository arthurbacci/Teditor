#include "ted.h"

struct LINE *lines = NULL;
unsigned int num_lines;
unsigned int len_line_number; // The length of the number of the last line

FILE *fp = NULL;
struct CURSOR cursor = {0, 0};
struct TEXT_SCROLL text_scroll = {0, 0};

char *filename;

char colors_on;
char needs_to_free_filename;

void setcolor(int c)
{
    if (colors_on)
    {
        attrset(c);
    }
}

unsigned int last_cursor_x = 0;

struct CFG config = {4, 0, 0};

unsigned int calculate_real_cx(unsigned int *last_one_size)
{
    unsigned int real_cx = 0;
    unsigned int offset = 0;
    *last_one_size = 1;
    for (unsigned int i = 0; i < cursor.x; i++)
    {
        if (lines[cy].data[i + offset] >= 0xC0 && lines[cy].data[i + offset] <= 0xDF)
        {
            offset++;
            real_cx++;
            *last_one_size = 2;
        }
        else if (lines[cy].data[i + offset] >= 0xE0 && lines[cy].data[i + offset] <= 0xEF)
        {
            offset += 2;
            real_cx += 2;
            *last_one_size = 3;
        }
        else if (lines[cy].data[i + offset] >= 0xF0 && lines[cy].data[i + offset] <= 0xF7)
        {
            offset += 3;
            real_cx += 3;
            *last_one_size = 4;
        }
        else {
            *last_one_size = 1;
        }
        real_cx++;
    }
    return real_cx;
}

void free_lines()
{
    for (unsigned int i = 0; i < num_lines; i++)
    {
        free(lines[i].data);
        lines[i].len = 0;
    }
}

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
            for (unsigned int i = 0; i < config.tablen; i++)
            {
                process_keypress(' ');
            }
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

    if (isprint(c))
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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        struct stat st = {0};

        char *home = getenv("HOME");

        char config[strlen(home) + strlen("/.config/") + 1];

        strcpy(config, home);
        strcat(config, "/.config/");


        if (stat(config, &st) == -1)
        {
            mkdir(config, 0777);
        }

        char config_ted[strlen(config) + strlen("ted/") + 1];

        strcpy(config_ted, config);
        strcat(config_ted, "ted/");

        if (stat(config_ted, &st) == -1)
        {
            mkdir(config_ted, 0777);
        }

        filename = malloc(strlen(config_ted) + strlen("buffer") + 1);
        
        strcpy(filename, config_ted);
        strcat(filename, "buffer");

        needs_to_free_filename = 1;
    }
    else {
        filename = argv[1];
        needs_to_free_filename = 0;
    }

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    
    fp = fopen(filename, "r");

    read_lines();

    if (fp != NULL)
    {
        fclose(fp);
    }

    {
        char tmp[50];
        len_line_number = snprintf(tmp, 50, "%u", num_lines + 1);
    }


    if (!has_colors())
    {
        colors_on = 0;
    }
    else if (start_color() != OK)
    {
        colors_on = 0;
    }
    else {
        colors_on = 1;
    }


    if (colors_on)
    {
        use_default_colors();
        init_pair(1, COLOR_RED, -1);
        init_pair(2, -1, -1);
    }


    int c;
    while (1)
    {
        config.LINES = LINES - 1;

        show_lines();
        show_menu();
        move(cursor.y - text_scroll.y, cursor.x - text_scroll.x + len_line_number + 1);
        refresh();

        c = getch();

        if (c == ctrl('c'))
        {
            break;
        }

        process_keypress(c);
    }


    free_lines();

    if (needs_to_free_filename == 1)
    {
        free(filename);
    }

    endwin();
    return 0;
}
