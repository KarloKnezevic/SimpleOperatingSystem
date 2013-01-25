/*! Assembler macros for some processor control instructions */

#pragma once

#define disable_interrupts()	asm volatile ( "cli\n\t" )
#define enable_interrupts()	asm volatile ( "sti\n\t" )

#define halt()			asm volatile ( "cli \n\t" "hlt \n\t" );

#define suspend()		asm volatile ( "hlt \n\t" );

#define raise_interrupt(p)	asm volatile ("int %0\n\t" :: "i" (p):"memory")

#define memory_barrier()	asm ("" : : : "memory")


#define EFLAGS_IF	0x00000200	// Interrupt Enable		( 9)

static inline int set_interrupts ( int enable )
{
	int old_flags;

	asm volatile (	"pushf\n\t"
			"pop	%0\n\t"
			: "=r" (old_flags) );
	if (enable)
		enable_interrupts();
	else
		disable_interrupts();

	return old_flags & EFLAGS_IF;
}