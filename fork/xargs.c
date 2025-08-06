#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif /* NARGS */

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Invalid input\n");
		_exit(EXIT_FAILURE);
	}

	char *args[NARGS + 2];
	args[0] = malloc(sizeof(char) * (strlen(argv[1]) +
	                                 1));  // +1 por el \0 al final del string
	strcpy(args[0], argv[1]);
	for (int i = 1; i <= NARGS; i++)
		args[i] = NULL;
	args[NARGS + 1] = NULL;

	char *line = NULL;
	size_t sz;
	int nargs = 0;

	while (getline(&line, &sz, stdin) != -1) {
		size_t len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = '\0';
		nargs++;
		args[nargs] = malloc(sizeof(char) * (strlen(line) + 1));
		strcpy(args[nargs], line);

		if (nargs == NARGS) {
			pid_t cpid = fork();
			if (cpid == -1)
				perror("Fork failed");  // No puedo hacer exit
				                        // del programa xq tengo memoria dinamica alocada
			else if (cpid == 0)
				execvp(args[0], args);
			else
				wait(NULL);

			for (int i = 1; i <= nargs; i++)
				free(args[i]);
			nargs = 0;
		}
	}

	if (nargs > 0) {
		args[nargs + 1] = NULL;
		pid_t cpid = fork();
		if (cpid == -1)
			perror("Fork failed");
		else if (cpid == 0)
			execvp(args[0], args);
		else
			wait(NULL);

		for (int i = 1; i <= nargs; i++)
			free(args[i]);
	}

	if (line != NULL)
		free(line);
	free(args[0]);
	exit(EXIT_SUCCESS);
}