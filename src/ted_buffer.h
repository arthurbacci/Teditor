#ifndef TED_BUFFER_H
#define TED_BUFFER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <ted_screen.h>
#include <ted_grapheme.h>

#define NUM_BUFFERS 16
#define SEL_BUF (buffer_list.bufs[buffer_list.selected])

typedef struct {
    size_t cap;
    size_t length;
    char *data;
} Line;

typedef struct {
    size_t lx_width;
    size_t x_width;
    size_t x_bytes;
    
    size_t y;
} Cursor;

typedef struct {
    size_t x_width;
    size_t y;
} TextScroll;

typedef struct {
    Cursor cursor;
    TextScroll scroll;
    
    Line *lines;
    size_t num_lines;
    
    char *filename;
    
    bool modified;
    bool read_only;
    bool can_write;
    bool crlf;
    
    bool autotab_on;
    uint8_t indent_size;
    uint8_t tab_width;
} Buffer;

typedef struct {
    Buffer bufs[NUM_BUFFERS];
    size_t len;
    size_t selected;
} BufferList;

extern BufferList buffer_list;

void calculate_scroll(Buffer *buf, ScreenInfo screen_info);
void recalc_cur(Buffer *buf);
void recalc_restore_cur(Buffer *buf);
void open_default_buffer(void);
void open_buffer(Buffer b);
void buffer_close(void);
void next_buffer(void);
void previous_buffer(void);
bool can_write(const char *fname);
Line blank_line(void);
void open_file(char *fname);
void savefile(Buffer *buf);
size_t get_line_indent_level(Line ln);
bool modify_buffer(Buffer *buf);
void add_char(Grapheme c, size_t x, Line *ln);
void remove_char(size_t x, Line *ln);
void reserve_line_cap(Line *ln, size_t x);


#endif
