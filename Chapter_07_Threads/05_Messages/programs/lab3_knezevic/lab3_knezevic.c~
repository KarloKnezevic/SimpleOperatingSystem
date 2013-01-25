#include <api/time.h>
#include <api/stdio.h>
#include <api/thread.h>
#include <api/messages.h>

#define THR_NUM 5
#define MSG_NUM 4
#define SIG_NUM 10
#define FUNC_NUM 3

char PROG_HELP[] = "Zadatak 7/4. Karlo Knezevic";

static time_t sleep = { .sec = 1, .nsec = 0 };

static void shp0 ( msg_t *msg )
{
      print ( "+0. funkcija obrade signala: tip poruke = %d, podatak = %d\n", msg->type, msg->data[0] );
}

static void shp1 ( msg_t *msg )
{
      print ( "++1. funkcija obrade signala: tip poruke = %d, podatak = %d\n", msg->type, msg->data[0] );
}

static void shp2 ( msg_t *msg )
{
      print ( "+++2. funkcija obrade signala: tip poruke = %d, podatak = %d\n", msg->type, msg->data[0] );
}

static void simple_thread ( void *param ) {
  
      int thr_no;
      thread_t self;
      
      //min sig/msg prio = 0,0
      thread_msg_set ( 0, 0 );
      //postavljanje handlera
      set_signal_handler( 0, shp0 );
      set_signal_handler( 1, shp1 );
      set_signal_handler( 2, shp2 );
      
      thread_self ( &self );
      
      thr_no = (int) param;
      print ("###Kreirana dretva %d. Cekam signale...\n", thr_no);
      
      int cnt = 0;
      while(cnt < 10) {
	  delay( &sleep );
	  cnt ++;
      }
}

int lab3_knezevic( char *args[] ) {
      
      print ( "Zadatak 7/4. Karlo Knezevic.\n" );
      print ( "===Pocetak izvodenja===\n\n" );
      
      //opisnici dretvi
      thread_t thread[THR_NUM];
      msg_t msg[MSG_NUM];
      int i;
      
      //upisivanje poruka
      for ( i = 0; i < MSG_NUM; i++) {
		msg[i].size = 1;
		//ovdje je upisan prioritet
		msg[i].type = i;
		msg[i].data[0] = i*i-4*i+5;
      }
      
      //stvaranje dretvi
      for ( i = 0; i < THR_NUM; i++ ) {
		create_thread ( simple_thread, (void *) i, THR_DEFAULT_PRIO - 1, &thread[i] );
      }
      
      delay( &sleep );
      delay( &sleep );
      
      //slanje signala
      for ( i = 0; i < SIG_NUM; i++) {
	    print(" -> Saljem signal %d dretvi %d\n", i%MSG_NUM, i%THR_NUM);
	    send_message (MSG_SIGNAL, &thread[i%THR_NUM], &msg[i%MSG_NUM], 0);
	    if (i%MSG_NUM >= FUNC_NUM)
		print("**Signal tipa %d nece biti obraden jer nije registrirana funkcija za obradu**\n", i%MSG_NUM);
      }

      //čekanje na završetak dretvi
      for ( i = 0; i < THR_NUM; i++ )
		wait_for_thread ( &thread[i], IPC_WAIT );
      
      print( "===Kraj izvodenja===" );
      
      return 0;
} 