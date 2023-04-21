#include "runcmd.h"

int status = 0;
struct cmd *parsed_pipe;
int back_process_count = 0;

void
wait_back_processes()
{
	// Try to wait background processes
	int updated_count = 0;
	for (int i = 0; i < back_process_count; i++) {
		pid_t ret = waitpid(-1, &status, WNOHANG);
		if (ret == 0)
			updated_count++;
		else if (ret == -1)
			perror_debug("Error waiting background processes");
	}
	back_process_count = updated_count;
}

// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	// TODO - se tiene que guardar el comando en memoria
	//        y la escritura en archivo se puede hacer al final de la sesion
	// TODO - si es spcae / end_of_line no guardar en historial


	wait_back_processes();

	pid_t p;
	struct cmd *parsed;

	// if the "enter" key is pressed
	// just print the prompt again
	if (cmd[0] == END_STRING)
		return 0;

	save_command(cmd);

	// "history" built-in call
	if (history(cmd))
		return 0;

	// "cd" built-in call
	if (cd(cmd))
		return 0;

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" built-in call
	if (pwd(cmd))
		return 0;

	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {
		exec_cmd(parsed);
		free_command(parsed);
		fflush(stdout);
		_exit(EXIT_FAILURE);
	}

	// stores the pid of the process
	parsed->pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is
	//		going to be run in the 'back'
	// - print info about it with
	// 	'print_back_info()'
	//
	// Your code here
	if (parsed->type == BACK) {
		print_back_info(parsed);
		back_process_count++;
	} else {
		// waits for the process to finish
		waitpid(p, &status, 0);
		print_status_info(parsed);
	}

	free_command(parsed);

	return 0;
}
