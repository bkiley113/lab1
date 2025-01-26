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

	char **command_args = &argv[1];
	command_args[argc - 1] = NULL;

	execlp(argv[1], command_args);
	perror("execlp");

	return 1;
}

