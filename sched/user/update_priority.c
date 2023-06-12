// update_priority
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int priority;
	cprintf("i am environment %08x running update_priority program\n",
	        thisenv->env_id);

	priority = sys_getpriority();
	cprintf("Actual priority is: %d \n", priority);

	int relative_percent = 50;
	sys_reduce_priority(relative_percent);
	cprintf("Priority reduce in a: %d %% \n", relative_percent);

	priority = sys_getpriority();
	cprintf("Updated priority now is: %d \n", priority);

	relative_percent = 20;
	sys_reduce_priority(relative_percent);
	cprintf("Priority reduce in a: %d %% \n", relative_percent);

	priority = sys_getpriority();
	cprintf("Updated priority now is: %d \n", priority);
}
