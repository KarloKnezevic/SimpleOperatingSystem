/*! simple shell interpreter */

/*
Promijenjen shell.
Dodana opcija automatskog proširivanja pritiskom tipke <tab>.
*/

#include <api/stdio.h>
#include <lib/string.h>
#include <api/time.h>
#include <api/syscalls.h>
#include <lib/types.h>

#include PROGRAMS

char PROG_HELP[] = "Simple command shell";

typedef struct _cmd_t_
{
	int (*func) ( char *argv[] );
	char *name;
	char *descr;
}
cmd_t;

#define MAXCMDLEN	72
#define MAXARGS		10
#define INFO_SIZE	1000

static int help ();
static int clear ();
static int exit ();
static int quit ();
static int sysinfo ( char *args[] );

static cmd_t sh_cmd[] =
{
	{ help, "help", "help - list available commands" },
	{ clear, "clear", "clear - clear screen" },
	{ sysinfo, "sysinfo", "system information; usage: sysinfo [options]" },
	{ exit, "exit", "exit from shell" },
	{ quit, "quit", "quit from shell" },
	{ NULL, "" }
};

static cmd_t prog[] = PROGRAMS_FOR_SHELL;


int shell ( char *args[] )
{
	//naredba iz ljuske
	char cmd[MAXCMDLEN + 1];
	int i, key;
	//dodao Karlo
	int k;	//brojač u petlji
	int brojPreklapanja; //brojilo preklapanja imena
	int mjestoPreklapanja;
	//------
	time_t t;
	int argnum;
	//
	char *argval[MAXARGS + 1];

	print ( "\n*** Simple shell interpreter ***\n\n" );
	help ();

	t.sec = 0;
	t.nsec = 100000000; /* 100 ms */

	while (1)
	{
		new_cmd:
		print ( "\n> " );

		i = 0;
		memset ( cmd, 0, MAXCMDLEN );

		/* get command - get chars until new line is received */
		while ( i < MAXCMDLEN )
		{
			key = get_char ();

			if ( !key )
			{
				delay ( &t );
				continue;
			}

			if ( key == '\n' )
			{
				if ( i > 0 )
					break;
				else
					goto new_cmd;
			}

			switch ( key )
			{
			case '\b':
				if ( i > 0 )
				{
					cmd[--i] = 0;
					print ( "%c", key );
				}
				break;
			//OVO JE <TAB>
			case '\t':
				//ako je redak prazan i ako korisnik pritisne <tab>, ispiši sve naredbe
				if (i==0) {
				    
				    //ispiši naredbe (kao na Linux shellu)
				    print("All possible commands:\n");
				    print("**Shell commands:**\n");
				    
				    for (k = 0; sh_cmd[k].func != NULL; k++) {
					
					print("%s\n",sh_cmd[k].name);
				      
				    }
				    print("**Programs:**\n");
				    
				    for (k = 0; prog[k].func != NULL; k++) {
				      
					if (prog[k+1].func != NULL)
					    print("%s\n", prog[k].name);
					else
					    print("%s", prog[k].name);
					
				    }
				    
				    goto new_cmd;
				    
				} else {
				
				    //ako ima znakova i pritisnut je tab, pronađi fje koje se preklapaju
				    //ako se preklapa jedno ime, dopuni ga
				    //ako se preklapa više imena, u novom retku ispiši imena svih mogućih naredbi
				    //---------------------------------------------------------------------------
				    //prebroji preklapanja
				    brojPreklapanja = 0;
				    mjestoPreklapanja = 0;
				    for (k = 0; k < sh_cmd[k].func != NULL; k++) {
					//ako su isti
					if (strncmp(cmd, sh_cmd[k].name, i) == 0) {
					    brojPreklapanja++;
					    mjestoPreklapanja = 1;
					}
				    }
				    
				    for (k = 0; k < prog[k].func != NULL; k++) {
					//ako su isti
					if (strncmp(cmd, prog[k].name, i) == 0) {
					    brojPreklapanja++;
					    mjestoPreklapanja = 2;
					}
				    }
				    
				    //ako nema preklapanja, čekaj novi znak
				    if (brojPreklapanja == 0) {
					continue;
				    }

				    //ako je samo jedno preklapanje, nadopuni naredbu
				    //skrati pretragu
				    if (brojPreklapanja == 1) {
					if (mjestoPreklapanja == 1) {
					    for (k = 0; k < sh_cmd[k].func != NULL; k++) {
						//ako su isti
						if (strncmp(cmd, sh_cmd[k].name, i) == 0) {
						    //izbriši napisano i ispiši naredbu
						    while(i < strlen(sh_cmd[k].name)) {
							print ( "%c", sh_cmd[k].name[i] );
							cmd[i++] = sh_cmd[k].name[i];
						    }
						    break;
						}
					    }
					    continue;
					} else {
					    for (k = 0; k < prog[k].func != NULL; k++) {
						//ako su isti
						if (strncmp(cmd, prog[k].name, i) == 0) {
						    //izbriši napisano i ispiši naredbu
						    while(i < strlen(prog[k].name)) {
							print ( "%c", prog[k].name[i] );
							cmd[i++] = prog[k].name[i];
						    }
						    break;
						}
					    }
					    continue;
					}
				    } else {
				      //ako je više preklapanja, ostavi napisane znakove
				      //u novom redu ispiši sve preklapajuće naredbe
				      print("\n");
				      for (k = 0; k < sh_cmd[k].func != NULL; k++) {
					  if (strncmp(cmd, sh_cmd[k].name, i) == 0) {
					      print("%s\n", sh_cmd[k].name);
					  }
				      }
				      
				      for (k = 0; k < prog[k].func != NULL; k++) {
					  if (strncmp(cmd, prog[k].name, i) == 0) {
					      print("%s\n", prog[k].name);
					  }
				      }
				      goto new_cmd;
				    }
				}

			default:
				print ( "%c", key );
				cmd[i++] = key;
				break;
			}
		}
		print ( "\n" );

		//___________FUNKCIONALNOST ISPOD OSTAJE ISTA________________

		/* parse command line */
		argnum = 0;
		for(i = 0; i < MAXCMDLEN && cmd[i]!=0 && argnum < MAXARGS; i++)
		{
			if ( cmd[i] == ' ' || cmd[i] == '\t')
				continue;

			argval[argnum++] = &cmd[i];
			while ( cmd[i] && cmd[i] != ' ' && cmd[i] != '\t'
				&& i < MAXCMDLEN )
				i++;

			cmd[i] = 0;
		}
		argval[argnum] = NULL;

		if ( strcmp ( argval[0], "quit" ) == 0 ||
			strcmp ( argval[0], "exit" ) == 0 )
			break;

		/* match command to shell command */
		for ( i = 0; sh_cmd[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], sh_cmd[i].name ) == 0 )
			{
				if ( sh_cmd[i].func ( argval ) )
					print ( "\nProgram returned error!\n" );

				goto new_cmd;
			}
		}

		/* match command to program */
		for ( i = 0; prog[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], prog[i].name ) == 0 )
			{
				if ( prog[i].func ( argval ) )
					print ( "\nProgram returned error!\n" );

				goto new_cmd;
			}
		}

		/* not program kernel or shell knows about it - report error! */
		print ( "Invalid command!" );
	}

	print ( "Exiting from shell\n" );

	return 0;
}

static int help ()
{
	int i;

	print ( "Shell commands: " );
	for ( i = 0; sh_cmd[i].func != NULL; i++ )
		print ( "%s ", sh_cmd[i].name );
	//print ( "quit/exit\n" );
	print("\n");

	print ( "Programs: " );
	for ( i = 0; prog[i].func != NULL; i++ )
		print ( "%s ", prog[i].name );
	print ( "\n" );

	return 0;
}

static int clear ()
{
	return clear_screen ();
}

static int sysinfo ( char *args[] )
{
	char info[INFO_SIZE];

	sys__sysinfo ( info, INFO_SIZE, args );

	print ( "%s\n", info );

	return 0;
}

static int exit() {
    return 0;
}

static int quit() {
    return 0;
}