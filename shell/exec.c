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
	case BACK:
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		return;
	case REDIR:
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
			}
			dup2(fd_out, STDOUT_FILENO);
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
			}
			dup2(fd_in, STDIN_FILENO);
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
			}
			dup2(fd_err, STDERR_FILENO);
		}

		execvp(r->argv[0], r->argv);
		eprint_debug(errno,
		             "Command execution failed: "
		             "%s\n File: %s. Line: %d",
		             r->scmd,
		             __FILE__,
		             __LINE__);
		return;
	case PIPE:
		// pipes two commands
		p = (struct pipecmd *) cmd;

		int pipe_fds[2];
		int ret = pipe(
		        pipe_fds);  // O_CLOEXEC works weirdly with dup2 so I just close them manually
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
		int read_pipe = pipe_fds[0];
		int write_pipe = pipe_fds[1];

		int left_pid, right_pid;

		left_pid = fork();
		if (left_pid == -1) {
			eprint_debug(errno,
			             "First Fork failed."
			             "Command: %s\n"
			             "File: %s. Line: %d\n",
			             p->scmd,
			             __FILE__,
			             __LINE__);
		} else if (left_pid == 0) {
			close(read_pipe);
			dup2(write_pipe, STDOUT_FILENO);
			close(write_pipe);
			exec_cmd(p->leftcmd);

			free_command(cmd);
			fflush(stdout);
			_exit(EXIT_FAILURE);
		}

		right_pid = fork();
		if (right_pid == -1) {
			eprint_debug(errno,
			             "Second Fork failed."
			             "Command: %s\n"
			             "File: %s. Line: %d\n",
			             p->scmd,
			             __FILE__,
			             __LINE__);
		} else if (right_pid == 0) {
			close(write_pipe);
			dup2(read_pipe, STDIN_FILENO);
			close(read_pipe);
			exec_cmd(p->rightcmd);

			free_command(cmd);
			fflush(stdout);
			_exit(EXIT_FAILURE);
		}

		close(read_pipe);
		close(write_pipe);
		free_command(cmd);

		int left_status, right_status;
		if (left_pid != -1)
			waitpid(left_pid, &left_status, 0);
		if (right_pid != -1)
			waitpid(right_pid, &right_status, 0);

		if (left_status != EXIT_SUCCESS || right_status != EXIT_SUCCESS)
			exit(EXIT_FAILURE);

		exit(EXIT_SUCCESS);
	}
}

/* Apendice sobre redirs y pipes
 * dup2 puede fallar y no estamos haciendo ningun chequeo aca.
 * Mismo con close
 * El programa en general no hace muchos chequeos asiq por ahora lo dejamos asi
 * pero que quede notado.
 */
