/* 
 * Copyright (c) 2000-2005 by Hewlett-Packard Company.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

/* Included indirectly from a thread-library-specific file.	*/
/* This is the interface for thread-local allocation, whose	*/
/* implementation is mostly thread-library-independent.		*/
/* Here we describe only the interface that needs to be known	*/
/* and invoked from the thread support layer;  the actual	*/
/* implementation also exports GC_malloc and friends, which	*/
/* are declared in gc.h.					*/

/* One of these should be declared as the tlfs field in the	*/
/* structure pointed to by a GC_thread.				*/
typedef struct thread_local_freelists {
#   ifdef THREAD_LOCAL_ALLOC
	void * ptrfree_freelists[TINY_FREELISTS];
	void * normal_freelists[TINY_FREELISTS];
#	ifdef GC_GCJ_SUPPORT
	  void * gcj_freelists[TINY_FREELISTS];
#	endif
		/* Free lists contain either a pointer or a small count */
		/* reflecting the number of granules allocated at that	*/
		/* size.						*/
		/* 0 ==> thread-local allocation in use, free list	*/
		/*       empty.						*/
		/* > 0, <= DIRECT_GRANULES ==> Using global allocation,	*/
		/*       too few objects of this size have been		*/
		/* 	 allocated by this thread.			*/
		/* >= HBLKSIZE  => pointer to nonempty free list.	*/
		/* > DIRECT_GRANULES, < HBLKSIZE ==> transition to	*/
		/*    local alloc, equivalent to 0.			*/
#	define DIRECT_GRANULES (HBLKSIZE/GRANULE_BYTES)
		/* Don't use local free lists for up to this much 	*/
		/* allocation.						*/

#   endif
} *GC_tlfs;

# if defined(USE_PTHREAD_SPECIFIC)
#   define GC_getspecific pthread_getspecific
#   define GC_setspecific pthread_setspecific
#   define GC_key_create pthread_key_create
#   define GC_remove_specific()  /* No need for cleanup on exit. */
    typedef pthread_key_t GC_key_t;
# elif defined(USE_COMPILER_TLS)
#   define GC_getspecific(x) (x)
#   define GC_setspecific(key, v) ((key) = (v), 0)
#   define GC_key_create(key, d) 0
#   define GC_remove_specific()  /* No need for cleanup on exit. */
    typedef void * GC_key_t;
# elif defined(USE_WIN32_SPECIFIC)
#   define GC_getspecific TlsGetValue
#   define GC_setspecific TlsSetValue
#   define GC_key_create(key, d)  \
	((d) != 0? (ABORT("Destructor unsupported by TlsAlloc"),0) \
	 	 : (*(key) = TlsAlloc(), 0))
#   define GC_remove_specific()  /* No need for cleanup on thread exit. */
    	/* Need TlsFree on process exit/detach ? */
# else
#   include "private/specific.h"
# endif


/* Each thread structure must be initialized.	*/
/* This call must be made from the new thread.	*/
/* Caller holds allocation lock.		*/
void GC_init_thread_local(GC_tlfs p);

/* Called when a thread is unregistered, or exits.	*/
/* We hold the allocator lock.				*/
void GC_destroy_thread_local(GC_tlfs p);

/* The thread support layer must arrange to mark thread-local	*/
/* free lists explicitly, since the link field is often 	*/
/* invisible to the marker.  It knows hoe to find all threads;	*/
/* we take care of an individual thread freelist structure.	*/
void GC_mark_thread_local_fls_for(GC_tlfs p);

#ifdef USE_COMPILER_TLS
  __thread
#endif
GC_key_t GC_thread_key;
/* This is set up by the thread_local_alloc implementation.  But the	*/
/* thread support layer calls GC_remove_specific(GC_thread_key)		*/
/* before a thread exits.						*/
/* And the thread support layer makes sure that GC_thread_key is traced,*/
/* if necessary.							*/


