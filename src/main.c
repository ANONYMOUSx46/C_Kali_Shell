// src/main.c
//
// Main shell loop - prompt, read input with readline, alias expansion, history, parse, execute,
// with tab completion support for builtin commands and executables.
// Enhanced with configuration system for prompt, theme, aliases and job notifications.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "parser.h"
#include "executor.h"
#include "builtins.h"
#include "history.h"
#include "config.h"
#include "prompt.h"
#include "utils.h" 

static volatile int keep_running = 1;

// Alias structure
typedef struct alias {
    char *name;
    char *command;
} alias_t;

#define MAX_ALIASES 64
static alias_t aliases[MAX_ALIASES];
static size_t alias_count = 0;

// List of builtin commands for completion
static const char *builtin_commands[] = {
    "cd",
    "exit",
    "history",
    "alias",
    "unalias",
    "jobs",
    "fg",
    "bg",
    NULL
};

// Shell configuration variable
static shell_config_t shell_config;

#define PROMPT_BUFFER_SIZE 512



// Signal handler for Ctrl-C to print prompt on new line (to avoid ^C breaking prompt line)
void sigint_handler(int signo) {
    (void)signo;
    const char prompt_str[] = "\n";
    write(STDOUT_FILENO, prompt_str, sizeof(prompt_str) - 1);
    rl_replace_line("", 0);
    rl_on_new_line();
    rl_redisplay();
}

// Signal handler for SIGCHLD: notify when background job finishes
void sigchld_handler(int signo) {
    (void)signo;
    int status;
    pid_t pid;

    // Reap all finished child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("\n[+] Job %d exited with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("\n[+] Job %d terminated by signal %d\n", pid, WTERMSIG(status));
        }
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

// Load aliases from ~/.kali_shellrc config file, lines starting with "alias "
void load_aliases() {
    char *home = getenv("HOME");
    if (!home)
        return;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.kali_shellrc", home);

    FILE *f = fopen(path, "r");
    if (!f)
        return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *trimline = trim_whitespace(line);
        if (strncmp(trimline, "alias ", 6) == 0) {
            char *alias_def = trimline + 6;
            char *eq = strchr(alias_def, '=');
            if (!eq)
                continue;

            *eq = '\0';
            char *name = trim_whitespace(alias_def);
            char *cmd = trim_whitespace(eq + 1);

            size_t len = strlen(cmd);
            if ((cmd[0] == '\'' && cmd[len - 1] == '\'') || (cmd[0] == '"' && cmd[len - 1] == '"')) {
                cmd[len - 1] = '\0';
                cmd++;
            }

            if (alias_count < MAX_ALIASES) {
                aliases[alias_count].name = strdup(name);
                aliases[alias_count].command = strdup(cmd);
                alias_count++;
            }
            *eq = '=';
        }
    }

    fclose(f);
}

// Expand aliases in input line, only first token; returns newly malloc'ed string
char *expand_aliases(const char *input) {
    if (!input || !*input)
        return strdup(input);

    const char *space = strchr(input, ' ');
    size_t first_len = space ? (size_t)(space - input) : strlen(input);

    for (size_t i = 0; i < alias_count; i++) {
        if (strlen(aliases[i].name) == first_len && strncmp(input, aliases[i].name, first_len) == 0) {
            const char *rest = input + first_len;
            while (*rest && isspace((unsigned char)*rest))
                rest++;
            size_t len = strlen(aliases[i].command) + (rest ? strlen(rest) : 0) + 2;
            char *expanded = malloc(len);
            if (!expanded) return strdup(input);
            strcpy(expanded, aliases[i].command);
            if (*rest) {
                strcat(expanded, " ");
                strcat(expanded, rest);
            }
            return expanded;
        }
    }

    return strdup(input);
}

// Returns 1 if path is executable
static int is_executable(const char *path) {
    return access(path, X_OK) == 0;
}

// Search PATH dirs for executables matching prefix
static char **get_path_executables(const char *prefix) {
    char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    char **matches = NULL;
    size_t matches_size = 0, matches_cap = 16;
    matches = malloc(matches_cap * sizeof(char *));
    if (!matches) return NULL;

    char *path_env_dup = strdup(path_env);
    if (!path_env_dup) {
        free(matches);
        return NULL;
    }

    char *saveptr = NULL;
    char *dir = strtok_r(path_env_dup, ":", &saveptr);

    while (dir) {
        DIR *dp = opendir(dir);
        if (dp) {
            struct dirent *entry;
            while ((entry = readdir(dp)) != NULL) {
                if (entry->d_type != DT_REG && entry->d_type != DT_LNK)
                    continue;

                if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
                    size_t fullpathlen = strlen(dir) + 1 + strlen(entry->d_name) + 1;
                    char *fullpath = malloc(fullpathlen);
                    if (!fullpath) continue;
                    snprintf(fullpath, fullpathlen, "%s/%s", dir, entry->d_name);
                    if (is_executable(fullpath)) {
                        if (matches_size >= matches_cap) {
                            matches_cap *= 2;
                            char **tmp = realloc(matches, matches_cap * sizeof(char *));
                            if (!tmp) {
                                free(fullpath);
                                closedir(dp);
                                dir = NULL;
                                break;
                            }
                            matches = tmp;
                        }
                        matches[matches_size++] = strdup(entry->d_name);
                    }
                    free(fullpath);
                }
            }
            closedir(dp);
        }
        if (!dir) break;
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_env_dup);

    if (matches_size == 0) {
        free(matches);
        return NULL;
    }

    matches[matches_size] = NULL;
    return matches;
}

// Command generator for first word completion (builtins + executables)
static char *command_generator(const char *text, int state) {
    static int list_index, len;
    static char **matches = NULL;

    if (state == 0) {
        list_index = 0;
        len = strlen(text);

        size_t capacity = 32;
        matches = malloc(capacity * sizeof(char *));
        if (!matches) return NULL;

        size_t count = 0;

        // Add builtin commands
        for (const char **cmd = builtin_commands; *cmd; cmd++) {
            if (strncmp(*cmd, text, len) == 0) {
                if (count >= capacity) {
                    capacity *= 2;
                    char **tmp = realloc(matches, capacity * sizeof(char *));
                    if (!tmp) {
                        for (size_t i = 0; i < count; i++) free(matches[i]);
                        free(matches);
                        matches = NULL;
                        return NULL;
                    }
                    matches = tmp;
                }
                matches[count++] = strdup(*cmd);
            }
        }

        // Add executables in PATH
        char **path_matches = get_path_executables(text);
        if (path_matches) {
            for (size_t i = 0; path_matches[i] != NULL; i++) {
                if (count >= capacity) {
                    capacity *= 2;
                    char **tmp = realloc(matches, capacity * sizeof(char *));
                    if (!tmp) {
                        for (size_t j = 0; j < count; j++) free(matches[j]);
                        free(matches);
                        matches = NULL;
                        for (size_t j = i; path_matches[j] != NULL; j++) free(path_matches[j]);
                        free(path_matches);
                        return NULL;
                    }
                    matches = tmp;
                }
                matches[count++] = strdup(path_matches[i]);
                free(path_matches[i]);
            }
            free(path_matches);
        }

        if (count == 0) {
            free(matches);
            matches = NULL;
            return NULL;
        }

        matches[count] = NULL;
    }

    if (!matches) return NULL;

    char *result = matches[list_index];
    if (result)
        list_index++;
    else {
        for (int i = 0; matches[i] != NULL; i++)
            free(matches[i]);
        free(matches);
        matches = NULL;
    }

    return result;
}

// Readline completion function
char **kali_shell_completion(const char *text, int start, int end) {
    (void)end;
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    } else {
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}

int main(void) {
    // Initialize shell configuration with defaults and load config
    config_init(&shell_config);
    config_load(&shell_config);

    // Setup Ctrl-C handler
    struct sigaction sa_int = {0};
    sa_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa_int, NULL);

    // Setup SIGCHLD handler for job notifications
    struct sigaction sa_chld = {0};
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);

    // Load persistent history, aliases from config
    history_init(".kali_shell_history");
    load_aliases();

    // Setup readline completion
    rl_attempted_completion_function = kali_shell_completion;

    while (keep_running) {
        char prompt_buf[PROMPT_BUFFER_SIZE];
        prompt_render(prompt_buf, sizeof(prompt_buf), &shell_config);

        char *input = readline(prompt_buf);
        if (!input) {
            printf("\n");
            break;
        }

        char *trimmed = trim_whitespace(input);
        if (*trimmed == '\0') {
            free(input);
            continue;
        }

        add_history(trimmed);
        history_add(trimmed);

        char *expanded = expand_aliases(trimmed);
        free(input);
        input = expanded;

        command_list_t *cmdlist = parse_input(input);
        free(input);

        if (!cmdlist) {
            fprintf(stderr, "parse error\n");
            continue;
        }

        for (size_t i = 0; i < cmdlist->count; i++) {
            command_t *cmd = cmdlist->commands[i];

            if (is_builtin(cmd->argv[0])) {
                if (builtin_execute(cmd) == SHELL_EXIT) {
                    keep_running = 0;
                    break;
                }
            } else {
                int ret = executor_execute(cmd);
                if (ret < 0) {
                    fprintf(stderr, "command execution failed\n");
                }
            }
        }

        command_list_free(cmdlist);

        if (!keep_running)
            break;
    }

    history_save();
    history_free();

    // Free alias memory
    for (size_t i = 0; i < alias_count; i++) {
        free(aliases[i].name);
        free(aliases[i].command);
    }

    return 0;
}
