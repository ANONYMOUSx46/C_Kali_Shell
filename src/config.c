#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "config.h"

#define CONFIG_PATH ".kali_shellrc"
#define LINE_MAX 512

static char *trim_whitespace(char *str) {
    if (!str) return NULL;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

void config_init(shell_config_t *config) {
    if (!config) return;
    strncpy(config->prompt_format, "\\u@\\h:\\w\\$ ", PROMPT_MAX_LEN - 1);
    config->prompt_format[PROMPT_MAX_LEN - 1] = '\0';
    config->theme = THEME_DEFAULT;
}

int config_load(shell_config_t *config) {
    if (!config) return -1;

    char path[512];
    char *home = getenv("HOME");
    if (!home) return -1;

    snprintf(path, sizeof(path), "%s/%s", home, CONFIG_PATH);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[LINE_MAX];
    while (fgets(line, sizeof(line), f)) {
        char *trimline = trim_whitespace(line);

        if (*trimline == '\0' || *trimline == '#') continue;

        if (strncmp(trimline, "prompt=", 7) == 0) {
            char *value = trim_whitespace(trimline + 7);
            if (value && *value) {
                strncpy(config->prompt_format, value, PROMPT_MAX_LEN - 1);
                config->prompt_format[PROMPT_MAX_LEN - 1] = '\0';
            }
        } else if (strncmp(trimline, "theme=", 6) == 0) {
            char *value = trim_whitespace(trimline + 6);
            if (strcmp(value, "dark") == 0) {
                config->theme = THEME_DARK;
            } else if (strcmp(value, "light") == 0) {
                config->theme = THEME_LIGHT;
            }
        }
        // Alias lines handled in main.c
    }

    fclose(f);
    return 0;
}
