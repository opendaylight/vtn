/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_REFPTR_H
#define	_PFC_REFPTR_H

/*
 * refptr.h - Definitions for pointer with reference counter.
 *
 * This is a simple implementation of object management based on reference
 * counter. Each refptr has a reference counter and object pointer.
 * Reference counter can be incremented by pfc_refptr_get(), and can be
 * decremented by pfc_refptr_put(). An object will be destroyed if its
 * reference counter becomes zero.
 */

#include <pfc/base.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include <stdlib.h>
#include <stdarg.h>

PFC_C_BEGIN_DECL

/*
 * Operations for pointer to abstract object with reference counter.
 */
typedef struct {
	/*
	 * void
	 * dtor(pfc_ptr_t object)  (optional)
	 *	Destructor of object.
	 *	This function is called when a reference counter becomes zero,
	 *	and it must release all resources grabbed by the given object.
	 *
	 *	If NULL, nothing will be done even if reference counter
	 *	becomes zero.
	 */
	void		(*dtor)(pfc_ptr_t object);

	/*
	 * int
	 * compare(pfc_cptr_t o1, pfc_cptr_t o2)  (optional)
	 *	Compare object value.
	 *	Zero must be returned if `o1' and `o2' has the same value,
	 *	negative value if `o1' is less than 'o2', and positive value
	 *	if `o1' is greater than `o2'.
	 *
	 *	If NULL, object address is considered as its value.
	 */
	int		(*compare)(pfc_cptr_t o1, pfc_cptr_t o2);

	/*
	 * pfc_bool_t
	 * equals(pfc_cptr_t o1, pfc_cptr_t o2)  (optional)
	 *	Determine whether the given two object is identical or not.
	 *	PFC_TRUE is returned if identical, otherwise PFC_FALSE.
	 *
	 *	If NULL, compare method is used.
	 */
	pfc_bool_t	(*equals)(pfc_cptr_t o1, pfc_cptr_t o2);

	/*
	 * uint32_t
	 * hashfunc(pfc_cptr_t object)  (optional)
	 *	Return hash value associated with the given object.
	 *	This function is used when refptr is used has hash key of
	 *	pfc_hash_t.
	 *
	 *	If NULL, hash value is derived from object address.
	 */
	uint32_t	(*hashfunc)(pfc_cptr_t object);
} pfc_refptr_ops_t;

/*
 * Object pointer with reference counter.
 *
 * Remarks:
 *	pfc_refptr_t members are public just for optimization.
 *	The user must not access them directly.
 */
typedef struct {
	const pfc_refptr_ops_t	*pr_ops;	/* operations */
	pfc_ptr_t		pr_object;	/* object pointer */
	uint32_t		pr_refcnt;	/* reference counter */
} pfc_refptr_t;

/*
 * Type of refptr assertions.
 * This type is defined only for internal use. Never use this.
 */
typedef enum {
	__REFPTR_ASSF_GET		= 0,
	__REFPTR_ASSF_PUT,
	__REFPTR_ASSF_OPS,
	__REFPTR_ASSF_VAL,
} __pfc_refptr_assert_t;

/*
 * Prototypes.
 *
 * Remarks:
 *	Functions which have '__pfc_' or '__PFC_' prefix are not public.
 */
extern pfc_refptr_t	*pfc_refptr_create(const pfc_refptr_ops_t
					   *PFC_RESTRICT ops,
					   pfc_ptr_t PFC_RESTRICT value);
extern void		__pfc_refptr_destroy(pfc_refptr_t *ref);
extern pfc_bool_t	pfc_refptr_equals(pfc_refptr_t *r1,
					  pfc_refptr_t *r2);

extern pfc_refptr_t	*pfc_refptr_int32_create(int32_t value);
extern pfc_refptr_t	*pfc_refptr_uint32_create(uint32_t value);
extern pfc_refptr_t	*pfc_refptr_int64_create(int64_t value);
extern pfc_refptr_t	*pfc_refptr_uint64_create(uint64_t value);

extern pfc_refptr_t	*pfc_refptr_string_create(const char *string);
extern pfc_refptr_t	*pfc_refptr_sprintf(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);
extern pfc_refptr_t	*pfc_refptr_vsprintf(const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(1, 0);
extern size_t		pfc_refptr_string_length(pfc_refptr_t *ref);

extern const pfc_refptr_ops_t	*pfc_refptr_int32_ops(void);
extern const pfc_refptr_ops_t	*pfc_refptr_uint32_ops(void);
extern const pfc_refptr_ops_t	*pfc_refptr_int64_ops(void);
extern const pfc_refptr_ops_t	*pfc_refptr_uint64_ops(void);
extern const pfc_refptr_ops_t	*pfc_refptr_string_ops(void);

extern void	__pfc_refptr_assfail(pfc_refptr_t *PFC_RESTRICT ref,
				     __pfc_refptr_assert_t type,
				     const char *file, uint32_t line,
				     const char *func)
	PFC_FATTR_NORETURN PFC_FATTR_NONNULL((1, 3));

#define	__PFC_REFPTR_CAST(value, type)	((type)(uintptr_t)(value))

#ifdef	PFC_REFPTR_DEBUG_ENABLED

/*
 * Assertion to detect illegal dereference to refptr.
 */
#define	__PFC_REFPTR_ASSERT(ref, refcnt, type, file, line, func)	\
	do {								\
		if (PFC_EXPECT_FALSE((refcnt) == 0)) {			\
			__pfc_refptr_assfail((ref), (type), (file),	\
					     (line), (func));		\
			/* NOTREACHED */				\
		}							\
	} while (0)

#else	/* !PFC_REFPTR_DEBUG_ENABLED */

#define	__PFC_REFPTR_ASSERT(ref, type, refcnt, file, line, func)	\
	((void)0)

#endif	/* PFC_REFPTR_DEBUG_ENABLED */

/*
 * const pfc_refptr_ops_t *
 * pfc_refptr_operation(pfc_refptr_t *ref)
 *	Return refptr operation pointer.
 */
static inline const pfc_refptr_ops_t * PFC_FATTR_ALWAYS_INLINE
__pfc_refptr_operation(pfc_refptr_t *ref, const char *file, uint32_t line,
		       const char *func)
{
	__PFC_REFPTR_ASSERT(ref, ref->pr_refcnt, __REFPTR_ASSF_OPS,
			    file, line, func);

	return ref->pr_ops;
}

#define	pfc_refptr_operation(ref)					\
	__pfc_refptr_operation(ref, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)

/*
 * pfc_ptr_t
 * pfc_refptr_value(pfc_refptr_t *ref)
 *	Return value in refptr object.
 */
static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
__pfc_refptr_value(pfc_refptr_t *ref, const char *file, uint32_t line,
		   const char *func)
{
	__PFC_REFPTR_ASSERT(ref, ref->pr_refcnt, __REFPTR_ASSF_VAL,
			    file, line, func);

	return ref->pr_object;
}

#define	pfc_refptr_value(ref)						\
	__pfc_refptr_value(ref, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)

/*
 * Return refptr object value with type cast.
 */
#define	PFC_REFPTR_VALUE(ref, type)			\
	__PFC_REFPTR_CAST(pfc_refptr_value(ref), type)

#ifdef	PFC_LP64

#define	__PFC_REFPTR_VALUE64(ref, type)		PFC_REFPTR_VALUE(ref, type)

#else	/* !PFC_LP64 */

/* 32-bit system can't keep 64-bit value in pointer. */
#define	__PFC_REFPTR_VALUE64(ref, type)		\
	(*(PFC_REFPTR_VALUE(ref, type *)))

#endif	/* PFC_LP64 */

/*
 * Declare inline functions to get value of builtin refptr objects.
 */
#define	__PFC_REFPTR_VFUNC_DECL(name, type)				\
	static inline type PFC_FATTR_ALWAYS_INLINE			\
	__pfc_refptr_##name##_value(pfc_refptr_t *PFC_RESTRICT ref,	\
				    const char *file, uint32_t line,	\
				    const char *func)			\
	{								\
		pfc_ptr_t	ptr =					\
			__pfc_refptr_value(ref, file, line, func);	\
									\
		return __PFC_REFPTR_CAST(ptr, type);			\
	}

#define	__PFC_REFPTR_VFUNC64_DECL(name, type)	\
	__PFC_REFPTR_VFUNC_DECL(name, type)

#ifndef	PFC_LP64

#undef	__PFC_REFPTR_VFUNC64_DECL

/* 32-bit system can't keep 64-bit value in pointer. */
#define	__PFC_REFPTR_VFUNC64_DECL(name, type)				\
	static inline type PFC_FATTR_ALWAYS_INLINE			\
	__pfc_refptr_##name##_value(pfc_refptr_t *PFC_RESTRICT ref,	\
				    const char *file, uint32_t line,	\
				    const char *func)			\
	{								\
		pfc_ptr_t	ptr =					\
			__pfc_refptr_value(ref, file, line, func);	\
									\
		return *(__PFC_REFPTR_CAST(ptr, type *));		\
	}

#endif	/* !PFC_LP64 */

#define	__PFC_REFPTR_GETVALUE(ref, name)			\
	__pfc_refptr_##name##_value(ref, __FILE__, __LINE__,	\
				    PFC_ASSERT_FUNCNAME)

/*
 * int32_t
 * pfc_refptr_int32_value(pfc_refptr_t *ref)
 *	Return value embedded in int32 refptr.
 */
__PFC_REFPTR_VFUNC_DECL(int32, int32_t)
#define	pfc_refptr_int32_value(ref)	__PFC_REFPTR_GETVALUE(ref, int32)

/*
 * uint32_t
 * pfc_refptr_uint32_value(pfc_refptr_t *ref)
 *	Return value embedded in uint32 refptr.
 */
__PFC_REFPTR_VFUNC_DECL(uint32, uint32_t)
#define	pfc_refptr_uint32_value(ref)	__PFC_REFPTR_GETVALUE(ref, uint32)

/*
 * const char *
 * pfc_refptr_string_value(pfc_refptr_t *ref)
 *	Return value embedded in string refptr.
 */
__PFC_REFPTR_VFUNC_DECL(string, const char *)
#define	pfc_refptr_string_value(ref)	__PFC_REFPTR_GETVALUE(ref, string)

/*
 * int64_t
 * pfc_refptr_int64_value(pfc_refptr_t *ref)
 *	Return value embedded in int64 refptr.
 */
__PFC_REFPTR_VFUNC64_DECL(int64, int64_t)
#define	pfc_refptr_int64_value(ref)	__PFC_REFPTR_GETVALUE(ref, int64)

/*
 * uint64_t
 * pfc_refptr_uint64_value(pfc_refptr_t *ref)
 *	Return value embedded in uint64 refptr.
 */
__PFC_REFPTR_VFUNC64_DECL(uint64, uint64_t)
#define	pfc_refptr_uint64_value(ref)	__PFC_REFPTR_GETVALUE(ref, uint64)

#undef	__PFC_REFPTR_VFUNC_DECL
#undef	__PFC_REFPTR_VFUNC64_DECL

/*
 * void
 * pfc_refptr_get(pfc_refptr_t *ref)
 *	Increment references to the given object.
 */
#ifdef	PFC_REFPTR_DEBUG_ENABLED

static inline void PFC_FATTR_ALWAYS_INLINE
__pfc_refptr_get(pfc_refptr_t *PFC_RESTRICT ref, const char *file,
		 uint32_t line, const char *func)
{
	uint32_t	refcnt = pfc_atomic_inc_uint32_old(&ref->pr_refcnt);

	__PFC_REFPTR_ASSERT(ref, refcnt, __REFPTR_ASSF_GET,
			    file, line, func);
}

#define	pfc_refptr_get(ref)						\
	__pfc_refptr_get(ref, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)

#else	/* !PFC_REFPTR_DEBUG_ENABLED */

static inline void PFC_FATTR_ALWAYS_INLINE
pfc_refptr_get(pfc_refptr_t *ref)
{
	pfc_atomic_inc_uint32(&ref->pr_refcnt);
}

#endif	/* PFC_REFPTR_DEBUG_ENABLED */

/*
 * void
 * pfc_refptr_put(pfc_refptr_t *ref)
 *	Decrement references to the given object.
 *	Destructor will be called if reference counter becomes zero.
 */
#ifdef	PFC_REFPTR_DEBUG_ENABLED

static inline void PFC_FATTR_ALWAYS_INLINE
__pfc_refptr_put(pfc_refptr_t *PFC_RESTRICT ref, const char *file,
		 uint32_t line, const char *func)
{
	uint32_t	refcnt = pfc_atomic_dec_uint32_old(&ref->pr_refcnt);

	if (refcnt == 1) {
		__pfc_refptr_destroy(ref);
	}
	else {
		__PFC_REFPTR_ASSERT(ref, refcnt, __REFPTR_ASSF_PUT,
				    file, line, func);
	}
}

#define	pfc_refptr_put(ref)						\
	__pfc_refptr_put(ref, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)

#else	/* !PFC_REFPTR_DEBUG_ENABLED */

static inline void PFC_FATTR_ALWAYS_INLINE
pfc_refptr_put(pfc_refptr_t *ref)
{
	if (pfc_atomic_dec_uint32_old(&ref->pr_refcnt) == 1) {
		__pfc_refptr_destroy(ref);
	}
}

#endif	/* PFC_REFPTR_DEBUG_ENABLED */

PFC_C_END_DECL

#endif	/* !_PFC_REFPTR_H */
