/*! Messages */
#ifdef MESSAGES

#define _KERNEL_

#define _K_MESSAGES_C_
#include "messages.h"

#include <kernel/thread.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/list.h>
#include <lib/string.h>
#include <api/prog_info.h>
#include <api/stdio.h>			

/* list of all global message queues */
static list_t kmsg_qs = LIST_T_NULL;

extern prog_info_t pi;

/*! Initialize messaging part of new thread descriptor */
/*
Sve funkcije su NULL
*/
void k_thr_msg_init ( kthrmsg_qs *thrmsg )
{
	list_init ( &thrmsg->msgq.msgs );
	k_threadq_init ( &thrmsg->msgq.thrq );
	thrmsg->msgq.min_prio = 0;

	thrmsg->sig_prio = 0;
	//dodano
	int index;
	for (index = 0; index < HANDLER_NUM; index++) {
	    thrmsg->signal_handler[index] = NULL;
	}
	//kraj dodavanja
}

/*! Define thread behavior towards messages and signals */
int sys__thread_msg_set (uint min_msg_prio, int min_sig_prio)
{
	/* local variables */
	kthread_t *kthr;
	kthrmsg_qs *thrmsg;

	SYS_ENTRY();

	kthr = k_get_active_thread ();
	thrmsg = k_get_thrmsg ( kthr );

	thrmsg->msgq.min_prio = min_msg_prio;
	thrmsg->sig_prio = min_sig_prio;
	//dodano
	int index;
	for (index = 0; index < HANDLER_NUM; index++) {
	    thrmsg->signal_handler[index] = NULL;
	}
	//kraj dodavanja


	SYS_EXIT ( SUCCESS );
}

int sys__set_signal_handler (int sig_prio, void *sig_handler)
{
	kthread_t *kthr;
	kthrmsg_qs *thrmsg;
	
	SYS_ENTRY();
	
	kthr = k_get_active_thread();
	thrmsg = k_get_thrmsg ( kthr );
	
	thrmsg->signal_handler[sig_prio] = sig_handler;
	
	SYS_EXIT( SUCCESS );
}

/*! Create global message queue */
int sys__create_msg_queue ( msg_q *msgq, uint min_prio )
{
	kgmsg_q *gmsgq;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_SYS_EXIT ( msgq, E_INVALID_HANDLE );

	gmsgq = kmalloc ( sizeof ( kgmsg_q ) );
	ASSERT_ERRNO_AND_SYS_EXIT ( gmsgq, E_NO_MEMORY );

	list_init ( &gmsgq->mq.msgs ); /* list for messages */
	k_threadq_init ( &gmsgq->mq.thrq ); /* list for blocked threads */

	gmsgq->mq.min_prio = min_prio;
	msgq->id = gmsgq->id = k_new_unique_id ();
	msgq->handle = gmsgq;

	list_append ( &kmsg_qs, gmsgq, &gmsgq->all ); /* all msg.q. list */

	SYS_EXIT ( SUCCESS );
}

/*! Delete global message queue */
int sys__delete_msg_queue ( msg_q *msgq )
{
	kgmsg_q *gmsgq;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_SYS_EXIT ( msgq, E_INVALID_HANDLE );

	gmsgq = msgq->handle;
	ASSERT_ERRNO_AND_SYS_EXIT ( gmsgq->id == msgq->id, E_INVALID_HANDLE );

	k_msgq_clean ( &gmsgq->mq );

	k_release_all_threads ( &gmsgq->mq.thrq );

	k_free_unique_id ( gmsgq->id );

	kfree ( gmsgq );

	msgq->id = 0;
	msgq->handle = NULL;

	SYS_EXIT ( SUCCESS );
}

/*! Send message to queue or signal to thread */
int sys__msg_post ( int dest_type, void *dest, msg_t *msg, uint flags )
{
	thread_t *thr;
	kthread_t *kthr, *new_kthr;
	kthrmsg_qs *thrmsg;
	kgmsg_q *kgmsgq;
	kmsg_q *kmsgq;
	msg_q *msgq;
	kmsg_t *kmsg;
	msg_t *cmsg;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_SYS_EXIT ( dest && msg, E_INVALID_HANDLE );

	if ( dest_type == MSG_THREAD || dest_type == MSG_SIGNAL )
	{
		thr = dest;
		kthr = k_get_kthread ( thr );
		ASSERT_ERRNO_AND_SYS_EXIT ( kthr, E_DONT_EXIST );
		thrmsg = k_get_thrmsg ( kthr );
		kmsgq = &thrmsg->msgq;
	}
	else if ( dest_type == MSG_QUEUE )
	{
		msgq = dest;
		kgmsgq = msgq->handle;
		ASSERT_ERRNO_AND_SYS_EXIT ( kgmsgq && kgmsgq->id == msgq->id,
					E_INVALID_HANDLE );
		kmsgq = &kgmsgq->mq;
	}
	else {
		SYS_EXIT ( E_INVALID_TYPE );
	}

	if ( dest_type == MSG_THREAD || dest_type == MSG_QUEUE )
	{
		/* send message to queue */
		if ( kmsgq->min_prio <= msg->type ) /* msg has required prio. */
		{
			kmsg = kmalloc ( sizeof (kmsg_t) + msg->size );
			ASSERT_ERRNO_AND_SYS_EXIT ( kmsg, E_NO_MEMORY );

			kmsg->msg.type = msg->type;
			kmsg->msg.size = msg->size;
			memcpy ( kmsg->msg.data, msg->data, msg->size );

			list_append ( &kmsgq->msgs, kmsg, &kmsg->list );

			/* is thread waiting for message? */
			if ( k_release_thread ( &kmsgq->thrq ) )
				k_schedule_threads ();

			SYS_EXIT ( SUCCESS );
		}
		else { /* ignore message */
			SYS_EXIT ( E_IGNORED );
		}
	}

	/* must be MSG_SIGNAL */
	//promijenjen uvijet
	//ako je signal_handler postavljen, tada šalji signal
	if ( thrmsg->sig_prio <= msg->type && thrmsg->signal_handler[msg->type] != NULL )
	{
		/* create thread that will service this signal */

		cmsg = k_create_thread_private_storage ( kthr,
				sizeof (msg_t) + msg->size );
		cmsg->type = msg->type;
		cmsg->size = msg->size;
		memcpy ( cmsg->data, msg->data, msg->size );

		new_kthr = k_create_thread (
			//koji handler???
			thrmsg->signal_handler[msg->type], cmsg, pi.exit,
			k_get_thread_prio ( kthr ) + 1, NULL, 0, 1
		);
		ASSERT_ERRNO_AND_SYS_EXIT ( new_kthr, k_get_errno() );

		k_set_thread_private_storage ( new_kthr, cmsg );

		//SET_ERRNO ( SUCCESS );

		k_schedule_threads ();

		SYS_EXIT ( SUCCESS );
	}
	else { /* ignore signal */
		SYS_EXIT ( E_IGNORED );
	}
}

/*! Receive message from queue (global or from own thread message queue) */
int sys__msg_recv ( int src_type, void *src, msg_t *msg, int type, size_t size,
		    uint flags )
{
	kthread_t *kthr;
	kthrmsg_qs *thrmsg;
	kgmsg_q *kgmsgq;
	kmsg_q *kmsgq;
	msg_q *msgq;
	kmsg_t *kmsg;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_SYS_EXIT (
		src_type == MSG_THREAD || ( src_type == MSG_QUEUE && src ),
		E_INVALID_TYPE );
	ASSERT_ERRNO_AND_SYS_EXIT ( msg && size > 0, E_INVALID_HANDLE );

	if ( src_type == MSG_THREAD )
	{
		kthr = k_get_active_thread ();
		thrmsg = k_get_thrmsg ( kthr );
		kmsgq = &thrmsg->msgq;
	}
	else { /* src_type == MSG_QUEUE */
		msgq = src;
		kgmsgq = msgq->handle;
		ASSERT_ERRNO_AND_SYS_EXIT ( kgmsgq && kgmsgq->id == msgq->id,
					E_INVALID_HANDLE );
		kmsgq = &kgmsgq->mq;
	}

	/* get first message from queue */
	kmsg = list_get ( &kmsgq->msgs, FIRST );

	if ( type != 0 ) /* type != 0 => search for first message 'type' */
		while ( kmsg && kmsg->msg.type != type )
			kmsg = list_get_next ( &kmsg->list );

	if ( kmsg ) /* have message */
	{
		if ( size < kmsg->msg.size )
		{
			msg->size = 0;
			SYS_EXIT ( E_TOO_BIG );
		}

		msg->type = kmsg->msg.type;
		msg->size = kmsg->msg.size;
		memcpy ( msg->data, kmsg->msg.data, msg->size );

		kmsg = list_remove ( &kmsgq->msgs, FIRST, &kmsg->list );
		ASSERT ( kmsg );
		kfree ( kmsg );

		SYS_EXIT ( SUCCESS );
	}
	else { /* queue empty! */
		if ( !( flags & IPC_WAIT ) )
			SYS_EXIT ( E_EMPTY );

		//SET_ERRNO ( E_RETRY );
		/* block thread */
		k_enqueue_thread ( NULL, &kmsgq->thrq );

		k_schedule_threads ();

		SYS_EXIT ( E_RETRY );
	}
}

/*! Delete all messages from queue */
void k_msgq_clean ( kmsg_q *kmsgq )
{
	kmsg_t *kmsg;

	kmsg = list_remove ( &kmsgq->msgs, FIRST, NULL );
	while ( kmsg )
	{
		kfree ( kmsg );
		kmsg = list_remove ( &kmsgq->msgs, FIRST, NULL );
	}
}

#endif /* MESSAGES */
