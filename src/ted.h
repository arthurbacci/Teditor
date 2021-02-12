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

#define READ_BLOCKSIZE 10
#define ctrl(x) ((x) & 0x1f)
#define cx cursor.x
#define cy cursor.y

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

// message_and_prompt.c
int message(char *msg);
char *prompt(char *msg);

// ted.c
void setcolor(int c);

// config_dialog.c
void config_dialog();

// open_and_save.c
void savefile();
void read_lines();
void detect_linebreak();

// show.c
void show_menu();
void show_lines();

// free.c
void free_lines();

// keypress.c
void process_keypress(int c);

// utils.c
unsigned int calculate_real_cx(unsigned int *last_one_size);

// cursor_in_valid_position.c
void cursor_in_valid_position();

struct CFG
{
    unsigned int tablen;
    unsigned int LINES;
    unsigned char line_break_type : 2; // 0: LF  1: CRLF  2: CR
    unsigned char use_spaces : 1;
    unsigned char autotab : 1;
};

struct LINE
{
    unsigned int len;
    unsigned char *data;
    unsigned int length;
    unsigned int real_length;
    unsigned int ident;
};

struct CURSOR
{
    unsigned int x;
    unsigned int y;
};

struct TEXT_SCROLL
{
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
