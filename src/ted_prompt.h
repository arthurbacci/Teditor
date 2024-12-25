#ifndef TED_PROMPT_H
#define TED_PROMPT_H

#include <ted_config.h>
#include <stddef.h>

typedef struct {
    const char *command;
    const char *hint;
} Hints;

size_t prompt_hints(char message[MSG_SZ], char *base_hint, Hints *hints);
void message(const char *msg);

extern char menu_message[MSG_SZ];

#endif
