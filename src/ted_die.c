// TODO: move to ted_utils.c, alongside moving ted_die.h to ted_utils.h

#include <stdio.h>
#include <stdarg.h>
#include <ted_longjmp.h>

void die(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    
    vfprintf(stderr, fmt, va);
    
    va_end(va);
    TED_CALL_LONGJMP(TED_LONGJMP_DIE);
}
