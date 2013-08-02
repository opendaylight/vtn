/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * refptr.c -
 *	Simple implementation of object management based on reference counter.
 */

#include <stdio.h>
#include <string.h>
#include <pfc/bitpos.h>
#include <pfc/rbtree.h>
#include <pfc/synch.h>
#include "refptr_impl.h"

/*
 * Error message which represents assertion failure.
 */
static const char	*refptr_assert_msg[] = {
	"Attempt to get invalid refptr",		/* __REFPTR_ASSF_GET */
	"Too many calls to pfc_refptr_put()",		/* __REFPTR_ASSF_PUT */
	"Reference to invalid refptr operation",	/* __REFPTR_ASSF_OPS */
	"Reference to invalid refptr value",		/* __REFPTR_ASSF_VAL */
};

/*
 * Allocation statistics.
 */
static refptr_stat_t	refptr_stat = {
	.rst_size	= sizeof(pfc_refptr_t),
};

#define	REFPTR_ALLOCSIZE	refptr_stat.rst_size

#define	REFPTR_STAT_ALLOC()				\
	pfc_atomic_inc_uint64(&refptr_stat.rst_alloc)
#define	REFPTR_STAT_FAIL()				\
	pfc_atomic_inc_uint64(&refptr_stat.rst_fail)
#define	REFPTR_STAT_FREE()				\
	pfc_atomic_inc_uint64(&refptr_stat.rst_free)
#define	REFPTR_STAT_STRING_INC()				\
	pfc_atomic_inc_uint64(&refptr_stat.rst_nstrings)
#define	REFPTR_STAT_STRING_DEC()			\
	pfc_atomic_dec_uint64(&refptr_stat.rst_nstrings)
#define	REFPTR_STAT_STRFAIL()				\
	pfc_atomic_inc_uint64(&refptr_stat.rst_strfail)

/*
 * Dummy refptr operation which contains no method.
 * This is used if no refptr operation is specified.
 */
const pfc_refptr_ops_t	refptr_default_ops PFC_ATTR_HIDDEN;

#define	LARGE_PRIME		75497467U

#ifdef	PFC_LP64

/*
 * static inline uint64_t
 * refptr_uint64_eval(pfc_cptr_t object)
 *	Convert object pointer in refptr object to 64-bit value.
 */
static inline uint64_t
refptr_uint64_eval(pfc_cptr_t object)
{
	return (uint64_t)object;
}

#else	/* !PFC_LP64 */

/*
 * static inline uint64_t
 * refptr_uint64_eval(pfc_cptr_t object)
 *	Convert object pointer in refptr object to 64-bit value.
 */
static inline uint64_t
refptr_uint64_eval(pfc_cptr_t object)
{
	uint64_t	*ptr = (uint64_t *)object;

	return *ptr;
}

#endif	/* PFC_LP64 */

/*
 * C style string using refptr.
 */
typedef struct {
	size_t		rs_length;		/* length of string */
	uint32_t	rs_hash;		/* hash value */
	char		rs_string[0];		/* pointer to string */
} refptr_str_t;

#define	REFPTR_STR_SIZE(len)	(sizeof(refptr_str_t) + (len) + 1)

/*
 * Return string object pointer to refptr_str_t pointer.
 */
static inline refptr_str_t PFC_FATTR_ALWAYS_INLINE *
refptr_string_ptr(pfc_ptr_t object)
{
	return PFC_CAST_CONTAINER(object, refptr_str_t, rs_string);
}

/*
 * Internal buffer size used by pfc_refptr_vsprintf().
 */
#define	REFPTR_STR_BUFSIZE	PFC_CONST_U(64)

/*
 * Internal prototypes.
 */
static int		refptr_int32_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static int		refptr_uint32_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static pfc_bool_t	refptr_uint32_equals(pfc_cptr_t o1, pfc_cptr_t o2);
static uint32_t		refptr_uint32_hash(pfc_cptr_t object);

#ifdef	PFC_LP64
#define	refptr_uint64_create(value)	((pfc_ptr_t)(value))
#define	refptr_uint64_dtor(object)	/* NOP */
#define	REFPTR_UINT64_DTOR		NULL
#else	/* !PFC_LP64 */
static pfc_ptr_t	refptr_uint64_create(uint64_t value);
static void		refptr_uint64_dtor(pfc_ptr_t object);
#define	REFPTR_UINT64_DTOR		refptr_uint64_dtor
#endif	/* PFC_LP64 */

static int		refptr_int64_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static int		refptr_uint64_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static pfc_bool_t	refptr_uint64_equals(pfc_cptr_t o1, pfc_cptr_t o2);
static uint32_t		refptr_uint64_hash(pfc_cptr_t object);

static void		refptr_string_dtor(pfc_ptr_t object);
static pfc_bool_t	refptr_string_equals(pfc_cptr_t o1, pfc_cptr_t o2);
static uint32_t		refptr_string_hash(pfc_cptr_t object);
static refptr_str_t	*refptr_string_create(const char *string);
static refptr_str_t	*refptr_string_vsprintf(uint32_t len, const char *fmt,
						va_list ap);
static pfc_refptr_t	*refptr_string_register(const char *string,
						pfc_bool_t is_refstr);

static pfc_refptr_t	*refptr_rstr_lookup(const char *string);
static pfc_cptr_t	refptr_rstr_getkey(pfc_rbnode_t *node);

/*
 * Strings created by pfc_refptr_string_create().
 */
typedef struct {
	pfc_refptr_t	*rrs_refptr;		/* refptr string */
	pfc_rbnode_t	rrs_node;		/* Red-Black tree node */
} refptr_rstr_t;

#define	REFPTR_RSTR_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), refptr_rstr_t, rrs_node)

/*
 * Red-Black tree which keeps all strings created by
 * pfc_refptr_string_create().
 */
static pfc_rbtree_t	refptr_string_tree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, refptr_rstr_getkey);

/*
 * Mutex which serializes creation of refptr string.
 */
static pfc_mutex_t	refptr_string_mutex = PFC_MUTEX_INITIALIZER;

#define	REFPTR_STRING_LOCK()	pfc_mutex_lock(&refptr_string_mutex)
#define	REFPTR_STRING_UNLOCK()	pfc_mutex_unlock(&refptr_string_mutex)

/*
 * Builtin refptr operations.
 */
const pfc_refptr_ops_t	refptr_int32_ops PFC_ATTR_HIDDEN = {
	.compare	= refptr_int32_compare,
	.equals		= refptr_uint32_equals,
	.hashfunc	= refptr_uint32_hash,
};

const pfc_refptr_ops_t	refptr_uint32_ops PFC_ATTR_HIDDEN = {
	.compare	= refptr_uint32_compare,
	.equals		= refptr_uint32_equals,
	.hashfunc	= refptr_uint32_hash,
};

const pfc_refptr_ops_t	refptr_int64_ops PFC_ATTR_HIDDEN = {
	.compare	= refptr_int64_compare,
	.equals		= refptr_uint64_equals,
	.hashfunc	= refptr_uint64_hash,
};

const pfc_refptr_ops_t	refptr_uint64_ops PFC_ATTR_HIDDEN = {
	.compare	= refptr_uint64_compare,
	.equals		= refptr_uint64_equals,
	.hashfunc	= refptr_uint64_hash,
	.dtor		= REFPTR_UINT64_DTOR
};

const pfc_refptr_ops_t	refptr_string_ops PFC_ATTR_HIDDEN = {
	.dtor		= refptr_string_dtor,
	.compare	= (int (*)(pfc_cptr_t, pfc_cptr_t))strcmp,
	.equals		= refptr_string_equals,
	.hashfunc	= refptr_string_hash,
};

/*
 * void PFC_ATTR_HIDDEN
 * pfc_refptr_libinit(void)
 *	Initialize reference pointer functionality.
 */
void PFC_ATTR_HIDDEN
pfc_refptr_libinit(void)
{
	/* Determine allocation size for pfc_refptr_t. */
	REFPTR_ALLOCSIZE = sizeof(pfc_refptr_t);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_refptr_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 */
void PFC_ATTR_HIDDEN
pfc_refptr_fork_prepare(void)
{
	REFPTR_STRING_LOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_refptr_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
void PFC_ATTR_HIDDEN
pfc_refptr_fork_parent(void)
{
	REFPTR_STRING_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_refptr_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
void PFC_ATTR_HIDDEN
pfc_refptr_fork_child(void)
{
	PFC_ASSERT_INT(PFC_MUTEX_INIT(&refptr_string_mutex), 0);
}

/*
 * pfc_refptr_t *
 * pfc_refptr_create(const pfc_refptr_ops_t *PFC_RESTRICT ops,
 *		     pfc_ptr_t PFC_RESTRICT value)
 *	Create a new refptr object.
 *	`value' is set as object pointer.
 *
 * Calling/Exit State:
 *	A non-NULL pointer is returned on success.
 *	NULL is returned on failure.
 *
 * Remarks:
 *	A pointer specified to `ops' will be shared with refptr objects.
 *	So `ops' must be located in static area.
 *	In addition, `ops' is used as object type identifier.
 *	That is, if two refptr objects represent the same object type,
 *	they must have the same `ops' value. If they have different object
 *	type, they must have different `ops' value.
 */
pfc_refptr_t *
pfc_refptr_create(const pfc_refptr_ops_t *PFC_RESTRICT ops,
		  pfc_ptr_t PFC_RESTRICT value)
{
	pfc_refptr_t	*ref;

	if (PFC_EXPECT_FALSE(ops == NULL)) {
		/* Use default operation. */
		ops = &refptr_default_ops;
	}

	ref = (pfc_refptr_t *)malloc(REFPTR_ALLOCSIZE);
	if (PFC_EXPECT_FALSE(ref == NULL)) {
		REFPTR_STAT_FAIL();

		return NULL;
	}

	REFPTR_STAT_ALLOC();
	ref->pr_refcnt = 1;
	ref->pr_object = value;
	ref->pr_ops = ops;

	return ref;
}

/*
 * void
 * __pfc_refptr_destroy(pfc_refptr_t *ref)
 *	Free up all resources grabbed by the given refptr object.
 *
 * Remarks:
 *	This function must be called via pfc_refptr_put().
 */
void
__pfc_refptr_destroy(pfc_refptr_t *ref)
{
	const pfc_refptr_ops_t	*ops = ref->pr_ops;

	if (ops->dtor != NULL) {
		ops->dtor(ref->pr_object);
	}

	free(ref);
	REFPTR_STAT_FREE();
}

/*
 * Declare APIs which return refptr operation for builtin types.
 */
#define	REFPTR_BUILTIN_OPS_API_DECL(type)	\
	const pfc_refptr_ops_t *		\
	pfc_refptr_##type##_ops(void)		\
	{					\
		return &refptr_##type##_ops;	\
	}

REFPTR_BUILTIN_OPS_API_DECL(int32)
REFPTR_BUILTIN_OPS_API_DECL(uint32)
REFPTR_BUILTIN_OPS_API_DECL(int64)
REFPTR_BUILTIN_OPS_API_DECL(uint64)
REFPTR_BUILTIN_OPS_API_DECL(string)

/*
 * pfc_bool_t
 * pfc_refptr_equals(pfc_refptr_t *r1, pfc_refptr_t *r2)
 *	Determine whether the given two refptr objects are identical.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the given two objects are identical.
 *	Otherwise PFC_FALSE.
 *
 * Remarks:
 *	This function always returns PFC_FALSE if the given two object have
 *	different refptr operation.
 */
pfc_bool_t
pfc_refptr_equals(pfc_refptr_t *r1, pfc_refptr_t *r2)
{
	pfc_cptr_t	o1, o2;

	if (r1 == NULL || r2 == NULL) {
		return (r1 == r2) ? PFC_TRUE : PFC_FALSE;
	}

	o1 = PFC_REFPTR_VALUE(r1, pfc_cptr_t);
	o2 = PFC_REFPTR_VALUE(r2, pfc_cptr_t);

	if (PFC_EXPECT_FALSE(r1->pr_ops != r2->pr_ops)) {
		return PFC_FALSE;
	}

	return pfc_refptr_ops_equals(r1->pr_ops, o1, o2);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_refptr_compare_default(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Default comparator for refptr object.
 *	This function compares object address.
 */
int PFC_ATTR_HIDDEN
pfc_refptr_compare_default(pfc_cptr_t o1, pfc_cptr_t o2)
{
	if (o1 == o2) {
		return 0;
	}

	return (o1 < o2) ? -1 : 1;
}

/*
 * uint32_t PFC_ATTR_HIDDEN
 * pfc_refptr_hashfunc_default(pfc_cptr_t object)
 *	Default hash function for refptr object.
 *	This function derives hash value from object address.
 */
uint32_t PFC_ATTR_HIDDEN
pfc_refptr_hashfunc_default(pfc_cptr_t object)
{
	uintptr_t	addr = (uintptr_t)object;

#ifdef	PFC_LP64
	return refptr_uint64_hash((pfc_cptr_t)addr);
#else	/* !PFC_LP64 */
	return refptr_uint32_hash((pfc_cptr_t)addr);
#endif	/* PFC_LP64 */
}

/*
 * pfc_refptr_t *
 * pfc_refptr_int32_create(int32_t value)
 *	Create refptr object that has the given signed 32-bit value.
 */
pfc_refptr_t *
pfc_refptr_int32_create(int32_t value)
{
	return pfc_refptr_create(&refptr_int32_ops,
				 (pfc_ptr_t)(uintptr_t)value);
}

/*
 * pfc_refptr_t *
 * pfc_refptr_uint32_create(uint32_t value)
 *	Create refptr object that has the given unsigned 32-bit value.
 */
pfc_refptr_t *
pfc_refptr_uint32_create(uint32_t value)
{
	return pfc_refptr_create(&refptr_uint32_ops,
				 (pfc_ptr_t)(uintptr_t)value);
}

/*
 * static int
 * refptr_int32_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Compare refptr objects which have signed 32-bit integer.
 */
static int
refptr_int32_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	int32_t	v1 = (int32_t)(uintptr_t)o1;
	int32_t	v2 = (int32_t)(uintptr_t)o2;

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * static int
 * refptr_uint32_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Compare refptr objects which have unsigned 32-bit integer.
 */
static int
refptr_uint32_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	uint32_t	v1 = (uint32_t)(uintptr_t)o1;
	uint32_t	v2 = (uint32_t)(uintptr_t)o2;

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * static pfc_bool_t
 * refptr_uint32_equals(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Determine whether the given two refptr integers are identical.
 */
static pfc_bool_t
refptr_uint32_equals(pfc_cptr_t o1, pfc_cptr_t o2)
{
	uint32_t	v1 = (uint32_t)(uintptr_t)o1;
	uint32_t	v2 = (uint32_t)(uintptr_t)o2;

	return (v1 == v2) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static uint32_t
 * refptr_uint32_hash(pfc_cptr_t object)
 *	Return hash value for the given 32-bit integer.
 */
static uint32_t
refptr_uint32_hash(pfc_cptr_t object)
{
	return (uint32_t)(uintptr_t)object * LARGE_PRIME;
}

/*
 * pfc_refptr_t *
 * pfc_refptr_int64_create(int64_t value)
 *	Create refptr object that has the given signed 64-bit integer value.
 */
pfc_refptr_t *
pfc_refptr_int64_create(int64_t value)
{
	pfc_ptr_t	object = refptr_uint64_create((uint64_t)value);
	pfc_refptr_t	*ref = pfc_refptr_create(&refptr_int64_ops, object);

	if (PFC_EXPECT_FALSE(ref == NULL)) {
		refptr_uint64_dtor(object);
	}

	return ref;
}

/*
 * pfc_refptr_t *
 * pfc_refptr_uint64_create(uint64_t value)
 *	Create refptr object that has the given unsigned 64-bit integer value.
 */
pfc_refptr_t *
pfc_refptr_uint64_create(uint64_t value)
{
	pfc_ptr_t	object = refptr_uint64_create(value);
	pfc_refptr_t	*ref = pfc_refptr_create(&refptr_uint64_ops, object);

	if (PFC_EXPECT_FALSE(ref == NULL)) {
		refptr_uint64_dtor(object);
	}

	return ref;
}

#ifndef	PFC_LP64

/*
 * static pfc_ptr
 * refptr_uint64_create(uint64_t value)
 *	Create refptr object that has the given 64-bit integer value.
 */
static pfc_ptr_t
refptr_uint64_create(uint64_t value)
{
	uint64_t	*obj;

	/* Allocate storage for 64-bit integer. */
	obj = (uint64_t *)malloc(sizeof(uint64_t));
	if (PFC_EXPECT_TRUE(obj != NULL)) {
		*obj = value;
	}

	return obj;
}

/*
 * static void
 * refptr_uint64_dtor(pfc_ptr_t object)
 *	Destroy 64-bit integer value created by refptr_uint64_create().
 */
static void
refptr_uint64_dtor(pfc_ptr_t object)
{
	free(object);
}

#endif	/* !PFC_LP64 */

/*
 * static int
 * refptr_int64_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Compare refptr objects which have signed 64-bit integer.
 */
static int
refptr_int64_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	int64_t	v1 = (int64_t)refptr_uint64_eval(o1);
	int64_t	v2 = (int64_t)refptr_uint64_eval(o2);

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * static int
 * refptr_uint64_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Compare refptr objects which have unsigned 64-bit integer.
 */
static int
refptr_uint64_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	uint64_t	v1 = refptr_uint64_eval(o1);
	uint64_t	v2 = refptr_uint64_eval(o2);

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * static pfc_bool_t
 * refptr_uint64_equals(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Determine whether the given two refptr integers are identical.
 */
static pfc_bool_t
refptr_uint64_equals(pfc_cptr_t o1, pfc_cptr_t o2)
{
	uint64_t	v1 = refptr_uint64_eval(o1);
	uint64_t	v2 = refptr_uint64_eval(o2);

	return (v1 == v2) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static uint32_t
 * refptr_uint64_hash(pfc_cptr_t object)
 *	Return hash value associated with the given 64-bit integer.
 */
static uint32_t
refptr_uint64_hash(pfc_cptr_t object)
{
	uint64_t	v = refptr_uint64_eval(object);
	uint32_t	lo = (uint32_t)v;
	uint32_t	hi = (uint32_t)(v >> 32);

	return (lo ^ hi) * LARGE_PRIME;
}

/*
 * pfc_refptr_t *
 * pfc_refptr_string_create(const char *string)
 *	Create refptr object that has the given C style string.
 *
 * Calling/Exit State:
 *	A non-NULL pointer is returned on success.
 *	NULL is returned on failure.
 */
pfc_refptr_t *
pfc_refptr_string_create(const char *string)
{
	return refptr_string_register(string, PFC_FALSE);
}

/*
 * pfc_refptr_t *
 * pfc_refptr_sprintf(const char *fmt, ...)
 *	Construct a new refptr string specified by the format and arguments
 *	in sprintf(3) style.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to pfc_refptr_t is
 *	returned.
 *	Otherwise NULL is returned. An error number which indicates the cause
 *	of error is set to `errno'.
 */
pfc_refptr_t *
pfc_refptr_sprintf(const char *fmt, ...)
{
	pfc_refptr_t	*ref;
	va_list		ap;

	va_start(ap, fmt);
	ref = pfc_refptr_vsprintf(fmt, ap);
	va_end(ap);

	return ref;
}

/*
 * pfc_refptr_t *
 * pfc_refptr_vsprintf(const char *fmt, va_list ap)
 *	Construct a new refptr string specified by the format and va_list
 *	in vsprintf(3) style.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to pfc_refptr_t is
 *	returned.
 *	Otherwise NULL is returned. An error number which indicates the cause
 *	of error is set to `errno'.
 */
pfc_refptr_t *
pfc_refptr_vsprintf(const char *fmt, va_list ap)
{
	va_list		copied;
	pfc_refptr_t	*ref;
	refptr_str_t	*rstr;
	const char	*string;
	pfc_bool_t	is_refstr;
	char		buf[REFPTR_STR_BUFSIZE];
	int		len, err;

	/* At first, try to construct a string using internal buffer. */
	va_copy(copied, ap);
	len = vsnprintf(buf, sizeof(buf), fmt, copied);
	va_end(copied);
	if (PFC_EXPECT_FALSE(len < 0)) {
		/* This should never happen. */
		err = EINVAL;
		goto error;
	}

	if ((size_t)len < sizeof(buf)) {
		is_refstr = PFC_FALSE;
		string = buf;
	}
	else {
		/* Create a new refptr string. */
		rstr = refptr_string_vsprintf(len, fmt, ap);
		if (PFC_EXPECT_FALSE(rstr == NULL)) {
			err = ENOMEM;
			goto error;
		}

		is_refstr = PFC_TRUE;
		string = rstr->rs_string;
	}

	/* Register a new string to the refptr string tree. */
	ref = refptr_string_register(string, is_refstr);
	if (PFC_EXPECT_FALSE(ref == NULL)) {
		errno = ENOMEM;
	}

	return ref;

error:
	REFPTR_STRING_LOCK();
	REFPTR_STAT_STRFAIL();
	REFPTR_STRING_UNLOCK();
	errno = err;

	return NULL;
}

/*
 * static pfc_refptr_t *
 * refptr_string_register(const char *string, pfc_bool_t is_refstr)
 *	Register the given string to the refptr string tree.
 *
 *	If `is_refstr' is PFC_TRUE, a pointer specified by `string' is
 *	considered as a pointer to rs_string in refptr_str_t.
 *	If `is_refstr' is PFC_TRUE, a string pointed by `string' is released
 *	by this function in the following cases:
 *	- The given string is already registered in the refptr string tree.
 *	- An error was happen.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to pfc_refptr_t which
 *	contains the given string is returned.
 *	Otherwise NULL is returned.
 */
static pfc_refptr_t *
refptr_string_register(const char *string, pfc_bool_t is_refstr)
{
	pfc_refptr_t	*ref;
	pfc_rbtree_t	*tree = &refptr_string_tree;
	refptr_str_t	*rstr;
	refptr_rstr_t	*rsp;

	PFC_ASSERT(string != NULL);

	rstr = (is_refstr) ? refptr_string_ptr((pfc_ptr_t)string) : NULL;

	/*
	 * At first, check whether the given string exists in the string
	 * tree.
	 */
	REFPTR_STRING_LOCK();
	ref = refptr_rstr_lookup(string);
	if (ref != NULL) {
		if (rstr != NULL) {
			free(rstr);
		}

		goto out;
	}

	if (rstr == NULL) {
		/* Create a new refptr string. */
		rstr = refptr_string_create(string);
		if (PFC_EXPECT_FALSE(rstr == NULL)) {
			goto error;
		}
	}

	rsp = (refptr_rstr_t *)malloc(sizeof(*rsp));
	if (PFC_EXPECT_FALSE(rsp == NULL)) {
		free(rstr);
		goto error;
	}

	/*
	 * Put address of rstr->rs_string into object pointer so that
	 * we can see string via pr_object member.
	 * Note that refptr operations for string rely on this behavior.
	 */
	ref = pfc_refptr_create(&refptr_string_ops, rstr->rs_string);
	if (PFC_EXPECT_FALSE(ref == NULL)) {
		free(rstr);
		free(rsp);
		goto error;
	}

	/* Register this string to the string tree. */
	rsp->rrs_refptr = ref;
	PFC_ASSERT_INT(pfc_rbtree_put(tree, &rsp->rrs_node), 0);
	REFPTR_STAT_STRING_INC();

out:
	REFPTR_STRING_UNLOCK();

	return ref;

error:
	REFPTR_STAT_STRFAIL();
	REFPTR_STRING_UNLOCK();

	return NULL;
}

/*
 * size_t
 * pfc_refptr_string_length(pfc_refptr_t *ref)
 *	Return length of string in the given refptr object.
 *	The given pointer must be a pointer created by
 *	pfc_refptr_string_create().
 */
size_t
pfc_refptr_string_length(pfc_refptr_t *ref)
{
	refptr_str_t	*rstr = refptr_string_ptr(pfc_refptr_value(ref));

	PFC_ASSERT(ref->pr_ops == &refptr_string_ops);

	return rstr->rs_length;
}

/*
 * void
 * __pfc_refptr_assfail(pfc_refptr_t *PFC_RESTRICT ref,
 *			__pfc_refptr_assert_t type, const char *file,
 *			uint32_t line, const char *func)
 *	Called when assertion against refptr specified by `refp' fails.
 *
 *	This function dumps error message to the standard error output,
 *	and then raise SIGABRT.
 *
 * Calling/Exit State:
 *	This function never returns.
 */
void
__pfc_refptr_assfail(pfc_refptr_t *PFC_RESTRICT ref,
		     __pfc_refptr_assert_t type, const char *file,
		     uint32_t line, const char *func)
{
	const char	*msg;

	if (PFC_EXPECT_TRUE((uint32_t)type <
			    (uint32_t)PFC_ARRAY_CAPACITY(refptr_assert_msg))) {
		msg = refptr_assert_msg[type];
	}
	else {
		/* This should never happen. */
		msg = "Unexpected assertion";
	}

	__pfc_assfail_printf(file, line, func, "%s: refptr=%p", msg, ref);
	/* NOTREACHED */
}

/*
 * void
 * pfc_refptr_getstat(refptr_stat_t *stp)
 *	Obtain allocation statistics of refptr.
 */
void
pfc_refptr_getstat(refptr_stat_t *stp)
{
	*stp = refptr_stat;
}

/*
 * static void
 * refptr_string_dtor(pfc_ptr_t object)
 *	Destroy refptr string object.
 */
static void
refptr_string_dtor(pfc_ptr_t object)
{
	refptr_str_t	*rstr = refptr_string_ptr(object);
	pfc_rbtree_t	*tree = &refptr_string_tree;
	pfc_rbnode_t	*node;

	/* Remove the given string from the string tree. */
	REFPTR_STRING_LOCK();
	node = pfc_rbtree_remove(tree, rstr->rs_string);
	REFPTR_STAT_STRING_DEC();
	REFPTR_STRING_UNLOCK();

	if (node != NULL) {
		free(REFPTR_RSTR_NODE2PTR(node));
	}

	free(rstr);
}

/*
 * static pfc_bool_t
 * refptr_string_equals(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Determine whether the given two refptr strings are identical.
 */
static pfc_bool_t
refptr_string_equals(pfc_cptr_t o1, pfc_cptr_t o2)
{
	const char	*s1 = (const char *)o1;
	const char	*s2 = (const char *)o2;

	return (strcmp(s1, s2) == 0) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static uint32_t
 * refptr_string_hash(pfc_cptr_t object)
 *	Return hash value associated with the given refptr string.
 */
static uint32_t
refptr_string_hash(pfc_cptr_t object)
{
	refptr_str_t	*rstr = refptr_string_ptr((pfc_ptr_t)object);
	const char	*str = (const char *)rstr->rs_string;
	uint32_t	h = rstr->rs_hash;

	/*
	 * Calculate hash value if it is not cached.
	 * We don't need any lock because refptr_string_hashfunc() with
	 * specifying the same string always returns the same value.
	 */
	if (h == 0) {
		h = refptr_string_hashfunc(str, rstr->rs_length);
		rstr->rs_hash = h;
	}

	return h;
}

/*
 * static refptr_str_t *
 * refptr_string_create(const char *string)
 *	Create a new refptr_str_t object which is identical to the given
 *	string.
 */
static refptr_str_t *
refptr_string_create(const char *string)
{
	refptr_str_t	*rstr;
	size_t		len, size;

	PFC_ASSERT(string != NULL);

	len = strlen(string);
	size = REFPTR_STR_SIZE(len);
	rstr = (refptr_str_t *)malloc(size);
	if (PFC_EXPECT_FALSE(rstr == NULL)) {
		return NULL;
	}

	rstr->rs_length = len;
	rstr->rs_hash = 0;
	memcpy(rstr->rs_string, string, len);
	*(rstr->rs_string + len) = '\0';

	return rstr;
}

/*
 * static refptr_str_t *
 * refptr_string_vsprintf(uint32_t len, const char *fmt, va_list ap)
 *	Create a new refptr_str_t object which contains a string specified
 *	by vsprintf(3) style format and va_list.
 *
 *	`len' must be the length of a string created by vsprintf(3).
 */
static refptr_str_t *
refptr_string_vsprintf(uint32_t len, const char *fmt, va_list ap)
{
	refptr_str_t	*rstr;
	size_t		size;

	size = REFPTR_STR_SIZE(len);
	rstr = (refptr_str_t *)malloc(size);
	if (PFC_EXPECT_FALSE(rstr == NULL)) {
		return NULL;
	}

	rstr->rs_length = len;
	rstr->rs_hash = 0;
	PFC_ASSERT_INT(vsnprintf(rstr->rs_string, len + 1, fmt, ap), len);

	return rstr;
}

/*
 * static pfc_refptr_t *
 * refptr_rstr_lookup(const char *string)
 *	Search for the refptr string which equals to the given string.
 *
 * Calling/Exit State:
 *	If the refptr string which equals to the given string exists,
 *	references to the refptr string is incremented, and a non-NULL
 *	pointer to the refptr string is returned.
 *	NULL is returned if not found.
 *
 * Calling/Exit State:
 *	This function must be called with holding string tree lock.
 */
static pfc_refptr_t *
refptr_rstr_lookup(const char *string)
{
	pfc_rbtree_t	*tree = &refptr_string_tree;
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(tree, string);
	if (node != NULL) {
		refptr_rstr_t	*rsp = REFPTR_RSTR_NODE2PTR(node);
		pfc_refptr_t	*ref = rsp->rrs_refptr;
		uint32_t	refcnt;

		/* Try to increment reference counter of this string. */
		refcnt = pfc_atomic_inc_uint32_old(&ref->pr_refcnt);
		if (refcnt != 0) {
			/* This string is still available. */
			return ref;
		}

		/*
		 * This string is about to be destroyed on another context.
		 * So we must drop this string.
		 */
		pfc_rbtree_remove_node(tree, node);
		free(rsp);
	}

	return NULL;
}

/*
 * static pfc_cptr_t
 * refptr_rstr_getkey(pfc_rbnode_t *node)
 *	Return the key of the specified refptr string.
 *	`node' must be a pointer to rrs_node in refptr_rstr_t.
 */
static pfc_cptr_t
refptr_rstr_getkey(pfc_rbnode_t *node)
{
	refptr_rstr_t	*rsp = REFPTR_RSTR_NODE2PTR(node);
	refptr_str_t	*rstr;

	/*
	 * We can't use common accessor, such as pfc_refptr_value(),
	 * because reference counter may be decremented to zero while the
	 * string tree search is running. But it's harmless because deletion
	 * of the string and the string tree search are always serialized by
	 * the refptr_string_mutex.
	 */
	rstr = refptr_string_ptr(rsp->rrs_refptr->pr_object);

	return (pfc_cptr_t)rstr->rs_string;
}
