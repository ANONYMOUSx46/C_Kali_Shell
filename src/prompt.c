#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include "config.h"

static const char *color_reset = "\033[0m";
static const char *color_user_light = "\033[1;32m";    // bright green
static const char *color_host_light = "\033[1;34m";    // bright blue
static const char *color_path_light = "\033[1;35m";    // bright magenta
static const char *color_user_dark = "\033[0;32m";     // green
static const char *color_host_dark = "\033[0;34m";     // blue
static const char *color_path_dark = "\033[0;35m";     // magenta

void prompt_render(char *buf, size_t bufsize, const shell_config_t *config) {
    if (!buf || !config) return;

    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));

    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        strcpy(cwd, "unknown");
    }

    struct passwd *pw = getpwuid(getuid());
    const char *user = pw ? pw->pw_name : "user";
    const int is_root = (geteuid() == 0);

    const char *c_user = (config->theme == THEME_DARK) ? color_user_dark : color_user_light;
    const char *c_host = (config->theme == THEME_DARK) ? color_host_dark : color_host_light;
    const char *c_path = (config->theme == THEME_DARK) ? color_path_dark : color_path_light;
    const char *c_reset = color_reset;

    const char *p = config->prompt_format;
    char *dst = buf;
    size_t remaining = bufsize;

    while (*p && remaining > 1) {
        if (*p == '\\') {
            p++;
            switch (*p) {
                case 'u': {
                    int n = snprintf(dst, remaining, "%s%s%s", c_user, user, c_reset);
                    dst += n;
                    remaining -= (size_t)n;
                    break;
                }
                case 'h': {
                    int n = snprintf(dst, remaining, "%s%s%s", c_host, hostname, c_reset);
                    dst += n;
                    remaining -= (size_t)n;
                    break;
                }
                case 'w': {
                    int n = snprintf(dst, remaining, "%s%s%s", c_path, cwd, c_reset);
                    dst += n;
                    remaining -= (size_t)n;
                    break;
                }
                case '$': {
                    int n = snprintf(dst, remaining, "%c", is_root ? '#' : '$');
                    dst += n;
                    remaining -= (size_t)n;
                    break;
                }
                case '\\':
                    if (remaining > 1) {
                        *dst++ = '\\';
                        remaining--;
                    }
                    break;
                default:
                    if (remaining > 2) {
                        *dst++ = '\\';
                        *dst++ = *p;
                        remaining -= 2;
                    }
                    break;
            }
            p++;
        } else {
            *dst++ = *p++;
            remaining--;
        }
    }
    *dst = '\0';
}
