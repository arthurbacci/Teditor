#include "ted.h"

void savefile() {
    FILE *fpw = fopen(filename, "w");

    if (fpw == NULL)
    {
        char buf[1000];
        snprintf(buf, 1000, "Could not open the file\nErrno: %d\nPress any key", errno);

        message(buf);

        return;
    }

    for (unsigned int i = 0; i < num_lines; i++) {
        fputs((const char *)lines[i].data, fpw);
        if (num_lines > 1) {
            if (config.line_break_type == 0)
                fputc('\n', fpw);
            else if (config.line_break_type == 1)
                fputs("\r\n", fpw);
            else if (config.line_break_type == 2)
                fputc('\r', fpw);
        }
    }

    fclose(fpw);
}

void read_lines() {
    if (fp == NULL) {
        num_lines = 1;
        lines = malloc(sizeof(struct LINE));

        lines[0].len = READ_BLOCKSIZE;
        lines[0].data = malloc(lines[0].len);
        lines[0].length = 0;
        lines[0].real_length = 0;
        lines[0].data[0] = '\0';
        lines[0].ident = 0;

        return;
    }

    detect_linebreak();

    char lineend = '\n';

    if (config.line_break_type != 0)
        lineend = '\r';

    num_lines = 0;
    for (unsigned int i = 0; !feof(fp); i++) {

        if (fgetc(fp) == EOF && num_lines > 0)
            break;
        else
            fseek(fp, -1, SEEK_CUR);

        lines = realloc(lines, ++num_lines * sizeof(struct LINE));

        lines[i].len = READ_BLOCKSIZE;
        lines[i].data = malloc(lines[i].len);
        lines[i].length = 0;
        lines[i].ident = 0;

        char c;
        unsigned int j;
        char passed_spaces = 0;


        for (j = 0; (c = fgetc(fp)) != lineend && c != EOF; j++) {

            if (j >= lines[i].len) {
                lines[i].len += READ_BLOCKSIZE;
                lines[i].data = realloc(lines[i].data, lines[i].len);
            }

            if (passed_spaces == 0 && c != ' ')
                passed_spaces = 1;
            else if (passed_spaces == 0)
                lines[i].ident++;

            unsigned char uc = *(unsigned char *)&c;

            
            if (!(isprint(uc) || (uc >= 0xC0 && uc <= 0xDF) || (uc >= 0xE0 && uc <= 0xEF) || (uc >= 0xF0 && uc <= 0xF7) || uc == '\r' || uc == '\t'))
                lines[i].data[j] = '+';
            else if (uc >= 0xC0 && uc <= 0xDF) {
                if (uc == '\0')
                    break;
                if (j + 1 >= lines[i].len) {
                    lines[i].len += READ_BLOCKSIZE;
                    lines[i].data = realloc(lines[i].data, lines[i].len);
                }
                for (unsigned int k = 0; k < 2; k++) {
                    lines[i].data[j + k] = c;
                    if (k < 1)
                        c = fgetc(fp);
                }
                j++;
            } else if (uc >= 0xE0 && uc <= 0xEF) {
                if (uc == '\0')
                    break;
                if (j + 2 >= lines[i].len) {
                    lines[i].len += READ_BLOCKSIZE;
                    lines[i].data = realloc(lines[i].data, lines[i].len);
                }
                for (unsigned int k = 0; k < 3; k++) {
                    lines[i].data[j + k] = c;
                    if (k < 2)
                        c = fgetc(fp);
                }
                j += 2;
            } else if (uc >= 0xF0 && uc <= 0xF7) {
                if (uc == '\0')
                    break;
                if (j + 3 >= lines[i].len) {
                    lines[i].len += READ_BLOCKSIZE;
                    lines[i].data = realloc(lines[i].data, lines[i].len);
                }
                for (unsigned int k = 0; k < 4; k++) {
                    lines[i].data[j + k] = c;
                    if (k < 3)
                        c = fgetc(fp);
                }
                j += 3;
            } else
                lines[i].data[j] = uc;

            lines[i].length++;
        }

        if (j >= lines[i].len) {
            lines[i].len += READ_BLOCKSIZE;
            lines[i].data = realloc(lines[i].data, lines[i].len);
        }

        lines[i].real_length = j;
        lines[i].data[j] = '\0';

        if (config.line_break_type == 1)
            fgetc(fp);
    }
}

void detect_linebreak() {
    char c;
    while (!feof(fp)) {
        c = fgetc(fp);

        if (c == '\r') {
            if (fgetc(fp) == '\n')
                config.line_break_type = 1;
            else
                config.line_break_type = 2;

            break;
        } else if (c == '\n') {
            config.line_break_type = 0;
            break;
        }
    }
    fseek(fp, 0, SEEK_SET);
}
