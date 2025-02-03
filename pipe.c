//!!!! TEST ONE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>

void pipeline(char *commands[], int cmd_count){

	//we want a process per command
	pid_t pids[cmd_count];
	//and we want a pipe between each process
	int pipefd[cmd_count - 1][2];

	//here we will iteratively create each pipe
	for (int i = 0; i < cmd_count; i++){
		if(pipe(pipefd[i]) < 0){
			perror("pipe");
			exit(1);
		}
	}
	//now, create processes
	for (int i = 0; i < cmd_count; i++){
		pids[i] = fork();
		if(pids[i] < 0){
			perror("fork");
			exit(1);
		}
		//now consider the child process
		if(pids[i] == 0){
			//since the first process not have a command to read from, it shouldn't redirect stdin
			//else redirect
			if (i != 0){
				dup2(pipefd[i-1][0], STDIN_FILENO);
			}
			//similarly, since the last process does not have another to output to, doesn't need to redirect
			//else redirect
			if (i != cmd_count - 1){
				dup2(pipefd[i][1], STDOUT_FILENO);
			}

			//since fork() duplicates processes, we must close all open file descriptors that were inherited
			for (int i = 0; i < cmd_count - 1; i++){
				close(pipefd[i][0]);
				close(pipefd[i][1]);
			}

			char *cmds[] = {commands[i], NULL};
			if(execvp(commands[i], cmds) < 0){
				perror("execvp");
				exit(errno);
			}
		}
	}
	//back in parent proces, close all pipe ends, no longer in use
	for (int i = 0; i < cmd_count - 1; i++){
		close(pipefd[i][0]);
		close(pipefd[i][1]);
	}

	//now that we are back in the parent process, we must address asynchronicity
	//that is, we must implement waiting for the child processes
	int status;
	int ex_code;

	//just like | , we want errors to propagate
	//use WIFEXITED from sys/wait.h "queries the child termination status, determines whether the child process ended normally."
	//use WEXITED from sys/wait.h to generally get the exit status of the child process
	//according to geeks for geeks, WEXITSTATUS should only be used in conjuction with WIFEXITED
	for (int i = 0; i < cmd_count; i++){
		//we will wait for each child process to finish
		waitpid(pids[i], &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0){
			ex_code = WEXITSTATUS(status);
		}
	}
	exit(ex_code);
}


int main(int argc, char *argv[])
{
	//argument validation per the spec
	if(argc < 2){
		fprintf(stderr, "Invalid argument\n");
		errno = EINVAL; 
		exit(errno);
	}

	//call our pipeline function!
	pipeline(&argv[1], argc-1);
	return 0;
}
