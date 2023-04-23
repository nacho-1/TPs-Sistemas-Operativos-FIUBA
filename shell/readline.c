#include "defs.h"
#include "readline.h"
#include "utils.h"
#include <sys/ioctl.h>
#include "history.h"

static char buffer[BUFLEN];

void delete(int *i, size_t *row, size_t *col, int MAX_COL) {
	if (*i <= 0) return;

	buffer[--(*i)] = 0;

	if ((*col)-- == 0) {
		(*row)--;
		(*col) = MAX_COL - 1;
		printf_debug("\b \b\b \b");
		printf_debug("\b \b\b \b\033[A\033[%iC", MAX_COL + 2);
	}
	printf_debug("\b \b");
}

void
writeChar(char c, size_t *row, size_t *col, int MAX_COL)
{
	putchar(c);
	if (++(*col) >= (size_t) MAX_COL) {
		printf_debug("\n> ");
		*col = 0;
		(*row)++;
	}
}

void
writeLine(char *str, size_t *row, size_t *col, int MAX_COL)
{
	for (size_t i = 0; i < strlen(str); i++)
		writeChar(str[i], row, col, MAX_COL);
}

void
clearLine(size_t *row, size_t *col)
{
	*col = 0;
	while (*row > 0) {
		(*row)--;
		printf_debug("\33[2K\r\033[A");
	};
	printf_debug("\33[2K\r$ ");
}

void navigateHistory(int *i, size_t *row, size_t *col, int MAX_COL) {
	int c = 0;
	read(STDIN_FILENO, &c, 1);
	switch (c) {
	case '[':  // character before arrows
		read(STDIN_FILENO, &c, 1);
		switch (c) {
		case 'A':  // up
			strcpy(buffer, get_previous_command());
			break;
		case 'B':  // down
			strcpy(buffer, get_next_command());
			break;
		default:
			return;
		}
		clearLine(row, col);
		writeLine(buffer, row, col, MAX_COL);
		*i = strlen(buffer);
		break;
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
		case 27:
			navigateHistory(&i, &row, &col, MAX_COL);
		case 127:
			delete(&i, &row, &col, MAX_COL);
			break;
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
