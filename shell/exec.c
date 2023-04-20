#include "exec.h"
#include "runcmd.h"


// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	printf("antes de for de get key, arg: %s\n", arg);
	for (i = 0; arg[i] != '='; i++) {
		printf("antes de asignacion\n");
		key[i] = arg[i];
		printf("en el loop: %s\n", key);
	}


	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		char key[BUFLEN];
		char value[BUFLEN];
		char *env_var = eargv[i];
		get_environ_key(env_var, key);
		int idx = block_contains(env_var, '=');
		get_environ_value(env_var, value, idx);
		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int open_fd = open(file, flags, S_IWUSR | S_IRUSR);

	return open_fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		eprint_debug(errno,
		             "Command execution failed: "
		             "%s\n File: %s. Line: %d",
		             e->scmd,
		             __FILE__,
		             __LINE__);

		return;

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		return;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero

		r = (struct execcmd *) cmd;

		int fd_out;
		int fd_err;
		int fd_in;
		if (strlen(r->out_file) > 0) {
			fd_out = open_redir_fd(r->out_file,
			                       O_CLOEXEC | O_CREAT | O_WRONLY |
			                               O_TRUNC);
			if (fd_out < 0) {
				eprint_debug(errno,
				             "Error opening file: %s."
				             "Invalid file descriptor %d.\n"
				             "Output Redirection cancelled.\n"
				             "File: %s. Line: %d\n",
				             r->out_file,
				             fd_out,
				             __FILE__,
				             __LINE__);
				return;
			} else {
				dup2(fd_out, STDOUT_FILENO);
			}
		}
		if (strlen(r->in_file) > 0) {
			fd_in = open_redir_fd(r->in_file, O_CLOEXEC | O_RDONLY);
			if (fd_in < 0) {
				eprint_debug(errno,
				             "Error opening file: %s."
				             "Invalid file descriptor %d.\n"
				             "Input Redirection cancelled.\n"
				             "File: %s. Line: %d\n",
				             r->in_file,
				             fd_in,
				             __FILE__,
				             __LINE__);
				return;
			} else {
				dup2(fd_in, STDIN_FILENO);
			}
		}
		if (strlen(r->err_file) > 0) {
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, fd_err);
			} else {
				fd_err = open_redir_fd(r->err_file,
				                       O_CLOEXEC | O_CREAT |
				                               O_WRONLY | O_TRUNC);
			}

			if (fd_err < 0) {
				eprint_debug(errno,
				             "Error opening file: %s."
				             "Invalid file descriptor %d.\n"
				             "Error Redirection cancelled.\n"
				             "File: %s. Line: %d\n",
				             r->err_file,
				             fd_err,
				             __FILE__,
				             __LINE__);
				return;
			} else {
				dup2(fd_err, STDERR_FILENO);
			}
		}
		execvp(r->argv[0], r->argv);

		return;
	}

	case PIPE: {
		// pipes two commands

		p = (struct pipecmd *) cmd;
		int pipe_fds[2];

		int ret = pipe2(pipe_fds, O_CLOEXEC);
		if (ret < 0) {
			eprint_debug(errno,
			             "Pipe failed."
			             "Command: %s\n"
			             "File: %s. Line: %d\n",
			             p->scmd,
			             __FILE__,
			             __LINE__);
			return;
		}

		int in_pipe = pipe_fds[1];
		int out_pipe = pipe_fds[0];

		/*
		 * Two childs will be created from the same parent.
		 * These will communicate with each other via pipes.
		 * The first will write and the second will read.
		 */
		int pid1 = fork();
		if (pid1 == 0) {
			// Child 1 won't read from Child 2.
			close(out_pipe);

			/*
			 * The stdout of the parent process should be
			 * bound to the 'in' pipe.
			 */
			dup2(in_pipe, STDOUT_FILENO);

			exec_cmd(p->leftcmd);
			/*
			 * If the execution of the command fails, then
			 * the flow will continue here.
			 */
			close(in_pipe);

		} else if (pid1 > 0) {
			// Parent process.

			int pid2 = fork();
			if (pid2 == 0) {
				// Child 2 won't write to Child 1.
				close(in_pipe);

				/*
				 * The stdin of the child 2 process should be
				 * bound to the 'out' pipe which is the stdout
				 * of the child 1 process.
				 */
				dup2(out_pipe, STDIN_FILENO);

				/*
				 * Repeat the process until the command
				 * is not a pipe type.
				 * If the redirection changes the STDIN
				 * the pipe has no effect.
				 */
				exec_cmd(p->rightcmd);
				close(out_pipe);

			} else if (pid2 > 0) {
				/*
				 * Parent created both child processes so
				 * there is no need for the fds of the pipe.
				 */
				close(in_pipe);
				close(out_pipe);
				/*
				 * This process will wait its second child.
				 */
				waitpid(pid2, &status, 0);
				waitpid(pid1, &status, 0);
			} else {
				eprint_debug(errno,
				             "Second Fork failed."
				             "Command: %s\n"
				             "File: %s. Line: %d\n",
				             p->scmd,
				             __FILE__,
				             __LINE__);
			}
		} else {
			eprint_debug(errno,
			             "First Fork failed."
			             "Command: %s\n"
			             "File: %s. Line: %d\n",
			             p->scmd,
			             __FILE__,
			             __LINE__);
		}
		return;
	}
	}
}
