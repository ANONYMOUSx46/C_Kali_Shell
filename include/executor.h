// src/executor.h
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

// Execute an external command or pipeline command_t chain.
// Returns 0 success, -1 failure
int executor_execute(command_t *cmd);

#endif
