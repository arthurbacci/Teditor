#ifndef TED_PLUGINS_H
#define TED_PLUGINS_H

#include <ted_buffer.h>

int invoke_editorconfig(const char *prop, const char *filename);
void configure_editorconfig(Buffer *b);

#endif
