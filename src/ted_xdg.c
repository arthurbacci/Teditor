#include <stdlib.h>
#include <sys/stat.h>
#include <ted_string_utils.h>
#include <ted_die.h>
#include <ted_config.h>

// Defaults in case there's no information about them in the path
#define XDG_DATA_HOME ".local/share"
#define XDG_CONFIG_HOME ".config"
#define XDG_STATE_HOME ".local/state"
#define XDG_CACHE_HOME ".cache"

#define HOME_ENV "HOME"

char *ted_data_home = NULL;
char *ted_config_home = NULL;
char *ted_state_home = NULL;
char *ted_cache_home = NULL;

char *get_ted_name_or(const char *envname, const char *default_rel2home) {
    const char *env = getenv(envname);
    if (env && *env) return printdup("%s/%s", env, TED_DIRECTORY_NAME);
    
    const char *home = getenv(HOME_ENV);
    if (!home || !(*home)) die("Couldn't find HOME");
    
    return printdup("%s/%s/%s", home, default_rel2home, TED_DIRECTORY_NAME);
}

void ensure_ted_dirs(void) {
    #define TED_XDG_SET_VAR(vn, xdgn) ted_##vn##_home = get_ted_name_or(#xdgn, xdgn)
    TED_XDG_SET_VAR(data, XDG_DATA_HOME); TED_XDG_SET_VAR(config, XDG_CONFIG_HOME);
    TED_XDG_SET_VAR(state, XDG_STATE_HOME); TED_XDG_SET_VAR(cache, XDG_CACHE_HOME);
    
    char *ted_dirs[] = {ted_data_home, ted_config_home, ted_state_home, ted_cache_home};
    for (int i = 0; i < (sizeof(ted_dirs) / sizeof(char *)); i++) {
        struct stat st = {0};
        if (-1 == stat(ted_dirs[i], &st))
            if (-1 == mkdir(ted_dirs[i], 0770))
                die("Couldn't create ted directories");
    }
}

