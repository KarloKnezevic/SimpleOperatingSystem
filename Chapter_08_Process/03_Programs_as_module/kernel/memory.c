/*! Memory management */
#define _KERNEL_

#include "memory.h"
#include <arch/multiboot.h>
#include <arch/processor.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <lib/string.h>
#include <lib/list.h>

/*! Memory map */
static mseg_t k_kernel;	/* kernel code and data */
static mseg_t k_heap;	/* kernel heap: for everything else */

static uint multiboot; /* save multiboot block address */

/*! Dynamic memory allocator for kernel */
MEM_ALLOC_T *k_mpool;

/*! Programs loaded as module */
kprog_t prog;

#define PNAME "prog_name="

/*!
 * Init memory layout: using variables "from" linker script and multiboot info
 */
void k_memory_init ( unsigned long magic, unsigned long addr )
{
	extern char kernel_code, k_kernel_end;
	multiboot_info_t *mbi;
	multiboot_module_t *mod;
	prog_info_t *pi;
	uint max;
	int i;
	char *name, *pos;

	/* implicitly from kernel linker script */
	k_kernel.start = &kernel_code;
	k_kernel.size = ( (uint) &k_kernel_end ) - (uint) &kernel_code;

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprint ( "Boot loader is not multiboot-compliant!\n" );
		halt();
	}

	/* from multiboot info */
	multiboot = addr;
	mbi = (void *) addr;

	max = (uint) &k_kernel_end;

	prog.pi = NULL;

	/* initialize modules */
	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		mod = (void *) mbi->mods_addr;

		for ( i = 0; i < mbi->mods_count; i++, mod++ )
		{
			if ( max < mod->mod_end )
				max = mod->mod_end + 1;

			/* Is this module a program?
			   Programs must have 'prog_name' in command line */
			name = strstr ( (char *) mod->cmdline, PNAME );
			if ( name && !prog.pi )
			{
				name += strlen ( PNAME );
				pos = strchr ( name, ' ' );
				if ( pos )
					*pos++ = 0;

				prog.prog_name = name;

				pi = (void *) mod->mod_start;

				prog.pi = pi->start_adr;

				prog.m.start = prog.pi;
				prog.m.size = (size_t) pi->end_adr -
					       (size_t) pi->start_adr + 1;

				prog.started = FALSE;

				memcpy ( prog.pi, pi, prog.m.size );

				if ( max < (uint) pi->end_adr  )
					max = (uint) pi->end_adr + 1;
			}
		}
	}

	if ( max % ALIGN_TO )
		max += ALIGN_TO - ( max % ALIGN_TO );

	k_heap.start = (void *) max;
	k_heap.size = ( mbi->mem_upper - 1024 ) * 1024 - max;

	/* initialize dynamic memory allocation subsystem (needed for boot) */
	k_mpool = k_mem_init ( k_heap.start, k_heap.size );
}

/*! unique system wide id numbers */
#define WBITS	( sizeof(word_t) * 8 )
static word_t idmask[ MAX_RESOURCES / WBITS ] = { 0 };
static uint last_id = 0;

/*! Allocate and return unique id for new system resource */
uint k_new_unique_id ()
{
	uint starting = last_id;

	do {
		last_id = ( last_id + 1 ) % MAX_RESOURCES;
		if ( last_id == starting )
		{
			LOG ( ERROR, "Don't have free unique id!\n" );
			halt();
		}
		if ( last_id == 0 ) /* don't assign 0 */
			continue;
	}
	while ( idmask [ last_id / WBITS ] & ( 1 << ( last_id % WBITS ) ) );

	idmask [ last_id / WBITS ] |= 1 << ( last_id % WBITS );

	return last_id;
}

/*! Release resource id */
void k_free_unique_id ( uint id )
{
	ASSERT ( id > 0 && id < MAX_RESOURCES &&
		 ( idmask [ id / WBITS ] & ( 1 << ( id % WBITS ) ) ) );

	idmask [ id / WBITS ] &= ~ ( 1 << ( id % WBITS ) );
}


/*! print (or return) system information (and details) */
int sys__sysinfo ( void *p )
{
	char *buffer;
	size_t buf_size;
	char **param; /* last param is NULL */
	char usage[] = "Usage: sysinfo [threads|memory]";
	char look_console[] = "(sysinfo printed on console)";

	buffer = *( (char **) p ); p += sizeof (char *);
	ASSERT_ERRNO_AND_EXIT ( buffer, E_PARAM_NULL );

	buf_size = *( (size_t *) p ); p += sizeof (size_t *);

	param = *( (char ***) p );

	if ( param[1] == NULL )
	{
		/* only basic info defined in kernel/startup.c */
		extern char system_info[];

		if ( strlen ( system_info ) > buf_size )
			EXIT ( E_TOO_BIG );

		strcpy ( buffer, system_info );

		EXIT ( SUCCESS );
	}
	else {
		/* extended info is requested */
		if ( strcmp ( "memory", param[1] ) == 0 )
		{
			k_memory_info ();
			if ( strlen ( look_console ) > buf_size )
				EXIT ( E_TOO_BIG );
			strcpy ( buffer, look_console );
			EXIT ( SUCCESS );
			/* TODO: "memory [segments|modules|***]" */
		}
		else if ( strcmp ( "threads", param[1] ) == 0 )
		{
			k_thread_info ();
			if ( strlen ( look_console ) > buf_size )
				EXIT ( E_TOO_BIG );
			strcpy ( buffer, look_console );
			EXIT ( SUCCESS );
			/* TODO: "thread thr_id" */
		}
		else {
			if ( strlen ( usage ) > buf_size )
				EXIT ( E_TOO_BIG );
			strcpy ( buffer, usage );
			EXIT ( E_DONT_EXIST );
		}
	}
}

/*! print memory layout */
void k_memory_info ()
{
	multiboot_info_t *mbi = (void *) multiboot;
	multiboot_module_t *mod;
	int i;
	char *name, *pos;

	kprint ( "MOOLTIBOOT info at %x flags=%x\n", mbi, mbi->flags );

	if ( mbi->flags & MULTIBOOT_INFO_MEMORY )
	{
		kprint ( "Available memory: low = %d kB, high = %d kB\n",
			  mbi->mem_lower, mbi->mem_upper );
	}

	if ( mbi->flags & MULTIBOOT_INFO_CMDLINE )
	{
		kprint ( "Command line: %s\n", (char *) mbi->cmdline );
	}

	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		kprint ( "Modules: count %d, at %x\n",
			  mbi->mods_count, mbi->mods_addr );

		mod = (void *) mbi->mods_addr;

		for ( i = 0; i < mbi->mods_count; i++ )
		{
			/* is it program? must have prog_name in command line */
			name = strstr ( (char *) mod->cmdline, PNAME );
			if ( name )
			{
				name += strlen ( PNAME );
				pos = strchr ( name, ' ' );
				if ( pos )
					*pos++ = 0;
			}
			//kprint ( "* module %d: [%x - %x] \tcmd = %s\n", i,
			//	  mod->mod_start, mod->mod_end,
			//	  (char *) mod->cmdline );
			kprint ( "* module %d: cmd=%s\n", i,
				  (char *) mod->cmdline );
				  //name );
			//kprint ( "Content:\n%s\n", (char *) mod->mod_start );
			mod++;
		}
	}
#if 1
	if ( mbi->flags & MULTIBOOT_INFO_MEM_MAP )
	{
		kprint ( "Memory map at %x (size=%d)\n",
			  mbi->mmap_addr, mbi->mmap_length );

		multiboot_memory_map_t *memmap = (void *) mbi->mmap_addr;

		for ( i = 0; (uint32) memmap < mbi->mmap_addr + mbi->mmap_length;
			i++ )
		{
			kprint ( "* segment %d size=%x [%x - %x], flags= %d\n",
				  i, (uint32) memmap->len & 0xffffffff,
				  (uint32) memmap->addr & 0xffffffff,
				  ( (uint32) memmap->addr & 0xffffffff ) +
				  ( (uint32) memmap->len & 0xffffffff ) - 1,
				  memmap->type );
			memmap = (void *) ( (uint32) memmap + memmap->size +
				sizeof (memmap->size));
		}
	}
#endif
	kprint ( "MEMORY MAP (mmap):\n" );

	kprint ( "* Kernel code&data:  %x, size=%x\n",
		  k_kernel.start, k_kernel.size );

	kprint ( "* Kernel heap:       %x, size=%x\n",
		  k_heap.start, k_heap.size );
}
