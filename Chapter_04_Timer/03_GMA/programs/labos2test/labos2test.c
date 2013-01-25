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

#define broj_kazaljki 20

int labos2test()
{
  //inicijalna veličina zauzetog prostora za dinamičku alokaciju
  int pool_size = 2048;
  void *pool, *mpool;
  //prostor za dinamičku alokaciju
  pool = malloc ( pool_size );
  
  if ( pool == NULL )
  {
    print ( "Malloc returned NULL\n" );
    return 1;
  }
  
  //inicializiraj prostor
  memset ( pool, 0, pool_size );
  
  mpool = MEM_INIT ( pool, pool_size );
  
  void *p[broj_kazaljki] = {NULL};
  uint rseed = 1;
  int i, n = 0;
  int velicina_bloka;
  int B_MIN = 32;
  int B_MAX = 512;
  
  while( n<20 )
  {
      n++;
      i = rand(&rseed) % broj_kazaljki;
      print("%d]. ", i);
      if ( p[i] )
      {
	  print(" blok osloboden...\n");
	  MEM_FREE(mpool, p[i]);
	  p[i] = NULL;
      }
      else
      {
	  velicina_bloka = B_MIN + rand(&rseed) % (B_MAX - B_MIN);
	  p[i] = MEM_ALLOC(mpool, velicina_bloka );
	  print(" blok se zauzima; velicina bloka: %d ", velicina_bloka);
	  if ( !p[i] )
	  {
	      print(" zahtjev %d nije posluzen", n);
	  }
	  print("\n");
      }
  }
 
  /*void *p1;
  void *p2;
  void *p3;
  void *p4;
  p1 = MEM_ALLOC(mpool, 32);
  p2 = MEM_ALLOC(mpool, 64);
  p3 = MEM_ALLOC(mpool, 96);
  p4 = MEM_ALLOC(mpool, 128);
  
  MEM_FREE(mpool, p3);
  MEM_FREE(mpool, p1);*/
 
  return 0;
  
}