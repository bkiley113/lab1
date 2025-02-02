#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

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

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefd[j]);
            }

            // Parse command arguments properly
            char *cmd_args[64];
            int arg_count = 0;
            char *token = strtok(commands[i], " ");
            while (token) {
                cmd_args[arg_count++] = token;
                token = strtok(NULL, " ");
            }
            cmd_args[arg_count] = NULL;

            execvp(cmd_args[0], cmd_args);
            perror("execvp"); // Only reaches here if execvp fails
            exit(127); // Exit with specific error code for command not found
        }
    }

    // Close all pipe file descriptors in parent process
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefd[i]);
    }

    // Wait for all child processes to finish & check for failures
    int exit_status = 0;
    for (int i = 0; i < num_commands; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            exit_status = WEXITSTATUS(status); // Capture the first failing exit code
        }
    }

    exit(exit_status); // Exit with the first failed command's status
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command1> <command2> ...\n", argv[0]);
        return 1;
    }

    // Parse commands into an array
    char *commands[argc - 1];
    for (int i = 1; i < argc; i++) {
        commands[i - 1] = argv[i];
    }

    create_pipeline(commands, argc - 1);
    return 0;
}