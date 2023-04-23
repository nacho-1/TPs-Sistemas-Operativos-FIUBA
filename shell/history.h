#ifndef HISTORY_H
#define HISTORY_H

#include "defs.h"
#include "utils.h"

void load_history();
void free_history();
void save_command(char * cmd);
char * get_previous_command();
char * get_next_command();

void _save_command_in_memory(char * cmd);
void _save_command_in_file(char * cmd);

#endif  // HISTORY_H
