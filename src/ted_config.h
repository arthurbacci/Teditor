#ifndef TED_CONFIG_H
#define TED_CONFIG_H

#include <stdbool.h>

// SET TO ZERO FOR INDENTING WITH TABS
#define DEFAULT_INDENT_SIZE 0
// THE WIDTH OF THE TAB CHARACTER
#define DEFAULT_TAB_WIDTH 8
// WHETHER TO AUTOMATICALLY MAINTAIN THE SAME INDENTATION LEVEL OR NOT
#define DEFAULT_AUTOTAB true

// Name for the subdirectories of the XDG config, data, state and cache dirs
#define TED_DIRECTORY_NAME "ted"

#define TED_EDITORCONFIG_PLUGIN_NAME "ted_editorconfig"

#define READ_BLOCKSIZE 256

#define MSG_SZ 512

#define INPUT_TIMEOUT 5

#endif
