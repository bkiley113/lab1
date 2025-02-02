#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

//I know that it is 4096 on Linux, but putting myself in the portability mindset...
//...it is probably good practice to not make more conservative assumptions
#define MAX_PATH_LEN 1024
#define MAX_COMMANDS 16

int is_executable(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return 0;

    //if they give us a path of any kind, we can check it directly
    if (strchr(cmd, '/')) {
        return (access(cmd, X_OK) == 0);
    }

    // if not, we will search for an executable in the environment variable
    char *path_env = getenv("PATH");
    if (!path_env || strlen(path_env) == 0) {
        return 0;
    }

    //we make a copy of path so we don't accidentally change the original
    char path_copy[MAX_PATH_LEN];
    strncpy(path_copy, path_env, MAX_PATH_LEN - 1);
    path_copy[MAX_PATH_LEN - 1] = '\0';

    // we can use strtok to tokenize the path, going one directory at a time
    char *saveptr;
    char *directory = strtok_r(path_copy, ":", &saveptr);

	//here is the buffer that will hold each path tested
    char full_path[MAX_PATH_LEN];

    while (directory) {
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, cmd);
        if (access(full_path, X_OK) == 0) {
            return 1;  //found executable!!!
        }
		//have strtok_r pick up where it left off
        directory = strtok_r(NULL, ":", &saveptr);
    }

    return 0;  //no bueno
}

char ***parse_commands(int argc, char *argv[], int *cmd_count) {
    char ***commands = malloc(MAX_COMMANDS * sizeof(char **));
    if (!commands) {
        perror("malloc failure");
        exit(1);
    }

    int cmd_index = -1;
    int arg_index = 0;
    for (int i = 1; i < argc; i++) {
        if (is_executable(argv[i])) {
            if (cmd_index >= 0) {
                commands[cmd_index][arg_index] = NULL;
            }
            cmd_index++;
            arg_index = 0;
            commands[cmd_index] = malloc((argc - i + 1) * sizeof(char *));  // Allocate only required space
            if (!commands[cmd_index]) {
                perror("malloc failed");
                exit(1);
            }
        }
        if (cmd_index >= 0) {
            commands[cmd_index][arg_index++] = argv[i];
        }
    }
    if (cmd_index >= 0 && arg_index > 0) {
        commands[cmd_index][arg_index] = NULL;
    }

    *cmd_count = cmd_index + 1;
    return commands;
}

void print_commands(char ***commands, int cmd_count) {
    for (int i = 0; i < cmd_count; i++) {
        printf("Command %d:", i);
        for (int j = 0; commands[i][j] != NULL; j++) {
            printf(" %s", commands[i][j]);
        }
        printf("\n");
    }
}

void free_commands(char ***commands, int cmd_count) {
    for (int i = 0; i < cmd_count; i++) {
        free(commands[i]);
    }
    free(commands);
}

void execute_pipeline(char ***commands, int cmd_count) {
    int pipes[cmd_count - 1][2];  
    pid_t pids[cmd_count];

    for (int i = 0; i < cmd_count; i++) {
        if (i < cmd_count - 1 && pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(1);
        }

        if (pids[i] == 0) {  // Child process
            if (i > 0) { 
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < cmd_count - 1) { 
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(commands[i][0], commands[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < cmd_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < cmd_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <commands>\n", argv[0]);
        return 1;
    }

    int cmd_count;
    char ***commands = parse_commands(argc, argv, &cmd_count);

    if (cmd_count == 0) {
        fprintf(stderr, "Error: No valid executable commands found.\n");
        free(commands);
        return 1;
    }

    print_commands(commands, cmd_count);
    execute_pipeline(commands, cmd_count);
    free_commands(commands, cmd_count);

    return 0;
}