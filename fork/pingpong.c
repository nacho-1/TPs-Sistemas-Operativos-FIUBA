#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

void
print_process_info(int fork_ret, int pid, int ppid);
void
print_process_info(int fork_ret, int pid, int ppid)
{
	printf("Donde fork me devuelve %d\n", fork_ret);
	printf("  - getpid me devuelve: %d\n", pid);
	printf("  - getppid me devuelve: %d\n", ppid);
}

int
main(void)
{
	srandom(time(NULL));

	printf("Hola, soy PID %d:\n", getpid());

	int fds1[2];
	int fds2[2];
	create_pipe(fds1);
	create_pipe(fds2);

	printf(" - primer pipe me devuelve: [%d, %d]\n", fds1[0], fds1[1]);
	printf(" - segundo pipe me devuelve: [%d, %d]\n\n", fds2[0], fds2[1]);

	pid_t cpid = fork_process();

	int readfd, writefd;
	int msg = 0;

	if (cpid == 0) {
		readfd = fds1[0];
		writefd = fds2[1];
		close(fds1[1]);
		close(fds2[0]);

		print_process_info(cpid, getpid(), getppid());

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

		print_process_info(cpid, getpid(), getppid());
		printf("  - random me devuelve: %d\n", msg);
		printf("  - envio el valor %d a travez de fd=%d\n\n", msg, writefd);

		writesome(writefd, &msg, sizeof(msg));
		readsome(readfd, &msg, sizeof(msg));

		printf("Hola, de nuevo PID %d\n", getpid());
		printf("  - recibí valor %d vía fd=%d\n", msg, readfd);
	}

	close(readfd);
	close(writefd);

	wait(NULL);
	exit(EXIT_SUCCESS);
}
