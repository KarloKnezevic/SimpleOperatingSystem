/*! Error numbers, macros, ... */

#pragma once

#include <lib/errno.h>
#include <kernel/thread.h>
#include <kernel/kprint.h>
#include <arch/processor.h>


/* error number is defined per thread (saved in thread descriptor) */

/* return from (internal kernel) function */
#define RETURN(ENUM)	return (-ENUM)

/* set errno */
#define SET_ERRNO(ENUM)		k_set_errno (-ENUM)

/* return error code */
#define EXIT(ENUM)	do { SET_ERRNO(ENUM); RETURN(ENUM); } while (0)

/* syscall enter procedures (mark IE flag and dissable interrupts) */
#define SYS_ENTRY()		int __FUNCTION__ ## ei = set_interrupts (FALSE)

/* return from syscall (save exit status as errno) */
#define SYS_EXIT(ENUM)	do {						\
				SET_ERRNO(ENUM);			\
				set_interrupts (__FUNCTION__ ## ei);	\
				RETURN(ENUM);				\
			} while (0)

/* return from syscall (return value) */
#define SYS_RETURN(RETVAL)	\
			do {						\
				SET_ERRNO(SUCCESS);			\
				set_interrupts (__FUNCTION__ ## ei);	\
				return(RETVAL);				\
			} while (0)


#ifdef DEBUG

/*! Debugging outputs (includes files and line numbers!) */
#define LOG(LEVEL, format, ...)	\
kprint ( "[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/*! Critical error - print it and stop */
#define ASSERT(expr)						\
do if ( !( expr ) )						\
{								\
	kprint ( "[BUG:%s:%d]\n", __FILE__, __LINE__);		\
	halt();							\
} while(0)

/* assert and return (inter kernel calls) */
#define ASSERT_ERRNO_AND_RETURN(expr, errnum) \
do { if ( !( expr ) ) RETURN ( errnum ); } while(0)

/* assert and return errno */
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)	\
do {	if ( !( expr ) )			\
	{					\
		LOG ( ASSERT, "\n");		\
		EXIT ( errnum );		\
	}					\
} while(0)

/* assert and return from syscall */
#define ASSERT_ERRNO_AND_SYS_EXIT(expr, errnum)	\
do {	if ( !( expr ) )			\
	{					\
		LOG ( ASSERT, "\n");		\
		SYS_EXIT ( errnum );		\
	}					\
} while(0)

#else /* !DEBUG */

#define SYS_ENTRY()
#define SYS_EXIT(ENUM)		RETURN(ENUM)
#define SYS_RETURN(RETVAL)	return(RETVAL)
#define EXIT(ENUM)		RETURN(ENUM)

#define ASSERT(expr)
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)
#define ASSERT_ERRNO_AND_RETURN(expr, errnum)
#define ASSERT_ERRNO_AND_SYS_EXIT(expr, errnum)
#define LOG(LEVEL, format, ...)

#endif /* DEBUG */
