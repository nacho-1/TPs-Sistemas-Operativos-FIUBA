#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void printProcessStatus(int fork, int pid, int ppid)
{
	printf( "Donde fork me devuelve %d\n", fork);
	printf( "  - getpid me devuelve: %d\n", pid);
	printf( "  - getppid me devuelve: %d\n", ppid);
}

ssize_t readsome(int fd, void *buf, size_t count)
{
	ssize_t s = read(fd, buf, count);

	if (s == -1) {
		int errsv = errno;
		fprintf(stderr, "Read from fd %d failed: %s\n", fd, strerror(errsv));
		_exit(EXIT_FAILURE);
	}

	return s;
}

ssize_t writesome(int fd, const void *buf, size_t count)
{
	ssize_t s = write(fd, buf, count);

	if (s == -1) {
		int errsv = errno;
		fprintf(stderr, "Write to fd %d failed: %s\n", fd, strerror(errsv));
		_exit(EXIT_FAILURE);
	}

	return s;
}

int main(void)
{
	srandom(time(NULL));

	printf("Hola, soy PID %d: \n", getpid());

	int fds1[2];
	int fds2[2];
	if (pipe(fds1) == -1) {
		int errsv = errno;
		fprintf(stderr, "First pipe failed: %s\n", strerror(errsv));
		_exit(EXIT_FAILURE);
	}
	if (pipe(fds2) == -1) {
		int errsv = errno;
		fprintf(stderr, "Second pipe failed: %s\n", strerror(errsv));
		_exit(EXIT_FAILURE);
	}

	printf(" - primer pipe me devuelve: [%d, %d]\n", fds1[0], fds1[1]);
	printf(" - segundo pipe me devuelve: [%d, %d]\n\n", fds2[0], fds2[1]);

	pid_t cpid = fork();
	if (cpid == -1) {
		int errsv = errno;
		fprintf(stderr, "Fork failed: %s\n", strerror(errsv));
		_exit(EXIT_FAILURE);
	}

	int readfd, writefd;
	int msg = 0;

	if(cpid == 0) {
		readfd = fds1[0];
		writefd = fds2[1];
		close(fds1[1]);
		close(fds2[0]);

		printProcessStatus(cpid, getpid(), getppid());

		readsome(readfd, &msg, sizeof(msg));

		printf("  - recibo el valor %d a travez de fd=%d\n", msg, readfd);
		printf("  - reenvio el valor en fd=%d y termino\n\n", writefd);

		writesome(writefd, &msg, sizeof(msg));
	} else {
		readfd = fds2[0];
		writefd = fds1[1];
		close(fds2[1]);
		close(fds1[0]);

		msg = random();

		printProcessStatus(cpid, getpid(), getppid());
		printf("  - random me devuelve: %d\n", msg );
		printf("  - envio el valor %d a travez de fd=%d\n\n", msg, writefd);

		writesome(writefd, &msg, sizeof(msg));
		readsome(readfd, &msg, sizeof(msg));

		printf("Hola, de nuevo PID %d\n", getpid());
		printf("  - recibí valor %d vía fd=%d\n", msg, readfd);
	}

	close(readfd);
	close(writefd);

	exit(EXIT_SUCCESS);
}
