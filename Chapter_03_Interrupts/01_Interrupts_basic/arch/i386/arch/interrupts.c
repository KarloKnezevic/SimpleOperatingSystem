/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#define _KERNEL_
#include "interrupts.h"

#include <arch/io.h>
#include <arch/processor.h>
#include <kernel/errno.h>
#include <lib/string.h>

//promijenio: Karlo Knežević, 25.3.2012.
//*************************************
//broj prekida
static int broj_prekida = INTERRUPTS;
//tekući prioritet prekida
static int tekuci_prioritet = 1000;
//opisnik oznake čekanja
struct opisnik {
    int ceka;	//boolean 
    int irq;	//redni broj zahtjeva
};
//struktura oznaka čekanja; inicijalizirana na (0,0)
static struct opisnik oznaka_cekanja[INTERRUPTS] = {{0}};
//prethodni tekući prioritet; inicijaliziran na (-1)
static int prethodni_tekuci_prioritet[INTERRUPTS] = {-1};
//prioriteti po prekidima; prekide pogledati na 32, 33 i 34
int prioritet[INTERRUPTS] = { 3, 2, 6, 7, 4, 1, 0, 8, 5, 5, 3, 2, 6, 7, 4, 1, 0, 8, 5, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 4, 5, 6, 1, 0, 8, 5, 5, 3, 2, 6, 7, 4, 1, 0, 8, 5};//49 brojeva


/*! interrupt handlers */
static int (*ihandler[INTERRUPTS]) ( unsigned int );

/*! Initialize interrupt subsystem (in 'arch' layer) */
void arch_init_interrupts ()
{
	memset ( ihandler, 0, sizeof(ihandler) );
}

/*! Register handler function for particular interrupt number */
void *arch_register_interrupt_handler ( unsigned int inum, void *handler )
{
	void *old_handler = NULL;

	if ( inum < INTERRUPTS )
	{
		old_handler = ihandler[inum];
		ihandler[inum] = handler;
	}
	else {
		LOG ( ERROR, "Interrupt %d can't be used!\n", inum );
		halt ();
	}

	return old_handler;
}

/*!
 * "Forward" interrupt handling to registered handler
 * (called from interrupts.S)
 */
//Karlo Knežević
//dodan kod za obradu prioritetnih prekida
void arch_interrupt_handler ( int irq_num )
{
	int prio;
	int i;
	
	disable_interrupts();
	kprint("Pojavio se prekid: %d\n", irq_num);
  
	if ( irq_num < INTERRUPTS && ihandler[irq_num] != NULL )
	{
		prio = prioritet[irq_num];
		oznaka_cekanja[prio].ceka = 1;
		oznaka_cekanja[prio].irq = irq_num;
		i = prio;
		
		while (i < broj_prekida && i < tekuci_prioritet) {
		    oznaka_cekanja[i].ceka = 0;
		    prethodni_tekuci_prioritet[i] = tekuci_prioritet;
		    tekuci_prioritet = i;
		    
		    enable_interrupts();
		    /* Call registered handler */
		    ihandler[oznaka_cekanja[i].irq] ( oznaka_cekanja[i].irq );
		    disable_interrupts();
		    
		    tekuci_prioritet = prethodni_tekuci_prioritet[i];
		    prethodni_tekuci_prioritet[i] = -1;
		    
		    while ( i < broj_prekida && i < tekuci_prioritet && oznaka_cekanja[i].ceka == 0 )
			i++;
		}
	}

	else {
		LOG ( ERROR, "Unregistered interrupt: %d !\n", irq_num );
		halt ();
	}
}
