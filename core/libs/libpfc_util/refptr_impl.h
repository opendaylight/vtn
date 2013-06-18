/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_REFPTR_IMPL_H
#define	_PFC_LIBPFC_UTIL_REFPTR_IMPL_H

/*
 * Internal definitions for refptr implementation.
 */

#include <pfc/base.h>

PFC_C_BEGIN_DECL

#include <pfc/refptr.h>

/*
 * Builtin refptr operations.
 */

#ifdef	_PFC_LIBPFC_UTIL_BUILD

/*
 * Dummy refptr operation which contains no method.
 * This is used if no refptr operation is specified.
 */
extern const pfc_refptr_ops_t	refptr_default_ops;
extern const pfc_refptr_ops_t	refptr_int32_ops;
extern const pfc_refptr_ops_t	refptr_uint32_ops;
extern const pfc_refptr_ops_t	refptr_int64_ops;
extern const pfc_refptr_ops_t	refptr_uint64_ops;
extern const pfc_refptr_ops_t	refptr_string_ops;

#endif	/* _PFC_LIBPFC_UTIL_BUILD */

/*
 * Prototype for `compare' method.
 */
typedef int		(*refptr_comp_t)(const void *o1, const void *o2);

/*
 * Prototype for `hashfunc' method.
 */
typedef uint32_t	(*refptr_hashfunc_t)(const void *objectx);

/*
 * Allocation statistics.
 */
typedef struct {
	size_t		rst_size;	/* allocation size */
	uint64_t	rst_alloc;	/* number of allocation */
	uint64_t	rst_fail;	/* number of allocation failure */
	uint64_t	rst_free;	/* number of free */
	uint64_t	rst_nstrings;	/* number of cached strings */
	uint64_t	rst_strfail;	/* number of string alloc failure */
} refptr_stat_t;

#ifdef	_PFC_LIBPFC_UTIL_BUILD

/*
 * Internal prototypes.
 */
extern void	pfc_refptr_libinit(void);
extern void	pfc_refptr_fork_prepare(void);
extern void	pfc_refptr_fork_parent(void);
extern void	pfc_refptr_fork_child(void);
extern int	pfc_refptr_compare_default(const void *o1, const void *o2);
extern uint32_t	pfc_refptr_hashfunc_default(const void *object);

/*
 * static inline refptr_comp_t
 * pfc_refptr_get_comparator(const pfc_refptr_ops_t *ops)
 *	Return comparator for the given refptr operation.
 */
static inline refptr_comp_t
pfc_refptr_get_comparator(const pfc_refptr_ops_t *ops)
{
	refptr_comp_t	comp = ops->compare;

	if (comp == NULL) {
		comp = pfc_refptr_compare_default;
	}

	return comp;
}

/*
 * static inline uint32_t
 * pfc_refptr_get_ops_hashfunc(const pfc_refptr_ops_t *ops, const void *key)
 *	Return hash value using the specified refptr operations.
 */
static inline uint32_t
pfc_refptr_ops_hashfunc(const pfc_refptr_ops_t *ops, const void *key)
{
	refptr_hashfunc_t	func = ops->hashfunc;

	if (func == NULL) {
		func = pfc_refptr_hashfunc_default;
	}

	return (*func)(key);
}

/*
 * static inline pfc_bool_t
 * pfc_refptr_ops_equals(const pfc_refptr_ops_t *ops,
 *			 const void *o1, const void *o2)
 *	Determine whether the given two objects are identical using the
 *	specified refptr operations.
 */
static inline pfc_bool_t
pfc_refptr_ops_equals(const pfc_refptr_ops_t *ops,
		      const void *o1, const void *o2)
{
	if (PFC_EXPECT_FALSE(o1 == NULL || o2 == NULL)) {
		return (o1 == o2) ? PFC_TRUE : PFC_FALSE;
	}

	if (ops->equals == NULL) {
		refptr_comp_t	comp = ops->compare;

		if (comp == NULL) {
			comp = pfc_refptr_compare_default;
		}

		return ((*comp)(o1, o2) == 0) ? PFC_TRUE : PFC_FALSE;
	}

	return ops->equals(o1, o2);
}

/*
 * static inline uint32_t
 * refptr_string_hashfunc(const char *str, size_t len)
 *	Compute hash value for the given string.
 */
static inline uint32_t
refptr_string_hashfunc(const char *str, size_t len)
{
	const char	*endp = str + len;
	uint32_t	h = 0;

	for (; str < endp; str++) {
		h = ((h << 5) - h) + *str;
	}

	return h;
}

#endif	/* _PFC_LIBPFC_UTIL_BUILD */

/*
 * Prototypes.
 */
extern void	pfc_refptr_getstat(refptr_stat_t *stp);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_REFPTR_IMPL_H */
