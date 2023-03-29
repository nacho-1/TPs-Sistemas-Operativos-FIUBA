#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Wrong format\n");
		_exit(EXIT_FAILURE);
	}

	unsigned int n = atoi(argv[1]);
	if (n < 2) {
		fprintf(stderr, "Invalid number\n");
		_exit(EXIT_FAILURE);
	}

	int readfd, writefd;
	unsigned int buf;

	int fds[2];
	create_pipe(fds);

	pid_t cpid = fork_process();
	if (cpid == 0) {
		int finished = 0;
		while (!finished) {
			readfd = fds[0];
			close(fds[1]);

			ssize_t s = readsome(readfd, &n, sizeof(n));
			if (s == 0) {
				finished = 1;
				continue;
			}
			printf("primo %d\n", n);
			create_pipe(fds);
			cpid = fork_process();
			if (cpid == 0) {
				close(readfd);
			} else {
				writefd = fds[1];
				close(fds[0]);
				while (readsome(readfd, &buf, sizeof(buf)) != 0)
					if (buf%n != 0)
						writesome(writefd, &buf, sizeof(buf));
				close(writefd);
				finished = 1;
			}
		}
		close(readfd);
	} else {
		writefd = fds[1];
		close(fds[0]);
		for (buf = 2; buf <= n; buf++)
			writesome(writefd, &buf, sizeof(buf));
		close(writefd);
	}

	wait(NULL);
	exit(EXIT_SUCCESS);
}