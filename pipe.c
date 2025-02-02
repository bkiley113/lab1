#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define CMD_BUFFER_SIZE 256

void create_pipeline(char *commands[], int num_commands) {
    int pipefd[2 * (num_commands - 1)];
    pid_t pids[num_commands];

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefd + 2 * i) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    // Create child processes
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(1);
        }

        if (pids[i] == 0) { // Child process
            // Set up input redirection if not the first command
            if (i != 0) {
                if (dup2(pipefd[2 * (i - 1)], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Set up output redirection if not the last command
            if (i != num_commands - 1) {
                if (dup2(pipefd[2 * i + 1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Close all pipe file descriptors in child
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefd[j]);
            }

            // Make a local, writable copy of the command string.
            char cmd_copy[CMD_BUFFER_SIZE];
            if (strlen(commands[i]) >= CMD_BUFFER_SIZE) {
                fprintf(stderr, "Command too long\n");
                exit(1);
            }
            strncpy(cmd_copy, commands[i], CMD_BUFFER_SIZE);
            cmd_copy[CMD_BUFFER_SIZE - 1] = '\0';

            // Parse command arguments properly using strtok on the copy.
            char *cmd_args[64];
            int arg_count = 0;
            char *token = strtok(cmd_copy, " ");
            while (token) {
                cmd_args[arg_count++] = token;
                token = strtok(NULL, " ");
            }
            cmd_args[arg_count] = NULL;

            execvp(cmd_args[0], cmd_args);
            perror("execvp"); // only reached if execvp fails
            exit(127);        // exit with common code for command not found
        }
    }

    // Close all pipe file descriptors in parent process
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefd[i]);
    }

    // Wait for all child processes to finish and collect exit status
    int exit_status = 0;
    for (int i = 0; i < num_commands; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        // As in shell pipelines, use the exit status of the last command
        if (i == num_commands - 1 && WIFEXITED(status)) {
            exit_status = WEXITSTATUS(status);
        }
    }
    exit(exit_status);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command1> <command2> ...\n", argv[0]);
        return 1;
    }

    // Parse commands into an array (each command may include arguments separated by spaces)
    char *commands[argc - 1];
    for (int i = 1; i < argc; i++) {
        commands[i - 1] = argv[i];
    }

    create_pipeline(commands, argc - 1);
    return 0;
}
