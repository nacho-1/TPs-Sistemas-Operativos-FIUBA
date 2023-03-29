#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

pid_t
fork_process(void)
{
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Fork failed");
		_exit(EXIT_FAILURE);
	}

	return cpid;
}

int
create_pipe(int *fds)
{
	int s = pipe(fds);
	if (s == -1) {
		perror("Pipe failed");
		_exit(EXIT_FAILURE);
	}

	return s;
}

ssize_t
readsome(int fd, void *buf, size_t count)
{
	ssize_t s = read(fd, buf, count);

	if (s == -1) {
		int errsv = errno;
		fprintf(stderr, "Read from fd %d failed: %s\n", fd, strerror(errsv));
		_exit(EXIT_FAILURE);
	}

	return s;
}

ssize_t
writesome(int fd, const void *buf, size_t count)
{
	ssize_t s = write(fd, buf, count);

	if (s == -1) {
		int errsv = errno;
		fprintf(stderr, "Write to fd %d failed: %s\n", fd, strerror(errsv));
		_exit(EXIT_FAILURE);
	}

	return s;
}
