//
// Created by sofia on 11/06/23.
//
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("THIS IS MY PROGRAM: program1\n");

	int priority;
	priority = sys_getpriority();
	cprintf("PROGRAM 1: i am environment %08x\n", thisenv->env_id);
	cprintf("Actual priority is: %d \n", priority);
	cprintf("\n");

	sys_yield();

	priority = sys_getpriority();
	cprintf("PROGRAM 1: i am environment %08x\n", thisenv->env_id);
	cprintf("Actual priority is: %d \n", priority);
	cprintf("\n");

	sys_yield();

	priority = sys_getpriority();
	cprintf("PROGRAM 1: i am environment %08x\n", thisenv->env_id);
	cprintf("Actual priority is: %d \n", priority);
	cprintf("\n");
}
