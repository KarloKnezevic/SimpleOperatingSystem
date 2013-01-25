/*! Hello world program */

#include <api/stdio.h>

int hello_world ()
{
	enable_cursor(1, U);
	print ( "Hello World!\n" );
	printattr ( CH_GREEN, BG_BLUE, "Ovo je dodatni zadatak.\n" );
	printattr ( CH_GRAY, BG_CYAN, "Konacno se pojavio kursor." );
	return 0;
}
