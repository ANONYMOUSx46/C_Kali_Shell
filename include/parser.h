#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

typedef struct command {
    char **argv;                   // Argument vector; null-terminated
    int argc;                     // Number of arguments
    char *raw;                    // Raw command string
    char *input_file;             // Input redirection file name
    char *output_file;            // Output redirection file name
    int append_output;            // 1 if output is append (>>), 0 if overwrite (>)
    struct command *pipe_to;      // Next command in pipeline or NULL
    int pipe_count;               // Number of pipes following
} command_t;

typedef struct command_list {
    command_t **commands;         // Array of parsed commands
    size_t count;                 // Number of commands in array
} command_list_t;

// Parse input command line into a command_list_t structure
command_list_t *parse_input(const char *input);

// Free memory allocated to a command_t
void command_free(command_t *cmd);

// Free memory allocated to a command_list_t and all contained commands
void command_list_free(command_list_t *cmdlist);

// Return 1 if the command name is a shell builtin, 0 otherwise
int is_builtin(const char *cmd);

#endif // PARSER_H
