#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#define TED_LONGJMP_DIE 2
#define TED_LONGJMP_USER_EXIT 1
#define TED_CALL_LONGJMP(x) { \
    if (is_ted_longjmp_set) longjmp(ted_longjmp_end, (x)); \
    else exit((x) - 1); }

extern bool is_ted_longjmp_set;
extern jmp_buf ted_longjmp_end;

