// myfirstprogram
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("THIS IS MY PROGRAM: myfirstprogram\n");
	cprintf("i am environment %08x\n", thisenv->env_id);
}
