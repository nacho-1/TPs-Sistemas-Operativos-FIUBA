#include "history.h"


char ** fcontent;

// load command history in a string array
void
load_history()
{
	FILE *fp;

	fcontent = (char **) malloc((INITIAL_HISTORY_LINES + 1) * sizeof(char *));
	int alloc_size = INITIAL_HISTORY_LINES + 1;
	char line[MAX_HISTORY_LINE_SIZE + 1];
	int pos = 0;

	char *histfile = getenv("HISTFILE");
	if (histfile == NULL) {
		char default_histfile[FNAMESIZE] = { '\0' };
		char *home = getenv("HOME");
		strcat(default_histfile, home);
		strcat(default_histfile, "/.sisop_history");
		printf("DEFAULT HISTFILE: %s\n", default_histfile);
		fp = fopen(default_histfile, "r");
	} else {
		printf("ENV HISTFILE: %s\n", histfile);
		fp = fopen(histfile, "r");
	}
	if (fp == NULL) {
		eprint_debug(errno, "Error opening history file\n");
		exit(-1);
	}

	// read file line by line with fgets()
	while (fgets(line, sizeof(line) * sizeof(char), fp) != NULL) {
		if (pos >= alloc_size) {
			fcontent = (char **) realloc(
			        fcontent,
			        alloc_size * HISTORY_GROWING_FACTOR *
			                sizeof(char *));
			alloc_size = alloc_size * HISTORY_GROWING_FACTOR;
		}

		fcontent[pos] = (char *) malloc(sizeof(line) * sizeof(char));
		strcpy(fcontent[pos], line);
		pos++;
	}

	fcontent[pos] = NULL;

	fclose(fp);

    return;
}