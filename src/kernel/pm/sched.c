/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>

unsigned bit;
unsigned lfsr = 0xACE1u;
unsigned rand2(){
	bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
	return lfsr = (lfsr >> 1) | (bit << 15);
}

/**
 * @brief Schedules a process to execution.
 *
 * @param proc Process to be scheduled.
 */
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
}

/**
 * @brief Stops the current running process.
 */
PUBLIC void stop(void)
{
	curr_proc->state = PROC_STOPPED;
	sndsig(curr_proc->father, SIGCHLD);
	yield();
}

/**
 * @brief Resumes a process.
 *
 * @param proc Process to be resumed.
 *
 * @note The process must stopped to be resumed.
 */
PUBLIC void resume(struct process *proc)
{
	/* Resume only if process has stopped. */
	if (proc->state == PROC_STOPPED)
		sched(proc);
}

/**
 * @brief Yields the processor.
 */

int tirage = 256;

PUBLIC void yield(void)
{
	struct process *p;    /* Working process.     */
	struct process *next; /* Next process to run. */

	/* Re-schedule process for execution. */
	if (curr_proc->state == PROC_RUNNING)
		sched(curr_proc);

	/* Remember this process. */
	last_proc = curr_proc;
	if(last_proc != IDLE && last_proc->state != PROC_DEAD && last_proc->state != PROC_ZOMBIE) {
		last_proc->priority--;
	}

	/* Check alarm. */
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip invalid processes. */
		if (!IS_VALID(p))
			continue;

		/* Alarm has expired. */
		if ((p->alarm) && (p->alarm < ticks))
			p->alarm = 0, sndsig(p, SIGALRM);
	}

	/* Choose a process to run next. */
	next = IDLE;
	int min_prio = 40;
	for(p = FIRST_PROC; p <= LAST_PROC; p++) {
		if(p->state == PROC_READY && p->priority < min_prio) {
			min_prio = p->priority;
		}
	}

	// //tirage = (119*tirage+nb)%10000;
	// tirage = rand2()%10000;
	// int tire = 0;
	// if(nb != 0) {
	// 	tire = tirage%nb;
	// }
	// int acc = 0;

	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip non-ready process. */
		if (p->state != PROC_READY || p->priority != min_prio)
			continue;

		/*
		 * Process with higher
		 * waiting time found.
		 */

		if(next->counter < p->counter) {
			if(next != IDLE) {
				next->counter++;
			}
			next = p;
		}

		/*
		 * Increment waiting
		 * time of process.
		 */

	}

	/* Switch to next process. */
	if(next != IDLE) {
		next->counter = PROC_QUANTUM;
	} else {
		next->priority = 0;
	}
	next->state = PROC_RUNNING;
	if (curr_proc != next)
		switch_to(next);
}
