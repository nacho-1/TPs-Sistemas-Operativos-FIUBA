#include "history.h"


char ** history_arr;
int alloc_size;
int pos = 0;

// load command history in a string array
void
load_history()
{
	FILE *fp;

	history_arr = (char **) malloc((INITIAL_HISTORY_LINES + 1) * sizeof(char *));
	alloc_size = INITIAL_HISTORY_LINES + 1;
	char line[MAX_HISTORY_LINE_SIZE + 1];

	char *histfile = getenv("HISTFILE");
	if (histfile == NULL) {
		char default_histfile[FNAMESIZE] = { '\0' };
		char *home = getenv("HOME");
		strcat(default_histfile, home);
		strcat(default_histfile, "/.sisop_history");
		fp = fopen(default_histfile, "a+");
	} else {
		fp = fopen(histfile, "a+");
	}
	if (fp == NULL) {
		eprint_debug(errno,"Error opening history file\n");
		return;
	}

	// read file line by line with fgets()
	while (fgets(line, sizeof(line) * sizeof(char), fp) != NULL) {
		if (pos >= alloc_size) {
			history_arr = (char **) realloc(
			        history_arr,
			        alloc_size * HISTORY_GROWING_FACTOR *
			                sizeof(char *));
			alloc_size = alloc_size * HISTORY_GROWING_FACTOR;
		}

		history_arr[pos] = (char *) malloc(sizeof(line) * sizeof(char));
		strcpy(history_arr[pos], line);
		pos++;
	}

	history_arr[pos] = NULL;

	fclose(fp);

    return;
}



void _save_command_in_memory(char * cmd){

	if (pos >= alloc_size) {
			history_arr = (char **) realloc(
			        history_arr,
			        alloc_size * HISTORY_GROWING_FACTOR *
			                sizeof(char *));
			alloc_size = alloc_size * HISTORY_GROWING_FACTOR;
		}



	history_arr[pos] = (char *) malloc((strlen(cmd) + 2) * sizeof(char));

	strcpy(history_arr[pos], cmd);

	pos++;	
	history_arr[pos] = NULL;

	return;

}



void _save_command_in_file(char * cmd){

	FILE *fp;

	char *histfile = getenv("HISTFILE");
	if (histfile == NULL) {
		char default_histfile[FNAMESIZE] = { '\0' };
		char *home = getenv("HOME");
		strcat(default_histfile, home);
		strcat(default_histfile, "/.sisop_history");
		fp = fopen(default_histfile, "a");
	
	} else {
		fp = fopen(histfile, "a");
	}
	if (fp == NULL) {
		eprint_debug(errno,"Error opening history file\n");
		return;
	}


	fwrite(cmd, sizeof(char), strlen(cmd), fp);
	fwrite("\n", sizeof(char), 1, fp);
	fclose(fp);
	if (ferror(fp) != 0) {
		eprint_debug(errno,"Failure writing the history file. The "
						"command won't be saved.\n");
	}
	
	return;
}


void save_command(char * cmd){

	_save_command_in_memory(cmd);

	_save_command_in_file(cmd);

	return;
}


void
free_history()
{
	int i=0;
	while(history_arr[i] != NULL){
		free(history_arr[i++]);
	}
	free(history_arr);

}