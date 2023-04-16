#include "exec.h"
#include "parsing.h"

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
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

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
	// Your code here
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
		//
		// Your code here
		printf("entro en exec");
		e = (struct execcmd *) cmd;
		execvp(e->argv[0], e->argv);
		printf_debug("%s: Command not found\n", e->scmd);
		free_command(cmd);
		_exit(-1);

	case BACK: {
		// runs a command in background
		//
		// Your code here
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		_exit(-1);
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd *) cmd;
		/*
		 * The command can have more than one type of redirection.
		 * So it is needed to check them all and point stdin, stdout and
		 * stderr to the proper file.
		 * Since the parser overwrite the buffer that contains the
		 * file path (when one redirection type is repeated),
		 * it is not possible to point stdin, stdout and
		 * stderr to more than one file each.
		 */
		int fd_out = -1;
		int fd_err = -1;
		int fd_in = -1;
		if (strlen(r->out_file) > 0) {
			/*
			 * Flag		| Desc
			 * O_CLOEXEC 	| Closes the file descriptor
			 * 		| asociated with the file
			 * 		| after execution with exve(2).
			 * 		|
			 * O_CREAT	| If the specified file doesn't exist
			 * 		| it will create it.
			 * 		|
			 * O_WRONLY	| The file can only be written.
			 *
			 * Mode: 0644   | Gives permission to access the
			 * 		| file system. Mode has effect if
			 * 		| O_CREAT or O_TMPFILE is present.
			 * 		| 0644 is a combination of:
			 * 		| S_IRUSR (00400) : user has read
			 * 		| permission.
			 * 	  	| S_IWUSR (00200) : user has write
			 * 		| permission.
			 * 		| S_IRGRP (00040) : group has read
			 * 		| permission.
			 * 		| S_IROTH (00004) : others have read
			 * 		| permission.
			 */
			fd_out = open_redir_fd(r->out_file,
			                       O_CLOEXEC | O_CREAT | O_WRONLY);
			dup2(fd_out, STDOUT_FILENO);
		}
		if (strlen(r->in_file) > 0) {
			fd_in = open_redir_fd(r->in_file, O_CLOEXEC | O_RDONLY);
			dup2(fd_in, STDIN_FILENO);
		}
		if (strlen(r->err_file) > 0) {
			/*
			 * If the redirection is 2>&1 then the stderr has to
			 * write in the same file as the stdout. To do that
			 * the fd_err points to stdout.
			 * If the redirection is not 2>&1  then
			 * fd_err will point to the specified file.
			 * Then stderr will point to fd_err.
			 */
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, fd_err);
			} else {
				fd_err = open_redir_fd(r->err_file,
				                       O_CLOEXEC | O_CREAT |
				                               O_WRONLY);
			}
			/*
			 * At the end it is needed to point the stderr to the
			 * file that fd_err points to.
			 */
			dup2(fd_err, STDERR_FILENO);
		}

		int failed = execvp(r->argv[0], r->argv);
		/*
		 * If execution continues here it means 'execvp' failed and
		 * returns -1. In that case, error message is printed and
		 * both file descriptors and the command have to be freed.
		 */
		if (failed == -1) {
			eprint_debug(errno,
			             "Command execution failed: %s\n"
			             "Line: %d. File: %s",
			             r->scmd,
			             __LINE__,
			             __FILE__);
			if (fd_out > 0) {
				close(fd_out);
			}
			if (fd_in > 0) {
				close(fd_in);
			}
			if (fd_err > 0) {
				close(fd_err);
			}
			free_command((struct cmd *) r);
			exit(1);
		}
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here
		/*
		 * The split_line functions with '|' parameter splits
		 * the command in two. The right part can have more '|'.
		 * That means that the right part has to be parsed again
		 * to get the next pipe.
		 */
		p = (struct pipecmd *) cmd;
		int pipe_fds[2];
		/*
		 * the flag O_CLOEXEC is useful to close the fd in excevp.
		 * However, if the function fails there is no way to close it,
		 * since the recursive call delegates that responsibility
		 * to the proper case.
		 */
		int ret = pipe2(pipe_fds, O_CLOEXEC);
		if (ret < 0) {
			eprint_debug(errno,
			             "Pipe failed."
			             "Command: %s\n"
			             "File: %s. Line: %d\n",
			             p->scmd,
			             __FILE__,
			             __LINE__);
		}
		/*
		 * The 'in' pipe is the fd to write.
		 * The 'out' pipe is the fd to read.
		 */
		int in_pipe = pipe_fds[1];
		int out_pipe = pipe_fds[0];
		int pid = fork();

		if (pid == 0) {
			// Child won't write to its parent.
			close(in_pipe);

			/*
			 * The stdin of the child process should be
			 * bound to the 'out' pipe.
			 */
			dup2(out_pipe, STDIN_FILENO);

			/*
			 * Repeat the process until the command
			 * is not a pipe type. If the excevp fails
			 * the fd out_pipe is leaked and not closed.
			 * The reason is EXEC and REDIR close
			 * their own fds and quit on error but know
			 * nothing about being a part of a pipe cmd.
			 * Also, if the redirection changes the STDIN
			 * the pipe has no effect.
			 */
			exec_cmd(parse_line(p->rightcmd->scmd));

			// For 2 process pipe:
			// exec_cmd(p->rightcmd);

		} else if (pid > 0) {
			// Parent won't read to its child.
			close(out_pipe);

			/*
			 * The stdout of the parent process should be
			 * bound to the 'in' pipe.
			 */
			dup2(in_pipe, STDOUT_FILENO);

			/*
			 * The left command is either EXEC or REDIR type.
			 * Why check and repeat code when exec_cmd
			 * can be used?
			 * The downside of this is that the remaining fd
			 * is not closed if execvp fails.
			 */
			exec_cmd(p->leftcmd);
		} else {
			eprint_debug(errno,
			             "Fork failed."
			             "Command: %s\n"
			             "File: %s. Line: %d\n",
			             p->scmd,
			             __FILE__,
			             __LINE__);
			free_command(parsed_pipe);
			exit(1);
		}
		break;
	}
	}
}
