#include <stddef.h>
// TODO: remove sys/types.h if ssize_t is not used anymore (prob wont)
#include <sys/types.h>
#include <stdbool.h>

#include <grapheme.h>

typedef struct {
    size_t sz;
    char *dt;
} Grapheme;

typedef enum {
    DISPLAYABLE_CHAR,
    TABULATION,
    INVALID_UNICODE,
    CONTROL_CHAR
} GraphemeType;

Grapheme get_next_grapheme(char **str, size_t len);
size_t grapheme_width(Grapheme g);
size_t wi_to_gi(size_t si, char *s);
size_t gi_to_wi(size_t gi, char *s);
ssize_t index_by_width_after(size_t _wi, char **s);
size_t index_by_width(size_t wi, char **s);
bool is_replacement_character(Grapheme g);
Grapheme replacement_character(void);
GraphemeType get_grapheme_type(Grapheme g);

