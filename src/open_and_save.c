#include "ted.h"

void savefile(Buffer *buf) {
    FILE *fpw = fopen(buf->filename, "w");

    if (fpw == NULL) {
        char buf[1000];
        snprintf(buf, 1000, "Could not open the file; Errno: %d", errno);

        message(buf);
        return;
    }


    for (size_t i = 0; i < buf->num_lines; i++) {

        char *at = buf->lines[i].data;
        while ('\0' != *at) {
            Grapheme grapheme = get_next_grapheme(&at, SIZE_MAX);

            fwrite(grapheme.dt, sizeof(char), grapheme.sz, fpw);
        }


        // If we're not at the last line
        if (buf->num_lines - 1 > i) {
            if (buf->crlf)
                fputc('\r', fpw);
            fputc('\n', fpw);
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
        false,              // true if CRLF false if LF
        NULL,               // lines
        0,                  // number of lines
        {0, 0, 0, 0},       // Cursor (lx_width, x_width, x_bytes, y)
        {0, 0},             // Scroll (x, y)
        bufn(buffer_count), // Buffer Name
        filename,
    };
    if (!fp) {
        message("New file");

        goto EMPTY_BUFFER;
    }

    b.num_lines = 0;
    for (size_t i = 0; !feof(fp); i++) {
        b.lines = realloc(b.lines, ++b.num_lines * sizeof(Line));
        b.lines[i] = blank_line();
        Line *curln = &b.lines[i];


        char c;
        size_t j;
        for (j = 0; EOF != (c = fgetc(fp)) && '\n' != c; j++) {
            if (c == '\r')
                b.crlf = true;

            // TODO: check unicode if file isn't tooooo big
            // For now the buffer is not being asserted to be encoded correctly
            
            if (++curln->length >= curln->cap) {
                curln->cap *= 2;
                curln->data = realloc(curln->data, curln->cap * sizeof(Line));
            }
            curln->data[j] = c;
        }

        curln->data[j] = '\0';
    }

    if (b.num_lines > 0)
        return b;

EMPTY_BUFFER:
    b.num_lines = 1;
    b.lines = malloc(b.num_lines * sizeof(Line));
    b.lines[0] = blank_line();
    b.modified = 1;

    return b;
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
