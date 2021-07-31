#include "ted.h"

void savefile(Buffer buf) {
    FILE *fpw = fopen(filename, "w");

    if (fpw == NULL) {
        char buf[1000];
        snprintf(buf, 1000, "Could not open the file\nErrno: %d\nPress any key", errno);

        message(buf);
        return;
    }

    if (config.insert_newline && buf.lines[buf.num_lines - 1].length > 0) {
        buf.lines = realloc(buf.lines, ++buf.num_lines * sizeof(*buf.lines));
        buf.lines[buf.num_lines - 1] = blank_line();
    }

    for (unsigned int i = 0; i < buf.num_lines; i++) {
        for (unsigned int j = 0; j < buf.lines[i].length; j++) {
            unsigned char b[4];
            int len = utf8ToMultibyte(buf.lines[i].data[j], b, 0);
            fwrite(b, sizeof(unsigned char), len, fpw);
        }
        if (buf.num_lines > 1) {
            if (buf.line_break_type == 0)
                fputc('\n', fpw);
            else if (buf.line_break_type == 1)
                fputs("\r\n", fpw);
            else if (buf.line_break_type == 2)
                fputc('\r', fpw);
        }
    }
    fclose(fpw);

    buf.modified = 0;
}

Buffer read_lines(FILE *fp, bool can_write) {
    Buffer b = {
        0,               // Modified
        !can_write,     // read-only
        can_write,     // data can be written to the buffer
        0,            // line break type: defaults to LF
        NULL,        // lines
        0,          // number of lines
        {0, 0, 0}, // Cursor (x, last_x, y)
        {0, 0},   // Scroll (x, y)
    };
    if (!fp) {
        b.num_lines = 1;
        b.lines = malloc(b.num_lines * sizeof(*(b.lines)));
        b.lines[0] = blank_line();
        return b;
    }

    b.line_break_type = detect_linebreak(fp);
    char lineend = b.line_break_type == 0 ? '\n' : '\r';

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

        for (j = 0; (c = fgetc(fp)) != lineend && c != EOF; j++) {

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
            if (fgetc(fp) == '\n')
                line_break_type = 1;
            else
                line_break_type = 2;

            break;
        } else if (c == '\n') {
            line_break_type = 0;
            break;
        }
    }
    rewind(fp);
    return line_break_type;
}

void open_file(char *fname, bool needs_to_free_new_filename, Buffer *buf) {
    if (needs_to_free_filename)
        free(filename);
    
    filename = fname;
    needs_to_free_filename = needs_to_free_new_filename;
    
    buf->scroll.x = 0;
    buf->scroll.y = 0;
    free_buffer(buf);
    
    FILE *fp = fopen(filename, "r");
    *buf = read_lines(fp, can_write(filename));
    if (fp != NULL)
        fclose(fp);
}

bool can_write(char *fname) {
    struct stat st;
    if (stat(fname, &st) == 0) {
        return (st.st_mode & S_IWOTH) || // all user write permission
            (getuid() == st.st_uid && (st.st_mode & S_IWUSR)) || // owner write permission
            (getgid() == st.st_gid && (st.st_mode & S_IWGRP));  // group write permission
    } else // if stat fails and errno is not EACCES, can_write will be true
        return errno != EACCES;
}
