#include "defs.h"
#include "readline.h"
#include "utils.h"
#include <sys/ioctl.h>

static char buffer[BUFLEN];

void
writeChar(char c, size_t *row, size_t *col, int MAX_COL)
{
	putchar(c);
	if (++(*col) >= (size_t) MAX_COL) {
		printf_debug("\n> ");  // artificial breakline in terminal
		*col = 0;              // to keep strict track of char placement
		(*row)++;
	}
}

// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *prompt)
{
	int i = 0, c = 0;

	// Track last char col & row for showing written commands
	size_t col = 0, row = 0;
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int MAX_COL = w.ws_col - 3;
	if (!MAX_COL || MAX_COL < 1)
		MAX_COL = 50;

#ifndef SHELL_NO_INTERACTIVE
	fprintf(stdout, "%s %s %s\n", COLOR_RED, prompt, COLOR_RESET);
	fprintf(stdout, "%s", "$ ");
#endif

	memset(buffer, 0, BUFLEN);

	while (true) {
		read(STDIN_FILENO, &c, 1);
		switch (c) {
		case EOF:
		case 4:  // EOF | CTRL+D
			return NULL;
		case END_LINE:
			buffer[i] = END_STRING;
			putchar(c);
			return buffer;
		default:
			buffer[i++] = c;
			writeChar(c, &row, &col, MAX_COL);
			break;
		}
		fflush(stdout);
	}
}
