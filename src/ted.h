#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#define READ_BLOCKSIZE 10
#define ctrl(x) ((x) & 0x1f)
#define cx cursor.x
#define cy cursor.y

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

typedef uint32_t uchar32_t;

// message_and_prompt.c
void message(char *msg);
char *prompt(const char *msgtmp, char *def);

// ted.c
void setcolor(int c);

// config_dialog.c
void config_dialog();

// open_and_save.c
void savefile();
void read_lines();
void detect_linebreak();
void openFile(char *fname, bool needs_to_free);

// show.c
void show_menu(char *message);
void show_lines();

// free.c
void free_lines();

// keypress.c
void expandLine(unsigned int at, int x);
void process_keypress(int c);

// cursor_in_valid_position.c
void cursor_in_valid_position();

// mouse.c
void processMouseEvent(MEVENT ev);

// utf8.c
void utf8ReadFile(unsigned char uc, unsigned int lc, unsigned int i, FILE *fp);
uint16_t utf8ToMultibyte(uchar32_t c, unsigned char *out);

// color.c
void syntaxHighlight();
void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg);

// utils.c
char *home_path(const char *path);

// buffers.c
void appendBuffer(char *bufname);
void nextBuffer();
void prevBuffer();
void freeBuffers();

struct KWD {
    const char *string;
    uint8_t color;
};

struct CFG {
    unsigned int tablen;
    unsigned int LINES;
    unsigned char line_break_type : 2; // 0: LF  1: CRLF  2: CR
    bool use_spaces : 1;
    bool autotab : 1;
    const char *word_separators;
    unsigned int kwdlen;
    struct KWD *keywords;
};


/*
ffffbbbb
00000000 == default with default background
00010000 == color1 with default background
00100000 == color2 with default background
00000001 == default with color1 background

There are 16 foreground colors and 16 background colors
This is good for now, in case where more colors be need, color can be changed from `unsigned char *` to `uint16_t *` (256 foregrounds + 256 backgrounds)
*/
struct LINE {
    unsigned int len;
    uchar32_t *data;
    unsigned char *color;
    unsigned int length;
    unsigned int ident;
};

struct CURSOR {
    unsigned int x;
    unsigned int y;
};

struct TEXT_SCROLL {
    unsigned int x;
    unsigned int y;
};

extern struct CFG config;
extern char *filename;
extern unsigned int num_lines;
extern unsigned int len_line_number;
extern struct LINE *lines;
extern FILE *fp;
extern struct CURSOR cursor;
extern struct TEXT_SCROLL text_scroll;
extern unsigned int last_cursor_x;
extern bool colors_on;
extern bool needs_to_free_filename;
extern char *menu_message;
