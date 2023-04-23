#include "builtin.h"
#include "runcmd.h"

char buf[BUFLEN] = { 0 };

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0) {
		status = 1;
		return true;
	}

	return false;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if ((cmd[0] == 'c') & (cmd[1] == 'd')) {
		if (cmd[2] == '\0') {
			char *home = getenv("HOME");
			if (chdir(home) < 0) {
				snprintf(buf, sizeof buf, "cannot cd to %s ", home);
				perror(buf);
				status = 1;
			} else {
				snprintf(prompt, sizeof prompt, "(%s)", home);
				return true;
			}
		} else if (cmd[2] == SPACE) {
			int cmd_len = strlen(cmd);
			char dir[cmd_len];
			strcpy(dir, cmd + 3);
			if (chdir(dir) < 0) {
				snprintf(buf, sizeof buf, "cannot cd to %s ", dir);
				perror(buf);
				status = 1;
			} else {
				snprintf(prompt,
				         sizeof prompt,
				         "(%s)",
				         get_current_dir_name());
				return true;
			}
		}
	}
	return false;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		char *curr_dir = get_current_dir_name();

		if (curr_dir == NULL) {
			snprintf(buf, sizeof buf, "cannot get current directory ");
			perror(buf);
			status = 1;
		} else {
			fprintf(stdout, "%s\n", curr_dir);
			return true;
		}
	}

	return false;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	size_t idx;
	char *command = get_token(cmd, 0);
	if (strcmp(command, "history") != 0) {
		free(command);
		return false;
	}


	idx = strlen(command);
	if (cmd[idx] == END_STRING) {
		// print all history commands
		int i = 0;
		while (history_arr[i] != NULL) {
			printf("%d: %s", i, history_arr[i]);
			i++;
		}
		free(command);
		return true;
	}

	idx++;

	char *arg = get_token(cmd, idx);

	for (size_t i = 0; i < strlen(arg); i++) {
		if (!isdigit(arg[i])) {
			free(command);
			return false;
		}
	}
	idx = idx + strlen(arg);
	if (cmd[idx] == END_STRING) {
		// print latest n commands
		int n = atoi(arg);
		int command_lines = 0;
		while (history_arr[command_lines++]) {
		}
		command_lines -= 1;

		if (n > command_lines) {
			n = command_lines;
		}
		for (int i = (command_lines - n); i < command_lines; i++) {
			printf("%d: %s", i, history_arr[i]);
		}
		free(command);
		return true;
	}
}
