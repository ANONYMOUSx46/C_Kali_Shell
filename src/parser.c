#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "utils.h"

#define MAX_ARGS 64
#define MAX_COMMANDS 64



// Parse a simple command (no pipes)
static command_t *parse_simple_command(const char *cmdstr) {
    if (!cmdstr) return NULL;

    command_t *cmd = calloc(1, sizeof(command_t));
    if (!cmd) return NULL;

    cmd->raw = strdup(cmdstr);
    if (!cmd->raw) {
        free(cmd);
        return NULL;
    }

    char *copy = strdup(cmdstr);
    if (!copy) {
        free(cmd->raw);
        free(cmd);
        return NULL;
    }

    cmd->argv = calloc(MAX_ARGS, sizeof(char *));
    if (!cmd->argv) {
        free(copy);
        free(cmd->raw);
        free(cmd);
        return NULL;
    }

    char *token;
    char *saveptr;
    int argc = 0;

    token = strtok_r(copy, " \t", &saveptr);
    while (token != NULL && argc < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " \t", &saveptr);
            if (!token) goto fail;
            cmd->input_file = strdup(token);
            if (!cmd->input_file) goto fail;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok_r(NULL, " \t", &saveptr);
            if (!token) goto fail;
            cmd->output_file = strdup(token);
            if (!cmd->output_file) goto fail;
            cmd->append_output = 1;
        } else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " \t", &saveptr);
            if (!token) goto fail;
            cmd->output_file = strdup(token);
            if (!cmd->output_file) goto fail;
            cmd->append_output = 0;
        } else {
            cmd->argv[argc] = strdup(token);
            if (!cmd->argv[argc]) goto fail;
            argc++;
        }
        token = strtok_r(NULL, " \t", &saveptr);
    }
    cmd->argv[argc] = NULL;
    cmd->argc = argc;

    free(copy);
    return cmd;

fail:
    for (int i = 0; i < argc; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);
    free(cmd->input_file);
    free(cmd->output_file);
    free(cmd->raw);
    free(cmd);
    free(copy);
    return NULL;
}

command_list_t *parse_input(const char *input) {
    if (!input) return NULL;

    command_list_t *cmdlist = calloc(1, sizeof(command_list_t));
    if (!cmdlist) return NULL;

    char *input_copy = strdup(input);
    if (!input_copy) {
        free(cmdlist);
        return NULL;
    }

    char *saveptr = NULL;
    char *token = strtok_r(input_copy, "|", &saveptr);

    command_t **commands = calloc(MAX_COMMANDS, sizeof(command_t *));
    if (!commands) {
        free(input_copy);
        free(cmdlist);
        return NULL;
    }

    size_t count = 0;

    while (token != NULL && count < MAX_COMMANDS) {
        char *trimmed = trim_whitespace(token);
        if (!trimmed) trimmed = token;

        command_t *cmd = parse_simple_command(trimmed);
        if (!cmd) {
            for (size_t i = 0; i < count; i++) {
                command_free(commands[i]);
            }
            free(commands);
            free(input_copy);
            free(cmdlist);
            return NULL;
        }
        commands[count++] = cmd;
        token = strtok_r(NULL, "|", &saveptr);
    }

    for (size_t i = 0; i + 1 < count; i++) {
        commands[i]->pipe_to = commands[i + 1];
        commands[i]->pipe_count = 1;
    }
    if (count > 0) commands[count - 1]->pipe_to = NULL;

    cmdlist->commands = commands;
    cmdlist->count = count;

    free(input_copy);
    return cmdlist;
}

void command_free(command_t *cmd) {
    if (!cmd) return;

    if (cmd->raw) free(cmd->raw);
    if (cmd->argv) {
        for (int i = 0; i < cmd->argc; i++) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    free(cmd);
}

void command_list_free(command_list_t *cmdlist) {
    if (!cmdlist) return;

    for (size_t i = 0; i < cmdlist->count; i++) {
        command_free(cmdlist->commands[i]);
    }
    free(cmdlist->commands);
    free(cmdlist);
}


