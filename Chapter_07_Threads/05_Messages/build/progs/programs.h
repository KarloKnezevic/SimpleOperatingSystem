int hello_world ( char *args[] );
int timer ( char *args[] );
int keyboard ( char *args[] );
int arguments ( char *args[] );
int shell ( char *args[] );
int user_threads ( char *args[] );
int threads ( char *args[] );
int semaphores ( char *args[] );
int monitors ( char *args[] );
int messages ( char *args[] );
int lab3_knezevic ( char *args[] );
#define PROGRAMS_FOR_SHELL { \
{ hello_world, "hello", " " }, \
{ timer, "timer", " " }, \
{ keyboard, "keyboard", " " }, \
{ arguments, "args", " " }, \
{ shell, "shell", " " }, \
{ user_threads, "uthreads", " " }, \
{ threads, "threads", " " }, \
{ semaphores, "semaphores", " " }, \
{ monitors, "monitors", " " }, \
{ messages, "messages", " " }, \
{ lab3_knezevic, "lab3_knezevic", " " }, \
{NULL,NULL,NULL} }
