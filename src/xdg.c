#include "ted.h"

// Defaults in case there's no information about them in the path
#define XDG_DATA_HOME ".local/share"
#define XDG_CONFIG_HOME ".config"
#define XDG_STATE_HOME ".local/state"
#define XDG_CACHE_HOME ".cache"

#define HOME_ENV "HOME"

char *get_ted_name_or(const char *envname, const char *default_rel2home) {
    const char *env = getenv(envname);
    if (env && *env) return printdup("%s/%s", env, TED_DIRECTORY_NAME);
    
    const char *home = getenv(HOME_ENV);
    if (!home || !(*home)) die("Couldn't find HOME");
    
    return printdup("%s/%s/%s", home, default_rel2home, TED_DIRECTORY_NAME);
}

char *get_ted_data_home() {return get_ted_name_or("XDG_DATA_HOME", XDG_DATA_HOME);}
char *get_ted_config_home() {return get_ted_name_or("XDG_CONFIG_HOME", XDG_CONFIG_HOME);}
char *get_ted_state_home() {return get_ted_name_or("XDG_STATE_HOME", XDG_STATE_HOME);}
char *get_ted_cache_home() {return get_ted_name_or("XDG_CACHE_HOME", XDG_CACHE_HOME);}


