#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

//I know that it is 4096 on Linux, but putting myself in the portability mindset...
//...it is probably good practice to not make more conservative assumptions
#define MAX_PATH_LEN 1024


// int is_executable(const char *cmd) {
//     if (!cmd || strlen(cmd) == 0) return 0;

//     // If the command contains '/', check directly if it's executable
//     if (strchr(cmd, '/')) {
//         printf("Checking absolute/relative path: %s\n", cmd);
//         return (access(cmd, X_OK) == 0);
//     }

//     // Get the PATH variable
//     char *path_env = getenv("PATH");
//     if (!path_env || strlen(path_env) == 0) {
//         fprintf(stderr, "PATH environment variable is not set!\n");
//         return 0;
//     }

//     // Copy PATH to avoid modifying the original
//     char path_copy[MAX_PATH_LEN];
//     strncpy(path_copy, path_env, MAX_PATH_LEN - 1);
//     path_copy[MAX_PATH_LEN - 1] = '\0';

//     // Tokenize PATH
//     char *saveptr;
//     char *directory = strtok_r(path_copy, ":", &saveptr);

//     char full_path[MAX_PATH_LEN];

//     printf("Checking command: %s\n", cmd);
    
//     while (directory) {
//         snprintf(full_path, sizeof(full_path), "%s/%s", directory, cmd);
//         printf("Trying path: %s\n", full_path); // Debug print

//         if (access(full_path, X_OK) == 0) {
//             printf("GOOD! Found executable: %s\n", full_path);
//             return 1;
//         }

//         directory = strtok_r(NULL, ":", &saveptr);
//     }

//     printf("BAD Command not found: %s\n", cmd);
//     return 0;
// }

int is_executable(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return 0;

    // If the command contains '/', check directly if it's executable
    if (strchr(cmd, '/')) {
        return (access(cmd, X_OK) == 0);
    }

    // Get the PATH variable
    char *path_env = getenv("PATH");
    if (!path_env || strlen(path_env) == 0) {
        return 0;
    }

    // Copy PATH to avoid modifying the original
    char path_copy[MAX_PATH_LEN];
    strncpy(path_copy, path_env, MAX_PATH_LEN - 1);
    path_copy[MAX_PATH_LEN - 1] = '\0';

    // Tokenize PATH
    char *saveptr;
    char *directory = strtok_r(path_copy, ":", &saveptr);

    char full_path[MAX_PATH_LEN];

    while (directory) {
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, cmd);
        if (access(full_path, X_OK) == 0) {
            return 1;  // Found executable
        }
        directory = strtok_r(NULL, ":", &saveptr);
    }

    return 0;  // Command not found
}

// int main(int argc, char *argv[])
// {
// 	if(argc < 2){
// 		fprintf(stderr, "Usage: %s  <command>\n", argv[0]);
// 		return 1;
// 	}

// 	int pipefd1[2];
// 	pid_t pid;

// 	//create a pipe and check for errors
// 	if (pipe(pipefd1) < 0){
// 		perror("pipe");
// 		exit(1);
// 	}

// 	//create new process and check for errors
// 	pid = fork();
// 	if(pid < 0){
// 		perror("fork");
// 		exit(1);
// 	}

// 	//child process
// 	if(pid == 0){
// 		//we want child process to run command so close read end of pipe
// 		close(pipefd1[0]);
// 		//any stdout of the process redirected to write end of pipe
// 		if(dup2(pipefd1[1], STDOUT_FILENO) < 0){
// 			perror("dup2");
// 			exit(1);
// 		}
// 		//now STDOUT points to the write end of pipe, we can close it to avoid memory leak
// 		close(pipefd1[1]);

// 		//here we create a subarray of the cmd line arguments
// 		char **command_args = &argv[1];
// 		//make sure it is null-terminated so execvp works, use pointer arithmetic
// 		command_args[argc-1] = NULL;

// 		//replace process image with cmd line program given by argv
// 		execvp(argv[1], command_args);

// 		//it should no longer execute this code but it will if there is an error
// 		perror("execvp");
// 		exit(1);

// 	}
// 	//parent process, want to read from child process
// 	else{
// 		close(pipefd1[1]);

// 		//use dup2 to redirect read end of pipe (child stdout) to parent stdin
// 		if(dup2(pipefd1[0], STDIN_FILENO) < 0){
// 			perror("dup2");
// 			exit(1);
// 		}
// 		close(pipefd1[0]);
// 		//initialize a buffer to read parent stdin from pipe into
// 		char buffer[1024];

// 		printf("Parent is reading child's stdout to its stdin via pipe: \n");
		
// 		//read from stdin into buffer, while it is not NULL we can print it!
// 		while(fgets(buffer, sizeof(buffer), stdin) != NULL){
// 			//printf directly to stdout of parent process
// 			printf("%s", buffer);
// 		}

// 		wait(NULL);
// 	}


// }

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command1> <command2> ...\n", argv[0]);
        return 1;
    }

    printf("Classification of arguments:\n");
    for (int i = 1; i < argc; i++) {
        if (is_executable(argv[i])) {
            printf("  ✅ '%s' is a command.\n", argv[i]);
        } else {
            printf("  🔹 '%s' is likely an argument.\n", argv[i]);
        }
    }

    return 0;
}

