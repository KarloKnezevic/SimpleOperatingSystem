/*! Threads */

#pragma once

#include <lib/types.h>

int create_thread ( void *start_func, void *param, int sched, int prio,
		    thread_t *handle );
void thread_exit ( int status );// __attribute__(( noinline ));
int wait_for_thread ( void *thread, int wait );
int cancel_thread ( void *thread );
int thread_self ( thread_t *thr );

int start_program ( char *prog_name, thread_t *handle, void *param,
		    int sched, int prio );

int set_sched_params ( thread_t *thread, int sched_policy, int prio,
		       sched_t *params );
int get_sched_params ( thread_t *thread, int *sched_policy, int *prio,
		       sched_t *params );
