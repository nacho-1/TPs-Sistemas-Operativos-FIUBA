#include "defs.h"
#include "readline.h"
#include "utils.h"

static char buffer[BUFLEN];

// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *prompt)
{
	int i = 0, c = 0;

#ifndef SHELL_NO_INTERACTIVE
	fprintf(stdout, "%s %s %s\n", COLOR_RED, prompt, COLOR_RESET);
	fprintf(stdout, "%s", "$ ");
#endif

	memset(buffer, 0, BUFLEN);

	while (true) {
		read(STDIN_FILENO, &c, 1);
		switch (c) {
		case EOF:
		case 4:    // EOF | CTRL+D
			return NULL;
		case END_LINE:
			buffer[i] = END_STRING;
			putchar(c);
			return buffer;
		default:
			buffer[i++] = c;
			putchar(c);
			printf_debug("\n");
			break;
		}
	}
}
