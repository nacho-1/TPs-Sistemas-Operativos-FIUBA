#ifndef READLINE_H
#define READLINE_H

#include "defs.h"
#include "utils.h"
#include <sys/ioctl.h>
#include "history.h"

void delete (int *i, size_t *row, size_t *col, int MAX_COL);
void writeChar(char c, size_t *row, size_t *col, int MAX_COL);
void writeLine(char *str, size_t *row, size_t *col, int MAX_COL);
void clearLine(size_t *row, size_t *col);
void navigateHistory(int *i, size_t *row, size_t *col, int MAX_COL);
char *read_line(const char *prompt);

#endif  // READLINE_H
