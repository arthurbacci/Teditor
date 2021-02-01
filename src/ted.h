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

struct CFG
{
    unsigned int tablen;
    unsigned int LINES;
    unsigned char line_break_type : 2; // 0: LF  1: CRLF  2: CR
};

struct line
{
    unsigned int len;
    unsigned char *data;
    unsigned int length;
    unsigned int real_length;
    unsigned int ident;
};

extern struct CFG config;
extern char *filename;
extern unsigned int num_lines;
extern struct line *lines;
extern FILE *fp;
