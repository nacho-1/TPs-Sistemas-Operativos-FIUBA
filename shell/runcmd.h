#ifndef RUNCMD_H
#define RUNCMD_H

#include "defs.h"
#include "parsing.h"
#include "exec.h"
#include "printstatus.h"
#include "freecmd.h"
#include "builtin.h"
#include "history.h"

int run_cmd(char *cmd);
void wait_back_processes(void);

#endif  // RUNCMD_H
