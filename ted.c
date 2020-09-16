/**************/
/* Appendices */
/**************/


#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>

#define CTRL_KEY(k) ((k)&0x1f)

#define TED_VERSION "0.0.1"

enum KEYS
{
  BACKSPACE = 127,
  ARROW_UP = 1000,
  ARROW_LEFT,
  ARROW_DOWN,
  ARROW_RIGHT,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY,
  CTRL_ARROW_UP,
  CTRL_ARROW_LEFT,
  CTRL_ARROW_DOWN,
  CTRL_ARROW_RIGHT
};

enum highlight
{
  HL_NORMAL = 0,
  HL_NUMBER,
  HL_MATCH,
  HL_STRING,
  HL_COMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_KEYWORD3,
  HL_MLCOMMENT
};

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)
#define HL_SIMPLE_AUTOCOMPLETION (1<<2)
#define HL_AUTOTAB (1<<3)

struct syntaxInfo
{
  char *filetype;
  char **filematch;
  int flags;
  char *singleline_comment_start;
  char *ml_comment_start;
  char *ml_comment_end;
  char **keywords;
};

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };
char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case", "default",
  
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|",
  
  "#include", "#define", "#undef", "#ifdef", "#ifndef", "#if", "#else", "#elif", "#endif", "#pragma", "#error", "#warning", NULL
};

struct syntaxInfo HLDB[] = {
  {
    "C/C++",
    C_HL_extensions,
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS | HL_SIMPLE_AUTOCOMPLETION | HL_AUTOTAB,
    "//", "/*", "*/",
    C_HL_keywords
  },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))


struct row
{
  int size;
  char *c;
  int rsize;
  char *r;
  unsigned char *hl;
  int idx;
  int hl_open_comment;
  int spaces;
};

struct AppendBuffer
{
  char *b;
  int len;
};

struct Teditor
{
  struct termios defaultTerminalConfig;
  int screencols;
  int screenrows;
  struct AppendBuffer buf;
  int cx, cy;
  int rx;
  int numrows;
  struct row *rows;
  int rowoff;
  int coloff;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  int dirty;
  int quittimes;
  struct syntaxInfo *syntax;
};

struct Teditor TED;

struct TeditorConfig
{
  int tablen;
  int statusmsgtime;
  int quittimes;
  int wrapcursor;
  int linejumpsize;
  int usespaces;
};

struct TeditorConfig TED_CFG = {2, 5, 3, 2, 5, 1};

void die(const char *s)
{
  printf("\r\n");
  perror(s);
  exit(1);
}

void appendBuf(struct AppendBuffer *buf, const char *s, int len)
{
  char *n = realloc(buf->b, buf->len + len);
  if (n == NULL)
    return;
  memcpy(&n[buf->len], s, len);
  buf->b = n;
  buf->len += len;
}

void freeBuf(struct AppendBuffer *buf)
{
  free(buf->b);
}

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &TED.defaultTerminalConfig) == -1)
    die("Can not disable raw mode, try typing \"reset\" if you can not see what you are typing\n");
}

void enableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &TED.defaultTerminalConfig) == -1)
    die("Can not get your terminal information, sorry for that\n");
  atexit(disableRawMode);
  struct termios raw = TED.defaultTerminalConfig;

  /*
    ECHO: Show what is being typed
    ICANON: Use canonical mode
    ISIG: Ctrl+C an Ctrl+Z
    IEXTEN: Ctrl+V
    ICRNL: Carriage return new line

    IXON: Ctrl+S and Ctrl+Q

    OPOST: Output processing
  */
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | ICRNL);
  raw.c_iflag &= ~(IXON);
  raw.c_oflag &= ~(OPOST);

  /* Some other flags */
  /* These dont really matters... ...to me... */
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP);
  raw.c_cflag |= CS8;

  /* Timeout for getting input */
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1; // 1 = 1/10s = 100ms

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("Can not turn on raw mode, sorry for that");
}

int cx2rx(struct row *row, int cx)
{
  int rx = 0;
  for (int j = 0; j < cx; j++)
  {
    if (row->c[j] == '\t')
      rx += TED_CFG.tablen - (rx % TED_CFG.tablen);
    else
      rx++;
  }
  return rx;
}

int rx2cx(struct row *row, int rx)
{
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++)
  {
    if (row->c[cx] == '\t')
      cur_rx += (TED_CFG.tablen - 1) - (cur_rx % TED_CFG.tablen);
    cur_rx++;
    if (cur_rx > rx)
      return cx;
  }
  return cx;
}

void updateSyntax(struct row *row);

void updateRow(struct row *row)
{
  int tabs = 0;
  for (int j = 0; j < row->size; j++)
    if (row->c[j] == '\t')
      tabs++;

  free(row->r);
  row->r = (char *)malloc(row->size + (tabs * (TED_CFG.tablen - 1)) + 1);

  row->spaces = 0;
  
  int bool_start = 1;
  int idx = 0;
  for (int j = 0; j < row->size; j++)
  {
    if (row->c[j] == '\t')
    {
      row->r[idx++] = ' ';
      row->spaces++;
      while (idx % TED_CFG.tablen != 0)
      {
        row->r[idx++] = ' ';
        row->spaces++;
      }
    }
    else
    {
      if ((row->r[idx++] = row->c[j]) == ' ' && bool_start)
        row->spaces++;
      else
        bool_start = 0;
    }
  }
  row->r[idx] = '\0';
  row->rsize = idx;

  updateSyntax(row);
}


void rowInsertChar(struct row *row, int at, int c);    

void insertRow(int at, char *line, size_t llen)
{
  if (at < 0 || at > TED.numrows)
    return;

  TED.rows = (struct row *)realloc(TED.rows, sizeof(struct row) * (TED.numrows + 1));
  memmove(&TED.rows[at + 1], &TED.rows[at], sizeof(struct row) * (TED.numrows - at));

  for (int j = at + 1; j <= TED.numrows; j++)
    TED.rows[j].idx++;
  TED.rows[at].idx = at;

  TED.rows[at].size = llen;
  TED.rows[at].c = malloc(llen + 1);
  memcpy(TED.rows[at].c, line, llen);
  TED.rows[at].c[llen] = '\0';

  TED.rows[at].r = NULL;
  TED.rows[at].rsize = 0;
  TED.rows[at].hl = NULL;

  updateRow(&TED.rows[at]);

  TED.numrows++;
  TED.dirty++;
}

void deleteRow(int at)
{
  if (at < 0 || at >= TED.numrows)
    return;

  free(TED.rows[at].r);
  free(TED.rows[at].c);
  free(TED.rows[at].hl);

  memmove(&TED.rows[at], &TED.rows[at + 1], sizeof(struct row) * (TED.numrows - at - 1));

  for (int j = at; j < TED.numrows - 1; j++)
    TED.rows[j].idx--;

  TED.numrows--;
  TED.dirty++;
}

void rowAppendString(struct row *row, char *s, size_t len)
{
  row->c = realloc(row->c, row->size + len + 1);
  memcpy(&row->c[row->size], s, len);
  row->size += len;
  row->c[row->size] = '\0';
  updateRow(row);
  TED.dirty++;
}

void selectSyntaxHighlight()
{
  TED.syntax = NULL;
  if (TED.filename == NULL)
    return;

  char *ext = strrchr(TED.filename, '.');

  for (unsigned int j = 0; j < HLDB_ENTRIES; j++)
  {
    struct syntaxInfo *s = &HLDB[j];
    
    unsigned int i = 0;
    while (s->filematch[i])
    {
      int is_ext = (s->filematch[i][0] == '.');
      if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
          (!ext && strstr(TED.filename, s->filematch[i])))
      {
        TED.syntax = s;

        for (int filerow = 0; filerow < TED.numrows; filerow++)
          updateSyntax(&TED.rows[filerow]);

        return;
      }
      i++;
    }
  }
}


void readFile(char *filename)
{
  free(TED.filename);
  TED.filename = strdup(filename);

  selectSyntaxHighlight();

  FILE *fp;
  fp = fopen(filename, "r");
  if (!fp)
  {
    FILE *tmpfp = fopen(filename, "w");
    fclose(tmpfp);
    fp = fopen(filename, "r");
  }

  char *line = NULL;
  size_t lineCapacity = 0;
  ssize_t lineLength;
  while ((lineLength = getline(&line, &lineCapacity, fp)) != -1)
  {
    while (lineLength > 0 && (line[lineLength - 1] == '\n' || line[lineLength - 1] == '\r'))
      lineLength--;

    int tabs = 0;
    for (int i = 0; i < lineLength; i++)
      tabs += (line[i] == '\t');

    char *nline = malloc(lineLength + tabs * (TED_CFG.tablen - 1));
    
    int idx = 0;
    for (int i = 0; i < lineLength; i++)
    {
      if (line[i] == '\t')
      {
        nline[idx++] = ' ';
        while ((idx % TED_CFG.tablen) != 0)
        {
          nline[idx++] = ' ';
        }
      }
      else
      {
        nline[idx++] = line[i];
      }
    }
   

    insertRow(TED.numrows, nline, idx);
  }
  free(line);
  fclose(fp);
  TED.dirty = 0;
}

void rowInsertChar(struct row *row, int at, int c)
{
  if (at < 0 || at > row->size)
    at = row->size;

  row->c = realloc(row->c, row->size + 2);
  memmove(&row->c[at + 1], &row->c[at], row->size - at + 1);
  row->size++;
  row->c[at] = c;
  updateRow(row);
  TED.dirty++;
}

void insertChar(int c)
{
  while (TED.cy >= TED.numrows)
    insertRow(TED.numrows, "", 0);

  if (c == '\t' && TED_CFG.usespaces)
  {
    if (TED.cx <= TED.rows[TED.cy].spaces)
    {
      rowInsertChar(&TED.rows[TED.cy], TED.cx, ' ');

      int i = TED.rows[TED.cy].spaces;
      while ((i % TED_CFG.tablen) != 0)
      {
        rowInsertChar(&TED.rows[TED.cy], TED.cx, ' ');
        i++;
      }
      TED.rows[TED.cy].spaces = i;
      TED.cx = i;
    }
    else
    {
      for (int i = 0; i < TED_CFG.tablen; i++)
        rowInsertChar(&TED.rows[TED.cy], TED.cx, ' ');
      TED.cx += TED_CFG.tablen;
    }
  }
  else
  {
    rowInsertChar(&TED.rows[TED.cy], TED.cx, c);
    TED.cx++;
  }
}

void insertNewLine()
{
  if (TED.cx == 0)
    insertRow(TED.cy, "", 0);
  else
  {
    struct row *row = &TED.rows[TED.cy];
    insertRow(TED.cy + 1, &row->c[TED.cx], row->size - TED.cx);
    row = &TED.rows[TED.cy];
    row->size = TED.cx;
    row->c[row->size] = '\0';
    updateRow(row);
  }
  TED.cy++;
  TED.cx = 0;

  if (TED.cy > 0)
  {
    for (int i = 0; i < TED.rows[TED.cy - 1].spaces; i++)
    {
      rowInsertChar(&TED.rows[TED.cy], 0, ' ');
      TED.cx++;
    }
  }
}

void rowDeleteChar(struct row *row, int at)
{
  if (at < 0 || at >= row->size)
    return;
  memmove(&row->c[at], &row->c[at + 1], row->size - at);
  row->size--;
  updateRow(row);
  TED.dirty++;
}

void deleteChar()
{
  if (TED.cy == TED.numrows)
    return;
  if (TED.cx == 0 && TED.cy == 0)
    return;
  struct row *row = &TED.rows[TED.cy];
  if (TED.cx > 0)
  {
    if (TED.cx <= TED.rows[TED.cy].spaces)
    {
      rowDeleteChar(row, TED.cx - 1);
      TED.cx--;

      int i = TED.rows[TED.cy].spaces;
      while ((i % TED_CFG.tablen) != 0)
      {
        rowDeleteChar(row, TED.cx - 1);
        i--;
        TED.cx--;
      }
    }
    else
    {
      rowDeleteChar(row, TED.cx - 1);
      TED.cx--;
    }
  }
  else
  {
    TED.cx = TED.rows[TED.cy - 1].size;
    rowAppendString(&TED.rows[TED.cy - 1], row->c, row->size);
    deleteRow(TED.cy);
    TED.cy--;
  }
}

char *rowsToString(int *buflen)
{
  int totlen = 0;
  for (int j = 0; j < TED.numrows; j++)
    totlen += TED.rows[j].size + 1;
  *buflen = totlen;

  char *buf = malloc(totlen);
  char *p = buf;
  for (int j = 0; j < TED.numrows; j++)
  {
    memcpy(p, TED.rows[j].c, TED.rows[j].size);
    p += TED.rows[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

void setStatusMessage(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(TED.statusmsg, sizeof(TED.statusmsg), fmt, ap);
  va_end(ap);
  TED.statusmsg_time = time(NULL);
}

char *prompt(char *s, void (*callback)(char *, int));

void saveFile()
{
  if (TED.filename == NULL)
  {
    TED.filename = prompt("Save as: %s", NULL);
    if (TED.filename == NULL)
    {
      setStatusMessage("Save aborted", NULL);
      return;
    }
    selectSyntaxHighlight();
  }

  int len;
  char *buf = rowsToString(&len);

  int fd = open(TED.filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1)
  {
    if (ftruncate(fd, len) != -1 && write(fd, buf, len) == len)
    {
      close(fd);
      free(buf);
      TED.dirty = 0;
      setStatusMessage("%d bytes writen to %.20s", len, TED.filename);
      return;
    }
    close(fd);
  }

  free(buf);
  setStatusMessage("Can not save: '%s', sorry for that", strerror(errno));
}

void searchCallback(char *buf, int c)
{
  static int last_match = -1;
  static int direction = 1;

  static int saved_hl_line;
  static char *saved_hl = NULL;

  if (saved_hl)
  {
    memcpy(TED.rows[saved_hl_line].hl, saved_hl, TED.rows[saved_hl_line].rsize);
    free(saved_hl);
    saved_hl = NULL;
  }

  if (c == '\r' || c == '\n' || c == '\x1b')
  {
    last_match = -1;
    direction = 1;
    return;
  }
  else if (c == ARROW_RIGHT || c == ARROW_DOWN)
    direction = 1;
  else if (c == ARROW_LEFT || c == ARROW_UP)
    direction = -1;
  else
  {
    last_match = -1;
    direction = 1;
  }


  if (last_match == -1)
    direction = 1;
  int current = last_match;

  for (int i = 0; i < TED.numrows; i++)
  {
    current += direction;

    if (current <= -1)
      current = TED.numrows - 1;
    else if (current >= TED.numrows)
      current = 0;

    struct row *row = &TED.rows[current];
    char *match = strstr(row->r, buf);
    if (match)
    {
      last_match = current;
      TED.cy = current;
      TED.cx = rx2cx(row, match - row->r);
      TED.rowoff = TED.numrows;

      saved_hl_line = current;
      saved_hl = malloc(row->rsize);
      memcpy(saved_hl, row->hl, row->rsize);

      memset(&row->hl[match - row->r], HL_MATCH, strlen(buf));
      break;
    }
  }
}


void search()
{

  int cx = TED.cx;
  int cy = TED.cy;
  int coloff = TED.coloff;
  int rowoff = TED.rowoff;

  char *query = prompt("Search: %s", searchCallback);
  
  if (query)
    free(query);
  else
  {
    TED.cx = cx;
    TED.cy = cy;
    TED.coloff = coloff;
    TED.rowoff = rowoff;
  }
}


void centerCursor()
{
  TED.coloff = TED.cx - (TED.screencols / 2);
  TED.rowoff = TED.cy - (TED.screenrows / 2);
  if (TED.coloff < 0)
    TED.coloff = 0;
  if (TED.rowoff < 0)
    TED.rowoff = 0;
}




int readKey()
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
  {
    if (nread == -1 && errno != EAGAIN)
      die("Can not read input data, sorry for that");
  }

  if (c == '\x1b')
  {
    char seq[5];

    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';

    if (seq[0] == '[')
    {
      if (seq[1] >= '0' && seq[1] <= '9')
      {
        if (read(STDIN_FILENO, &seq[2], 1) != 1)
          return '\x1b';
        if (seq[2] == '~')
        {
          switch (seq[1])
          {
          case '5':
            return PAGE_UP;
          case '6':
            return PAGE_DOWN;
          case '1':
            return HOME_KEY;
          case '4':
            return END_KEY;
          case '7':
            return HOME_KEY;
          case '8':
            return END_KEY;
          case '3':
            return DEL_KEY;
          }
        } else if (seq[2] == ';')
        {
          if (read(STDIN_FILENO, &seq[3], 1) != 1)
            return '\x1b';
          if (seq[3] == '5')
          {
            if (read(STDIN_FILENO, &seq[4], 1) != 1)
              return '\x1b';
            switch(seq[4])
            {
              case 'A':
                return CTRL_ARROW_UP;
              case 'B':
                return CTRL_ARROW_DOWN;
              case 'C':
                return CTRL_ARROW_RIGHT;
              case 'D':
                return CTRL_ARROW_LEFT;
            }
          }
        }
      }
      else
      {
        switch (seq[1])
        {
        case 'A':
          return ARROW_UP;
        case 'B':
          return ARROW_DOWN;
        case 'C':
          return ARROW_RIGHT;
        case 'D':
          return ARROW_LEFT;
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
        }
      }
    }
    else if (seq[0] == 'O')
    {
      switch (seq[1])
      {
      case 'H':
        return HOME_KEY;
      case 'F':
        return END_KEY;
      }
    }

    return '\x1b';
  }
  return c;
}

void moveCursor(int key, int p)
{
  struct row *row = (TED.cy >= TED.numrows) ? NULL : &TED.rows[TED.cy];
  switch (key)
  {
  case ARROW_LEFT:
  case CTRL_ARROW_LEFT:
    if (TED.cx > 0)
      TED.cx--;
    else if (TED.cy > 0)
    {
      if (p == 1 || (p == 2 && key == CTRL_ARROW_LEFT))
      {
        TED.cy--;
        TED.cx = TED.rows[TED.cy].size;
      }
    }
    break;
  case ARROW_RIGHT:
  case CTRL_ARROW_RIGHT:
    if (row && TED.cx < row->size)
      TED.cx++;
    else if (TED.cx == row->size && TED.cy < TED.numrows - 1)
    {
      if (p == 1 || (p == 2 && key == CTRL_ARROW_RIGHT))
      {
        TED.cy++;
        TED.cx = 0;
      }
    }
    break;
  case ARROW_UP:
  case CTRL_ARROW_UP:
    for (int i = 0; i < ((key == CTRL_ARROW_UP) ? TED_CFG.linejumpsize : 1); i++)
    {
      if (TED.cy > 0)
        TED.cy--;
    }
    break;
  case ARROW_DOWN:
  case CTRL_ARROW_DOWN:
    for (int i = 0; i < ((key == CTRL_ARROW_DOWN) ? TED_CFG.linejumpsize : 1); i++)
    {
      if (TED.cy < TED.numrows - 1)
        TED.cy++;
    }
    break;
  }

  row = (TED.cy >= TED.numrows) ? NULL : &TED.rows[TED.cy];
  int rowlen = row ? row->size : 0;
  if (TED.cx > rowlen)
  {
    TED.cx = rowlen;
  }
}


void gotoLine()
{
  char *_line = prompt("Go to: %s", NULL);
  if (_line == NULL)
    return;
  
  int line;
  sscanf(_line, "%d", &line);
  line--;  

  if (line >= TED.numrows || line < 0)
  {
    setStatusMessage("The line does not exists");
    return;
  }
  
  TED.cy = line;
  struct row row = TED.rows[TED.cy];
  if (TED.cx > row.size)
    TED.cx = row.size;
}

void autoComplete(int c)
{
  if (TED.syntax != NULL && TED.syntax->flags & HL_SIMPLE_AUTOCOMPLETION)
  {
    switch (c)
    {
    case '{':
      insertChar(c);
      insertChar('}');
      moveCursor(ARROW_LEFT, 0);
      break;
    case '(':
      insertChar(c);
      insertChar(')');
      moveCursor(ARROW_LEFT, 0);
      break;
    case '[':
      insertChar(c);
      insertChar(']');
      moveCursor(ARROW_LEFT, 0);
      break;

    default:
      insertChar(c);
      break;
    }
  }
  else
  {
    insertChar(c);
  }
}

void processCommand()
{
  char *command = prompt(":%s", NULL);
  size_t commandlen = strlen(command);
  
  if (commandlen == 0)
  {
    setStatusMessage("No commands");
    return;
  }
  
  for (unsigned int i = 0; i < commandlen; i++)
  {
    char c = command[i];
    switch (c)
    {
      case 'w':
        saveFile();
        break;
      case 'q':
        if (TED.quittimes > 0 && TED.dirty > 0)
        {
          setStatusMessage("WARNING!!! File has unsaved changes. Press Ctrl-C %d more times to quit.", TED.quittimes);
          TED.quittimes--;
          return;
        }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
      case 'h':
        // TODO: help command
        break;
      case 'c':
        centerCursor();
        break;
      case 'f':
        search();
        break;
      case 'g':
        gotoLine();
        break;
      default:
        setStatusMessage("Command not exists: ", c);
        commandlen = 0; // To break the loop
        
    } // switch
  } // for
} //  process command

void processKeypress()
{

  int c = readKey();
  switch (c)
  {
  case '\r':
  case '\n':
    insertNewLine();
    break;

  case CTRL_KEY('c'):
    if (TED.quittimes > 0 && TED.dirty > 0)
    {
      setStatusMessage("WARNING!!! File has unsaved changes. Press Ctrl-C %d more times to quit.", TED.quittimes);
      TED.quittimes--;
      return;
    }
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;

  case PAGE_UP:
  case PAGE_DOWN:
  {
    if (c == PAGE_UP)
      TED.cy = TED.rowoff;
    else if (c == PAGE_DOWN)
    {
      TED.cy = TED.rowoff + TED.screenrows - 1;
      if (TED.cy > TED.numrows)
        TED.cy = TED.numrows;
    }

    int times = TED.screenrows;
    while (times--)
      moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN, 0);
  }
  break;

  case HOME_KEY:
    TED.cx = 0;
    break;
  case END_KEY:
    if (TED.cy < TED.numrows)
      TED.cx = TED.rows[TED.cy].size;
    break;

  case ARROW_UP:
  case ARROW_DOWN:
  case ARROW_LEFT:
  case ARROW_RIGHT:
  case CTRL_ARROW_UP:
  case CTRL_ARROW_DOWN:
  case CTRL_ARROW_LEFT:
  case CTRL_ARROW_RIGHT:
    moveCursor(c, TED_CFG.wrapcursor);
    break;

  case BACKSPACE:
  case CTRL_KEY('h'):
  case DEL_KEY:
    if (c == DEL_KEY)
      moveCursor(ARROW_RIGHT, 1);
    deleteChar();
    break;

  case '\x1b':
  case CTRL_KEY('l'):
    break;

  case CTRL_KEY('s'):
    saveFile();
    break;

  case CTRL_KEY('f'):
    search();
    break;

  case CTRL_KEY('g'):
    gotoLine();
    break;

  case CTRL_KEY('e'):
    centerCursor();
    break;

  case CTRL_KEY('t'):
    processCommand();
    break;

  default:
    // autoComplete(c);
    // autoComplete will come back soon
    insertChar(c); // for now
    break;
  }
  TED.quittimes = TED_CFG.quittimes;
}

int welcomeMessage(struct AppendBuffer *buf, int y)
{
  if (y == TED.screenrows / 3)
  {
    char welcome[80];
    int wlen = snprintf(welcome, 80, "TED Text Editor");
    if (wlen > TED.screencols)
      wlen = TED.screencols;
    int padding = (TED.screencols - wlen) / 2;
    if (padding--)
      appendBuf(buf, "~", 1);
    while (padding--)
      appendBuf(buf, " ", 1);
    appendBuf(buf, welcome, wlen);
  }
  else if (y == TED.screenrows / 3 + 1)
  {
    char welcome[80];
    int wlen = snprintf(welcome, 80, "Teditor v%s - by Arthur Bacci", TED_VERSION);
    if (wlen > TED.screencols)
      wlen = TED.screencols;
    int padding = (TED.screencols - wlen) / 2;
    if (padding--)
      appendBuf(buf, "~", 1);
    while (padding--)
      appendBuf(buf, " ", 1);
    appendBuf(buf, welcome, wlen);
  }
  else
  {
    return 0;
  }
  return 1;
}

int syntax2color(int hl, char **c);

void drawRows(struct AppendBuffer *buf)
{
  for (int y = 0; y < TED.screenrows; y++)
  {
    int filerow = TED.rowoff + y;
    if (filerow >= TED.numrows)
    {
      if (TED.numrows > 0 || !welcomeMessage(buf, y))
        appendBuf(buf, "~", 1);
    }
    else
    {
      int llen = TED.rows[filerow].rsize - TED.coloff;
      if (llen < 0)
        llen = 0;
      if (llen > TED.screencols)
        llen = TED.screencols;

      char *c = &TED.rows[filerow].r[TED.coloff];
      unsigned char *hl = &TED.rows[filerow].hl[TED.coloff];

      char *current_color = NULL;
      int current_clen = 0;

      for (int j = 0; j < llen; j++)
      {
        if (iscntrl(c[j]))
        {
          appendBuf(buf, "\x1b[7m?\x1b[m", 8);
        }
        else if (hl[j] == HL_NORMAL)
        {
          if (current_color != NULL)
          {
            appendBuf(buf, "\x1b[m", 3);
            current_color = NULL;
          }
          appendBuf(buf, &c[j], 1);
        }
        else
        {
          char *color = NULL;
          int clen = syntax2color(hl[j], &color);
          if (clen != current_clen || memcmp(current_color, color, clen) != 0)
          {          
            appendBuf(buf, color, strlen(color));
            current_color = color;
          }
          appendBuf(buf, &c[j], 1);
        }
      }
      appendBuf(buf, "\x1b[m", 3);
    }

    appendBuf(buf, "\x1b[K", 3); // Clear the rest of the line
    appendBuf(buf, "\r\n", 2);
  }
}

void drawStatusBar(struct AppendBuffer *buf)
{
  appendBuf(buf, "\x1b[7m", 4);

  char status[80];
  char rstatus[80];
  int len = snprintf(
      status,
      sizeof(status),
      "%.20s %s M:%d S:%d",
      TED.filename ? TED.filename : "[No Name]",
      TED.syntax ? TED.syntax->filetype : "Plain text",
      TED.dirty,
      (TED.cy < TED.numrows) ? TED.rows[TED.cy].spaces : 0);
  int rlen = snprintf(
      rstatus,
      sizeof(rstatus),
      "%d/%d : %d/%d",
      TED.cy + 1,
      TED.numrows,
      TED.cx + 1,
      (TED.cy < TED.numrows) ? TED.rows[TED.cy].size + 1 : 1);

  if (len > TED.screencols)
    len = TED.screencols;
  appendBuf(buf, status, len);

  while (len < TED.screencols)
  {
    if (TED.screencols - len == rlen)
    {
      appendBuf(buf, rstatus, rlen);
      break;
    }
    appendBuf(buf, " ", 1);
    len++;
  }
  appendBuf(buf, "\x1b[m", 3);
  appendBuf(buf, "\r\n", 2);
}

void drawMessageBar(struct AppendBuffer *buf)
{
  appendBuf(buf, "\x1b[K", 3);
  int msglen = strlen(TED.statusmsg);
  if (msglen > TED.screencols)
    msglen = TED.screencols;
  if (msglen && time(NULL) - TED.statusmsg_time < TED_CFG.statusmsgtime)
    appendBuf(buf, TED.statusmsg, msglen);
}

void processScroll()
{
  TED.rx = 0;
  if (TED.cy < TED.numrows)
    TED.rx = cx2rx(&TED.rows[TED.cy], TED.cx);

  if (TED.cy < TED.rowoff)
    TED.rowoff = TED.cy;
  if (TED.cy >= TED.rowoff + TED.screenrows)
    TED.rowoff = TED.cy - TED.screenrows + 1;
  if (TED.rx < TED.coloff)
    TED.coloff = TED.rx;
  if (TED.rx > TED.coloff + TED.screencols)
    TED.coloff = TED.rx - TED.screencols + 1;
}

void reloadScreen()
{
  processScroll();
  
  struct AppendBuffer buf = {NULL, 0};
  
  appendBuf(&buf, "\x1b[?25l", 6);
  appendBuf(&buf, "\x1b[H", 3); // Move cursor to top left
  
  drawRows(&buf);
  drawStatusBar(&buf);
  drawMessageBar(&buf);
  
  char bf[32];
  snprintf(bf, sizeof(bf), "\x1b[%d;%dH", (TED.cy - TED.rowoff) + 1, (TED.rx - TED.coloff) + 1);
  appendBuf(&buf, bf, strlen(bf));

  appendBuf(&buf, "\x1b[?25h", 6);

  write(STDOUT_FILENO, buf.b, buf.len);
  freeBuf(&buf);
}

char *prompt(char *s, void (*callback)(char *, int))
{
  size_t bufsize = 128;
  char *buf = malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1)
  {
    setStatusMessage(s, buf);
    reloadScreen();

    int c = readKey();

    if ((c == '\r' || c == '\n') && buflen != 0)
    {
      setStatusMessage("");
      if (callback)
        callback(buf, c);
      return buf;
    }
    else if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE)
    {
      if (buflen != 0)
        buf[--buflen] = '\0';
    }
    else if (c == '\x1b')
    {
      setStatusMessage("");
      if (callback)
        callback(buf, c);
      free(buf);
      return NULL;
    }
    else if (!iscntrl(c) && c < 128)
    {
      if (buflen == bufsize - 1)
      {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
    if (callback)
      callback(buf, c);
  }
}


int getCursorPosition(int *y, int *x)
{
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1)
  {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", y, x) != 2)
    return -1;

  return 0;
}


int getWindowSize(int *rows, int *cols)
{
  struct winsize ws;
  if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  }
  else
  {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}


int is_separator(int c)
{
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];{}:", c) != NULL;
}

void updateSyntax(struct row *row)
{
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);

  if (TED.syntax == NULL)
    return;

  char **keywords = TED.syntax->keywords;

  char *scs = TED.syntax->singleline_comment_start;
  char *mcs = TED.syntax->ml_comment_start;
  char *mce = TED.syntax->ml_comment_end;
  int scs_len = scs ? strlen(scs) : 0;
  int mcs_len = mcs ? strlen(mcs) : 0;
  int mce_len = mce ? strlen(mce) : 0;

  int prev_sep = 1;
  int in_string = 0;  
  int in_comment = (row->idx > 0 && TED.rows[row->idx - 1].hl_open_comment);

  int i = 0; 
  while (i < row->rsize)
  {
    char c = row->r[i];
    unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

    if (scs_len && !in_string && !in_comment)
    {
      if (!strncmp(&row->r[i], scs, scs_len))
      {
        memset(&row->hl[i], HL_COMMENT, row->rsize - i);
        break;
      }
    }

    if (mcs_len && mce_len && !in_string)
    {
      if (in_comment)
      {
        row->hl[i] = HL_MLCOMMENT;
        if (!strncmp(&row->r[i], mce, mce_len))
        {
          memset(&row->hl[i], HL_MLCOMMENT, mce_len);
          i += mce_len;
          in_comment = 0;
          prev_sep = 1;
        }
        else
          i++;
        continue;
      }
      else if (!strncmp(&row->r[i], mcs, mcs_len))
      {
        memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
        i += mcs_len;
        in_comment = 1;
        continue;
      }
    }

    if (TED.syntax->flags & HL_HIGHLIGHT_STRINGS)
    {
      if (in_string)
      {
        row->hl[i] = HL_STRING;
        if (c == '\\' && i + 1 < row->rsize)
        {
          row->hl[i + 1] = HL_STRING;
          i += 2;
          continue;
        }

        if (c == in_string)
          in_string = 0;
        i++;
        prev_sep = 1;
        continue;
      }
      else if (c == '"' || c == '\'')
      {
        in_string = c;
        row->hl[i] = HL_STRING;
        i++;
        continue;
      }
    }
    
    if (TED.syntax->flags & HL_HIGHLIGHT_NUMBERS)
    {    
      if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER))
      {
        row->hl[i] = HL_NUMBER;
        i++;
        prev_sep = 0;
        continue;
      }
    }

    if (prev_sep)
    {
      int j;
      for (j = 0; keywords[j]; j++)
      {
        int klen = strlen(keywords[j]);
        int kw = 1;
        if (keywords[j][klen-1] == '|')
          kw = 2;
        else if (keywords[j][0] == '#')
          kw = 3;
        
        if (kw == 2)
          klen--;
        
        if (!strncmp(&row->r[i], keywords[j], klen) &&
            is_separator(row->r[i + klen]))
        {
          switch (kw)
          {
          case 1:
            memset(&row->hl[i], HL_KEYWORD1, klen);
            break;
          case 2:
            memset(&row->hl[i], HL_KEYWORD2, klen);
            break;
          case 3:
            memset(&row->hl[i], HL_KEYWORD3, klen);
            break;
          }
          i += klen;
          break;
        }
      }
      if (keywords[j] != NULL)
      {
        prev_sep = 0;
        continue;
      }
    }

    prev_sep = is_separator(c);  
    i++;
  }

  int changed = (row->hl_open_comment != in_comment);
  row->hl_open_comment = in_comment;
  if (changed && row->idx + 1 < TED.numrows)
    updateSyntax(&TED.rows[row->idx + 1]);
}

int syntax2color(int hl, char **c)
{
  switch (hl)
  {
  case HL_NUMBER:
    *c = "\x1b[92m";
    return 5;
  case HL_MATCH:
    *c = "\x1b[1;7m";
    return 6;
  case HL_STRING:
    *c = "\x1b[93m";
    return 5;
  case HL_COMMENT:
  case HL_MLCOMMENT:
    *c = "\x1b[36m";
    return 5;
  case HL_KEYWORD1:
    *c = "\x1b[1;95m";
    return 5;
  case HL_KEYWORD2:
    *c = "\x1b[95m";
    return 5;
  case HL_KEYWORD3:
    *c = "\x1b[91m";
    return 5;
  default:
    *c = "\x1b[m";
    return 3;
  }
}


void initEditor()
{
  TED.cx = 0;
  TED.cy = 0;
  TED.rx = 0;
  TED.numrows = 0;
  TED.rows = NULL;
  TED.rowoff = 0;
  TED.coloff = 0;
  TED.filename = NULL;
  TED.statusmsg[0] = '\0';
  TED.statusmsg_time = 0;
  TED.dirty = 0;
  TED.quittimes = TED_CFG.quittimes;
  TED.syntax = NULL;

  if (getWindowSize(&TED.screenrows, &TED.screencols) == -1)
    die("Can not get your terminal size, sorry for that\n");
  TED.screenrows -= 2;
}

int main(int argc, char **argv)
{
  enableRawMode();
  initEditor();
  if (argc >= 2)
    readFile(argv[1]);
  if (TED.numrows == 0)
  {
    reloadScreen();
    insertRow(0, "", 0);
    processKeypress();
  }

  setStatusMessage("Press Ctrl+T and type \"h\" to get help");

  while (1)
  {
    reloadScreen();
    processKeypress();
  }

  return 0; 
}
