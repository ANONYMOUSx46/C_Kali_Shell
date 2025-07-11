// src/executor.c
#define _GNU_SOURCE
#include "executor.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Recursive helper to execute pipeline commands
// cmd: current command_t node
// input_fd: fd to use as standard input (or -1 for default)
// Returns pid of last created child or -1 on error
static pid_t exec_pipeline(command_t *cmd, int input_fd) {
    if (!cmd) return -1;

    int pipefd[2];
    pid_t pid;

    // If there is a next pipe command, create pipe
    int has_pipe = (cmd->pipe_to != NULL);

    if (has_pipe) {
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return -1;
        }
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // CHILD PROCESS

        // If input_fd != -1, dup as stdin
        if (input_fd != -1) {
            if (dup2(input_fd, STDIN_FILENO) == -1) {
                perror("dup2 stdin");
                exit(EXIT_FAILURE);
            }
            close(input_fd);
        }

        // If piped to another command, redirect stdout to pipe write end
        if (has_pipe) {
            close(pipefd[0]); // close unused read end
            if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup2 stdout");
                exit(EXIT_FAILURE);
            }
            close(pipefd[1]);
        }

        // Handle input redirection if any
        if (cmd->input_file) {
            int fd = open(cmd->input_file, O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "cannot open input file '%s': %s\n", cmd->input_file, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2 input file");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        // Handle output redirection if any
        if (cmd->output_file) {
            int flags = O_WRONLY | O_CREAT;
            if (cmd->append_output)
                flags |= O_APPEND;
            else
                flags |= O_TRUNC;

            int fd = open(cmd->output_file, flags, 0644);
            if (fd == -1) {
                fprintf(stderr, "cannot open output file '%s': %s\n", cmd->output_file, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2 output file");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        execvp(cmd->argv[0], cmd->argv);
        fprintf(stderr, "exec failed: %s: %s\n", cmd->argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // PARENT PROCESS

        // Close pipe write end (unused)
        if (has_pipe) {
            close(pipefd[1]);
        }

        // Close inherited input_fd if valid
        if (input_fd != -1) close(input_fd);

        // If has next pipe, recurse with pipe read end as new input
        pid_t next_pid = -1;
        if (has_pipe) {
            next_pid = exec_pipeline(cmd->pipe_to, pipefd[0]);
            if (next_pid == -1) {
                // Error in recursion, close remaining fds
                close(pipefd[0]);
                return -1;
            }
        }

        // Wait for current child before returning if no next pipe
        int status;
        waitpid(pid, &status, 0);

        // If pipeline, wait for last child and return that pid
        if (has_pipe) {
            waitpid(next_pid, &status, 0);
            return next_pid;
        }
        return pid;
    }
}

int executor_execute(command_t *cmd) {
    if (!cmd) return -1;

    pid_t last_pid = exec_pipeline(cmd, -1);
    if (last_pid == -1) return -1;

    return 0;
}
