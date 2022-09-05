#ifndef TED_HEADER
#define TED_HEADER

#define _POSIX_C_SOURCE 1

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

// suckless' libgrapheme
#include <grapheme.h>


#define READ_BLOCKSIZE 100
#define ctrl(x) ((x) & 0x1f)

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

#define NUM_PAIRS 6

#define IN_RANGE(x, min, max)  ((x) >= (min)) && ((x) <= (max))
#define OUT_RANGE(x, min, max) ((x) < (min)) || ((x) > (max))

// timeout for input in ncurses (in milliseconds)
#define INPUT_TIMEOUT 5

#define USE(x) (void)(x)

#define MSG_SZ 512

/*--*--TYPES--*--*/

typedef struct {
    size_t sz;
    char *dt;
} Grapheme;

typedef struct {
    size_t cap;
    char *data;
    size_t length;
    size_t ident;
} Line;

// `x` points to the byte, `x_grapheme` indexes the grapheme, the existense of
// `x` makes it faster to horizontally move the cursor, and `x_grapheme` is
// used to recalculate `x` when the cursor is moved vertically
// 
// `last_x_grapheme` keeps track of the value `x_grapheme` had before it was
// truncated for a smaller line, so that the cursor doesn't changes position
// horizontally when you move through smaller lines
typedef struct {
    size_t x;
    size_t x_grapheme;
    size_t last_x_grapheme;
    size_t y;
} Cursor;

typedef struct {
    size_t x;
    size_t y;
} TextScroll;

typedef struct {
    bool modified;
    bool read_only;
    bool can_write;
    bool crlf; // 0: LF  1: CRLF
    Line *lines;
    size_t num_lines;
    Cursor cursor;
    TextScroll scroll;
    char *name;
    char *filename;
} Buffer;

typedef struct {
    bool strict_utf8; // high/low surrogates will be replaced (for now leave it always set)
    unsigned int tablen;
    int lines;
    bool use_spaces;
    bool autotab;
    bool automatch;
    bool insert_newline;
    char *word_separators;
} GlobalCfg;

typedef struct {
    const char *command;
    const char *hint;
} Hints;

typedef struct Node {
    Buffer data;
    struct Node *next;
    struct Node *prev;
} Node;


// message_and_prompt.c
char *prompt(const char *msgtmp, char *def);
char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints);
void message(char *msg);

// ted.c
void setcolor(int c);

// config_dialog.c
bool config_dialog(Node **n);
int run_command(char **words, int words_len, Node **n);
bool parse_command(char *command, Node **n);

// open_and_save.c
void savefile(Buffer *buf);
Buffer read_lines(FILE *fp, char *filename, bool read_only);
unsigned char detect_linebreak(FILE *fp);
void open_file(char *fname, Node **n);
bool can_write(char *fname);

// display.c
void display_menu(const char *message, const char *shadow, const Node *n);
void display_buffer(Buffer buf, int len_line_number);

// free.c
void free_buffer(Buffer *buf);

// keypress.c
void expand_line(size_t at, int x, Buffer *buf);
void new_line(size_t at, int x, Buffer *buf);
bool process_keypress(int c, Node **n);

// cursor.c

// mouse.c
bool process_mouse_event(MEVENT ev, Node **n);

// utils.c
void die(const char *s);
char *home_path(const char *path);
char *split_spaces(char *str, char **save);
char **split_str(const char *str, int *num_str);
int calculate_len_line_number(Buffer buf);
Line blank_line(void);
char *bufn(int a);

// modify.c
bool modify(Buffer *buf);
bool add_char(size_t x, size_t y, const Grapheme c, Buffer *buf);
bool remove_char(size_t x, size_t y, Buffer *buf);

// scroll.c
void calculate_scroll(Buffer *buf, int len_line_number);

// buffer_list.c
Node *allocate_node(Node n);
void deallocate_node(Node *n);
Node *single_buffer(Buffer b);
void buffer_add_next(Node *n, Buffer b);
void buffer_add_prev(Node *n, Buffer b);
void buffer_close(Node *n);
void free_buffer_list(Node *n);

// grapheme.c
Grapheme get_next_grapheme(char *str, size_t len);

// cursor.c
void calculate_cursor_x(Buffer *buf);


extern GlobalCfg config;
extern char *menu_message;
extern jmp_buf end;

#endif

