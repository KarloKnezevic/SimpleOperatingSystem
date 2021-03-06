/*!  Dynamic memory allocator - best fit */

#define _BF_C_
#include "bf.h"

#ifndef ASSERT
#include <kernel/errno.h>
#endif

/*
 * Iskorišten kod od bf. Logika inicijalizacije ostaje ista.
 *
 * Najbolji odgovarajući metoda:
 * Lista slobodnih blokova složena je prema veličini blokova (sortirana je), 
 * omogućujući dodjeljivanje najmanjeg slobodnog bloka koji je dovoljno velik za traženi zahtjev.
 * Činjenica da je lista sortirana povećava složenost ubacivanja slobodnog bloka u listu (potrebmo sortiranje).
 *
*/

/*!
 * Initialize dynamic memory manager
 * \param mem_segm Memory pool start address
 * \param size Memory pool size
 * \return memory pool descriptor
*/


/*
Inicijalizacija ostaje ista kao kod prvog odgovarajućeg.
Iz bloka slobodne memorije, registriramo se na blok veličine size i na njemu ćemo raditi alokaciju.
Veličina podsustava za alokaciju određen je u jezgri i inicijalizaciju podsustava radi jezgra!
*/
void *bf_init ( void *mem_segm, size_t size )
{
	size_t start, end;
	bf_hdr_t *chunk, *border;
	bf_mpool_t *mpool;

	ASSERT ( mem_segm && size > sizeof (bf_hdr_t) * 2 );

	/* align all on 'size_t' (if already not aligned) */
	start = (size_t) mem_segm;
	end = start + size;
	ALIGN_FW ( start );
	mpool = (void *) start;		/* place mm descriptor here */
	start += sizeof (bf_mpool_t);
	ALIGN ( end );

	mpool->first = NULL;

	if ( end - start < 2 * HEADER_SIZE )
		return NULL;

	border = (bf_hdr_t *) start;
	border->size = sizeof (size_t);
	MARK_USED ( border );

	chunk = GET_AFTER ( border );
	chunk->size = end - start - 2 * sizeof(size_t);
	MARK_FREE ( chunk );
	CLONE_SIZE_TO_TAIL ( chunk );

	border = GET_AFTER ( chunk );
	border->size = sizeof (size_t);
	MARK_USED ( border );

	bf_insert_chunk ( mpool, chunk ); /* first and only free chunk */

	//vraćamo blok nad kojim ćemo raditi alokacije
	return mpool;
}

/*!
 * Get free chunk with required size (or slightly bigger)
 * \param mpool Memory pool to be used (if NULL default pool is used)
 * \param size Requested chunk size
 * \return Block address, NULL if can't find adequate free chunk
 */

/*
 * Logika alokacije ista je kao i kod prvog odgovarajućeg. 
 * Dodjeljuje se blok zahtjevane veličine.
 * Pretraživanje započinje od početka liste.
 *
 * VAŽNO:
 * Glavna razlika u odnosu na prvog odgovarajućeg je što je lista SORTIRANA (uzlazno!).
 *
*/
void *bf_alloc ( bf_mpool_t *mpool, size_t size )
{
	bf_hdr_t *iter, *chunk;

	ASSERT ( mpool );

	size += sizeof (size_t) * 2; /* add header and tail size */
	if ( size < HEADER_SIZE )
		size = HEADER_SIZE;

	/* align request size to higher 'size_t' boundary */
	ALIGN_FW ( size );

	//TRAŽENJE SLOBODNOG BLOKA
	//PRETRAGA ZAPOČINJE OD PRVOG SLOBODNOG BLOKA
	iter = mpool->first;
	//dok postoje slobodni blokovi i dok je veličina manja od potrebne veličine, traži
	while ( iter != NULL && iter->size < size )
		iter = iter->next;

	//ako nisam našao blok dovoljne veličine, vrati null
	if ( iter == NULL )
		return NULL; /* no adequate free chunk found */
		
	//inače, ako je veličina pronađenog slobodnog bloka 
	if ( iter->size >= size + HEADER_SIZE )
	{
		/* split chunk */
		/* first part remains in free list, just update size */
		iter->size -= size;
		CLONE_SIZE_TO_TAIL ( iter );

		chunk = GET_AFTER ( iter );
		chunk->size = size;
	}
	else { /* give whole chunk */
		chunk = iter;

		/* remove it from free list */
		bf_remove_chunk ( mpool, chunk );
	}

	//označi kao zauzetog
	MARK_USED ( chunk );
	CLONE_SIZE_TO_TAIL ( chunk );
	//vrati blok
	return ( (void *) chunk ) + sizeof (size_t);
}

/*!
 * Free memory chunk
 * \param mpool Memory pool to be used (if NULL default pool is used)
 * \param chunk Chunk location (starting address)
 * \return 0 if successful, -1 otherwise
 */

/*
 * Nakon oslobađanja bloka radi se sortiranje liste tako da je na početku najmanji blok, 
 * a na kraju najveći blok.
 * Blok se dodaje listu koja se:
 *				1. blok spaja sa susjedima ukoliko su slobodni
 *				2. sortira listu
*/
int bf_free ( bf_mpool_t *mpool, void *chunk_to_be_freed )
{
	bf_hdr_t *chunk, *before, *after;

	ASSERT ( mpool && chunk_to_be_freed );

	chunk = chunk_to_be_freed - sizeof (size_t);
	MARK_FREE ( chunk ); /* mark it as free */

	/* join with left? */
	before = ( (void *) chunk ) - sizeof(size_t);
	if ( CHECK_FREE ( before ) )
	{
		before = GET_HDR ( before );
		bf_remove_chunk ( mpool, before );
		before->size += chunk->size; /* join */
		chunk = before;
	}

	/* join with right? */
	after = GET_AFTER ( chunk );
	if ( CHECK_FREE ( after ) )
	{
		bf_remove_chunk ( mpool, after );
		chunk->size += after->size; /* join */
	}

	/* insert chunk in free list */
	//dodaje slobodan blok u listu i sortira
	bf_insert_chunk ( mpool, chunk );

	/* set chunk tail */
	CLONE_SIZE_TO_TAIL ( chunk );
	
	bf_hdr_t *iter;
	iter = mpool->first;
	print("Ispis slobodnih blokova:\n");
	while( iter != NULL )
	{
	    print("%d -> ", iter->size);
	    iter = iter->next;
	}
	print("\n");

	return 0;
}

/*!
 * Routine that removes an chunk from 'free' list (free_list)
 * \param mpool Memory pool to be used
 * \param chunk Chunk header
 */
static void bf_remove_chunk ( bf_mpool_t *mpool, bf_hdr_t *chunk )
{
	if ( chunk == mpool->first ) /* first in list? */
		mpool->first = chunk->next;
	else
		chunk->prev->next = chunk->next;

	if ( chunk->next != NULL )
		chunk->next->prev = chunk->prev;
}

/*!
 * Routine that insert chunk into 'free' list (free_list)
 * \param mpool Memory pool to be used
 * \param chunk Chunk header
 */

/*
 * Oslobođeni blok dodaje se na 1. mjesto. 
 * Kad se doda blok, poziva se funkcija za soritiranje.
*/
static void bf_insert_chunk ( bf_mpool_t *mpool, bf_hdr_t *chunk )
{
	chunk->next = mpool->first;
	chunk->prev = NULL;

	if ( mpool->first )
		mpool->first->prev = chunk;

	mpool->first = chunk;
	//sortiranje
	sort(&mpool);
}

/*
 * Funkcija dobija pokazivač na pokazivač na listu.
 * Sortiranje se vrši prema veličini slobodnog bloka.
 * Insertion sort.
*/
void sort(bf_mpool_t **mpool)
{
	int n;
	bf_hdr_t *cur;
	//trenutno gledam na prvi element liste
	cur = (*mpool)->first;
	
	//ako je samo jedan element u listi
	if (cur->next == NULL)
		return;

	
	bf_hdr_t *ptr,*tmp;
	cur = cur->next;

	//dok nisam došao do kraja
	while(cur != NULL)
	{
		n = 0;
		ptr = cur;
		tmp = cur->prev;
		cur = cur->next;
		
		while (tmp!=NULL && tmp->size > ptr->size)
		{
			n++;
			tmp = tmp->prev;
		}
	
		if (n)
		{
			ptr->prev->next=ptr->next;
			if (ptr->next!=NULL)
				ptr->next->prev=ptr->prev;
			if (tmp==NULL)
			{
				tmp = (*mpool)->first;
				ptr->prev = NULL;
				ptr->next = tmp;
				ptr->next->prev = ptr;
				(*mpool)->first =ptr;
			}
			else
			{
				tmp = tmp->next;
				tmp->prev->next = ptr;
				ptr->prev = tmp->prev;
				tmp->prev = ptr;
				ptr->next = tmp;
			}
		}
	}
}
