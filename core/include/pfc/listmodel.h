/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LISTMODEL_H
#define	_PFC_LISTMODEL_H

/*
 * Definitions for object list models.
 */

#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/hash.h>
#include <pfc/syncblock.h>
#include <pfc/clock.h>
#include <errno.h>

PFC_C_BEGIN_DECL

/*
 * Type for abstract list model.
 */
struct __pfc_listm;
typedef struct __pfc_listm	*pfc_listm_t;

/*
 * Invalid list model instance.
 */
#define	PFC_LISTM_INVALID	((pfc_listm_t)NULL)

/*
 * Identifier of list implementation.
 */
typedef enum {
	PFC_LISTM_LINKED	= 0,		/* linked list */
	PFC_LISTM_VECTOR,			/* vector */
	PFC_LISTM_HASH,				/* hash list */
} pfc_listm_type_t;

/*
 * Abstract list iterator.
 */
struct __pfc_listiter;
typedef struct __pfc_listiter	*pfc_listiter_t;

/*
 * Element indices which have special meaning.
 */
#define	PFC_LISTIDX_NOENT	(-1)		/* no more element */
#define	PFC_LISTIDX_CHANGED	(-2)		/* list has been changed */
#define	PFC_LISTIDX_BAD		(-3)		/* list is already destroyed */
#define	PFC_LISTIDX_INVALID	(-4)		/* invalid type of list */
#define	PFC_LISTIDX_NOMEM	(-5)		/* no memory */

/*
 * Maximum number of elements supported by vector implementation.
 */
#define	PFC_VECTOR_MAX_CAPACITY		0x10000000

/*
 * Maximum capacity increment value for vector.
 */
#define	PFC_VECTOR_MAX_CAPINC		0x10000

/*
 * Default parameters for vector implementation.
 */
#define	PFC_VECTOR_DEFAULT_CAPACITY	16
#define	PFC_VECTOR_DEFAULT_CAPINC	16

/*
 * Prototype for sort comparator.
 */
typedef int	(*pfc_listcomp_t)(pfc_cptr_t v1, pfc_cptr_t v2);

/*
 * Prototypes for common list operations.
 */
extern int	pfc_listm_push(pfc_listm_t PFC_RESTRICT list,
			       pfc_cptr_t PFC_RESTRICT value);
extern int	pfc_listm_push_tail(pfc_listm_t PFC_RESTRICT list,
				    pfc_cptr_t PFC_RESTRICT value);
extern int	__pfc_listm_pop(pfc_listm_t PFC_RESTRICT list,
				pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_listm_pop_tail(pfc_listm_t PFC_RESTRICT list,
				     pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_listm_first(pfc_listm_t PFC_RESTRICT list,
				  pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_listm_last(pfc_listm_t PFC_RESTRICT list,
				 pfc_cptr_t *PFC_RESTRICT valuep);
extern int	pfc_listm_index(pfc_listm_t PFC_RESTRICT list,
				pfc_cptr_t PFC_RESTRICT value);
extern int	pfc_listm_index_tail(pfc_listm_t PFC_RESTRICT list,
				     pfc_cptr_t PFC_RESTRICT value);
extern int	pfc_listm_remove(pfc_listm_t PFC_RESTRICT list,
				 pfc_cptr_t PFC_RESTRICT value);
extern int	pfc_listm_remove_tail(pfc_listm_t PFC_RESTRICT list,
				      pfc_cptr_t PFC_RESTRICT value);
extern int	__pfc_listm_getat(pfc_listm_t PFC_RESTRICT list, int index,
				  pfc_cptr_t *PFC_RESTRICT valuep);
extern int	pfc_listm_insertat(pfc_listm_t PFC_RESTRICT list, int index,
				   pfc_cptr_t PFC_RESTRICT value);
extern int	__pfc_listm_removeat(pfc_listm_t PFC_RESTRICT list, int index,
				     pfc_cptr_t *PFC_RESTRICT valuep);
extern int	pfc_listm_clear(pfc_listm_t list);
extern int	pfc_listm_sort_comp(pfc_listm_t list, pfc_listcomp_t comp);
extern int	pfc_listm_get_size(pfc_listm_t list);
extern pfc_listm_type_t	pfc_listm_get_type(pfc_listm_t list);
extern void	pfc_listm_destroy(pfc_listm_t list);
extern void	__pfc_listm_sync_begin(__pfc_syncblock_t *frame);
extern void	__pfc_listm_sync_end(void *arg);

extern pfc_listiter_t	pfc_listiter_create(pfc_listm_t list);
extern pfc_listiter_t	pfc_listiter_create_range(pfc_listm_t list, int from,
						  int to);
extern void	pfc_listiter_destroy(pfc_listiter_t it);
extern int	__pfc_listiter_next(pfc_listiter_t PFC_RESTRICT it,
				    pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_listiter_prev(pfc_listiter_t PFC_RESTRICT it,
				    pfc_cptr_t *PFC_RESTRICT valuep);
extern int	pfc_listiter_next_index(pfc_listiter_t it);
extern int	pfc_listiter_prev_index(pfc_listiter_t it);
extern int	pfc_listiter_rewind(pfc_listiter_t it);
extern int	pfc_listiter_eol(pfc_listiter_t it);
extern int	pfc_listiter_remove(pfc_listiter_t it);

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_listm_sort(pfc_listm_t list)
 *	Sort elements in the list using default comparator.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_sort(pfc_listm_t list)
{
	return pfc_listm_sort_comp(list, NULL);
}

/*
 * Prototypes for linked list.
 */
extern int	pfc_llist_create(pfc_listm_t *listp);
extern int	pfc_llist_create_ref(pfc_listm_t *PFC_RESTRICT listp,
				     const pfc_refptr_ops_t
				     *PFC_RESTRICT refops);
extern int	__pfc_llist_pop_wait(pfc_listm_t PFC_RESTRICT list,
				     pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_llist_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
					  pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_llist_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
					  pfc_cptr_t *PFC_RESTRICT valuep,
					  const pfc_timespec_t
					  *PFC_RESTRICT timeout);
extern int	__pfc_llist_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
					       pfc_cptr_t *PFC_RESTRICT valuep,
					       const pfc_timespec_t
					       *PFC_RESTRICT timeout);
extern void	pfc_llist_shutdown(pfc_listm_t list, pfc_bool_t clear);

/*
 * Prototypes for vector.
 */
extern int	pfc_vector_create(pfc_listm_t *listp, int capacity, int capinc);
extern int	pfc_vector_create_ref(pfc_listm_t *PFC_RESTRICT listp,
				      const pfc_refptr_ops_t
				      *PFC_RESTRICT refops,
				      int capacity, int capinc);
extern int	pfc_vector_get_capacity(pfc_listm_t list);
extern int	pfc_vector_set_capacity(pfc_listm_t list, int capacity);
extern int	pfc_vector_get_capacity_inc(pfc_listm_t list);
extern int	pfc_vector_set_capacity_inc(pfc_listm_t list, int capinc);
extern int	pfc_vector_set_size(pfc_listm_t list, int size);
extern int	pfc_vector_trim(pfc_listm_t list);
extern int	pfc_vector_pack(pfc_listm_t list);
extern int	pfc_vector_setat(pfc_listm_t PFC_RESTRICT list, int index,
				 pfc_cptr_t PFC_RESTRICT value);
extern int	pfc_vector_updateat(pfc_listm_t PFC_RESTRICT list, int index,
				    pfc_cptr_t PFC_RESTRICT value);

/*
 * Vector constructor with default parameters.
 */
#define	PFC_VECTOR_CREATE(listp)					\
	(pfc_vector_create((listp), PFC_VECTOR_DEFAULT_CAPACITY,	\
			   PFC_VECTOR_DEFAULT_CAPINC))

#define	PFC_VECTOR_CREATE_REF(listp, refops)				\
	(pfc_vector_create_ref((listp), (refops),			\
			       PFC_VECTOR_DEFAULT_CAPACITY,		\
			       PFC_VECTOR_DEFAULT_CAPINC))

/*
 * Prototypes for hash list.
 */
extern int	pfc_hashlist_create(pfc_listm_t *PFC_RESTRICT listp,
				    const pfc_hash_ops_t *PFC_RESTRICT ops,
				    uint32_t nbuckets, uint32_t hflags);
extern int	pfc_hashlist_create_ref(pfc_listm_t *PFC_RESTRICT listp,
					const pfc_refptr_ops_t
					*PFC_RESTRICT refops,
					uint32_t nbuckets, uint32_t hflags);
extern int	pfc_hashlist_get_capacity(pfc_listm_t list);
extern int	pfc_hashlist_set_capacity(pfc_listm_t list, int capacity);

/*
 * The following list operations are implemented as inline function
 * to embed destination object size check.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_pop(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listm_pop(list, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_pop_tail(pfc_listm_t PFC_RESTRICT list,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listm_pop_tail(list, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_first(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listm_first(list, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_last(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listm_last(list, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_getat(pfc_listm_t PFC_RESTRICT list, int index,
		pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listm_getat(list, index, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listm_removeat(pfc_listm_t PFC_RESTRICT list, int index,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listm_removeat(list, index, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listiter_next(pfc_listiter_t PFC_RESTRICT it,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listiter_next(it, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_listiter_prev(pfc_listiter_t PFC_RESTRICT it,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_listiter_prev(it, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_llist_pop_wait(pfc_listm_t PFC_RESTRICT list,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_llist_pop_wait(list, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_llist_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
			pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_llist_pop_tail_wait(list, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_llist_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
			pfc_cptr_t *PFC_RESTRICT valuep,
			const pfc_timespec_t *PFC_RESTRICT timeout)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_llist_pop_timedwait(list, valuep, timeout);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_llist_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
			     pfc_cptr_t *PFC_RESTRICT valuep,
			     const pfc_timespec_t *PFC_RESTRICT timeout)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_llist_pop_tail_timedwait(list, valuep, timeout);
}

/*
 * Start synchronized block on the list instance.
 */
#define	pfc_listm_wsync_begin(list)				\
	__pfc_wsync_block_begin(list, __pfc_listm_sync_begin,	\
				__pfc_listm_sync_end)
#define	pfc_listm_rsync_begin(list)				\
	__pfc_rsync_block_begin(list, __pfc_listm_sync_begin,	\
				__pfc_listm_sync_end)
#define	pfc_listm_sync_end()	__pfc_sync_block_end()

/*
 * Builtin 64-bit integer list support.
 */
extern int	pfc_listm64_push(pfc_listm_t list, uint64_t value);
extern int	pfc_listm64_push_tail(pfc_listm_t list, uint64_t value);
extern int	pfc_listm64_pop(pfc_listm_t PFC_RESTRICT list,
				uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listm64_pop_tail(pfc_listm_t PFC_RESTRICT list,
				     uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listm64_first(pfc_listm_t PFC_RESTRICT list,
				  uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listm64_last(pfc_listm_t PFC_RESTRICT list,
				 uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listm64_index(pfc_listm_t list, uint64_t value);
extern int	pfc_listm64_index_tail(pfc_listm_t list, uint64_t value);
extern int	pfc_listm64_remove(pfc_listm_t list, uint64_t value);
extern int	pfc_listm64_remove_tail(pfc_listm_t list, uint64_t value);
extern int	pfc_listm64_getat(pfc_listm_t PFC_RESTRICT list, int index,
				  uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listm64_insertat(pfc_listm_t PFC_RESTRICT list, int index,
				     uint64_t value);
extern int	pfc_listm64_removeat(pfc_listm_t PFC_RESTRICT list, int index,
				     uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listiter64_next(pfc_listiter_t PFC_RESTRICT it,
				    uint64_t *PFC_RESTRICT valuep);
extern int	pfc_listiter64_prev(pfc_listiter_t PFC_RESTRICT it,
				    uint64_t *PFC_RESTRICT valuep);

/* Linked list which contains 64-bit integer */
extern int	pfc_llist_create_i64(pfc_listm_t *listp);
extern int	pfc_llist_create_u64(pfc_listm_t *listp);

extern int	pfc_llist64_pop_wait(pfc_listm_t PFC_RESTRICT list,
				     uint64_t *PFC_RESTRICT valuep);
extern int	pfc_llist64_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
					  uint64_t *PFC_RESTRICT valuep,
					  const pfc_timespec_t
					  *PFC_RESTRICT timeout);
extern int	pfc_llist64_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
					  uint64_t *PFC_RESTRICT valuep);
extern int	pfc_llist64_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
					       uint64_t *PFC_RESTRICT valuep,
					       const pfc_timespec_t
					       *PFC_RESTRICT timeout);

/* Vector which contains 64-bit integer */
extern int	pfc_vector_create_i64(pfc_listm_t *listp, int capacity,
				      int capinc);
extern int	pfc_vector_create_u64(pfc_listm_t *listp, int capacity,
				      int capinc);
extern int	pfc_vector64_setat(pfc_listm_t list, int index,
				   uint64_t value);
extern int	pfc_vector64_updateat(pfc_listm_t list, int index,
				      uint64_t value);

#define	PFC_VECTOR_CREATE_I64(listp)					\
	(pfc_vector_create_i64((listp), PFC_VECTOR_DEFAULT_CAPACITY,	\
			       PFC_VECTOR_DEFAULT_CAPINC))
#define	PFC_VECTOR_CREATE_U64(listp)					\
	(pfc_vector_create_u64((listp), PFC_VECTOR_DEFAULT_CAPACITY,	\
			       PFC_VECTOR_DEFAULT_CAPINC))

/* Hash list which contains 64-bit integer */
extern int	pfc_hashlist_create_i64(pfc_listm_t *PFC_RESTRICT listp,
					uint32_t nbuckets, uint32_t hflags);
extern int	pfc_hashlist_create_u64(pfc_listm_t *PFC_RESTRICT listp,
					uint32_t nbuckets, uint32_t hflags);

PFC_C_END_DECL

#endif	/* !_PFC_LISTMODEL_H */
