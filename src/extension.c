#include "ted.h"

void detect_extension(void) {
    char *ext = strrchr(filename, '.');
    if (!ext) {
        config.syntax_on = 0;
        return;
    }
    
    ext++; // Removes the .
    
    const int fextlen = strlen(ext);
    
    for (unsigned int i = 0; i < config.syntax_len; i++) {
        struct SHD *syntax = config.syntaxes[i];
        for (unsigned int j = 0; j < syntax->exts_len; j++) {
            int sextlen = strlen(syntax->extensions[j]);
            
            // fextlen == sextlen can be removed, but it gives more speed
            if (fextlen == sextlen && !strncmp(ext, syntax->extensions[j], fextlen)) {
                config.syntax_on = 1;
                config.current_syntax = syntax;
                return;
            }
        }
    }
    
    config.syntax_on = 0;
}
