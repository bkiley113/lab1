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
	if(argc > 2){
		fprintf(stdout, "I haven't figured this out yet lol");
		return 1;
	}

	execlp(argv[1], argv[1], NULL);
	perror("execlp");
	return 1;
}
