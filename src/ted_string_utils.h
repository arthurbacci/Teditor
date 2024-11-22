#ifndef TED_STRING_UTILS_H
#define TED_STRING_UTILS_H

// Converts strings of the form "0", "t", "false", "F" or "TrUE" to a boolean, or returns -1 if it
// is not a valid boolean.
int string_to_bool(const char *s);

// Similar to sprintf, but dynamically allocates enough space for what you need to format
char *printdup(const char *fmt, ...);

#endif
