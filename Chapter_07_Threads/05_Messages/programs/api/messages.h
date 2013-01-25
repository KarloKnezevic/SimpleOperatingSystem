/*! Messages and signals (threads) */

#pragma once

#include <lib/types.h>

//maknut handler
int thread_msg_set ( uint min_msg_prio, int min_sig_prio );

//----------------sučelje za korištenje obrade signala s registriranom funkcijom
int set_signal_handler( int sig_prio, void *sig_handler );

int create_message_queue ( msg_q *queue, uint min_prio );
int delete_message_queue ( msg_q *queue );
int send_message ( int dest_type, void *dest, msg_t *msg, uint flags );
int receive_message ( int src_type, void *src, msg_t *msg, int type,
		      size_t size, uint flags );