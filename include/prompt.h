#ifndef PROMPT_H
#define PROMPT_H

#include <stddef.h>
#include "config.h"

void prompt_render(char *buf, size_t bufsize, const shell_config_t *config);

#endif // PROMPT_H
