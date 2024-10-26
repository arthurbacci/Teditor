#include "ted.h"

void savefile(Buffer *buf) {
    FILE *fpw = fopen(buf->filename, "w");

    if (fpw == NULL) {
        static char buf[1000];
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
        false,              // Modified
        !can_write,         // read-only
        can_write,          // data can be written to the buffer
        false,              // true if CRLF false if LF
        NULL,               // lines
        1,                  // number of lines
        {0, 0, 0, 0},       // Cursor (lx_width, x_width, x_bytes, y)
        {0, 0},             // Scroll (x, y)
        bufn(buffer_count), // Buffer Name
        filename,
    };


    if (!fp) {
        message("New file");

        b.lines = malloc(b.num_lines * sizeof(Line));
        b.lines[0] = blank_line();
        b.read_only = false;
        b.modified = true;

        return b;
    }
    
    for (; !feof(fp); b.num_lines++) {
        b.lines = realloc(b.lines, b.num_lines * sizeof(Line));

        Line *curln = &b.lines[b.num_lines - 1];
        *curln = blank_line();

        for (int c; EOF != (c = fgetc(fp)) && '\n' != c; curln->length++) {
            if ('\r' == c) {
                b.crlf = true;
                continue;
            }

            if (curln->length + 1 >= curln->cap) {
                curln->cap *= 2;
                curln->data = realloc(curln->data, curln->cap * sizeof(char));
            }

            curln->data[curln->length] = c;
        }
        curln->data[curln->length] = '\0';
    }

    b.num_lines--;
    

    fclose(fp);


    return b;
}

void open_file(char *fname) {
    FILE *fp = fopen(fname, "r");
    open_buffer(read_lines(fp, fname, can_write(fname)));
}

// FIXME: make it check the directory if the files doesn't exist
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

