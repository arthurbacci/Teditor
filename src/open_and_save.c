#include "ted.h"

void savefile(Buffer *buf) {
    FILE *fpw = fopen(buf->filename, "w");

    if (fpw == NULL) {
        char buf[1000];
        snprintf(buf, 1000, "Could not open the file; Errno: %d", errno);

        message(buf);
        return;
    }

    if (config.insert_newline && buf->lines[buf->num_lines - 1].length > 0) {
        buf->lines = realloc(buf->lines, ++buf->num_lines * sizeof(*buf->lines));
        buf->lines[buf->num_lines - 1] = blank_line();
    }

    for (unsigned int i = 0; i < buf->num_lines; i++) {
        for (unsigned int j = 0; j < buf->lines[i].length; j++) {
            unsigned char b[4];
            int len = utf8ToMultibyte(buf->lines[i].data[j], b, 0);
            fwrite(b, sizeof(unsigned char), len, fpw);
        }
        if (buf->num_lines > 1) {
            if (buf->line_break_type == 0)
                fputc('\n', fpw);
            else
                fputs("\r\n", fpw);
        }
    }
    fclose(fpw);

    buf->modified = 0;
}

Buffer read_lines(FILE *fp, char *filename, bool can_write) {
    static int buffer_count = -1;
    buffer_count++;
    Buffer b = {
        0,                  // Modified
        !can_write,         // read-only
        can_write,          // data can be written to the buffer
        0,                  // line break type: defaults to LF
        NULL,               // lines
        0,                  // number of lines
        {0, 0, 0},          // Cursor (x, last_x, y)
        {0, 0},             // Scroll (x, y)
        bufn(buffer_count), // Buffer Name
        filename,
    };
    if (!fp) {
        b.num_lines = 1;
        b.lines = malloc(b.num_lines * sizeof(*(b.lines)));
        b.lines[0] = blank_line();
        b.modified = 1;
        return b;
    }

    b.line_break_type = detect_linebreak(fp);

    b.num_lines = 0;
    for (unsigned int i = 0; !feof(fp); i++) {

        if (fgetc(fp) == EOF && b.num_lines > 0)
            break;
        else
            fseek(fp, -1, SEEK_CUR);

        b.lines = realloc(b.lines, ++b.num_lines * sizeof(*(b.lines)));

        b.lines[i] = blank_line();

        char c;
        unsigned int j;
        char passed_spaces = 0;

        for (j = 0; (c = fgetc(fp)) != '\n' && c != EOF; j++) {
            if (c == '\r')
                continue;

            if (b.lines[i].length + 1 >= b.lines[i].len) {
                b.lines[i].len += READ_BLOCKSIZE;
                b.lines[i].data = realloc(b.lines[i].data, b.lines[i].len * sizeof(*b.lines[i].data));
            }

            if (passed_spaces == 0 && c != ' ')
                passed_spaces = 1;
            else if (passed_spaces == 0)
                b.lines[i].ident++;

            unsigned char uc = c;
            utf8ReadFile(uc, &b.lines[i].data[j], fp);
            b.lines[i].length++;
        }
        
        b.lines[i].data[j] = '\0';
        
        if (b.line_break_type == 1)
            fgetc(fp);
    }
    return b;
}

unsigned char detect_linebreak(FILE *fp) {
    unsigned char line_break_type = 0;
    char c;
    while (!feof(fp)) {
        c = fgetc(fp);

        if (c == '\r') {
            line_break_type = 1;
            break;
        } else if (c == '\n') {
            line_break_type = 0;
            break;
        }
    }
    rewind(fp);
    return line_break_type;
}

void open_file(char *fname, Node **n) {
    FILE *fp = fopen(fname, "r");
    buffer_add_next(*n, read_lines(fp, fname, can_write(fname)));
    parse_command("next", n);
    if (fp != NULL)
        fclose(fp);
}

bool can_write(char *fname) {
    struct stat st;
    if (stat(fname, &st) == 0) {
        // all user write permission
        return (st.st_mode & S_IWOTH)
            // owner write permission
            || (getuid() == st.st_uid && (st.st_mode & S_IWUSR))
            // group write permission
            || (getgid() == st.st_gid && (st.st_mode & S_IWGRP));
    } else
        // if stat fails and errno is not EACCES, can_write will be true
        return errno != EACCES;
}
