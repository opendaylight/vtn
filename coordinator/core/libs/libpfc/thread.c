/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * thread.c - Logical thread on the thread pool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include <pfc/log.h>
#include "thread_impl.h"

/*
 * Hash table that keeps pairs of logical thread ID and thread pool job.
 * We use 2 level hash table for logical thread ID management to reduce
 * lock collision.
 */

#define	PFC_TIDHASH_L1_SHIFT		5U
#define	PFC_TIDHASH_L1_SIZE		(1U << PFC_TIDHASH_L1_SHIFT)
#define	PFC_TIDHASH_L1_MASK		(PFC_TIDHASH_L1_SIZE - 1)
#define	PFC_TIDHASH_L1_INDEX(tid)	((tid) & PFC_TIDHASH_L1_MASK)

#define	PFC_TIDHASH_L2_SHIFT		6U
#define	PFC_TIDHASH_L2_SIZE		(1U << PFC_TIDHASH_L2_SHIFT)
#define	PFC_TIDHASH_L2_MASK		(PFC_TIDHASH_L2_SIZE - 1)
#define	PFC_TIDHASH_L2_INDEX(tid)				\
	(((tid) >> PFC_TIDHASH_L1_SHIFT) & PFC_TIDHASH_L2_MASK)

typedef struct {
	/*
	 * Lock for hash table.
	 * - If you want to acquire both this lock and job mutex,
	 *   you must acquire hash table lock at first.
	 * - If you want to acquire multiple tid_lock, you must acquire
	 *   locks in its address order, from lower to higher.
	 */
	pfc_rwlock_t		tid_lock;
	tpool_job_t		*tid_table[PFC_TIDHASH_L2_SIZE];
} tidhash_t;

static tidhash_t	thread_id_hash[PFC_TIDHASH_L1_SIZE];

/*
 * Set up variables required to look up the thread ID hash.
 *
 * @tid:	Thread ID to be searched.
 * @thp:	Pointer to level 2 hash table is set.
 * @jobpp:	The head of hash collision list is set.
 */
#define	PFC_TIDHASH_SETUP(tid, thp, jobpp)				\
	do {								\
		(thp) = &thread_id_hash[PFC_TIDHASH_L1_INDEX(tid)];	\
		(jobpp) = &((thp)->tid_table[PFC_TIDHASH_L2_INDEX(tid)]); \
	} while (0)

/*
 * Thread ID for the next allocation.
 * This must be incremented by atomic operation.
 */
static pfc_thread_t	thread_id_next;

/*
 * Thread specific data key for the next allocation.
 */
static pfc_tsd_key_t	tsd_key_next;

/*
 * TSD key attributes.
 */
typedef struct {
	pfc_rbnode_t	tk_node;	/* Red-Black Tree node */
	union {
		pfc_tsd_dtor_t		dtor;		/* destructor */
		__pfc_tsd_dtor_t	dtor_arg;	/* dtor with arg */
	} tk_dtor;
	pfc_ptr_t	tk_arg;		/* argument for destructor */
	pfc_bool_t	tk_hasarg;	/* dtor takes tk_arg as argument */
	pfc_tsd_key_t	tk_key;		/* TSD key */
} tsd_keyattr_t;

#define	TSD_KEYATTR_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), tsd_keyattr_t, tk_node)

/*
 * Pair of TSD key and value.
 */
typedef struct {
	pfc_tsd_key_t	tv_key;		/* TSD key */
	pfc_cptr_t	tv_value;	/* value */
	pfc_rbnode_t	tv_node;	/* Red-Black Tree node */
} tsd_value_t;

#define	TSD_VALUE_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), tsd_value_t, tv_node)

/*
 * Current job on the thread.
 */
__thread tpool_job_t	*tpool_job_current PFC_ATTR_HIDDEN;

/*
 * Internal prototypes.
 */
static void	pfc_thread_tsd_cleanup(tpool_job_t *job);
static void	pfc_thread_job_destroy(tpool_job_t *job);
static int	pfc_tidhash_get(pfc_thread_t tid, tpool_job_t **jobp);
static int	pfc_tidhash_put(pfc_thread_t tid, tpool_job_t *job);
static int	pfc_tidhash_remove(pfc_thread_t tid);

static void	pfc_tsd_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
				   tsd_keyattr_t *PFC_RESTRICT attr);
static void	pfc_tsd_value_dtor(pfc_rbnode_t *tvnode, pfc_ptr_t unused);

static pfc_cptr_t	pfc_tsd_keyattr_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	pfc_tsd_value_getkey(pfc_rbnode_t *node);

/*
 * The Red-Black Tree which keeps pairs of thread specific data key and
 * its destructor.
 */
static pfc_rbtree_t	tsd_tree =
	PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare,
			       pfc_tsd_keyattr_getkey);

/*
 * Global lock for TSD key.
 * Any access to tsd_tree must be done with holding this lock.
 */
static pfc_rwlock_t	tsd_lock = PFC_RWLOCK_INITIALIZER;

#define	TSD_KEY_RDLOCK()	PFC_ASSERT_INT(pfc_rwlock_rdlock(&tsd_lock), 0)
#define	TSD_KEY_WRLOCK()	PFC_ASSERT_INT(pfc_rwlock_wrlock(&tsd_lock), 0)
#define	TSD_KEY_UNLOCK()	PFC_ASSERT_INT(pfc_rwlock_unlock(&tsd_lock), 0)

/*
 * Initialize TSD storage per job.
 */
#define	TSD_STORAGE_INIT(job)						\
	pfc_rbtree_init(&(job)->tj_storage, pfc_rbtree_uint32_compare,	\
			pfc_tsd_value_getkey)

/*
 * static inline pfc_thread_t
 * pfc_thread_new_id(void)
 *	Allocate new thread ID.
 *
 * Remarks:
 *	We assume that this function never fails.
 *	We believe that it is impossible to create UINT32_MAX threads
 *	at a time!
 */
static inline pfc_thread_t
pfc_thread_new_id(void)
{
	pfc_thread_t	tid;

	tid = pfc_atomic_inc_uint32_old(&thread_id_next) + 1;
	if (PFC_EXPECT_FALSE(tid == PFC_THREAD_INVALID)) {
		tid = pfc_atomic_inc_uint32_old(&thread_id_next) + 1;
	}

	return tid;
}

/*
 * static inline tpool_job_t **
 * pfc_tidhash_lookup_by_id(tpool_job_t **headpp, pfc_thread_t tid)
 *	Search for a hash entry which contains the given ID in the
 *	thread ID hash collision list.
 *
 * Calling/Exit State:
 *	Pointer which contains pointer to hash entry is returned.
 *	If found, a valid pointer to tpool_job_t is contained in returned
 *	address. If not found, NULL is contained in returned address.
 *	Modifying pointer in returned address affects the hash table.
 *
 * Remarks:
 *	Appropriate lock associated with the given hash collision list
 *	must be acquired by the caller.
 */
static inline tpool_job_t **
pfc_tidhash_lookup_by_id(tpool_job_t **headpp, pfc_thread_t tid)
{
	tpool_job_t	**jobpp;

	for (jobpp = headpp; *jobpp != NULL; jobpp = &((*jobpp)->tj_next)) {
		tpool_job_t	*job = *jobpp;

		if (job->tj_id == tid) {
			break;
		}
	}

	return jobpp;
}

/*
 * static inline void
 * pfc_tsd_value_free(pfc_rbnode_t *node)
 *	Free TSD value entry.
 *	`node' must be a pointer to tv_node in tsd_value_t.
 *	This function does nothing if NULL is specified to `node'.
 */
static inline void
pfc_tsd_value_free(pfc_rbnode_t *node)
{
	if (node != NULL) {
		tsd_value_t	*tvp = TSD_VALUE_NODE2PTR(node);

		free(tvp);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_thread_libinit(void)
 *	Initialize logical thread management layer.
 */
void PFC_ATTR_HIDDEN
pfc_thread_libinit(void)
{
	int		i;
	tidhash_t	*thp;

	for (i = 0, thp = thread_id_hash;
	     (uint32_t)i < PFC_ARRAY_CAPACITY(thread_id_hash); i++, thp++) {
		int	err;

		err = pfc_rwlock_init(&thp->tid_lock);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fprintf(stderr,
				"thread_init: Failed to initialize lock: %s\n",
				strerror(err));
			abort();
			/* NOTREACHED */
		}
	}
}

/*
 * int
 * pfc_thread_create(pfc_thread_t *PFC_RESTRICT thread, pfc_thfunc_t func,
 *		     void *PFC_RESTRICT arg, uint32_t flags)
 *	Create a new thread from the system default thread pool.
 */
int
pfc_thread_create(pfc_thread_t *PFC_RESTRICT thread, pfc_thfunc_t func,
		  void *PFC_RESTRICT arg, uint32_t flags)
{
	return pfc_thread_createat(thread, NULL, func, arg, flags);
}

/*
 * int
 * pfc_thread_createat(pfc_thread_t *PFC_RESTRICT thread,
 *		       const char *PFC_RESTRICT poolname, pfc_thfunc_t func,
 *		       void *PFC_RESTRICT arg, uint32_t flags)
 *	Create a new thread at the thread pool specified by the given
 *	pool name. Default pool is selected if NULL is specified as pool name.
 */
int
pfc_thread_createat(pfc_thread_t *PFC_RESTRICT thread,
		    const char *PFC_RESTRICT poolname, pfc_thfunc_t func,
		    void *PFC_RESTRICT arg, uint32_t flags)
{
	pfc_thread_t	tid;
	tpool_job_t	*job;
	int	err;

	/* Allocate a new job instance. */
	job = (tpool_job_t *)malloc(sizeof(*job));
	if (PFC_EXPECT_FALSE(job == NULL)) {
		return ENOMEM;
	}

	PFC_ASSERT_INT(PFC_MUTEX_INIT(&job->tj_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&job->tj_cond), 0);
	job->tj_func = func;
	job->tj_arg = arg;
	job->tj_status = PFC_THREAD_CANCELED;
	job->tj_flags = (flags & PFC_THREAD_DETACHED)
		? TPOOL_JOB_DETACHED : 0;
	TSD_STORAGE_INIT(job);

	while (1) {
		/* Allocate a new thread ID. */
		tid = pfc_thread_new_id();

		/* Register thread ID and job instance. */
		err = pfc_tidhash_put(tid, job);
		if (PFC_EXPECT_TRUE(err == 0)) {
			break;
		}

		/*
		 * Assigned ID is not available.
		 * We must assign another ID.
		 */
		PFC_ASSERT(err == EEXIST);
	}

	/* Dispatch the job. */
	err = pfc_tpool_dispatch(poolname, job);
	if (PFC_EXPECT_TRUE(err == 0)) {
		*thread = tid;

		return err;
	}

	PFC_ASSERT_INT(pfc_tidhash_remove(tid), 0);
	free(job);

	return err;
}

/*
 * pfc_thread_t
 * pfc_self(void)
 *	Return PFC thread ID of the calling thread.
 *
 * Remarks:
 *	This function must be called on a thread created by pfc_thread_create()
 *	or pfc_thread_createat(). Calling this function on a POSIX
 *	native thread causes undefined behavior.
 */
pfc_thread_t
pfc_thread_self(void)
{
	return pfc_thread_current();
}

/*
 * int
 * pfc_thread_join(pfc_thread_t thread, void **statusp)
 *	Block current thread until the specified thread quits.
 */
int
pfc_thread_join(pfc_thread_t thread, void **statusp)
{
	return pfc_thread_timedjoin(thread, statusp, NULL);
}

/*
 * int
 * pfc_thread_timedjoin(pfc_thread_t thread, void *PFC_RESTRICT *statusp,
 *			const pfc_timespec_t *PFC_RESTICT timeout)
 *	Block current thread until the specified thread quits within
 *	the specified timeout. NULL timeout means no timeout is specified.
 */
int
pfc_thread_timedjoin(pfc_thread_t thread, void *PFC_RESTRICT *statusp,
		     const pfc_timespec_t *PFC_RESTRICT timeout)
{
	tpool_job_t	*job;
	int	err;

	if (PFC_EXPECT_FALSE(pfc_thread_current() == thread)) {
		return EDEADLK;
	}

	/* Obtain job instance associated with the give thread ID. */
	err = pfc_tidhash_get(thread, &job);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Job is locked by pfc_tidhash_get(). */
	if (job->tj_flags & TPOOL_JOB_JOINING) {
		/* Another thread is waiting for this thread. */
		err = EINVAL;
		goto error;
	}
	if (job->tj_flags & TPOOL_JOB_DETACHED) {
		/* Detached thread can't be joined. */
		err = EINVAL;
		goto error;
	}

	job->tj_flags |= TPOOL_JOB_JOINING;

	/* Now wait for the specified job to be finished. */
	if (TPOOL_JOB_IS_ACTIVE(job)) {
		if (timeout == NULL) {
			do {
				TPOOL_JOB_WAIT(job);
				PFC_MEMORY_RELOAD();
			} while (TPOOL_JOB_IS_ACTIVE(job));
		}
		else {
			while (1) {
				err = TPOOL_JOB_TIMEDWAIT(job, timeout);
				PFC_MEMORY_RELOAD();
				if (!TPOOL_JOB_IS_ACTIVE(job)) {
					break;
				}

				if (PFC_EXPECT_FALSE(err != 0)) {
					job->tj_flags &= ~TPOOL_JOB_JOINING;
					goto error;
				}
			}
		}
	}

	if (statusp != NULL) {
		*statusp = job->tj_status;
	}
	TPOOL_JOB_UNLOCK(job);

	pfc_thread_job_destroy(job);

	return 0;

error:
	TPOOL_JOB_UNLOCK(job);

	return err;
}

/*
 * int
 * pfc_thread_detach(pfc_thread_t thread)
 *	Detach the specified thread.
 */
int
pfc_thread_detach(pfc_thread_t thread)
{
	tpool_job_t	*job;
	int	err;

	/* Obtain job instance associated with the give thread ID. */
	err = pfc_tidhash_get(thread, &job);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	if (!TPOOL_JOB_IS_ACTIVE(job)) {
		/* This job is already finished. */
		err = ESRCH;
		goto out;
	}

	if (job->tj_flags & TPOOL_JOB_JOINING) {
		/* Do nothing if another thread is joining the thread. */
		goto out;
	}

	if (job->tj_flags & TPOOL_JOB_DETACHED) {
		/* Already detached. */
		err = EINVAL;
		goto out;
	}

	job->tj_flags |= TPOOL_JOB_DETACHED;

out:
	TPOOL_JOB_UNLOCK(job);

	return err;
}

/*
 * int
 * pfc_thread_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
 *			 pfc_tsd_dtor_t dtor)
 *	Create a PFC thread specific data key which is visible to all PFC
 *	threads. NULL is associated with a new key for all PFC threads.
 *
 *	If a non-NULL function pointer is specified to `dtor', it is associated
 *	with a new key. At PFC thread exit, if a destructor is associated
 *	with a key, and its value for the thread has non-NULL value, destructor
 *	is called with specifying the current value associated with the key.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new key value is set to `*keyp', and
 *	zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_thread_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp, pfc_tsd_dtor_t dtor)
{
	tsd_keyattr_t	*attr;

	/* Allocate key attribute. */
	attr = (tsd_keyattr_t *)malloc(sizeof(*attr));
	if (PFC_EXPECT_FALSE(attr == NULL)) {
		return ENOMEM;
	}

	attr->tk_dtor.dtor = dtor;
	attr->tk_arg = NULL;
	attr->tk_hasarg = PFC_FALSE;

	/* Create a TSD key. */
	pfc_tsd_key_create(keyp, attr);

	return 0;
}

/*
 * int
 * __pfc_thread_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
 *			   __pfc_tsd_dtor_t dtor, pfc_ptr_t *PFC_RESTRICT arg)
 *	Create a PFC thread specific data key which is visible to all PFC
 *	threads.
 *
 *	__pfc_tsd_dtor_t takes two arguments, thread-specific data and
 *	user-specified value. Value specified to `arg' will be passed to the
 *	call of __pfc_tsd_dtor_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new key value is set to `*keyp', and
 *	zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	__pfc_thread_key_create() is provided only for libpfcxx.
 *	Others must not use this.
 */
int
__pfc_thread_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp, __pfc_tsd_dtor_t dtor,
			pfc_ptr_t PFC_RESTRICT arg)
{
	tsd_keyattr_t	*attr;

	/* Allocate key attribute. */
	attr = (tsd_keyattr_t *)malloc(sizeof(*attr));
	if (PFC_EXPECT_FALSE(attr == NULL)) {
		return ENOMEM;
	}

	attr->tk_dtor.dtor_arg = dtor;
	attr->tk_arg = arg;
	attr->tk_hasarg = PFC_TRUE;

	/* Create a TSD key. */
	pfc_tsd_key_create(keyp, attr);

	return 0;
}

/*
 * int
 * pfc_thread_key_delete(pfc_tsd_key_t key)
 *	Delete the specified thread specific data key.
 *
 *	After the call to this function, destructor associated with the key
 *	is not called at any PFC thread exit. An application is responsible
 *	for clean up data associated with the key.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_thread_key_delete(pfc_tsd_key_t key)
{
	pfc_rbnode_t	*node;
	int		err;

	TSD_KEY_WRLOCK();
	node = pfc_rbtree_remove(&tsd_tree, PFC_TSD_KEY(key));
	TSD_KEY_UNLOCK();

	if (PFC_EXPECT_TRUE(node != NULL)) {
		tsd_keyattr_t	*attr = TSD_KEYATTR_NODE2PTR(node);

		/* Release key attributes. */
		free(attr);
		err = 0;
	}
	else {
		err = ENOENT;
	}

	return err;
}

/*
 * int
 * pfc_thread_setspecific(pfc_tsd_key_t key, pfc_cptr_t value)
 *	Associate a thread specific value with a key for the calling thread.
 *	`key' must be a key value previously created by
 *	pfc_thread_key_create().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Calling from thread specific data destructor may cause infinite loop.
 *	So this function returns ELOOP if it is called from TSD destructor.
 */
int
pfc_thread_setspecific(pfc_tsd_key_t key, pfc_cptr_t value)
{
	tpool_job_t	*current = tpool_job_current;
	pfc_rbtree_t	*storage;
	pfc_rbnode_t	*node;
	tsd_value_t	*tvp;

	if (PFC_EXPECT_FALSE(current == NULL)) {
		return EINVAL;
	}

	/*
	 * Check to see whether this function is called from TSD destructor.
	 * We can test TPOOL_JOB_DONE flag without mutex because it is set
	 * by current thread. No other thread set TPOOL_JOB_DONE flag.
	 */
	if (PFC_EXPECT_FALSE(!TPOOL_JOB_IS_ACTIVE(current))) {
		/* Calling pfc_thread_setspecific() may cause infinite loop. */
		return ELOOP;
	}

	storage = &current->tj_storage;
	if (value == NULL) {
		/*
		 * Remove TSD value entry.
		 * At TSD removing, we don't ensure the key is valid so that
		 * an application can remove TSD value associated with a key
		 * deleted by pfc_thread_key_delete().
		 */
		node = pfc_rbtree_remove(storage, PFC_TSD_KEY(key));
		pfc_tsd_value_free(node);

		return 0;
	}

	/* Allocate a new TSD value entry. */
	tvp = (tsd_value_t *)malloc(sizeof(*tvp));
	if (PFC_EXPECT_FALSE(tvp == NULL)) {
		return ENOMEM;
	}
	tvp->tv_key = key;
	tvp->tv_value = value;

	/* Ensure that the specified key is valid. */
	TSD_KEY_RDLOCK();
	if (PFC_EXPECT_FALSE(pfc_rbtree_get(&tsd_tree, PFC_TSD_KEY(key))
			     == NULL)) {
		TSD_KEY_UNLOCK();
		free(tvp);

		return ENOENT;
	}

	/*
	 * Associate the specified value with the key.
	 * The TSD key lock should be held here in order to protect against
	 * removing the TSD key.
	 */
	node = pfc_rbtree_update(storage, &tvp->tv_node);
	TSD_KEY_UNLOCK();

	pfc_tsd_value_free(node);

	return 0;
}

/*
 * pfc_ptr_t
 * pfc_thread_getspecific(pfc_tsd_key_t key)
 *	Return a thread specific value associated with a key for calling thread.
 *
 * Remarks:
 *	This function doesn't check whether the specified key is valid or not,
 *	so that an application can retrieve TSD value associated with a key
 *	deleted by pfc_thread_key_delete().
 */
pfc_ptr_t
pfc_thread_getspecific(pfc_tsd_key_t key)
{
	tpool_job_t	*current = tpool_job_current;
	pfc_rbnode_t	*node;
	pfc_ptr_t	value;

	if (PFC_EXPECT_FALSE(current == NULL)) {
		return NULL;
	}

	/* Search for a key in the thread specific storage. */
	node = pfc_rbtree_get(&current->tj_storage, PFC_TSD_KEY(key));
	if (node != NULL) {
		tsd_value_t	*tvp = TSD_VALUE_NODE2PTR(node);

		value = (pfc_ptr_t)tvp->tv_value;
	}
	else {
		value = NULL;
	}

	return value;
}

/*
 * int
 * pfc_thread_once(pfc_once_t *PFC_RESTRICT control,
 *		   pfc_once_init_t init_routine)
 *	Call function specified to `init' only once.
 *
 *	The caller must initialize `control' value as PFC_THREAD_ONCE_INIT
 *	in advance. Even if two or more threads call pfc_thread_once() with
 *	the same parameter, it is guaranteed that only one thread calls
 *	the `init' function, and subsequent calls don't.
 *
 * Calling/Exit State:
 *	Upon successful completion, pfc_thread_once() returns zero.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_thread_once(pfc_once_t *PFC_RESTRICT control, pfc_once_init_t init_routine)
{
	if (PFC_EXPECT_FALSE(control == NULL || init_routine == NULL)) {
		return EINVAL;
	}

	return pthread_once(control, init_routine);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_thread_dispatch(tpool_job_t *job)
 *	Dispatch the specified job.
 *	This function must be called on a physical thread in a thread pool.
 */
void PFC_ATTR_HIDDEN
pfc_thread_dispatch(tpool_job_t *job)
{
	void	*status;

	/* Disable cancellation. */
	(void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	/* Install new job. */
	tpool_job_current = job;

	/* Call job function. */
	status = job->tj_func(job->tj_arg);

	/* Clean up the job. */
	pfc_thread_cleanup(job, status);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_thread_cleanup(tpool_job_t *PFC_RESTRICT job, void *PFC_RESTRICT status)
 *	Clean up the specified job.
 */
void PFC_ATTR_HIDDEN
pfc_thread_cleanup(tpool_job_t *PFC_RESTRICT job, void *PFC_RESTRICT status)
{
	PFC_ASSERT(job != NULL);

	/* At first, mark this job as invalid. */
	TPOOL_JOB_LOCK(job);
	job->tj_flags |= TPOOL_JOB_DONE;
	job->tj_status = status;

	/* Clean up TSD. */
	pfc_thread_tsd_cleanup(job);

	if (job->tj_flags & TPOOL_JOB_DETACHED) {
		/* We can free job instance here. */
		TPOOL_JOB_UNLOCK(job);
		pfc_thread_job_destroy(job);
	}
	else {
		/*
		 * This thread must be joined.
		 * Job instance must be released by pfc_thread_timedjoin().
		 */
		if (job->tj_flags & TPOOL_JOB_JOINING) {
			/* Wake up joining thread. */
			TPOOL_JOB_SIGNAL(job);
		}

		TPOOL_JOB_UNLOCK(job);
	}

	tpool_job_current = NULL;
}

/*
 * static void
 * pfc_thread_tsd_cleanup(tpool_job_t *job)
 *	Clean up all thread specific data for the given thread.
 *
 * Remarks:
 *	This function don't acquire any lock.
 *	The caller must guarantee that the given job is not active.
 */
static void
pfc_thread_tsd_cleanup(tpool_job_t *job)
{
	pfc_rbtree_t	storage;

	PFC_ASSERT(!TPOOL_JOB_IS_ACTIVE(job));

	/* Copy TSD storage tree. */
	storage = job->tj_storage;

	/*
	 * TSD storage needs to be initialized before calling TSD destructor
	 * in order to make TSD invisible to TSD destructor.
	 */
	TSD_STORAGE_INIT(job);

	/* Clean up TSD storage, and call TSD destructor. */
	pfc_rbtree_clear(&storage, pfc_tsd_value_dtor, NULL);
}

/*
 * static void
 * pfc_thread_job_destroy(tpool_job_t *job)
 *	Destroy the given job.
 */
static void
pfc_thread_job_destroy(tpool_job_t *job)
{
	/* Unregister thread ID. */
	PFC_ASSERT_INT(pfc_tidhash_remove(job->tj_id), 0);

	/* Release job instance. */
	free(job);
}

/*
 * static int
 * pfc_tidhash_get(pfc_thread_t tid, tpool_job_t **jobp)
 *	Get job instance associated with the thread ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, pointer to job instance is set to *jobp
 *	and zero is returned. Returned job instance is always locked.
 *	Unlocking the job instance is up to the caller.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
static int
pfc_tidhash_get(pfc_thread_t tid, tpool_job_t **jobp)
{
	tidhash_t	*thp;
	tpool_job_t	*job, **jobpp;

	PFC_TIDHASH_SETUP(tid, thp, jobpp);

	/* Acquire hash lock in reader mode. */
	pfc_rwlock_rdlock(&thp->tid_lock);

	/* Search for a hash entry which contains the given thread ID. */
	jobpp = pfc_tidhash_lookup_by_id(jobpp, tid);
	if (PFC_EXPECT_FALSE((job = *jobpp) == NULL)) {
		pfc_rwlock_unlock(&thp->tid_lock);

		return ESRCH;
	}

	*jobp = job;

	TPOOL_JOB_LOCK(job);
	pfc_rwlock_unlock(&thp->tid_lock);

	return 0;
}

/*
 * static int
 * pfc_tidhash_put(pfc_thread_t tid, tpool_job_t *job)
 *	Register the given thread ID and job instance into the thread ID
 *	hash table.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	EEXIST is returned if the given thread ID exists.
 */
static int
pfc_tidhash_put(pfc_thread_t tid, tpool_job_t *job)
{
	tidhash_t	*thp;
	tpool_job_t	**jobpp;
	int	err = 0;

	PFC_TIDHASH_SETUP(tid, thp, jobpp);

	/* Acquire hash lock in writer mode. */
	pfc_rwlock_wrlock(&thp->tid_lock);

	/* Ensure that the given ID is available. */
	jobpp = pfc_tidhash_lookup_by_id(jobpp, tid);
	if (PFC_EXPECT_FALSE(*jobpp != NULL)) {
		err = EEXIST;
		goto out;
	}

	/* Put a new entry into the hash collision list. */
	*jobpp = job;
	job->tj_next = NULL;
	job->tj_id = tid;

out:
	pfc_rwlock_unlock(&thp->tid_lock);

	return err;
}

/*
 * static int
 * pfc_tidhash_remove(pfc_thread_t tid)
 *	Remove the given thread ID from the thread ID hash table.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	ESRCH is returned if the thread ID does not exist in the hash table.
 */
static int
pfc_tidhash_remove(pfc_thread_t tid)
{
	tidhash_t	*thp;
	tpool_job_t	*job, **jobpp;

	PFC_TIDHASH_SETUP(tid, thp, jobpp);

	/* Acquire hash lock in writer mode. */
	pfc_rwlock_wrlock(&thp->tid_lock);

	/* Search for a hash entry which contains the given thread ID. */
	jobpp = pfc_tidhash_lookup_by_id(jobpp, tid);
	if (PFC_EXPECT_FALSE(*jobpp == NULL)) {
		pfc_rwlock_unlock(&thp->tid_lock);

		return ESRCH;
	}

	/* Remove this hash entry. */
	job = *jobpp;
	*jobpp = job->tj_next;
	pfc_rwlock_unlock(&thp->tid_lock);

	return 0;
}

/*
 * static void
 * pfc_tsd_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
 *		      tsd_keyattr_t *PFC_RESTRICT attr)
 *	Create a PFC thread specific data key.
 *	`attr' must be a pointer to key attributes.
 */
static void
pfc_tsd_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
		   tsd_keyattr_t *PFC_RESTRICT attr)
{
	while (1) {
		pfc_tsd_key_t	key;
		int		err;

		/* Assign new key. */
		key = pfc_atomic_inc_uint32_old(&tsd_key_next) + 1;
		if (PFC_EXPECT_FALSE(key == PFC_TSD_KEY_INVALID)) {
			key = pfc_atomic_inc_uint32_old(&tsd_key_next) + 1;
		}

		/* Record a pair of key and its attributes. */
		attr->tk_key = key;
		TSD_KEY_WRLOCK();
		err = pfc_rbtree_put(&tsd_tree, &attr->tk_node);
		TSD_KEY_UNLOCK();
		if (PFC_EXPECT_TRUE(err == 0)) {
			*keyp = key;
			break;
		}
		PFC_ASSERT(err == EEXIST);
	}
}

/*
 * static void
 * pfc_tsd_value_dtor(pfc_rbnode_t *tvnode, pfc_ptr_t arg)
 *	Destructor of TSD value.
 *	`node' must be a pointer to tv_node in tsd_value_t.
 *
 *	This function is called by thread cleanup routine in order to call
 *	TSD destructor registered by the call of pfc_thread_key_create().
 */
static void
pfc_tsd_value_dtor(pfc_rbnode_t *tvnode, pfc_ptr_t PFC_ATTR_UNUSED unused)
{
	__pfc_tsd_dtor_t	dtor_arg = NULL;
	pfc_tsd_dtor_t	dtor = NULL;
	pfc_ptr_t	arg = NULL;
	tsd_value_t	*tvp = TSD_VALUE_NODE2PTR(tvnode);
	pfc_rbnode_t	*node;

	/*
	 * Retrieve TSD destructor.
	 * Note that we must copy pointer to destructor and argument in
	 * synchronized block, or they may be deleted by the call of
	 * pfc_thread_key_delete().
	 */
	TSD_KEY_RDLOCK();
	node = pfc_rbtree_get(&tsd_tree, PFC_TSD_KEY(tvp->tv_key));
	if (node != NULL) {
		tsd_keyattr_t	*attr = TSD_KEYATTR_NODE2PTR(node);

		if (attr->tk_hasarg) {
			dtor_arg = attr->tk_dtor.dtor_arg;
		}
		else {
			dtor = attr->tk_dtor.dtor;
		}
		arg = attr->tk_arg;
	}
	TSD_KEY_UNLOCK();

	if (dtor_arg != NULL) {
		(*dtor_arg)((pfc_ptr_t)tvp->tv_value, arg);
	}
	else if (dtor != NULL) {
		(*dtor)((pfc_ptr_t)tvp->tv_value);
	}

	/* Free TSD value entry. */
	pfc_tsd_value_free(tvnode);
}

/*
 * static pfc_cptr_t
 * pfc_tsd_keyattr_getkey(pfc_rbnode_t *node)
 *	Return the key of TSD key attribute specified by `node'.
 *	`node' must be a pointer to tk_node in tsd_keyattr_t.
 */
static pfc_cptr_t
pfc_tsd_keyattr_getkey(pfc_rbnode_t *node)
{
	tsd_keyattr_t	*attr = TSD_KEYATTR_NODE2PTR(node);

	return PFC_TSD_KEY(attr->tk_key);
}

/*
 * static pfc_cptr_t
 * pfc_tsd_value_getkey(pfc_rbnode_t *node)
 *	Return the key of TSD value entry specified by `node'.
 *	`node' must be a pointer to tv_node in tsd_value_t.
 */
static pfc_cptr_t
pfc_tsd_value_getkey(pfc_rbnode_t *node)
{
	tsd_value_t	*tvp = TSD_VALUE_NODE2PTR(node);

	return PFC_TSD_KEY(tvp->tv_key);
}
