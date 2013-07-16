/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_hash_delete.cc - Test for pfc_hash_delete() variants.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <stdint.h>
#include <pfc/hash.h>
#include <pfc/listmodel.h>
#include <gtest/gtest.h>
#include "random.hh"
#include "misc.hh"

#define	NBUCKETS		200U
#define	MAX_NELEMS		700U
#define	MIN_NELEMS		16U
#define	MAX_STRLEN		64

#define	TEST_LOOP		400

/* Use C style cast to simplify code. */
#define	DO_CAST(type, v)	((type)(v))
#define	CAST_UINTPTR(v)		DO_CAST(uintptr_t, v)
#define	CAST_INT(v)		DO_CAST(int, CAST_UINTPTR(v))
#define	CAST_UINT32(v)		DO_CAST(uint32_t, CAST_UINTPTR(v))
#define	CAST_UINT64(v)		DO_CAST(uint64_t, CAST_UINTPTR(v))
#define	CAST_PTR(v)		DO_CAST(pfc_ptr_t, CAST_UINTPTR(v))
#define	CAST_CPTR(v)		DO_CAST(pfc_cptr_t, CAST_UINTPTR(v))
#define	CAST_CPTR_P(v)		DO_CAST(pfc_cptr_t *, CAST_UINTPTR(v))
#define	CAST_REFPTR(v)		DO_CAST(pfc_refptr_t *, CAST_UINTPTR(v))

#define	VALUE_SIMPLE(key)		((key) + 10000)

#define	KEY_STRING(key, buf)	snprintf(buf, sizeof(buf), "key: %d", key)
#define	VALUE_STRING(key)	((key) * 7)

#define	VALUE_U64(key)                                          \
    CAST_UINT32((((key) >> 32) ^ ((key) & 0xffffffffULL)) * 3)

/*
 * Wrapper class to clean up hash table.
 */
class HashTable
{
public:
    HashTable(pfc_hash_t hash = PFC_HASH_INVALID) : _hash(hash) {}

    ~HashTable()
    {
        if (_hash != PFC_HASH_INVALID) {
            pfc_hash_destroy(_hash);
        }
    }

private:
    pfc_hash_t	_hash;
};

class HashDtor;
static HashDtor	*dtor_instance;

/*
 * Class to keep track of the call of hash destructor.
 */
class HashDtor
{
public:
    HashDtor() : _kcount(0), _vcount(0), _klast(NULL), _vlast(NULL)
    {
        // Install dtor instance.
        dtor_instance = this;
    }

    virtual ~HashDtor()
    {
        // Uninstall dtor instance.
        dtor_instance = NULL;
    }

    inline uint32_t
    getKeyCount(void) const
    {
        return _kcount;
    }

    inline uint32_t
    getValueCount(void) const
    {
        return _vcount;
    }

    inline pfc_cptr_t
    getLastKey(void) const
    {
        return _klast;
    }

    inline pfc_cptr_t
    getLastValue(void) const
    {
        return _vlast;
    }

    virtual void	keyCalled(pfc_ptr_t key, uint32_t flags);
    virtual void	valueCalled(pfc_ptr_t value, uint32_t flags);

    static void		keyDtor(pfc_ptr_t key, uint32_t flags);
    static void		valueDtor(pfc_ptr_t key, uint32_t flags);

private:
    uint32_t	_kcount;
    uint32_t	_vcount;
    pfc_cptr_t	_klast;
    pfc_cptr_t	_vlast;
};

/*
 * void
 * HashDtor::keyCalled(pfc_ptr_t key, uint32_t flags)
 *	Hash table key destructor.
 */
void
HashDtor::keyCalled(pfc_ptr_t key, uint32_t flags)
{
    _kcount++;
    _klast = key;
}

/*
 * void
 * HashDtor::valueCalled(pfc_ptr_t value, uint32_t flags)
 *	Hash table value destructor.
 */
void
HashDtor::valueCalled(pfc_ptr_t value, uint32_t flags)
{
    _vcount++;
    _vlast = value;
}

/*
 * void
 * HashDtor::keyDtor(pfc_ptr_t key, uint32_t flags)
 *	Common entry point of hash table key destructor.	
 */
void
HashDtor::keyDtor(pfc_ptr_t key, uint32_t flags)
{
    dtor_instance->keyCalled(key, flags);
}

/*
 * void
 * HashDtor::valueDtor(pfc_ptr_t value, uint32_t flags)
 *	Common entry point of hash table value destructor.
 */
void
HashDtor::valueDtor(pfc_ptr_t value, uint32_t flags)
{
    dtor_instance->valueCalled(value, flags);
}

static const pfc_hash_ops_t	dtor_key_value = {
    NULL,			// equals
    NULL,			// hashfunc
    HashDtor::keyDtor,		// key_dtor
    HashDtor::valueDtor,	// value_dtor
};

/*
 * Hash destructor class for string hash table.
 */
class StrHashDtor
    : public HashDtor
{
public:
    std::string &
    getLastString(void)
    {
        return _lastString;
    }

    void	keyCalled(pfc_ptr_t key, uint32_t flags);

private:
    std::string	_lastString;
};

/*
 * void
 * StrHashDtor::keyCalled(pfc_ptr_t key, uint32_t flags)
 *	Key destructor for string hash table.
 */
void
StrHashDtor::keyCalled(pfc_ptr_t key, uint32_t flags)
{
    pfc_refptr_t	*kref(CAST_REFPTR(key));

    if (kref != NULL) {
        // Copy key string because this key will be destroyed immediately.
        const char	*str(pfc_refptr_string_value(kref));
        _lastString = str;
    }
    else {
        _lastString = "";
    }

    // Pass to superclass.
    HashDtor::keyCalled(key, flags);
}

/*
 * Hash destructor class for u64 hash table.
 */
class U64HashDtor
    : public HashDtor
{
public:
    U64HashDtor() : _lastU64(0) {}

    uint64_t
    getLastU64Key(void) const
    {
        return _lastU64;
    }

    void	keyCalled(pfc_ptr_t key, uint32_t flags);

private:
    uint64_t	_lastU64;
};

/*
 * void
 * U64HashDtor::keyCalled(pfc_ptr_t key, uint32_t flags)
 *	Key destructor for u64 hash table.
 */
void
U64HashDtor::keyCalled(pfc_ptr_t key, uint32_t flags)
{
    // Preserve unsigned 64-bit key value.
#ifdef	PFC_LP64
    // On LP64 system, u64 hash key is kept as pointer.
    _lastU64 = CAST_UINT64(key);
#else	/* !PFC_LP64 */
    // On ILP32 system, u64 hash key is kept as uint64 refptr object.
    pfc_refptr_t	*kref(CAST_REFPTR(key));
    _lastU64 = pfc_refptr_uint64_value(kref);
#endif	/* PFC_LP64 */

    // Pass to superclass.
    HashDtor::keyCalled(key, flags);
}

/*
 * int32 refptr factory class.
 */
class Int32Factory
{
public:
    Int32Factory();
    ~Int32Factory();

    void	create(pfc_refptr_t *&object, int32_t value);
    void	get(int index, pfc_refptr_t *&object);
    void	checkReference(void);

private:
    pfc_listm_t		_list;
    int			_error;
};

/*
 * Int32Factory::Int32Factory()
 *	Create int32 refptr object factory.
 */
Int32Factory::Int32Factory()
{
    _error = PFC_VECTOR_CREATE_REF(&_list, pfc_refptr_int32_ops());
}

/*
 * Int32Factory::~Int32Factory()
 *	Destroy all int32 refptr objects.
 */
Int32Factory::~Int32Factory()
{
    if (_error == 0) {
        pfc_listm_destroy(_list);
    }
}

/*
 * void
 * Int32Factory::create(pfc_refptr_t *&object, int32_t value)
 *	Create a int32 refptr object which has the specified value.
 *	Google Test's fatal error will be raised on error.
 */
void
Int32Factory::create(pfc_refptr_t *&object, int32_t value)
{
    ASSERT_EQ(0, _error);

    pfc_refptr_t	*obj(pfc_refptr_int32_create(value));
    ASSERT_NE(CAST_REFPTR(NULL), obj);

    int	err(pfc_listm_push_tail(_list, obj));
    pfc_refptr_put(obj);
    ASSERT_EQ(0, err);

    object = obj;
}

/*
 * void
 * Int32Factory::get(int index, pfc_refptr_t *&object)
 *	Return a int32 refptr object at the specified index.
 *	Google Test's fatal error will be raised on error.
 */
void
Int32Factory::get(int index, pfc_refptr_t *&object)
{
    pfc_refptr_t	*obj;

    ASSERT_EQ(0, pfc_listm_getat(_list, index, CAST_CPTR_P(&obj)));
    object = obj;
}

/*
 * void
 * Int32Factory::checkReference(void)
 *	Ensure that no one refers to any refptr object in this instance.
 *	Google Test's fatal error will be raised on error.
 */
void
Int32Factory::checkReference(void)
{
    int	size(pfc_listm_get_size(_list));

    for (int i(0); i < size; i++) {
        pfc_refptr_t	*ref;

        ASSERT_EQ(0, pfc_listm_getat(_list, i, CAST_CPTR_P(&ref)));
        ASSERT_EQ(1U, ref->pr_refcnt);
    }
}

/*
 * Wrapper macros to detect error in Int32Factory class.
 */
#define	INT32_CREATE(factory, object, value)     \
    do {                                         \
        (factory).create((object), (value));     \
        if (HasFatalFailure()) {                 \
            return;                              \
        }                                        \
    } while (0)

#define	INT32_GET(factory, index, obj)           \
    do {                                         \
        (factory).get(CAST_INT(index), (obj));   \
        if (HasFatalFailure()) {                 \
            return;                              \
        }                                        \
    } while (0)

#define	INT32_CHECK_REFERENCE(factory)          \
    do {                                        \
        (factory).checkReference();             \
        if (HasFatalFailure()) {                \
            return;                             \
        }                                       \
    } while (0)

/*
 * Below are test cases.
 */

/*
 * Test with simple hash table which takes integer for key and value.
 * - No destructor.
 */
TEST(hash, delete_simple)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_hash_create(&hash, NULL, NBUCKETS, 0));
        HashTable	h(hash);

        int	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        int	key(kstart);
        for (uint32_t i(0); i < nelems; i++, key++) {
            int	value(VALUE_SIMPLE(key));
 
            ASSERT_EQ(0, pfc_hash_put(hash, CAST_CPTR(key), CAST_CPTR(value),
                                      0));
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }

        for (uint32_t i(0); i < nelems; i++, key++) {
            pfc_cptr_t	value;

            if (key & 1) {
                int	required(VALUE_SIMPLE(key));

                ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
                ASSERT_EQ(required, CAST_INT(value));
            }
            else {
                // This call is identical to the call of pfc_hash_remove().
                ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }
        ASSERT_EQ(0U, pfc_hash_get_size(hash));

        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }
    }
}

/*
 * Test with simple hash table which takes integer for key and value.
 * - Use key and value destructor.
 */
TEST(hash, delete_simple_dtor)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        HashDtor	dtor;
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_hash_create(&hash, &dtor_key_value, NBUCKETS, 0));
        HashTable	h(hash);

        int	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        int	key(kstart);
        for (uint32_t i(0); i < nelems; i++, key++) {
            int	value(VALUE_SIMPLE(key));
 
            ASSERT_EQ(0, pfc_hash_put(hash, CAST_CPTR(key), CAST_CPTR(value),
                                      0));
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }

        uint32_t	kcount(dtor.getKeyCount());
        uint32_t	vcount(dtor.getValueCount());
        ASSERT_EQ(0U, kcount);
        ASSERT_EQ(0U, vcount);

        size_t	retain;
        if (loop == 0) {
            retain = 0;
        }
        else {
            RANDOM_INTEGER_MAX(rand, retain, count >> 3);
        }

        pfc_cptr_t	lastv(NULL);
        for (uint32_t i(0); i < nelems - retain; i++, key++) {
            pfc_cptr_t	value;

            if (key & 1) {
                int	required(VALUE_SIMPLE(key));

                ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
                ASSERT_EQ(required, CAST_INT(value));

                // Value destructor must not be called.
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(lastv, dtor.getLastValue());
            }
            else {
                // This call is identical to the call of pfc_hash_remove().
                ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));

                // Value destructor must be called.
                vcount++;
                lastv = CAST_CPTR(VALUE_SIMPLE(key));
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(CAST_CPTR(lastv), dtor.getLastValue());
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            // Key destructor must be always called.
            kcount++;
            ASSERT_EQ(kcount, dtor.getKeyCount());
            ASSERT_EQ(CAST_CPTR(key), dtor.getLastKey());

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }
        ASSERT_EQ(retain, pfc_hash_get_size(hash));

        key = kstart + nelems;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
            ASSERT_EQ(kcount, dtor.getKeyCount());
            ASSERT_EQ(vcount, dtor.getValueCount());
        }

        // Clear hash table.
        // This must call key and value destructor.
        ASSERT_EQ(retain, pfc_hash_clear(hash));
        ASSERT_EQ(kcount + retain, dtor.getKeyCount());
        ASSERT_EQ(vcount + retain, dtor.getValueCount());
    }
}

/*
 * Test with hash table which takes int32 refptr object for key and value.
 * - No destructor.
 */
TEST(hash, delete_kref)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_refhash_create(&hash, NULL, pfc_refptr_int32_ops(),
                                        NBUCKETS, 0));
        HashTable	h(hash);

        int32_t	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        Int32Factory	kfac, vfac;
        int32_t	key(kstart);
        for (uint32_t i(0); i < nelems; i++, key++) {
            int32_t	value(VALUE_SIMPLE(key));
            pfc_refptr_t	*kobj, *vobj;

            INT32_CREATE(kfac, kobj, key);
            INT32_CREATE(vfac, vobj, value);
            ASSERT_EQ(0, pfc_hash_put_kvref(hash, kobj, vobj));

            // These refcnt must be held by factory class and hash table.
            ASSERT_EQ(2U, kobj->pr_refcnt);
            ASSERT_EQ(2U, vobj->pr_refcnt);
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        // Put NULL key.
        ASSERT_EQ(0, pfc_hash_put_kvref(hash, NULL, NULL));
        count++;
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }

        for (uint32_t i(0); i < nelems; i++, key++) {
            pfc_refptr_t	*value;
            int32_t	flag(key & 0x3);

            pfc_refptr_t	*kreq, *vreq;
            INT32_GET(kfac, i, kreq);
            INT32_GET(vfac, i, vreq);
            ASSERT_EQ(key, pfc_refptr_int32_value(kreq));
            ASSERT_EQ(VALUE_SIMPLE(key), pfc_refptr_int32_value(vreq));

            if (flag <= 1) {
                if (flag == 0) {
                    ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key),
                                                 CAST_CPTR_P(&value), 0));
                }
                else {
                    ASSERT_EQ(0, pfc_hash_delete_kref(hash, kreq,
                                                      CAST_CPTR_P(&value)));
                }
                ASSERT_EQ(vreq, value);

                // Value reference must be left alone.
                ASSERT_EQ(2U, vreq->pr_refcnt);
                pfc_refptr_put(vreq);
            }
            else {
                if (flag == 2) {
                    // This call is identical to the call of pfc_hash_remove().
                    ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key),
                                                 NULL, 0));
                }
                else {
                    // This call is identical to the call of
                    // pfc_hash_remove_kref().
                    ASSERT_EQ(0, pfc_hash_delete_kref(hash, kreq, NULL));
                }

                // Value reference must be decremented.
                ASSERT_EQ(1U, vreq->pr_refcnt);
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            // Key reference must be always decremented.
            ASSERT_EQ(1U, kreq->pr_refcnt);

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key),
                                              CAST_CPTR_P(&value), 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete_kref(hash, kreq,
                                                   CAST_CPTR_P(&value)));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete_kref(hash, kreq, NULL));
        }
        ASSERT_EQ(1U, pfc_hash_get_size(hash));

        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t		value;
            pfc_refptr_t	*k(pfc_refptr_int32_create(key));
            ASSERT_NE(CAST_REFPTR(NULL), k);

            int	err1(pfc_hash_delete_kref(hash, k, &value));
            int	err2(pfc_hash_delete_kref(hash, k, NULL));
            pfc_refptr_put(k);
            ASSERT_EQ(ENOENT, err1);
            ASSERT_EQ(ENOENT, err2);
        }

        // Remove NULL key.
        if (loop & 1) {
            pfc_cptr_t		value;
            ASSERT_EQ(0, pfc_hash_delete_kref(hash, NULL, &value));
            ASSERT_EQ(CAST_CPTR(NULL), value);
        }
        else {
            ASSERT_EQ(0, pfc_hash_delete_kref(hash, NULL, NULL));
        }
        ASSERT_EQ(0U, pfc_hash_get_size(hash));

        // Ensure all references by hash table are cleared.
        INT32_CHECK_REFERENCE(kfac);
        INT32_CHECK_REFERENCE(vfac);
    }
}

/*
 * Test with hash table which takes int32 refptr object for key and value.
 * - Use key and value destructor.
 */
TEST(hash, delete_kref_dtor)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        HashDtor	dtor;
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_refhash_create(&hash, &dtor_key_value,
                                        pfc_refptr_int32_ops(), NBUCKETS, 0));
        HashTable	h(hash);

        int32_t	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        Int32Factory	kfac, vfac;
        int32_t	key(kstart);
        for (uint32_t i(0); i < nelems; i++, key++) {
            int32_t	value(VALUE_SIMPLE(key));
            pfc_refptr_t	*kobj, *vobj;

            INT32_CREATE(kfac, kobj, key);
            INT32_CREATE(vfac, vobj, value);
            ASSERT_EQ(0, pfc_hash_put_kvref(hash, kobj, vobj));

            // These refcnt must be held by factory class and hash table.
            ASSERT_EQ(2U, kobj->pr_refcnt);
            ASSERT_EQ(2U, vobj->pr_refcnt);
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        // Put NULL key.
        ASSERT_EQ(0, pfc_hash_put_kvref(hash, NULL, NULL));
        count++;
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), &value, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
        }

        uint32_t	kcount(dtor.getKeyCount());
        uint32_t	vcount(dtor.getValueCount());
        ASSERT_EQ(0U, kcount);
        ASSERT_EQ(0U, vcount);

        size_t	retain;
        if (loop == 0) {
            retain = 0;
        }
        else {
            RANDOM_INTEGER_MAX(rand, retain, count >> 3);
        }

        pfc_cptr_t	lastv(NULL);
        for (uint32_t i(0); i < nelems - retain; i++, key++) {
            pfc_refptr_t	*value;
            int32_t	flag(key & 0x3);

            pfc_refptr_t	*kreq, *vreq;
            INT32_GET(kfac, i, kreq);
            INT32_GET(vfac, i, vreq);
            ASSERT_EQ(key, pfc_refptr_int32_value(kreq));
            ASSERT_EQ(VALUE_SIMPLE(key), pfc_refptr_int32_value(vreq));

            if (flag <= 1) {
                if (flag == 0) {
                    ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key),
                                                 CAST_CPTR_P(&value), 0));
                }
                else {
                    ASSERT_EQ(0, pfc_hash_delete_kref(hash, kreq,
                                                      CAST_CPTR_P(&value)));
                }
                ASSERT_EQ(vreq, value);

                // Value reference must be left alone.
                ASSERT_EQ(2U, vreq->pr_refcnt);
                pfc_refptr_put(vreq);

                // Value destructor must not be called.
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(lastv, dtor.getLastValue());
            }
            else {
                if (flag == 2) {
                    // This call is identical to the call of pfc_hash_remove().
                    ASSERT_EQ(0, pfc_hash_delete(hash, CAST_CPTR(key),
                                                 NULL, 0));
                }
                else {
                    // This call is identical to the call of
                    // pfc_hash_remove_kref().
                    ASSERT_EQ(0, pfc_hash_delete_kref(hash, kreq, NULL));
                }

                // Value reference must be decremented.
                ASSERT_EQ(1U, vreq->pr_refcnt);

                // Value destructor must be called.
                vcount++;
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(CAST_CPTR(vreq), dtor.getLastValue());
                lastv = CAST_CPTR(vreq);
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            // Key reference must be always decremented.
            ASSERT_EQ(1U, kreq->pr_refcnt);

            // Key destructor must be always called.
            kcount++;
            ASSERT_EQ(kcount, dtor.getKeyCount());
            ASSERT_EQ(CAST_CPTR(kreq), dtor.getLastKey());

            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key),
                                              CAST_CPTR_P(&value), 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete_kref(hash, kreq,
                                                   CAST_CPTR_P(&value)));
            ASSERT_EQ(ENOENT, pfc_hash_delete(hash, CAST_CPTR(key), NULL, 0));
            ASSERT_EQ(ENOENT, pfc_hash_delete_kref(hash, kreq, NULL));
        }
        ASSERT_EQ(retain + 1, pfc_hash_get_size(hash));

        key = kstart + nelems;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t		value;
            pfc_refptr_t	*k(pfc_refptr_int32_create(key));
            ASSERT_NE(CAST_REFPTR(NULL), k);

            int	err1(pfc_hash_delete_kref(hash, k, &value));
            int	err2(pfc_hash_delete_kref(hash, k, NULL));
            pfc_refptr_put(k);
            ASSERT_EQ(ENOENT, err1);
            ASSERT_EQ(ENOENT, err2);
        }

        // Remove NULL key.
        if (loop & 1) {
            pfc_cptr_t		value;
            ASSERT_EQ(0, pfc_hash_delete_kref(hash, NULL, &value));
            ASSERT_EQ(CAST_CPTR(NULL), value);
        }
        else {
            ASSERT_EQ(0, pfc_hash_delete_kref(hash, NULL, NULL));

            vcount++;
            ASSERT_EQ(vcount, dtor.getValueCount());
            ASSERT_EQ(CAST_CPTR(NULL), dtor.getLastValue());
        }

        kcount++;
        ASSERT_EQ(kcount, dtor.getKeyCount());
        ASSERT_EQ(CAST_CPTR(NULL), dtor.getLastKey());

        // Clear hash table.
        // This must call key and value destructor.
        ASSERT_EQ(retain, pfc_hash_clear(hash));
        ASSERT_EQ(kcount + retain, dtor.getKeyCount());
        ASSERT_EQ(vcount + retain, dtor.getValueCount());

        // Ensure all references by hash table are cleared.
        INT32_CHECK_REFERENCE(kfac);
        INT32_CHECK_REFERENCE(vfac);
    }
}

/*
 * Test with string hash table which takes string for key and integer for value.
 * - No destructor.
 */
TEST(hash, delete_string)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_strhash_create(&hash, NULL, NBUCKETS, 0));
        HashTable	h(hash);

        int	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        int	key(kstart);
        char	keybuf[MAX_STRLEN];
        for (uint32_t i(0); i < nelems; i++, key++) {
            int	value(VALUE_STRING(key));

            KEY_STRING(key, keybuf);
            ASSERT_EQ(0, pfc_strhash_put(hash, keybuf, CAST_CPTR(value), 0));
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        // Put NULL key.
        int	vnull;
        RANDOM_INTEGER(rand, vnull);
        ASSERT_EQ(0, pfc_strhash_put(hash, NULL, CAST_CPTR(vnull), 0));
        count++;
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        // Put empty string key.
        int	vempty;
        RANDOM_INTEGER(rand, vempty);
        ASSERT_EQ(0, pfc_strhash_put(hash, "", CAST_CPTR(vempty), 0));
        count++;
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            KEY_STRING(key, keybuf);
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, &value));
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, NULL));
        }

        for (uint32_t i(0); i < nelems; i++, key++) {
            pfc_cptr_t	value;

            KEY_STRING(key, keybuf);
            if (key & 1) {
                int	required(VALUE_STRING(key));

                ASSERT_EQ(0, pfc_strhash_delete(hash, keybuf, &value));
                ASSERT_EQ(required, CAST_INT(value));
            }
            else {
                // This call is identical to the call of pfc_strhash_remove().
                ASSERT_EQ(0, pfc_strhash_delete(hash, keybuf, NULL));
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, &value));
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, NULL));
        }
        ASSERT_EQ(2U, pfc_hash_get_size(hash));

        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            KEY_STRING(key, keybuf);
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, &value));
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, NULL));
        }

        // Remove empty string key.
        if (loop & 1) {
            ASSERT_EQ(0, pfc_strhash_delete(hash, "", NULL));
        }
        else {
            pfc_cptr_t		value;
            ASSERT_EQ(0, pfc_strhash_delete(hash, "", &value));
            ASSERT_EQ(CAST_CPTR(vempty), value);
        }
        ASSERT_EQ(1U, pfc_hash_get_size(hash));

        // Remove NULL key.
        if (loop & 1) {
            pfc_cptr_t		value;
            ASSERT_EQ(0, pfc_strhash_delete(hash, NULL, &value));
            ASSERT_EQ(CAST_CPTR(vnull), value);
        }
        else {
            ASSERT_EQ(0, pfc_strhash_delete(hash, NULL, NULL));
        }
        ASSERT_EQ(0U, pfc_hash_get_size(hash));
    }
}

/*
 * Test with string hash table which takes string for key and integer for value.
 * - Use key and value destructor.
 */
TEST(hash, delete_string_dtor)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        StrHashDtor	dtor;
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_strhash_create(&hash, &dtor_key_value, NBUCKETS, 0));
        HashTable	h(hash);

        int	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        int	key(kstart);
        char	keybuf[MAX_STRLEN];
        for (uint32_t i(0); i < nelems; i++, key++) {
            int	value(VALUE_STRING(key));

            KEY_STRING(key, keybuf);
            ASSERT_EQ(0, pfc_strhash_put(hash, keybuf, CAST_CPTR(value), 0));
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        // Put NULL key.
        int	vnull;
        RANDOM_INTEGER(rand, vnull);
        ASSERT_EQ(0, pfc_strhash_put(hash, NULL, CAST_CPTR(vnull), 0));
        count++;
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        // Put empty string key.
        int	vempty;
        RANDOM_INTEGER(rand, vempty);
        ASSERT_EQ(0, pfc_strhash_put(hash, "", CAST_CPTR(vempty), 0));
        count++;
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            KEY_STRING(key, keybuf);
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, &value));
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, NULL));
        }

        uint32_t	kcount(dtor.getKeyCount());
        uint32_t	vcount(dtor.getValueCount());
        ASSERT_EQ(0U, kcount);
        ASSERT_EQ(0U, vcount);

        size_t	retain;
        if (loop == 0) {
            retain = 0;
        }
        else {
            RANDOM_INTEGER_MAX(rand, retain, count >> 3);
        }

        pfc_cptr_t	lastv(NULL);
        for (uint32_t i(0); i < nelems - retain; i++, key++) {
            pfc_cptr_t	value;

            KEY_STRING(key, keybuf);
            if (key & 1) {
                int	required(VALUE_STRING(key));

                ASSERT_EQ(0, pfc_strhash_delete(hash, keybuf, &value));
                ASSERT_EQ(required, CAST_INT(value));

                // Value destructor must not be called.
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(lastv, dtor.getLastValue());
            }
            else {
                // This call is identical to the call of pfc_strhash_remove().
                ASSERT_EQ(0, pfc_strhash_delete(hash, keybuf, NULL));

                // Value destructor must be called.
                vcount++;
                lastv = CAST_CPTR(VALUE_STRING(key));
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(lastv, dtor.getLastValue());
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            // Key destructor must be always called.
            kcount++;
            ASSERT_EQ(kcount, dtor.getKeyCount());
            ASSERT_STREQ(keybuf, dtor.getLastString().c_str());

            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, &value));
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, NULL));
        }
        ASSERT_EQ(retain + 2U, pfc_hash_get_size(hash));

        key = kstart + nelems;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            KEY_STRING(key, keybuf);
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, &value));
            ASSERT_EQ(ENOENT, pfc_strhash_delete(hash, keybuf, NULL));
        }

        // Remove empty string key.
        if (loop & 1) {
            ASSERT_EQ(0, pfc_strhash_delete(hash, "", NULL));

            vcount++;
            ASSERT_EQ(vcount, dtor.getValueCount());
            ASSERT_EQ(CAST_CPTR(vempty), dtor.getLastValue());
        }
        else {
            pfc_cptr_t		value;
            ASSERT_EQ(0, pfc_strhash_delete(hash, "", &value));
            ASSERT_EQ(CAST_CPTR(vempty), value);
        }

        kcount++;
        ASSERT_EQ(kcount, dtor.getKeyCount());
        ASSERT_NE(CAST_CPTR(NULL), dtor.getLastKey());
        ASSERT_STREQ("", dtor.getLastString().c_str());

        ASSERT_EQ(retain + 1U, pfc_hash_get_size(hash));

        // Remove NULL key.
        if (loop & 1) {
            pfc_cptr_t		value;
            ASSERT_EQ(0, pfc_strhash_delete(hash, NULL, &value));
            ASSERT_EQ(CAST_CPTR(vnull), value);
        }
        else {
            ASSERT_EQ(0, pfc_strhash_delete(hash, NULL, NULL));

            vcount++;
            ASSERT_EQ(vcount, dtor.getValueCount());
            ASSERT_EQ(CAST_CPTR(vnull), dtor.getLastValue());
        }

        kcount++;
        ASSERT_EQ(kcount, dtor.getKeyCount());
        ASSERT_EQ(CAST_CPTR(NULL), dtor.getLastKey());
        ASSERT_STREQ("", dtor.getLastString().c_str());

        // Clear hash table.
        // This must call key and value destructor.
        ASSERT_EQ(retain, pfc_hash_clear(hash));
        ASSERT_EQ(kcount + retain, dtor.getKeyCount());
        ASSERT_EQ(vcount + retain, dtor.getValueCount());
    }
}

/*
 * Test with u64 hash table which takes unsigned 64-bit integer for key and
 * unsigned 32-bit integer for value.
 * - No destructor.
 */
TEST(hash, delete_u64)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_u64hash_create(&hash, NULL, NBUCKETS, 0));
        HashTable	h(hash);

        uint64_t	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        uint64_t	key(kstart);
        for (uint32_t i(0); i < nelems; i++, key++) {
            uint32_t	value(VALUE_U64(key));

            ASSERT_EQ(0, pfc_u64hash_put(hash, key, CAST_CPTR(value), 0));
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, &value));
            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, NULL));
        }

        for (uint32_t i(0); i < nelems; i++, key++) {
            pfc_cptr_t	value;

            if (key & 1) {
                uint32_t	required(VALUE_U64(key));

                ASSERT_EQ(0, pfc_u64hash_delete(hash, key, &value));
                ASSERT_EQ(required, CAST_UINT32(value));
            }
            else {
                // This call is identical to the call of pfc_u64hash_remove().
                ASSERT_EQ(0, pfc_u64hash_delete(hash, key, NULL));
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, &value));
            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, NULL));
        }
        ASSERT_EQ(0U, pfc_hash_get_size(hash));

        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, &value));
            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, NULL));
        }
    }
}

/*
 * Test with u64 hash table which takes unsigned 64-bit integer for key and
 * unsigned 32-bit integer for value.
 * - Use key and value destructor.
 */
TEST(hash, delete_u64_dtor)
{
    RandomGenerator	rand;

    for (int loop(0); loop < TEST_LOOP; loop++) {
        U64HashDtor	dtor;
        pfc_hash_t	hash;
        ASSERT_EQ(0, pfc_u64hash_create(&hash, &dtor_key_value, NBUCKETS, 0));
        HashTable	h(hash);

        uint64_t	kstart;
        RANDOM_INTEGER(rand, kstart);

        uint32_t	nelems;
        RANDOM_INTEGER_MAX(rand, nelems, MAX_NELEMS);
        if (nelems < MIN_NELEMS) {
            nelems = MAX_NELEMS;
        }

        uint64_t	key(kstart);
        for (uint32_t i(0); i < nelems; i++, key++) {
            uint32_t	value(VALUE_U64(key));

            ASSERT_EQ(0, pfc_u64hash_put(hash, key, CAST_CPTR(value), 0));
        }

        size_t	count(nelems);
        ASSERT_EQ(count, pfc_hash_get_size(hash));

        uint32_t	delta;
        RANDOM_INTEGER_MAX(rand, delta, 100U);
        key = kstart - delta;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, &value));
            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, NULL));
        }

        uint32_t	kcount(dtor.getKeyCount());
        uint32_t	vcount(dtor.getValueCount());
        ASSERT_EQ(0U, kcount);
        ASSERT_EQ(0U, vcount);

        size_t	retain;
        if (loop == 0) {
            retain = 0;
        }
        else {
            RANDOM_INTEGER_MAX(rand, retain, count >> 3);
        }

        pfc_cptr_t	lastv(NULL);
        for (uint32_t i(0); i < nelems - retain; i++, key++) {
            pfc_cptr_t	value;

            if (key & 1) {
                uint32_t	required(VALUE_U64(key));

                ASSERT_EQ(0, pfc_u64hash_delete(hash, key, &value));
                ASSERT_EQ(required, CAST_UINT32(value));

                // Value destructor must not be called.
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(lastv, dtor.getLastValue());
            }
            else {
                // This call is identical to the call of pfc_u64hash_remove().
                ASSERT_EQ(0, pfc_u64hash_delete(hash, key, NULL));

                // Value destructor must be called.
                vcount++;
                lastv = CAST_CPTR(VALUE_U64(key));
                ASSERT_EQ(vcount, dtor.getValueCount());
                ASSERT_EQ(CAST_CPTR(lastv), dtor.getLastValue());
            }

            count--;
            ASSERT_EQ(count, pfc_hash_get_size(hash));

            // Key destructor must be always called.
            kcount++;
            ASSERT_EQ(kcount, dtor.getKeyCount());
            ASSERT_EQ(key, dtor.getLastU64Key());

            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, &value));
            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, NULL));
        }
        ASSERT_EQ(retain, pfc_hash_get_size(hash));

        key = kstart + nelems;
        for (uint32_t i(0); i < delta; i++, key++) {
            pfc_cptr_t	value;

            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, &value));
            ASSERT_EQ(ENOENT, pfc_u64hash_delete(hash, key, NULL));
            ASSERT_EQ(kcount, dtor.getKeyCount());
            ASSERT_EQ(vcount, dtor.getValueCount());
        }

        // Clear hash table.
        // This must call key and value destructor.
        ASSERT_EQ(retain, pfc_hash_clear(hash));
        ASSERT_EQ(kcount + retain, dtor.getKeyCount());
        ASSERT_EQ(vcount + retain, dtor.getValueCount());
    }
}
