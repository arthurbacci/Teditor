#ifndef TED_OUTPUT_H
#define TED_OUTPUT_H

#include <ted_buffer.h>

void display_menu(const char *message, const char *shadow);
void display_buffer(Buffer buf, int len_line_number);

#endif
