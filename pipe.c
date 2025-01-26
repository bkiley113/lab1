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
		dup2(pipefd[1], STDOUT_FILENO);
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

		char buffer[1024];
		ssize_t nbytes;

		while((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0){
			buffer[nbytes] = '/0';
			printf("stdout from child given to parent is: \n%s", buffer);
		}

		close(pipefd[0]);

		wait(NULL);
	}


}

