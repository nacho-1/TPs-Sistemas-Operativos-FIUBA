#include "builtin.h"


char buf[BUFLEN] = { 0 };

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0) {
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
			} else {
				snprintf(prompt, sizeof prompt, "(%s)", home);
				return true;
			}
		}else if(cmd[2] == SPACE){
			int cmd_len = strlen(cmd);
			char dir[cmd_len];
			strcpy(dir, cmd + 3);
			if (chdir(dir) < 0) {
				snprintf(buf, sizeof buf, "cannot cd to %s ", dir);
				perror(buf);
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
	// TODO - add optional argument number of commands to show
	if (strcmp(cmd, "history") == 0){
		FILE * fp;
   		char * line = NULL;
   		size_t len = 0;
		char *histfile = getenv("HISTFILE");		

		if(histfile == NULL){
			char default_histfile[100] = {'\0'}	;
			char *home = getenv("HOME");
			strcat(default_histfile, home);
			strcat(default_histfile, "/.sisop_history");
			printf("DEFAULT HISTFILE: %s\n", default_histfile);
			fp = fopen(default_histfile, "r");
		}    
		else{
			printf("ENV HISTFILE: %s\n", histfile);
			fp = fopen(histfile, "r");
		}			

		if (fp == NULL)
			exit(EXIT_FAILURE);

		while (getline(&line, &len, fp) != -1) {
			printf("%s", line);
		}

		fclose(fp);
		if (line)
			free(line);

		return true;
	}

	return false;
}
