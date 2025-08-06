//
// Created by sofia on 11/06/23.
//
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("I am simple, I don't use yield\n");

	int priority;
	priority = sys_getpriority();
	cprintf("SIMPLE PROGRAM: i am environment %08x\n", thisenv->env_id);
	cprintf("Actual priority is: %d \n", priority);
	cprintf("Bye!\n");
	cprintf("\n");
}
