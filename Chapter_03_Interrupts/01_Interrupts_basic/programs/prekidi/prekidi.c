#include <api/stdio.h>
#include <arch/interrupts.h>
#include <arch/processor.h>

void int_32 (int irq)
{
    print ("Pocetak obrade prekida %d prioriteta %d.\n", irq, prioritet[irq]);
    raise_interrupt ( 34 );
    print ("Kraj obrade prekida %d prioriteta %d.\n", irq, prioritet[irq]);
}

void int_33 (int irq)
{
    print ("Pocetak obrade prekida %d prioriteta %d.\n", irq, prioritet[irq]);
    raise_interrupt ( 32 );
    print ("Kraj obrade prekida %d prioriteta %d.\n", irq, prioritet[irq]);

}

void int_34 (int irq)
{
    print ("Pocetak obrade prekida %d prioriteta %d.\n", irq, prioritet[irq]);
    print ("Kraj obrade prekida %d prioriteta %d.\n", irq, prioritet[irq]);
}