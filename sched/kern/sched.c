#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

extern int total_tickets;
int prev_random = 1234;  // seed

void sched_halt(void);
void get_next_runnable_process(int first, int last);
int generate_pseudorandom_value();
void get_stats();

// stats
int sched_calls = 0;

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;
	sched_calls++;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	// Without scheduler, keep running the last environment while it exists
	/*
	        if (curenv) {
	                env_run(curenv);
	        }
	*/
	int pos = 0;
	idle = curenv;

	if (idle != NULL) {
		pos = ENVX(idle->env_id) + 1;
	}

	// recorrer el arreglo de manera circular
	get_next_runnable_process(pos, NENV);
	get_next_runnable_process(0, pos);

	// si no hay otro proceso runnable sigo corriendo el actual
	if (idle && idle->env_status == ENV_RUNNING) {
		env_run(idle);
	}
#elif SCHED_PROPORTIONAL_SHARE
	// counter: used to track if we’ve found the winner yet
	int counter = 0;
	idle = curenv;
	// winner: use some call to a random number generator to
	// get a value, between 0 and the total # of tickets
	int winner = generate_pseudorandom_value();

	for (int i = 0; i < NENV; i++) {
		if (envs != NULL && envs[i].env_status == ENV_RUNNABLE) {
			idle = &envs[i];
			counter += envs[i].tickets;
			if (counter >= winner) {
				break;  // found the winner
			}
		}
	}
	env_run(idle);

#endif

	// sched_halt never returns
	sched_halt();
}

void
get_next_runnable_process(int first, int last)
{
	int pos = first;
	for (pos; pos < last; pos++) {
		if (envs[pos].env_status == ENV_RUNNABLE) {
			env_run(&envs[pos]);
		}
	}
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		get_stats();
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

int
generate_pseudorandom_value()
{
	unsigned long long int a = 4294967296;
	int c = 1013904223;
	int m = 1664525;

	unsigned int random = (a * prev_random + c) % m;
	int rand_tickets = random * total_tickets / m;
	int percent = random * 100 / m;
	prev_random = random;

	return rand_tickets;
}

void
get_stats()
{
	cprintf("----- Scheduler stats -----\n");

	cprintf("Historial de procesos ejecutados\n");
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs == 0)
			continue;
		cprintf("- %d \n", envs[i].env_id);
	}
	cprintf("\n");
	cprintf("Cantidad de ejecuciones por proceso\n");
	for (int j = 0; j < NENV; j++) {
		if (envs[j].env_runs == 0)
			continue;
		cprintf("- %d: %d veces\n", envs[j].env_id, envs[j].env_runs);
	}
	cprintf("\n");
	cprintf("Cantidad de llamadas al scheduler: %d\n", sched_calls);
}

void
reduce_current_env_prio(void)
{
	if (curenv->tickets > 10) {
		total_tickets -= 10;
		curenv->tickets -= 10;
	} else {
		total_tickets -= curenv->tickets - 1;
		curenv->tickets = 1;
	}
}

void
sched_boost(void)
{
	total_tickets = 0;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE ||
		    envs[i].env_status == ENV_RUNNING) {  // otros estados?
			envs[i].tickets = 100;
			total_tickets += 100;
		}
	}
}
