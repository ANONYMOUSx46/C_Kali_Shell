#define _GNU_SOURCE
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_builtin(const char *cmd) {
    if (!cmd || *cmd == '\0') return 0;
    static const char *builtins[] = {
        "cd", "exit", "alias", "unalias", "history", "jobs", "fg", "bg", "help", NULL
    };
    for (int i = 0; builtins[i] != NULL; i++) {
        if (strcmp(cmd, builtins[i]) == 0) return 1;
    }
    return 0;
}

static void print_help() {
    puts("kali-shell builtin commands:");
    puts("  cd [dir]       Change current directory");
    puts("  exit           Exit shell");
    puts("  help           Show this help");
}

int builtin_execute(command_t *cmd) {
    if (!cmd || !cmd->argv || !cmd->argv[0]) return SHELL_OK;

    if (strcmp(cmd->argv[0], "exit") == 0) {
        return SHELL_EXIT;
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc < 2) {
            fprintf(stderr, "cd: missing argument\n");
            return SHELL_OK;
        }
        if (chdir(cmd->argv[1]) != 0) {
            perror("cd");
        }
        return SHELL_OK;
    } else if (strcmp(cmd->argv[0], "help") == 0) {
        print_help();
        return SHELL_OK;
    }

    return SHELL_OK;
}
