#ifndef PARSING_H
#define PARSING_H

#include "defs.h"
#include "types.h"
#include "createcmd.h"
#include "utils.h"

struct cmd *parse_line(char *b);

char *get_token(char *buf, int idx);
void expand_environ_var(char *arg);

#endif  // PARSING_H
