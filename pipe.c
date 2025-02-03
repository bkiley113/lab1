#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

void create_pipeline(char *commands[], int num_commands) {
    int pipefd[num_commands - 1][2];
    pid_t pids[num_commands];

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefd[i]) < 0) {
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
                dup2(pipefd[i - 1][0], STDIN_FILENO);
            }

            // Set up output redirection if not the last command
            if (i != num_commands - 1) {
                dup2(pipefd[i][1], STDOUT_FILENO);
            }

            // Close all pipes in the child process
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }

            // Execute command
            if (execlp(commands[i], commands[i], (char *)NULL) == -1){
                perror("execlp");
                exit(1);
            }
        }
    }

    // Close all pipes in the parent process
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    // Wait for all child processes and return last command's status
    int status;
    int last_exit_code =0;

    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], &status, 0);
        if(WIFEXITED(status) && WEXISTSTATUS(status)!= 0){
            last_exit_code = WEXITSTATUS(status);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        errno = EINVAL;
        perror("Invalid argument");
        exit(errno);
    }

    // Pass commands to pipeline function
    create_pipeline(&argv[1], argc - 1);
    return 0;
}