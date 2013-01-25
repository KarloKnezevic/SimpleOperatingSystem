/*! 'serial port' module (UART) */
#ifdef UART

#pragma once

#define PARITY_NONE	( 0 << 3 )
#define PARITY_ODD	( 1 << 3 )
#define PARITY_EVEN	( 3 << 3 )
#define PARITY_MARK	( 5 << 3 )
#define PARITY_SPACE	( 7 << 3 )

#define STOPBIT_1	( 0 << 2 )
#define STOPBIT_15	( 1 << 2 )

/* operating mode */
#define UART_BYTE	1	/* "byte" mode - irq on every byte received */
#define UART_STREAM	2	/* "stream" mode - irq when input buffer is
				   (almost) full */

#ifdef _UART_C_

#include <lib/types.h>
#include <devices/devices.h>

#define COM1_BASE	0x3f8
#define COM2_BASE	0x2f8
#define COM3_BASE	0x3e8
#define COM4_BASE	0x2e8

#define THR 	0	/* Transmitter Holding Buffer */
#define RBR	0	/* Receiver Buffer */
#define DLL	0	/* Divisor Latch Low Byte */
#define IER	1	/* Interrupt Enable Register */
#define DLM	1	/* Divisor Latch High Byte */
#define IIR	2	/* Interrupt Identification Register */
#define FCR	2	/* FIFO Control Register */
#define LCR	3	/* Line Control Register */
#define MCR	4	/* Modem Control Register */
#define LSR	5	/* Line Status Register */
#define MSR	6	/* Modem Status Register */
#define SR	7	/* Scratch Register */

#define IER_DEFAULT	7
#define IER_DISABLE	0

#define FCR_ENABLE	0x01
#define FCR_CLEAR	0x06
#define FCR_BYTE_MODE	0x00
#define FCR_STREAM_MODE	0xC0
#define FCR_64BYTES	0x20

#define LCR_DLAB_ON	0x80
#define LCR_DLAB_OFF	0x00
#define LCR_BREAK	0x40

#define MCR_DEFAULT	0x08

#define IIR_INT_PENDING	( 1 << 0 )
#define IIR_MODEM	( 0 << 1 )
#define IIR_THR_EMPTY	( 1 << 1 )
#define IIR_RECV_DATA	( 2 << 1 )
#define IIR_LINE	( 3 << 1 )
#define IIR_TIMEOUT	( 6 << 1 )

#define LSR_DATA_READY	( 1 << 0 )
#define LSR_BREAK	( 1 << 4 )
#define LSR_THR_EMPTY	( 1 << 5 )
#define LSR_DHR_EMPTY	( 1 << 6 )


#define BUFFER_SIZE	256	/* software buffer size */

typedef struct _arch_uart_t_
{
	int initialized;

	uart_t params;

	int port;

	uint8 inbuff[BUFFER_SIZE];
	int inf, inl, insz;
	uint8 outbuff[BUFFER_SIZE];
	int outf, outl, outsz;
}
arch_uart_t;

#define INC_MOD(X,MOD)	do { X = ( X+1 < MOD ? X+1 : 0 ); } while(0)

#define UART_DEFAULT_SETTING	\
{ 115200, 8, PARITY_NONE, STOPBIT_1, UART_BYTE }

/* UART chip type */
enum {
	UT8250 = 1,
	UT16450,
	UT16550,
	UT16550A,
	UT16750
};


static void uart_read ( arch_uart_t *up );
static void uart_write ( arch_uart_t *up );
static int uart_config ( device_t *dev, uart_t *params );

#endif /* _UART_C_ */
#endif /* UART */
