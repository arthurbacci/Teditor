#ifndef TED_HEADER
#define TED_HEADER

#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>

// suckless' libgrapheme
#include <grapheme.h>

// Screen rows
#define SROW (LINES - 1)
// Screen cols
#define SCOL (COLS - 1)

#define READ_BLOCKSIZE 100
#define ctrl(x) ((x) & 0x1f)

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

#define NUM_PAIRS 6

#define IN_RANGE(x, min, max)  ((x) >= (min)) && ((x) <= (max))
#define OUT_RANGE(x, min, max) ((x) < (min)) || ((x) > (max))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// timeout for input in ncurses (in milliseconds)
#define INPUT_TIMEOUT 5

#define USE(x) (void)(x)

#define MSG_SZ 512

#define CMD_WORD_SZ 128
#define CMD_ARR_SZ 30

#define TED_LONGJMP_DIE 2
#define TED_LONGJMP_USER_EXIT 1
#define TED_CALL_LONGJMP(x) { \
    if (is_jmp_set) longjmp(end, (x)); \
    else exit((x) - 1); }

#define PRETEND_TO_USE(x) (void)(x)

#define NUM_BUFFERS 16

#define SEL_BUF (buffer_list.bufs[buffer_list.selected])

/*--*--TYPES--*--*/

typedef struct {
    size_t sz;
    char *dt;
} Grapheme;

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

typedef struct {
    const char *command;
    const char *hint;
} Hints;

typedef enum {
    DISPLAYABLE_CHAR,
    TABULATION,
    INVALID_UNICODE,
    CONTROL_CHAR
} GraphemeType;


// message_and_prompt.c
char *prompt(const char *msgtmp, char *def);
char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints);
void message(char *msg);

// config_dialog.c
void config_dialog(void);
void parse_command(char *command);

// open_and_save.c
void savefile(Buffer *buf);
Buffer read_lines(FILE *fp, char *filename, bool read_only);
void open_file(char *fname);
bool can_write(char *fname);

// display.c
void display_menu(const char *message, const char *shadow);
void display_buffer(Buffer buf, int len_line_number);

// free.c
void free_buffer(Buffer *buf);

// keypress.c
void expand_line(Line *ln, size_t x);
void process_keypress(int c);

// utils.c
void die(const char *s);
size_t split_cmd_string(const char *s, char ret[CMD_ARR_SZ + 1][CMD_WORD_SZ]);
Line blank_line(void);
char *bufn(int a);
size_t get_ident_sz(char *s);
bool is_whitespace(char c);
int process_as_bool(const char *s);
void ensure_ted_dirs(void);
int invoke_editorconfig(const char *prop, const char *filename);
void configure_editorconfig(Buffer *b);
void replace_fd(int fd, const char *filename, int flags);
char *printdup(const char *format, ...);

// modify.c
bool modify(Buffer *buf);
void add_char(Grapheme c, size_t x, Line *ln);
void remove_char(size_t x, Line *ln);

// cursor.c
void calculate_scroll(Buffer *buf, size_t screen_width);
void truncate_cur(Buffer *buf);
void recalc_cur(Buffer *buf);

// buffers.c
Buffer default_buffer();
void open_buffer(Buffer b);
void buffer_close(void);
void next_buffer(void);
void previous_buffer(void);

// grapheme.c
Grapheme get_next_grapheme(char **str, size_t len);
size_t grapheme_width(Grapheme g);
size_t wi_to_gi(size_t si, char *s);
size_t gi_to_wi(size_t gi, char *s);
ssize_t index_by_width_after(size_t _wi, char **s);
size_t index_by_width(size_t wi, char **s);
bool is_replacement_character(Grapheme g);
Grapheme replacement_character(void);
GraphemeType get_grapheme_type(Grapheme g);

// uuid.c
void uuid_version4(char uuid[37]);

// xdg.c
char *get_ted_data_home();
char *get_ted_config_home();
char *get_ted_state_home();
char *get_ted_cache_home();


extern char *menu_message;
extern bool is_jmp_set;
extern jmp_buf end;
extern BufferList buffer_list;

#include "config.h"

#endif

