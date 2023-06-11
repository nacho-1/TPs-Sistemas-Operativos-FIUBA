// update_priority
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int priority;
	cprintf("i am environment %08x running update_priority program\n", thisenv->env_id);

	priority = sys_getpriority();
	cprintf("Actual priority is: %d \n", priority);

	priority = (priority / 2) + 1;
	sys_setpriority(priority);
	cprintf("New priority set: %d \n", priority);

	priority = sys_getpriority();
	cprintf("Updated priority now is: %d \n", priority);

}
