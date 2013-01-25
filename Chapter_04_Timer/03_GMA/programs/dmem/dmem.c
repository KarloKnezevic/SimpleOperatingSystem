/*! memory allocator test */

#include <api/stdio.h>
#include <api/malloc.h>
#include <lib/string.h>
#include <lib/bits.h>

#if MEM_ALLOCATOR == FIRST_FIT

#include <lib/mm/ff_simple.h>

#define	MEM_INIT(ADDR, SIZE)		ffs_init ( ADDR, SIZE )
#define MEM_ALLOC(MP, SIZE)		ffs_alloc ( MP, SIZE )
#define MEM_FREE(MP, ADDR)		ffs_free ( MP, ADDR )

#elif MEM_ALLOCATOR == GMA

#include <lib/mm/gma.h>

#define	MEM_INIT(ADDR, SIZE)		gma_init ( ADDR, SIZE, 32, 0 )
#define MEM_ALLOC(MP, SIZE)		gma_alloc ( MP, SIZE )
#define MEM_FREE(MP, ADDR)		gma_free ( MP, ADDR )

#elif MEM_ALLOCATOR == BF

#include <lib/mm/bf.h>

#define	MEM_INIT(ADDR, SIZE)		bf_init ( ADDR, SIZE)
#define MEM_ALLOC(MP, SIZE)		bf_alloc ( MP, SIZE )
#define MEM_FREE(MP, ADDR)		bf_free ( MP, ADDR )

#endif

#define VERBOSE		FALSE
#if VERBOSE == TRUE
#define PRINT(format, ...)		print ( format, ##__VA_ARGS__ );
#else
#define PRINT(format, ...)
#endif

/* testing */
int dmem ()
{
	uint pool_size = 1234567;
	uint max_block_size = 1512;
	uint init_requests = 1500;
	uint requests = 100000;
	uint i, j, k, used, fail;
	ssize_t inuse;
	void *pool, *mpool;
	struct req { void *ptr; ssize_t size; } *m;
	uint rseed = 1;

	pool = malloc ( pool_size );
	m = malloc ( requests * sizeof ( struct req ) );

	if ( pool == NULL || m == NULL )
	{
		print ( "Malloc returned NULL\n" );
		return 1;
	}

	memset ( pool, 0, pool_size );

	for ( j = 0; j < requests; j++)
	{
		m[j].ptr = NULL;
		m[j].size = 0;
	}

	mpool = MEM_INIT ( pool, pool_size );

	used = 0;
	fail = 0;
	inuse = 0;

	/* initial allocations */
	for ( j = 0; j < init_requests; j++)
	{
		m[j].size = rand ( &rseed ) % max_block_size + 4;
		m[j].ptr = MEM_ALLOC ( mpool, m[j].size );

		if ( m[j].ptr != NULL )
		{
			PRINT ("[%d] alloc=%x\t[%d]\n", j, m[j].ptr, m[j].size);

			memset ( m[j].ptr, 5, m[j].size );
			used++;
			inuse += m[j].size;
		}
		else {
			fail++;
			PRINT ("[%d] alloc=%x\t[%d]\n", j, m[j].ptr, m[j].size);
			PRINT ( "FAIL(%d)\n", fail );

			break;
		}
	}

	print ( "Start of tests (j=%d, fail=%d, InUseBlocks=%d, inuse=%d)!\n",
		j, fail, used, inuse );

	fail = 0;

	for ( i = 0; i < requests; i++ )
	{
		if ( rand ( &rseed ) & 1 )
		{
			/* alloc */
			for ( j = 0; j < requests && m[j].ptr != NULL; j++)
				;

			if ( j >= requests )
			{
				print ( "No free element in m[]!\n" );
				break;
			}

			m[j].size = rand ( &rseed ) % (max_block_size) + 4;

			m[j].ptr = MEM_ALLOC ( mpool, m[j].size );

			if ( m[j].ptr != NULL )
			{
				PRINT ( "[%d] alloc=%p\t[%d]\n", i, m[j].ptr,
					 m[j].size );
				memset ( m[j].ptr, 3, m[j].size );
				used++;
				inuse += m[j].size;
			}
			else {
				fail++;
				PRINT ( "FAIL(%d)\n", fail );
				if ( fail == 1 )
					print ( "\tFirst fail (i=%d)!\n", i );
			}
		}
		else {
			/* free */
			while ( used > 0 )
			{
				k = rand ( &rseed ) % requests;
				if ( m[k].ptr != NULL )
				{
					PRINT ( "[%d] free =%p\t[%u]\n",
						 i, m[k].ptr, m[k].size );

					MEM_FREE ( mpool, m[k].ptr );

					m[k].ptr = NULL;

					used--;
					inuse -= m[k].size;

					break;
				}
			}
		}
	}

	PRINT ( "\n" );

	print ( "End of tests (i=%d, fail=%d, InUseBlocks=%d, inuse=%d)!\n",
		i, fail, used, inuse );

	free ( m );
	free ( mpool );

	return 0;
}
