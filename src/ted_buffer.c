#include <ted_buffer.h>
#include <ted_grapheme.h>
#include <ted_xdg.h>
#include <ted_string_utils.h>
#include <ted_longjmp.h>
#include <ted_prompt.h>
#include <ted_plugins.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ted_config.h>

BufferList buffer_list = {0};

void calculate_scroll(Buffer *buf, ScreenInfo screen_info) {
    // TODO: DRY
    
    // y
    if (buf->cursor.y < buf->scroll.y)
        buf->scroll.y = buf->cursor.y;
    if (buf->cursor.y > buf->scroll.y + screen_info.text_height)
        buf->scroll.y = buf->cursor.y - screen_info.text_height;
    
    // x
    if (buf->cursor.x_width < buf->scroll.x_width)
        buf->scroll.x_width = buf->cursor.x_width;
    if (buf->cursor.x_width > buf->scroll.x_width + screen_info.text_width)
        buf->scroll.x_width = buf->cursor.x_width - screen_info.text_width;
}

// TODO: check if there's any usage of truncate_cur except for recalc_cur
// and if not, join the two functions into one
void truncate_cur(Buffer *buf) {
    Line *ln = &buf->lines[buf->cursor.y];
    char *s = ln->data;
    
    buf->cursor.x_width -= index_by_width(buf->cursor.x_width, &s);
    buf->cursor.x_bytes = s - ln->data;
}

void recalc_cur(Buffer *buf) {
    buf->cursor.x_width = buf->cursor.lx_width;
    truncate_cur(buf);
}

void open_default_buffer(void) {
    char *filename = printdup("%s/buffer", ted_data_home);
    open_file(filename);
}

void open_buffer(Buffer b) {
    if (buffer_list.len + 1 < NUM_BUFFERS) {
        buffer_list.selected = buffer_list.len;
        buffer_list.bufs[buffer_list.len++] = b;
    }
}

void buffer_close(void) {
    if (buffer_list.len == 1)
        TED_CALL_LONGJMP(TED_LONGJMP_USER_EXIT);

    buffer_list.len--;

    memmove(
        &buffer_list.bufs[buffer_list.selected],
        &buffer_list.bufs[buffer_list.selected + 1],
        buffer_list.len - buffer_list.selected
    );

    if (buffer_list.selected > 0)
        buffer_list.selected--;
}

// TODO: DRY with the similar structures on the scroll functions
void next_buffer(void) {
    buffer_list.selected++;
    if (buffer_list.selected >= buffer_list.len)
        buffer_list.selected = 0;
}
void previous_buffer(void) {
    if (buffer_list.selected == 0)
        buffer_list.selected = buffer_list.len;
    buffer_list.selected--;
}

void free_buffer(Buffer *buf) {
    for (size_t i = 0; i < buf->num_lines; i++)
        free(buf->lines[i].data);
    free(buf->lines);
    free(buf->filename);
}

bool can_write(const char *fname) {
    struct stat st;
    if (stat(fname, &st) == 0) {
        const bool all_permission = st.st_mode & S_IWOTH;
        const bool owner_permission = getuid() == st.st_uid && st.st_mode && S_IWUSR;
        const bool group_permission = getgid() == st.st_gid && st.st_mode && S_IWGRP;
        return all_permission || owner_permission || group_permission;
    }
    // FIXME: this part of the code is incomplete and may lead to errors, it may be interesting to
    // try to actually write on the file to check if there's permission.
    return errno != EACCES;
}

Line blank_line(void) {
    Line ln = {READ_BLOCKSIZE, 0, malloc(READ_BLOCKSIZE)};
    *ln.data = '\0';
    return ln;
}

void open_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    
    const bool writable = can_write(filename);
    
    Buffer b = {
        {0, 0, 0, 0}, // Cursor
        {0, 0},       // Scroll
        NULL,
        1,
        filename,
        false,      // Modified
        !writable,  // Read-only
        writable,   // Can-write
        false,      // CRFL buffer (if a carriage return is detected this is set)
        DEFAULT_AUTOTAB,
        DEFAULT_INDENT_SIZE,
        DEFAULT_TAB_WIDTH
    };
    
    configure_editorconfig(&b);

    if (!fp) {
        message("New file");

        b.lines = malloc(b.num_lines * sizeof(Line));
        b.lines[0] = blank_line();
        b.read_only = false;
        b.modified = true;

        open_buffer(b);
        return;
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
    
    open_buffer(b);
}

void savefile(Buffer *buf) {
    FILE *fpw = fopen(buf->filename, "w");

    if (fpw == NULL) {
        // TODO: after making message accept dynamic strings print the errno code
        message("could not open file");
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

