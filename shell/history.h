#ifndef HISTORY_H
#define HISTORY_H

#include "defs.h"
#include "utils.h"

void load_history();
void save_command(char * cmd);

void _save_command_in_memory(char * cmd);
void _save_command_in_file(char * cmd);

#endif  // HISTORY_H
