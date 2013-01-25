/*! Printing on stdout, reading from stdin */

#pragma once

extern inline int clear_screen ();
extern inline int goto_xy ( int x, int y );
extern inline int print_char ( int c, int attr );
extern inline void enable_cursor( int enable, int shape );
int print ( char *format, ... );
//dodani atributi za znak i pozadinu
int printattr ( int chattr, int bgattr, char *format, ... );

#define CH_BLACK 	0
#define CH_BLUE 	1
#define CH_GREEN 	2
#define CH_CYAN 	3
#define CH_RED 		4
#define CH_MAGENTA 	5
#define CH_BROWN 	6
#define CH_GRAY 	7

#define BG_BLACK 	0
#define BG_BLUE 	1
#define BG_GREEN 	2
#define BG_CYAN 	3
#define BG_RED 		4
#define BG_MAGENTA 	5
#define BG_BROWN 	6
#define BG_GRAY 	7

#define NB 		0
#define BL 		1

#define B		0	//block
#define U		1	//underline
#define	H		2	//half
