#include <ted_grapheme.h>
#include <ted_buffer.h>
#include <ctype.h>

struct CodepointRange {
    uint_least32_t start;
    uint_least32_t end;
};

struct CodepointRange fullwidth_ranges[] = {
    #include <fullwidth.def>
    {0, 0}
};

bool is_codepoint_fullwidth(uint_least32_t cp) {
    // TODO: binary search for speed and maybe adding an if case for all ascii
    // characters considering that they're very common and it's not really
    // necessary to access the list for them, except if the user really wants
    // to do so for some reason (by tweaking the fullwidth.def file. In this
    // case it may be interesting to add a #define ASCII_WIDTH_OPTIMIZE that
    // can be disabled (#undef) in case such a thing is done
    
    for (size_t i = 0; fullwidth_ranges[i].end > 0; i++) {
        if (cp >= fullwidth_ranges[i].start && cp <= fullwidth_ranges[i].end)
            return true;
    }
    return false;
}

Grapheme get_next_grapheme(char **str, size_t len) {
    Grapheme grapheme = {
        grapheme_next_character_break_utf8(*str, len),
        *str,
    };

    *str += grapheme.sz;

    return grapheme;
}


size_t grapheme_width(Grapheme g) {
    // TODO: the current mechanism of getting the width of a grapheme cluster
    // is "if it has any fullwidth, it's fullwidth, if not, it's not", which
    // isn't true for emojis. A better way of handling this is needed later.
    //
    // Why later? Neither vim nor neovim support them correctly anyway, so it's
    // probably not something that will affect many people.
    //
    // Resources that may be useful:
    // - https://www.unicode.org/reports/tr51/

    switch (get_grapheme_type(g)) {
        case TABULATION: return SEL_BUF.tab_width;
        case CONTROL_CHAR: return 2;
        case INVALID_UNICODE: return 1;
        case DISPLAYABLE_CHAR: {
            for (size_t off = 0; off < g.sz; ) {
                uint_least32_t cp;

                off += grapheme_decode_utf8(g.dt + off, g.sz - off, &cp);
                // In case of an error
                if (off > g.sz) break;

                if (is_codepoint_fullwidth(cp)) return 2;
            }
            return 1;
        }
    }
    return 0;
}

size_t wi_to_gi(size_t si, char *s) {
    for (size_t i = 0;; i++) {
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);
    
        if (0 == g.sz)
            return i;

        size_t width = grapheme_width(g);

        if (width > si)
            return i;
        si -= width;
    }
}

size_t gi_to_wi(size_t gi, char *s) {
    size_t r = 0;
    for (size_t i = 0; i < gi; i++) {
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);

        if (g.sz == 0)
            break;

        r += grapheme_width(g);
    }
    return r;
}

size_t word_to_wi(size_t word_n, char *s) {
    size_t ret = 0;
    for (size_t i = 0; i < word_n; i++) {
        size_t wordl = grapheme_next_word_break_utf8(s, SIZE_MAX);
        
        size_t word_width = calc_string_width(s, wordl);
        ret += word_width;
        s += wordl;
    }
    return ret;
}

size_t wi_to_word(size_t wi, char *s) {
    for (size_t i = 0;; i++) {
        size_t wordl = grapheme_next_word_break_utf8(s, SIZE_MAX);
        
        if (wordl == 0)
            return i;
        
        size_t word_width = calc_string_width(s, wordl);
        
        if (word_width > wi)
            return i;
        wi -= word_width;
        s += wordl;
    }
}

size_t calc_string_width(char *s, size_t len) {
    size_t ret = 0;
    const char *const end_of_s = s + len;
    while (s < end_of_s) {
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);
        ret += grapheme_width(g);
    }
    return ret;
}

ssize_t index_by_width_after(size_t _wi, char **s) {
    ssize_t wi = _wi;
    while (1) {
        if (wi <= 0)
            return -wi;

        Grapheme g = get_next_grapheme(s, SIZE_MAX);
        
        if (g.sz <= 0)
            return -wi;

        wi -= grapheme_width(g);
    }
}

size_t index_by_width(size_t wi, char **s) {
    Grapheme g;
    while ((g = get_next_grapheme(s, SIZE_MAX)).sz > 0) {
        size_t gw = grapheme_width(g);
        if (gw > wi) {
            *s -= g.sz;
            return wi;
        }
        wi -= gw;
    }
    return wi;
}

bool is_replacement_character(Grapheme g) {
    return g.sz == 3
        && g.dt[0] == (char)0xef
        && g.dt[1] == (char)0xbf
        && g.dt[2] == (char)0xbd;
}

Grapheme replacement_character(void) {
    static unsigned char dt[3] = {0xef, 0xbf, 0xbd};
    Grapheme g = {3, (char *)dt};

    return g;
}

GraphemeType get_grapheme_type(Grapheme g) {
    if (1 == g.sz && '\t' == *g.dt) return TABULATION;
    if (1 == g.sz && !isprint(*g.dt)) return CONTROL_CHAR;
    if (is_replacement_character(g)) return INVALID_UNICODE;
    return DISPLAYABLE_CHAR;
}

