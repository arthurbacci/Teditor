#include "ted.h"

void detect_extension(void) {
    char *ext = strrchr(filename, '.');
    if (!ext)
        goto END;
    
    ext++; // Removes the .
    
    const int fextlen = strlen(ext);
    
    for (unsigned int i = 0; i < config.syntax_len; i++) {
        struct SHD *syntax = config.syntaxes[i];
        
        for (unsigned int j = 0; j < syntax->exts_len; j++) {
            int sextlen = strlen(syntax->extensions[j]);
            
            // fextlen == sextlen can be removed, but it gives more speed
            if (fextlen == sextlen && !strcmp(ext, syntax->extensions[j])) {
                config.syntax_on = 1;
                config.current_syntax = syntax;
                return;
            }
        }
    }
    
  END:
    config.syntax_on = 1;
    config.current_syntax = &default_syntax;
}


