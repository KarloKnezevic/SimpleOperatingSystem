/*! Dynamic memory allocator
 *
 * In this implementation double linked list are used.
 * Free list is sorted. Search is started from first element until chunk
 * with adequate size is found (same or greater than required).
 * When chunk is freed, first join is tried with left and right neighbor chunk
 * (by address). If not joined, chunk is marked as free and put at list start.
 */

/*
 * Iskorišten kod od bf. Logika inicijalizacije ostaje ista.
 *
 * Najbolji odgovarajući metoda:
 * Lista slobodnih blokova složena je prema veličini blokova (sortirana je), 
 * omogućujući dodjeljivanje najmanjeg slobodnog bloka koji je dovoljno velik za traženi zahtjev.
 * Činjenica da je lista sortirana povećava složenost ubacivanja slobodnog bloka u listu (potrebmo sortiranje).
 *
*/

#pragma once

#include <lib/types.h>

#ifndef _BF_C_

typedef void bf_mpool_t;

/*! interface */
void *bf_init ( void *mem_segm, size_t size );
void *bf_alloc ( bf_mpool_t *mpool, size_t size );
int bf_free ( bf_mpool_t *mpool, void *chunk_to_be_freed );

#else

/* free chunk header (in use chunk header is just 'size') */
typedef struct _bf_hdr_t_
{
	size_t  size; /* chunk size, including head and tail headers */
	struct _bf_hdr_t_ *prev; /* previous free in list */
	struct _bf_hdr_t_ *next; /* next free in list */
}
bf_hdr_t;

/* chunk tail (and header for in use chunks) */
typedef struct _bf_tail_t_
{
	size_t  size; /* chunk size, including head and tail headers */
}
bf_tail_t;

typedef struct _bf_mpool_t_
{
	bf_hdr_t *first;
}
bf_mpool_t;

#define HEADER_SIZE ( sizeof (bf_hdr_t) + sizeof (bf_tail_t) )

/* use LSB of 'size' to mark chunk as used (otherwise size is always even) */
#define MARK_USED(HDR)	do { (HDR)->size |= 1;  } while(0)
#define MARK_FREE(HDR)	do { (HDR)->size &= ~1; } while(0)

#define CHECK_USED(HDR)	((HDR)->size & 1)
#define CHECK_FREE(HDR)	!CHECK_USED(HDR)

#define GET_SIZE(HDR)	((HDR)->size & ~1)

#define GET_AFTER(HDR)	(((void *) (HDR)) +  GET_SIZE(HDR))
#define GET_TAIL(HDR)	(GET_AFTER(HDR) - sizeof (bf_tail_t))
#define GET_HDR(TAIL)	(((void *)(TAIL)) - GET_SIZE(TAIL) + sizeof(bf_tail_t))

#define CLONE_SIZE_TO_TAIL(HDR)	\
	do { ( (bf_tail_t *) GET_TAIL(HDR) )->size = (HDR)->size; } while(0)

#define ALIGN_VAL	( (size_t) sizeof(size_t) )
#define ALIGN_MASK	( ~( ALIGN_VAL - 1 ) )
#define ALIGN(P)	\
	do { (P) = ALIGN_MASK & ( (size_t) (P) ); } while(0)
#define ALIGN_FW(P)	\
	do { (P) = ALIGN_MASK & (((size_t) (P)) + (ALIGN_VAL - 1)) ; } while(0)

void *bf_init ( void *mem_segm, size_t size );
void *bf_alloc ( bf_mpool_t *mpool, size_t size );
int bf_free ( bf_mpool_t *mpool, void *chunk_to_be_freed );

static void bf_remove_chunk ( bf_mpool_t *mpool, bf_hdr_t *chunk );
static void bf_insert_chunk ( bf_mpool_t *mpool, bf_hdr_t *chunk );
void sort(bf_mpool_t **mpool);

#endif
