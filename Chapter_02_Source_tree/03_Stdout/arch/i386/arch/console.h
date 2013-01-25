/*! Console interface */

#pragma once

/*! Console interface (used in kernel mode) */
typedef struct _console_t_
{
	int (*init) (void *);
	int (*clear) ();
	int (*gotoxy) ( int x, int y );
	int (*print_char) ( int c, int attr );
	void (*enable_cursor) ( int enable, int shape );
}
console_t;
