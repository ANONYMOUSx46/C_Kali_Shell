// src/builtins.h
#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

#define SHELL_OK       0
#define SHELL_EXIT    -1

// Is command a builtin (checks first argv token)
int is_builtin(const char *cmd);

// Execute builtin command, return SHELL_OK or SHELL_EXIT
int builtin_execute(command_t *cmd);

#endif
