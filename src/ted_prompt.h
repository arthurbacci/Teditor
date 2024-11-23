#ifndef TED_PROMPT_H
#define TED_PROMPT_H

typedef struct {
    const char *command;
    const char *hint;
} Hints;

char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints);
void message(char *msg);

extern const char *menu_message;

#endif
