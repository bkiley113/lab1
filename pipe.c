#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	if(argc < 2){
		fprintf(stderr, "Usage: %s  <command>\n", argv[0]);
		return 1;
	}

	int pipefd[2];
	pid_t pid;

	//create a pipe and check for errors
	if (pipe(pipefd) < 0){
		perror("pipe");
		exit(1);
	}

	//create new process and check for errors
	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(1);
	}

	//child process
	if(pid == 0){
		//we want child process to run command so close read end of pipe
		close(pipefd[0]);
		//any stdout of the process redirected to write end of pipe
		if(dup2(pipefd[1], STDOUT_FILENO) < 0){
			perror("dup2");
			exit(1);
		}
		//now STDOUT points to the write end of pipe, we can close it to avoid memory leak
		close(pipefd[1]);

		//here we create a subarray of the cmd line arguments
		char **command_args = &argv[1];
		//make sure it is null-terminated so execvp works
		command_args[argc-1] = NULL;

		//replace process image with cmd line program
		execvp(argv[1], command_args);

		//it should no longer execute this code but it will if there is an error
		perror("execvp");
		exit(1);

	}
	//parent process, want to read from child process
	else{
		close(pipefd[1]);

		//use dup2 to redirect read end of pipe (child stdout) to parent stdin
		if(dup2(pipefd[0], STDIN_FILENO) < 0){
			perror("dup2");
			exit(1);
		}
		close(pipefd[0]);
		//initialize a buffer to read parent stdin from pipe into
		char buffer[1024];

		printf("Parent is reading child's stdout to its stdin via pipe: \n");
		
		//read from stdin into buffer, while it is not NULL we can print it!
		while(fgets(buffer, sizeof(buffer), stdin) != NULL){
			//printf directly to stdout of parent process
			printf("%s", buffer);
		}

		wait(NULL);
	}


}

