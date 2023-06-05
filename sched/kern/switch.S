/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:
	popl %esp; // point esp to the beginning of the trapframe
	popal;
	popl %es;
	// add $2, %esp // skip padding1
	popl %ds;
	// add $2, %esp // skip padding2
	add $4, %esp; // skip trapno
	add $4, %esp; // skip err
	iret;
spin:
	jmp spin
