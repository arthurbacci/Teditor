#include <ted_plugins.h>
#include <ted_utils.h>
#include <ted_xdg.h>
#include <ted_config.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int invoke_editorconfig(const char *prop, const char *filename) {
    char *program_path = printdup("%s/" TED_EDITORCONFIG_PLUGIN_NAME, ted_data_home);

    int status;
    pid_t child;

    switch (child = fork()) {
        case -1:
            return 127;
        case 0:
            execl(program_path, TED_EDITORCONFIG_PLUGIN_NAME, prop, filename, (char *) NULL);
            _exit(127);
        default:
            if (waitpid(child, &status, 0) == -1)
                return 127;
    }

    free(program_path);

    return WEXITSTATUS(status);
}

void configure_editorconfig(Buffer *b) {
    bool use_tabs = 1 == invoke_editorconfig("indent_style", b->filename);
    int indent_size = invoke_editorconfig("indent_size", b->filename);
    int tab_width = invoke_editorconfig("tab_width", b->filename);

    if (indent_size < 100)
        b->indent_size = indent_size;
    if (tab_width < 100)
        b->tab_width = tab_width;
    if (use_tabs)
        b->indent_size = 0;
}
