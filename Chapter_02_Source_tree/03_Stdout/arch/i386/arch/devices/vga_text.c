/*! Print on console using video memory */
#ifdef VGA_TEXT /* using this devices? */

#include <arch/io.h>
#include <arch/console.h>
#include <lib/types.h>
#include <lib/string.h>

#define VIDEO		0x000B8000 /* video memory address */
#define COLS		80 /* number of characters in a column */
#define ROWS		24 /* number of characters in a row */
//dodao Karlo Knežević
//#define CURSOR_MSL 	0x02	/*cursor minimum scan line*/
#define CURSOR_START	0x0A	/*cursor start*/
#define CURSOR_END	0x0B	/*cursor end*/

/*! cursor position */
static int xpos;
static int ypos;


/*! starting address of video memory */
static unsigned char *video;

//ovo nije potrebno
/*! font color */
//static int color[8] = {
//	0, 								//black
//	1,								//blue
//	2, /* 'program' font - green */					//green
//	3,								//cyan
//	4, /* 'kernel' font - red */					//red
//	5,								//magenta
//	6,								//brown
//	7 /* 'normal' characters - white on black background */		//gray
//};
static int cursor_shape[] = {
    0x00, 0x0F,	//blok
    0x0E, 0x0F,	//underline
    0x08, 0x09	//polovica underline
};

static int vga_text_init ();
static int vga_text_clear ();
static int vga_text_gotoxy ( int x, int y );
static int vga_text_print_char ( int c, int attr );
//dodao Karlo Knežević
static void vga_enable_cursor( int enable, int shape );
static void vga_init_cursor( int shape );

/*! Init console */
static int vga_text_init ( void *x )
{
	video = (unsigned char *) VIDEO;
	xpos = ypos = 0;

	return vga_text_clear ();
}

/*! Clear console */
static int vga_text_clear ()
{
	int i;

	for ( i = 0; i < COLS * ROWS; i++ )
	{
		video[2*i] = 0;
		//boja kursora
		video[2*i+1] = 7;
	}

	return vga_text_gotoxy ( 0, 0 );
}

/*!
 * Move cursor to specified location
 * \param x Row where to put cursor
 * \param y Column where to put cursor
 */
static int vga_text_gotoxy ( int x, int y )
{
	unsigned short int t;

	xpos = x;
	ypos = y;
	t = ypos * 80 + xpos;

	outb ( 0x3D4, 14 );
	outb ( 0x3D5, t >> 8 );
	outb ( 0x3D4, 15 );
	outb ( 0x3D5, t & 0xFF );
 
	return 0;
}

/*!
 * Print single character on console on current cursor position
 * \param c Character to print
 */
static int vga_text_print_char ( int c, int attr )
{
	switch ( c ) {
	case '\t': /* tabulator */
		xpos = ( xpos / 8 + 1 ) * 8;
		break;
	case '\r': /* carriage return */
		xpos = 0;
	case '\n': /* new line */
		break;
	case '\b': /* backspace */
		if ( xpos > 0 )
		{
		xpos --;
		*( video + ( xpos + ypos * COLS ) * 2 ) = ' ' & 0x00FF;
		*( video + ( xpos + ypos * COLS ) * 2 + 1 ) = attr;
		}
		break;
	default:
		*(video + (xpos + ypos * COLS) * 2) = c & 0x00FF;
		*(video + (xpos + ypos * COLS) * 2 + 1) = attr;
		xpos++;
	}

	if ( xpos >= COLS || c == '\n' ) /* continue on new line */
	{
		xpos = 0;
		if ( ypos < ROWS )
		{
			ypos++;
		}
		else { /* scroll one line */
			memmove ( video, video + 80 * 2, 80 * 2 * 24 );
			memsetw ( video + 80 * 2 * 24,
				  ' ' | ( attr << 8 ), 80 );
		}
	}

	vga_text_gotoxy ( xpos, ypos );

	return 0;
}

//dodao Karlo Knežević
static void vga_enable_cursor( int enable, int shape )
{ 
	//dodao Karlo Knežević
	vga_init_cursor( shape );
  
	outb(0x3d4, 0x0a);
	unsigned char tmp = inb(0x3d5);
	//http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0A
	if (enable == 0)
	      tmp |= (1<<5);
	else
	      tmp &= ~(1<<5);
	
	outb(0x3d4, 0x0a);
	outb(0x3d5, tmp);
}

//dodao Karlo Knežević
static void vga_init_cursor( int shape )
{
	//unsigned char val;
	//http://www.osdever.net/FreeVGA/vga/crtcreg.htm#09  , 0A, 0B
	//outb(0x3d4, CURSOR_MSL);	
	//val = inb(0x3d5) & 0x1f;
	int cursor_start = cursor_shape[2*shape];
	int cursor_end = cursor_shape [2*shape + 1];
	
	outb(0x3d4, CURSOR_START); 
	outb(0x3d5, cursor_start);
	outb(0x3d4, CURSOR_END );
	outb(0x3d5, cursor_end); //val-2
}

/*! vga_text as console */
console_t vga_text = (console_t)
{
	.init		= vga_text_init,
	.clear		= vga_text_clear,
	.gotoxy		= vga_text_gotoxy,
	.print_char	= vga_text_print_char,
	//dodao Karlo Knežević
	.enable_cursor	= vga_enable_cursor
};

/*! /dev/null emulation */
static void _dev_null_ () { return; }

console_t dev_null = (console_t)
{
	.init		= (void *) _dev_null_,
	.clear		= (void *) _dev_null_,
	.gotoxy		= (void *) _dev_null_,
	.print_char	= (void *) _dev_null_,
	//dodao Karlo Knežević
	.enable_cursor	= (void *) _dev_null_
};

#endif /* VGA_TEXT */
