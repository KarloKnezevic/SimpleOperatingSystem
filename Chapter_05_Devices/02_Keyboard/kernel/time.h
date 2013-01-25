/*! Time managing functions */

#pragma once

#include <lib/types.h>

/*! interface to programs */
int sys__get_time ( time_t *time );
int sys__alarm_new ( void **id, alarm_t *alarm );
int sys__alarm_set ( void *id, alarm_t *alarm );
int sys__alarm_get ( void *id, alarm_t *alarm );
int sys__alarm_remove ( void *id );

#ifdef _KERNEL_

/*! interface to kernel */
void k_time_init ();
int k_alarm_new ( void **id, alarm_t *alarm );
int k_alarm_set ( void *id, alarm_t *alarm );
int k_alarm_remove ( void *id );
void k_get_time ( time_t *time );

#endif /* _KERNEL_ */

/*! rest of the file is only for 'kernel/time.c' ---------------------------- */

#ifdef	_K_TIME_C_

#include <lib/list.h>

/*! Kernel alarm */
typedef struct _kalarm_t_
{
	alarm_t alarm;	/* alarm data */

	int active;	/* is alarm active (waiting) */

#ifdef DEBUG
	unsigned int magic;	/* alarm magic number - for error checking */
#endif
	list_h list;	/* active alarms are in list (in kernel) */
}
kalarm_t;

#define ALARM_MAGIC	0xD7422F8	/* alarm identifier (random number) */

/*! local functions */
static void k_timer_interrupt ();
static void k_schedule_alarms ();
static void k_alarm_add ( kalarm_t *alarm );


/*!
 * Compare alarms by expiration times (used when inserting new alarm in list)
 * \param a First alarm
 * \param b Second alarm
 * \return -1 when a < b, 0 when a == b, 1 when a > b
 */
static inline int alarm_cmp ( void *_a, void *_b )
{
	kalarm_t *a = _a, *b = _b;

	return time_cmp ( &a->alarm.exp_time, &b->alarm.exp_time );
}

#endif	/* _K_TIME_C_ */
