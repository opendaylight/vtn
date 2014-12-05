/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_atomic.cc - Test for atomic operations.
 */

#include <cstring>
#include <time.h>
#include <sched.h>
#include <pfc/base.h>
#include <pfc/synch.h>
#include <pfc/atomic.h>
#include <gtest/gtest.h>
#include "test.h"
#include "random.hh"
#include "thread_subr.hh"

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

#define	SHORT_DELAY()                           \
    do {                                        \
        struct timespec	__ts = {0, 100};        \
        nanosleep(&__ts, NULL);                 \
    } while (0)

#define	ATOMIC_NTHREADS			4
#define	ATOMIC_RACE_DURATION		0
#define	ATOMIC_RACE_DURATION_NSEC	200000000

/*
 * Atomic operation wrappers.
 */
#define	ATOMIC_ADD_DECL(type)                   \
    static inline void PFC_FATTR_ALWAYS_INLINE  \
    atomic_add(type *addr, type value)          \
    {                                           \
        pfc_atomic_add_##type(addr, value);     \
    }

#define	ATOMIC_INC_DECL(type)                   \
    static inline void PFC_FATTR_ALWAYS_INLINE  \
    atomic_inc(type *addr)                      \
    {                                           \
        pfc_atomic_inc_##type(addr);            \
    }

#define	ATOMIC_ADD_OLD_DECL(type)                               \
    static inline type PFC_FATTR_ALWAYS_INLINE                  \
    atomic_add_old(type *addr, type value)                      \
    {                                                           \
        return pfc_atomic_add_##type##_old(addr, value);        \
    }

#define	ATOMIC_INC_OLD_DECL(type)                               \
    static inline type PFC_FATTR_ALWAYS_INLINE                  \
    atomic_inc_old(type *addr)                                  \
    {                                                           \
        return pfc_atomic_inc_##type##_old(addr);               \
    }

#define	ATOMIC_SUB_DECL(type)                   \
    static inline void PFC_FATTR_ALWAYS_INLINE  \
    atomic_sub(type *addr, type value)          \
    {                                           \
        pfc_atomic_sub_##type(addr, value);     \
    }

#define	ATOMIC_DEC_DECL(type)                   \
    static inline void PFC_FATTR_ALWAYS_INLINE  \
    atomic_dec(type *addr)                      \
    {                                           \
        pfc_atomic_dec_##type(addr);            \
    }

#define	ATOMIC_SUB_OLD_DECL(type)                               \
    static inline type PFC_FATTR_ALWAYS_INLINE                  \
    atomic_sub_old(type *addr, type value)                      \
    {                                                           \
        return pfc_atomic_sub_##type##_old(addr, value);        \
    }

#define	ATOMIC_DEC_OLD_DECL(type)                               \
    static inline type PFC_FATTR_ALWAYS_INLINE                  \
    atomic_dec_old(type *addr)                                  \
    {                                                           \
        return pfc_atomic_dec_##type##_old(addr);               \
    }

#define	ATOMIC_AND_DECL(type)                                   \
    static inline void PFC_FATTR_ALWAYS_INLINE                  \
    atomic_and(type *addr, type bits)                           \
    {                                                           \
        return pfc_atomic_and_##type(addr, bits);               \
    }

#define	ATOMIC_OR_DECL(type)                                    \
    static inline void PFC_FATTR_ALWAYS_INLINE                  \
    atomic_or(type *addr, type bits)                            \
    {                                                           \
        return pfc_atomic_or_##type(addr, bits);                \
    }

#define	ATOMIC_CAS_DECL(type)                                   \
    static inline type PFC_FATTR_ALWAYS_INLINE                  \
    atomic_cas(type *addr, type newval, type oldval)            \
    {                                                           \
        return pfc_atomic_cas_##type(addr, newval, oldval);     \
    }

#define	ATOMIC_CAS_ACQ_DECL(type)                               \
    static inline type PFC_FATTR_ALWAYS_INLINE                  \
    atomic_cas_acq(type *addr, type newval, type oldval)        \
    {                                                           \
        return pfc_atomic_cas_##type(addr, newval, oldval);     \
    }

#define	ATOMIC_SWAP_DECL(type)                                   \
    static inline type PFC_FATTR_ALWAYS_INLINE                   \
    atomic_swap(type *addr, type value)                          \
    {                                                            \
        return pfc_atomic_swap_##type(addr, value);              \
    }

#define	ATOMIC_SWAP_REL_DECL(type)                               \
    static inline type PFC_FATTR_ALWAYS_INLINE                   \
    atomic_swap_rel(type *addr, type value)                      \
    {                                                            \
        return pfc_atomic_swap_rel_##type(addr, value);          \
    }

#define	ATOMIC_WRAPPER_DECL(type)	\
    ATOMIC_ADD_DECL(type)               \
    ATOMIC_INC_DECL(type)               \
    ATOMIC_ADD_OLD_DECL(type)           \
    ATOMIC_INC_OLD_DECL(type)           \
    ATOMIC_SUB_DECL(type)               \
    ATOMIC_DEC_DECL(type)               \
    ATOMIC_SUB_OLD_DECL(type)           \
    ATOMIC_DEC_OLD_DECL(type)           \
    ATOMIC_AND_DECL(type)               \
    ATOMIC_OR_DECL(type)                \
    ATOMIC_CAS_DECL(type)               \
    ATOMIC_CAS_ACQ_DECL(type)           \
    ATOMIC_SWAP_DECL(type)              \
    ATOMIC_SWAP_REL_DECL(type)

ATOMIC_WRAPPER_DECL(uint8);
ATOMIC_WRAPPER_DECL(uint16);
ATOMIC_WRAPPER_DECL(uint32);
ATOMIC_WRAPPER_DECL(uint64);

static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
atomic_cas(pfc_ptr_t *addr, pfc_ptr_t newval, pfc_ptr_t oldval)
{
    return pfc_atomic_cas_ptr(addr, newval, oldval);
}

static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
atomic_swap(pfc_ptr_t *addr, pfc_ptr_t value)
{
    return pfc_atomic_swap_ptr(addr, value);
}

/*
 * Add uint32_t value to integer or pfc_ptr_t.
 */
template <class T>
static inline void PFC_FATTR_ALWAYS_INLINE
add_value(T &value, uint32_t addend)
{
    value += addend;
}

static inline void PFC_FATTR_ALWAYS_INLINE
add_value(pfc_ptr_t &value, uint32_t addend)
{
    uint8_t	*ptr(reinterpret_cast<uint8_t *>(value));

    value = reinterpret_cast<pfc_ptr_t>(ptr + addend);
}

/*
 * Define atomic operation test buffer type.
 */
#define	REDZONE_SIZE		32
#define	ATOMIC_BUFFER_SIZE(sz)	((REDZONE_SIZE * 2) + (sz))

extern "C" {
    typedef struct {
        uint8_t		redzone1[REDZONE_SIZE];
        uint8_t		target;
        uint8_t		redzone2[REDZONE_SIZE];
    } atomic_uint8_t;
    PFC_TYPE_SIZE_ASSERT(atomic_uint8_t, ATOMIC_BUFFER_SIZE(1));

    typedef struct {
        uint8_t		redzone1[REDZONE_SIZE];
        uint16_t	target;
        uint8_t		redzone2[REDZONE_SIZE];
    } atomic_uint16_t;
    PFC_TYPE_SIZE_ASSERT(atomic_uint16_t, ATOMIC_BUFFER_SIZE(2));

    typedef struct {
        uint8_t		redzone1[REDZONE_SIZE];
        uint32_t	target;
        uint8_t		redzone2[REDZONE_SIZE];
    } atomic_uint32_t;
    PFC_TYPE_SIZE_ASSERT(atomic_uint32_t, ATOMIC_BUFFER_SIZE(4));

    typedef struct {
        uint8_t		redzone1[REDZONE_SIZE];
        uint64_t	target;
        uint8_t		redzone2[REDZONE_SIZE];
    } atomic_uint64_t;
    PFC_TYPE_SIZE_ASSERT(atomic_uint64_t, ATOMIC_BUFFER_SIZE(8));

    typedef struct {
        uint8_t		redzone1[REDZONE_SIZE];
        pfc_ptr_t	target;
        uint8_t		redzone2[REDZONE_SIZE];
    } atomic_ptr_t;
#ifdef	PFC_LP64
    PFC_TYPE_SIZE_ASSERT(atomic_ptr_t, ATOMIC_BUFFER_SIZE(8));
#else	/* !PFC_LP64 */
    PFC_TYPE_SIZE_ASSERT(atomic_ptr_t, ATOMIC_BUFFER_SIZE(4));
#endif	/* PFC_LP64 */

#define	ATOMIC_BUFFER_uint8	atomic_uint8_t
#define	ATOMIC_BUFFER_uint16	atomic_uint16_t
#define	ATOMIC_BUFFER_uint32	atomic_uint32_t
#define	ATOMIC_BUFFER_uint64	atomic_uint64_t
#define	ATOMIC_BUFFER_ptr	atomic_ptr_t

    typedef union {
        uint8_t		u8;
        uint16_t	u16;
        uint32_t	u32;
        uint64_t	u64;
        pfc_ptr_t	ptr;
    } atomic_any_t;

#define	ATOMIC_ANY_uint8_t	u8
#define	ATOMIC_ANY_uint16_t	u16
#define	ATOMIC_ANY_uint32_t	u32
#define	ATOMIC_ANY_uint64_t	u64
#define	ATOMIC_ANY_pfc_ptr_t	ptr

}

/*
 * Base class of thread race test environment.
 */
class AtomicThread;

class AtomicThreadEnv
{
public:
    typedef enum {
        ATS_INIT,
        ATS_RUNNING,
        ATS_FINI,
    } test_state_t;

#define	TYPE_OVERLOAD_DECL(type)                                        \
    AtomicThreadEnv(type initial)                                       \
        : _state(ATS_INIT), _count(0)                                   \
    {                                                                   \
        _value.ATOMIC_ANY_##type = initial;                             \
        _initial.ATOMIC_ANY_##type = initial;                           \
        initCommon();                                                   \
    }                                                                   \
                                                                        \
    void                                                                \
    getAddr(type *&addr)                                                \
    {                                                                   \
        addr = &_value.ATOMIC_ANY_##type;                               \
    }                                                                   \
                                                                        \
    void                                                                \
    getValue(type &value)                                               \
    {                                                                   \
        value = _value.ATOMIC_ANY_##type;                               \
    }                                                                   \
                                                                        \
    void                                                                \
    getInitialValue(type &value)                                        \
    {                                                                   \
        value = _initial.ATOMIC_ANY_##type;                             \
    }                                                                   \
                                                                        \
    virtual void                                                        \
    getRequired(type &value)                                            \
    {                                                                   \
        /*                                                              \
         * Default required value is initial value. It means the test   \
         * never changes the value.                                     \
         * This method may be overwritten as appropriate.               \
         */                                                             \
        getInitialValue(value);                                         \
    }

    TYPE_OVERLOAD_DECL(uint8_t)
    TYPE_OVERLOAD_DECL(uint16_t)
    TYPE_OVERLOAD_DECL(uint32_t)
    TYPE_OVERLOAD_DECL(uint64_t)
    TYPE_OVERLOAD_DECL(pfc_ptr_t)

#undef	TYPE_OVERLOAD_DECL

    virtual ~AtomicThreadEnv()
    {
        shutdownTest();
    }

    template <class T>
    void
    runTest(AtomicThread **threads, const int nthreads, T unused)
    {
        invoke(threads, nthreads, _count);
        if (PFC_EXPECT_TRUE(!::testing::Test::HasFatalFailure())) {
            T	value, required;
            getValue(value);
            getRequired(required);
            ASSERT_EQ(required, value);
        }
    }

    bool
    checkState(void)
    {
        PfcMutex  m(_mutex);
        return (_state == ATS_RUNNING) ? true : false;
    }

    void
    startTest(void)
    {
        setState(ATS_RUNNING);
    }

    void
    shutdownTest(void)
    {
        setState(ATS_FINI);
    }

    uint32_t
    getCount(void) const
    {
        return _count;
    }

    bool	waitForStart(void);
    void	invoke(AtomicThread **threads, int nthreads, uint32_t &count);

private: 
    void	initCommon(void);
    void	setState(test_state_t state);

    volatile test_state_t	_state;
    uint32_t			_count;
    atomic_any_t		_value;
    atomic_any_t		_initial;
    pfc_mutex_t			_mutex;
    pfc_cond_t			_cond;
};

bool
AtomicThreadEnv::waitForStart(void)
{
    PfcMutex  m(_mutex);

    for (;;) {
        test_state_t	state(_state);

        if (state == ATS_RUNNING) {
            break;
        }

        if (PFC_EXPECT_FALSE(state == ATS_FINI)) {
            // Test was cancelled.
            return false;
        }

        pfc_cond_wait(&_cond, &_mutex);
    }

    return true;
}

void
AtomicThreadEnv::initCommon(void)
{
    PFC_MUTEX_INIT(&_mutex);
    pfc_cond_init(&_cond);
}

void
AtomicThreadEnv::setState(test_state_t state)
{
    PfcMutex  m(_mutex);
    if (_state != ATS_FINI) {
        _state = state;
        pfc_cond_broadcast(&_cond);
    }
}

/*
 * Base class of thread race test environment, which uses spinlock
 * implemented by atomic operation.
 */
class LockThreadEnv
    : public AtomicThreadEnv
{
public:
#define	TYPE_OVERLOAD_DECL(type)                                        \
    LockThreadEnv(type initial) : AtomicThreadEnv(initial)              \
    {                                                                   \
        initLock();                                                     \
        if (PFC_EXPECT_FALSE(_locked.ATOMIC_ANY_##type ==               \
                             _unlocked.ATOMIC_ANY_##type)) {            \
            add_value(_unlocked.ATOMIC_ANY_##type, 7);                  \
        }                                                               \
        _lockWord.ATOMIC_ANY_##type = _unlocked.ATOMIC_ANY_##type;      \
    }                                                                   \
                                                                        \
    void                                                                \
    getLockedValue(type &value)                                         \
    {                                                                   \
        value = _locked.ATOMIC_ANY_##type;                              \
    }                                                                   \
                                                                        \
    void                                                                \
    getUnlockedValue(type &value)                                       \
    {                                                                   \
        value = _unlocked.ATOMIC_ANY_##type;                            \
    }                                                                   \
                                                                        \
    type                                                                \
    swapLock(type newvalue)                                             \
    {                                                                   \
        return atomic_swap(&_lockWord.ATOMIC_ANY_##type, newvalue);     \
    }

#define	TYPE_OVERLOAD_UINT_DECL(type)                                   \
    TYPE_OVERLOAD_DECL(type)                                            \
                                                                        \
    type                                                                \
    swapRelLock(type newvalue)                                          \
    {                                                                   \
        return atomic_swap_rel(&_lockWord.ATOMIC_ANY_##type, newvalue); \
    }                                                                   \
                                                                        \
    type                                                                \
    casAcqLock(type newvalue, type oldvalue)                            \
    {                                                                   \
        return atomic_cas_acq(&_lockWord.ATOMIC_ANY_##type, newvalue,   \
                              oldvalue);                                \
    }

    TYPE_OVERLOAD_UINT_DECL(uint8_t)
    TYPE_OVERLOAD_UINT_DECL(uint16_t)
    TYPE_OVERLOAD_UINT_DECL(uint32_t)
    TYPE_OVERLOAD_UINT_DECL(uint64_t)
    TYPE_OVERLOAD_DECL(pfc_ptr_t)

#undef	TYPE_OVERLOAD_DECL

private:
    void	initLock(void);

    atomic_any_t	_lockWord;
    atomic_any_t	_locked;
    atomic_any_t	_unlocked;
};

void
LockThreadEnv::initLock(void)
{
    RandomGenerator	rand;
    try {
        uint8_t	*ptr(reinterpret_cast<uint8_t *>(&_locked));
        rand.fillRandom(ptr, sizeof(_locked));

        ptr = reinterpret_cast<uint8_t *>(&_unlocked);
        rand.fillRandom(ptr, sizeof(_unlocked));
    }
    catch (const RandomError &e) {
        FAIL() << e.what();
    }
}

/*
 * Base class of atomic operation test thread.
 */
class AtomicThread
    : public TempThread
{
public:
    static const uint32_t	QUANTUM = 10;
    static const uint32_t	MIN_COUNT = 20;

    AtomicThread(AtomicThreadEnv &env)
        : _env(&env), _succeeded(false), _count(0), _quantum(0)
    {}

    virtual ~AtomicThread()
    {
        _env->shutdownTest();
    }

    void
    run(void)
    {
        if (_env->waitForStart()) {
            doLoop();
            if (PFC_EXPECT_FALSE(!_succeeded)) {
                _env->shutdownTest();
            }
        }
    }

    bool
    checkState(void)
    {
        return (_env->checkState() || _count < MIN_COUNT);
    }

    uint32_t
    getCount(void) const
    {
        return _count;
    }

    void
    incCount(void)
    {
        _count++;
    }

    void
    setSucceeded(void)
    {
        _succeeded = true;
    }

    void
    checkQuantum(void)
    {
        _quantum++;
        if (_quantum >= QUANTUM) {
            _quantum = 0;
            sched_yield();
        }
    }

    void
    clearQuantum(void)
    {
        _quantum = 0;
    }

protected:
    virtual void	doLoop(void) {}

    AtomicThreadEnv	*_env;
    bool		_succeeded;

private:
    uint32_t		_count;
    uint32_t		_quantum;
};

void
AtomicThreadEnv::invoke(AtomicThread **threads, int nthreads, uint32_t &count)
{
    for (int i(0); i < nthreads; i++) {
        ASSERT_EQ(0, threads[i]->start());
    }

    SHORT_DELAY();
    startTest();

    pfc_timespec_t  ts = {ATOMIC_RACE_DURATION, ATOMIC_RACE_DURATION_NSEC};
    pfc_timespec_t  abs;

    {
        PfcMutex  m(_mutex);
        ASSERT_EQ(0, pfc_clock_abstime(&abs, &ts));

        for (;;) {
            test_state_t	state(_state);
            if (PFC_EXPECT_FALSE(state != ATS_RUNNING)) {
                return;
            }

            int  err(pfc_cond_timedwait_abs(&_cond, &_mutex, &abs));
            if (err == ETIMEDOUT) {
                // Terminate all test threads.
                _state = ATS_FINI;
                pfc_cond_broadcast(&_cond);
                break;
            }
        }
    }

    uint32_t	cnt(0);
    for (int i(0); i < nthreads; i++) {
        ASSERT_EQ(0, threads[i]->join());
        uint32_t  c(threads[i]->getCount());
        ASSERT_NE(0U, c);
        cnt += c;
    }

    count = cnt;
}

/*
 * Fill redzone with random bits.
 */
#define	REDZONE_SETUP(test, saved)                                      \
    do {                                                                \
        RandomGenerator	__rand;                                         \
                                                                        \
        try {                                                           \
            uint8_t	*__ptr(reinterpret_cast<uint8_t *>(&(saved)));  \
            __rand.fillRandom(__ptr, sizeof(saved));                    \
        }                                                               \
        catch (const RandomError &__e) {                                \
            FAIL() << __e.what();                                       \
        }                                                               \
        (test) = (saved);                                               \
    } while (0)

/*
 * Ensure that redzone is not corrupted.
 */
#define	REDZONE_ASSERT(test, saved)                             \
    do {                                                        \
        ASSERT_EQ(0, memcmp((saved).redzone1, (test).redzone1,  \
                            sizeof((saved).redzone1)));         \
        ASSERT_EQ(0, memcmp((saved).redzone2, (test).redzone2,  \
                            sizeof((saved).redzone2)));         \
    } while (0)

/*
 * Body of pfc_atomic_add_XXX() tests.
 */
template <class T, class A>
static void
atomic_add_test(T initial, T diff, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        atomic_add(&buf.target, diff);
        ASSERT_EQ(prev + diff, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial + (diff * loop), buf.target);
}

/*
 * Test case for pfc_atomic_add_XXX().
 */
#define	ATOMIC_ADD_TEST_DECL(type, diff, initial, loop)                 \
    TEST(atomic, add_##type##_##diff)                                   \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_add_test(static_cast<type>(initial),                     \
                        static_cast<type>(diff), buf,                   \
                        static_cast<uint32_t>(loop));                   \
    }

ATOMIC_ADD_TEST_DECL(uint8, 1, 0, UINT8_MAX);
ATOMIC_ADD_TEST_DECL(uint8, 2, 0, UINT8_MAX >> 1);
ATOMIC_ADD_TEST_DECL(uint16, 1, 0, UINT16_MAX);
ATOMIC_ADD_TEST_DECL(uint16, 2, 0, UINT16_MAX >> 1);
ATOMIC_ADD_TEST_DECL(uint32, 1, UINT16_MAX - 100, 100000);
ATOMIC_ADD_TEST_DECL(uint32, 2, UINT32_MAX - 100000, 100000);
ATOMIC_ADD_TEST_DECL(uint64, 1, UINT32_MAX - 100, 100000);
ATOMIC_ADD_TEST_DECL(uint64, 2, UINT64_MAX - 100000, 100000);

/*
 * Body of pfc_atomic_add_XXX() race tests.
 */
class AddThreadEnv
    : public AtomicThreadEnv
{
public:
    template <class T>
    AddThreadEnv(T initial, uint32_t diff = 1)
        : AtomicThreadEnv(initial), _diff(diff) {}

#define	TYPE_OVERLOAD_DECL(type)                             \
    void                                                     \
    getRequired(type &value)                                 \
    {                                                        \
        getInitialValue(value);                              \
        value += (getCount() * _diff);                       \
    }

    TYPE_OVERLOAD_DECL(uint8_t);
    TYPE_OVERLOAD_DECL(uint16_t);
    TYPE_OVERLOAD_DECL(uint32_t);
    TYPE_OVERLOAD_DECL(uint64_t);

#undef	TYPE_OVERLOAD_DECL

    uint32_t
    getDiff(void) const
    {
        return _diff;
    }

private:
    uint32_t	_diff;
};

template <class T>
class AddThread
    : public AtomicThread
{
public:
    AddThread<T>(AddThreadEnv &env) : AtomicThread(env) {}

protected:
    void
    doLoop(void)
    {
        AddThreadEnv	*env(dynamic_cast<AddThreadEnv *>(_env));
        T	*addr, diff(env->getDiff());
        env->getAddr(addr);

        while (checkState()) {
            atomic_add(addr, diff);
            incCount();
            checkQuantum();
        }

        setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_add_XXX().
 */
#define	ATOMIC_RACE_TEST_COMMON(testname, type, t_class, env_class,     \
                                initial, diff)                          \
    TEST(atomic, testname##_##type##_race)                              \
    {                                                                   \
        type  ini(initial);                                             \
        env_class  env(ini, diff);                                      \
                                                                        \
        {                                                               \
            const int  nthreads(ATOMIC_NTHREADS);                       \
            t_class<type>	thr[] = {                               \
                t_class<type>(env),                                     \
                t_class<type>(env),                                     \
                t_class<type>(env),                                     \
                t_class<type>(env),                                     \
            };                                                          \
            AtomicThread	*threads[] = {                          \
                &thr[0], &thr[1], &thr[2], &thr[3],                     \
            };                                                          \
                                                                        \
            env.runTest(threads, nthreads, ini);                        \
        }                                                               \
    }

#define	ATOMIC_ADD_RACE_TEST_COMMON(testname, type, t_class, initial, diff) \
    ATOMIC_RACE_TEST_COMMON(testname, type, t_class, AddThreadEnv,      \
                            initial, diff)
#define	ATOMIC_ADD_RACE_TEST_DECL(type, initial)                        \
    ATOMIC_ADD_RACE_TEST_COMMON(add, type, AddThread, initial, 3)

ATOMIC_ADD_RACE_TEST_DECL(uint8, 0);
ATOMIC_ADD_RACE_TEST_DECL(uint16, 0);
ATOMIC_ADD_RACE_TEST_DECL(uint32, 0xff00);
ATOMIC_ADD_RACE_TEST_DECL(uint64, 0xffffff00);

/*
 * Body of pfc_atomic_inc_XXX() tests.
 */
template <class T, class A>
static void
atomic_inc_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        atomic_inc(&buf.target);
        ASSERT_EQ(prev + 1, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial + loop, buf.target);
}

/*
 * Test case for pfc_atomic_inc_XXX().
 */
#define	ATOMIC_INC_TEST_DECL(type, initial, loop)                       \
    TEST(atomic, inc_##type)                                            \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_inc_test(static_cast<type>(initial), buf,                \
                        static_cast<uint32_t>(loop));                   \
    }

ATOMIC_INC_TEST_DECL(uint8, 0, UINT8_MAX);
ATOMIC_INC_TEST_DECL(uint16, 0, UINT16_MAX);
ATOMIC_INC_TEST_DECL(uint32, UINT16_MAX - 1000, 100000);
ATOMIC_INC_TEST_DECL(uint64, UINT32_MAX - 1000, 100000);

/*
 * Body of pfc_atomic_inc_XXX() race tests.
 */
template <class T>
class IncThread
    : public AddThread<T>
{
public:
    IncThread<T>(AddThreadEnv &env) : AddThread<T>(env) {}

protected:
    void
    doLoop(void)
    {
        AtomicThreadEnv	*env(this->_env);
        T	*addr;
        env->getAddr(addr);

        while (this->checkState()) {
            atomic_inc(addr);
            this->incCount();
            this->checkQuantum();
        }

        this->setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_inc_XXX().
 */
#define	ATOMIC_INC_RACE_TEST_DECL(type, initial)                        \
    ATOMIC_ADD_RACE_TEST_COMMON(inc, type, IncThread, initial, 1)

ATOMIC_INC_RACE_TEST_DECL(uint8, 0);
ATOMIC_INC_RACE_TEST_DECL(uint16, 0);
ATOMIC_INC_RACE_TEST_DECL(uint32, 0xff00);
ATOMIC_INC_RACE_TEST_DECL(uint64, 0xffffff00);

/*
 * Body of pfc_atomic_add_XXX_old() tests.
 */
template <class T, class A>
static void
atomic_add_old_test(T initial, T diff, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	old(atomic_add_old(&buf.target, diff));
        ASSERT_EQ(prev, old);
        ASSERT_EQ(prev + diff, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial + (diff * loop), buf.target);
}

/*
 * Test case for pfc_atomic_add_XXX_old().
 */
#define	ATOMIC_ADD_OLD_TEST_DECL(type, initial, diff, loop)             \
    TEST(atomic, add_##type##_old_##diff)                               \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
         atomic_add_old_test(static_cast<type>(initial),                \
                             static_cast<type>(diff), buf,              \
                             static_cast<uint32_t>(loop));              \
    }

ATOMIC_ADD_OLD_TEST_DECL(uint8, 0, 1, UINT8_MAX);
ATOMIC_ADD_OLD_TEST_DECL(uint8, 0, 2, UINT8_MAX >> 1);
ATOMIC_ADD_OLD_TEST_DECL(uint16, 0, 1, UINT16_MAX);
ATOMIC_ADD_OLD_TEST_DECL(uint16, 0, 2, UINT16_MAX >> 1);
ATOMIC_ADD_OLD_TEST_DECL(uint32, UINT16_MAX - 100, 1, 10000);
ATOMIC_ADD_OLD_TEST_DECL(uint32, UINT32_MAX - (2 * 100000), 2, 100000);
ATOMIC_ADD_OLD_TEST_DECL(uint64, UINT32_MAX - 100, 1, 100000);
ATOMIC_ADD_OLD_TEST_DECL(uint64, UINT64_MAX - (2 * 100000), 2, 100000);

/*
 * Body of pfc_atomic_add_XXX_old() race tests.
 */
template <class T>
class AddOldThread
    : public AddThread<T>
{
public:
    AddOldThread<T>(AddThreadEnv &env) : AddThread<T>(env) {}

protected:
    void
    doLoop(void)
    {
        AddThreadEnv	*env(dynamic_cast<AddThreadEnv *>(this->_env));
        T	*addr, diff(env->getDiff());
        env->getAddr(addr);

        while (this->checkState()) {
            (void)atomic_add_old(addr, diff);
            this->incCount();
            this->checkQuantum();
        }

        this->setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_add_XXX_old().
 */
#define	ATOMIC_ADD_OLD_RACE_TEST_DECL(type, initial)                    \
    ATOMIC_ADD_RACE_TEST_COMMON(add_old, type, AddOldThread, initial, 7)

ATOMIC_ADD_OLD_RACE_TEST_DECL(uint8, 0);
ATOMIC_ADD_OLD_RACE_TEST_DECL(uint16, 0);
ATOMIC_ADD_OLD_RACE_TEST_DECL(uint32, 0xfff0);
ATOMIC_ADD_OLD_RACE_TEST_DECL(uint64, 0xfffffff0);

/*
 * Body of pfc_atomic_inc_XXX_old() tests.
 */
template <class T, class A>
static void
atomic_inc_old_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	old(atomic_inc_old(&buf.target));
        ASSERT_EQ(prev, old);
        ASSERT_EQ(prev + 1, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial + loop, buf.target);
}

/*
 * Test case for pfc_atomic_inc_XXX_old().
 */
#define	ATOMIC_INC_OLD_TEST_DECL(type, initial, loop)                   \
    TEST(atomic, inc_##type##_old)                                      \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_inc_old_test(static_cast<type>(initial), buf,            \
                            static_cast<uint32_t>(loop));               \
    }

ATOMIC_INC_OLD_TEST_DECL(uint8, 0, UINT8_MAX);
ATOMIC_INC_OLD_TEST_DECL(uint16, 0, UINT16_MAX);
ATOMIC_INC_OLD_TEST_DECL(uint32, UINT32_MAX - 100000, 100000);
ATOMIC_INC_OLD_TEST_DECL(uint64, UINT64_MAX - 100000, 100000);

/*
 * Body of pfc_atomic_inc_XXX_old() race tests.
 */
template <class T>
class IncOldThread
    : public AddThread<T>
{
public:
    IncOldThread<T>(AddThreadEnv &env) : AddThread<T>(env) {}

protected:
    void
    doLoop(void)
    {
        AtomicThreadEnv	*env(this->_env);
        T	*addr;
        env->getAddr(addr);

        while (this->checkState()) {
            (void)atomic_inc_old(addr);
            this->incCount();
            this->checkQuantum();
        }

        this->setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_inc_XXX_old().
 */
#define	ATOMIC_INC_OLD_RACE_TEST_DECL(type, initial)                    \
    ATOMIC_ADD_RACE_TEST_COMMON(inc_old, type, IncOldThread, initial, 1)

ATOMIC_INC_OLD_RACE_TEST_DECL(uint8, 0);
ATOMIC_INC_OLD_RACE_TEST_DECL(uint16, 0);
ATOMIC_INC_OLD_RACE_TEST_DECL(uint32, 0xfff0);
ATOMIC_INC_OLD_RACE_TEST_DECL(uint64, 0xfffffff0);

/*
 * Body of pfc_atomic_sub_XXX() tests.
 */
template <class T, class A>
static void
atomic_sub_test(T initial, T diff, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        atomic_sub(&buf.target, diff);
        ASSERT_EQ(prev - diff, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial - (diff * loop), buf.target);
}

/*
 * Test case for pfc_atomic_sub_XXX().
 */
#define	ATOMIC_SUB_TEST_DECL(type, initial, diff, loop)                 \
    TEST(atomic, sub_##type##_##diff)                                   \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_sub_test(static_cast<type>(initial),                     \
                        static_cast<type>(diff), buf,                   \
                        static_cast<uint32_t>(loop));                   \
    }

ATOMIC_SUB_TEST_DECL(uint8, UINT8_MAX, 1, UINT8_MAX);
ATOMIC_SUB_TEST_DECL(uint8, UINT8_MAX, 2, UINT8_MAX >> 1);
ATOMIC_SUB_TEST_DECL(uint16, UINT16_MAX, 1, UINT16_MAX);
ATOMIC_SUB_TEST_DECL(uint16, UINT16_MAX, 2, UINT16_MAX >> 1);
ATOMIC_SUB_TEST_DECL(uint32, UINT32_MAX, 1, 100000);
ATOMIC_SUB_TEST_DECL(uint32, UINT16_MAX + 100, 2, 100000);
ATOMIC_SUB_TEST_DECL(uint64, UINT64_MAX, 1, 100000);
ATOMIC_SUB_TEST_DECL(uint64, UINT32_MAX + 100, 2, 100000);

/*
 * Body of pfc_atomic_sub_XXX() race tests.
 */
class SubThreadEnv
    : public AddThreadEnv
{
public:
    template <class T>
    SubThreadEnv(T initial, uint32_t diff = 1) : AddThreadEnv(initial, diff) {}

#define	TYPE_OVERLOAD_DECL(type)                             \
    void                                                     \
    getRequired(type &value)                                 \
    {                                                        \
        getInitialValue(value);                              \
        value -= (getCount() * getDiff());                   \
    }

    TYPE_OVERLOAD_DECL(uint8_t);
    TYPE_OVERLOAD_DECL(uint16_t);
    TYPE_OVERLOAD_DECL(uint32_t);
    TYPE_OVERLOAD_DECL(uint64_t);

#undef	TYPE_OVERLOAD_DECL
};

template <class T>
class SubThread
    : public AtomicThread
{
public:
    SubThread<T>(SubThreadEnv &env) : AtomicThread(env) {}

protected:
    void
    doLoop(void)
    {
        SubThreadEnv	*env(dynamic_cast<SubThreadEnv *>(_env));
        T	*addr, diff(env->getDiff());
        env->getAddr(addr);

        while (checkState()) {
            atomic_sub(addr, diff);
            incCount();
            checkQuantum();
        }

        setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_sub_XXX().
 */
#define	ATOMIC_SUB_RACE_TEST_COMMON(testname, type, t_class, initial, diff) \
    ATOMIC_RACE_TEST_COMMON(testname, type, t_class, SubThreadEnv,      \
                            initial, diff)

#define	ATOMIC_SUB_RACE_TEST_DECL(type, initial)                        \
    ATOMIC_SUB_RACE_TEST_COMMON(sub, type, SubThread, initial, 3)

ATOMIC_SUB_RACE_TEST_DECL(uint8, UINT8_MAX);
ATOMIC_SUB_RACE_TEST_DECL(uint16, UINT16_MAX);
ATOMIC_SUB_RACE_TEST_DECL(uint32, UINT16_MAX + 0x1000);
ATOMIC_SUB_RACE_TEST_DECL(uint64, UINT32_MAX + 0x1000);

/*
 * Body of pfc_atomic_dec_XXX() tests.
 */
template <class T, class A>
static void
atomic_dec_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        atomic_dec(&buf.target);
        ASSERT_EQ(prev - 1, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial - loop, buf.target);
}

/*
 * Test case for pfc_atomic_dec_XXX().
 */
#define	ATOMIC_DEC_TEST_DECL(type, initial, loop)                       \
    TEST(atomic, dec_##type)                                            \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_dec_test(static_cast<type>(initial), buf,                \
                        static_cast<uint32_t>(loop));                   \
    }

ATOMIC_DEC_TEST_DECL(uint8, UINT8_MAX, UINT8_MAX);
ATOMIC_DEC_TEST_DECL(uint16, UINT16_MAX, UINT16_MAX);
ATOMIC_DEC_TEST_DECL(uint32, UINT32_MAX, 100000);
ATOMIC_DEC_TEST_DECL(uint64, UINT64_MAX, 100000);

/*
 * Body of pfc_atomic_dec_XXX() race tests.
 */
template <class T>
class DecThread
    : public SubThread<T>
{
public:
    DecThread<T>(SubThreadEnv &env) : SubThread<T>(env) {}

protected:
    void
    doLoop(void)
    {
        SubThreadEnv	*env(dynamic_cast<SubThreadEnv *>(this->_env));
        T	*addr;
        env->getAddr(addr);

        while (this->checkState()) {
            atomic_dec(addr);
            this->incCount();
            this->checkQuantum();
        }

        this->setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_dec_XXX().
 */
#define	ATOMIC_DEC_RACE_TEST_DECL(type, initial)                        \
    ATOMIC_SUB_RACE_TEST_COMMON(dec, type, DecThread, initial, 1)

ATOMIC_DEC_RACE_TEST_DECL(uint8, UINT8_MAX);
ATOMIC_DEC_RACE_TEST_DECL(uint16, UINT16_MAX);
ATOMIC_DEC_RACE_TEST_DECL(uint32, UINT16_MAX + 0x10000);
ATOMIC_DEC_RACE_TEST_DECL(uint64, UINT32_MAX + 0x10000);

/*
 * Body of pfc_atomic_sub_XXX_old() tests.
 */
template <class T, class A>
static void
atomic_sub_old_test(T initial, T diff, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    T	required;
    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	old(atomic_sub_old(&buf.target, diff));
        ASSERT_EQ(prev, old);
        required = prev - diff;
        ASSERT_EQ(required, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    required = initial - (diff * loop);
    ASSERT_EQ(required, buf.target);
}

/*
 * Test case for pfc_atomic_sub_XXX_old().
 */
#define	ATOMIC_SUB_OLD_TEST_DECL(type, index, initial, diff, loop)      \
    TEST(atomic, sub_##type##_old_##index)                              \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_sub_old_test(static_cast<type>(initial),                 \
                            static_cast<type>(diff), buf,               \
                            static_cast<uint32_t>(loop));               \
    }

ATOMIC_SUB_OLD_TEST_DECL(uint8, 1, UINT8_MAX, 1, UINT8_MAX);
ATOMIC_SUB_OLD_TEST_DECL(uint8, 2, UINT8_MAX, 2, UINT8_MAX >> 1);
ATOMIC_SUB_OLD_TEST_DECL(uint8, 3, 0, -1, UINT8_MAX);
ATOMIC_SUB_OLD_TEST_DECL(uint8, 4, 10, -4, UINT8_MAX >> 2);
ATOMIC_SUB_OLD_TEST_DECL(uint16, 1, UINT16_MAX, 1, UINT16_MAX);
ATOMIC_SUB_OLD_TEST_DECL(uint16, 2, UINT16_MAX, 2, UINT16_MAX >> 1);
ATOMIC_SUB_OLD_TEST_DECL(uint16, 3, 0, -1, UINT16_MAX);
ATOMIC_SUB_OLD_TEST_DECL(uint16, 4, 10, -4, UINT16_MAX >> 4);
ATOMIC_SUB_OLD_TEST_DECL(uint32, 1, UINT16_MAX + 100, 1, 10000);
ATOMIC_SUB_OLD_TEST_DECL(uint32, 2, UINT32_MAX, 2, 100000);
ATOMIC_SUB_OLD_TEST_DECL(uint32, 3, 0, -1, 100000);
ATOMIC_SUB_OLD_TEST_DECL(uint32, 4, 10, -3, 100000);
ATOMIC_SUB_OLD_TEST_DECL(uint64, 1, UINT32_MAX + 100, 1, 100000);
ATOMIC_SUB_OLD_TEST_DECL(uint64, 2, UINT64_MAX, 2, 100000);
ATOMIC_SUB_OLD_TEST_DECL(uint64, 3, 0, -1, 100000);
ATOMIC_SUB_OLD_TEST_DECL(uint64, 4, 10, -3, 100000);

/*
 * Body of pfc_atomic_sub_XXX_old() race tests.
 */
template <class T>
class SubOldThread
    : public SubThread<T>
{
public:
    SubOldThread<T>(SubThreadEnv &env) : SubThread<T>(env) {}

protected:
    void
    doLoop(void)
    {
        SubThreadEnv	*env(dynamic_cast<SubThreadEnv *>(this->_env));
        T	*addr, diff(env->getDiff());
        env->getAddr(addr);

        while (this->checkState()) {
            (void)atomic_sub_old(addr, diff);
            this->incCount();
            this->checkQuantum();
        }

        this->setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_sub_XXX_old().
 */
#define	ATOMIC_SUB_OLD_RACE_TEST_DECL(type, initial)                    \
    ATOMIC_SUB_RACE_TEST_COMMON(sub_old, type, SubOldThread, initial, 7)

ATOMIC_SUB_OLD_RACE_TEST_DECL(uint8, UINT8_MAX);
ATOMIC_SUB_OLD_RACE_TEST_DECL(uint16, UINT16_MAX);
ATOMIC_SUB_OLD_RACE_TEST_DECL(uint32, UINT32_MAX);
ATOMIC_SUB_OLD_RACE_TEST_DECL(uint64, UINT64_MAX);

/*
 * Body of pfc_atomic_dec_XXX_old() tests.
 */
template <class T, class A>
static void
atomic_dec_old_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	old(atomic_dec_old(&buf.target));
        ASSERT_EQ(prev, old);
        ASSERT_EQ(prev - 1, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(initial - loop, buf.target);
}

/*
 * Test case for pfc_atomic_dec_XXX_old().
 */
#define	ATOMIC_DEC_OLD_TEST_DECL(type, initial, loop)                   \
    TEST(atomic, dec_##type##_old)                                      \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_dec_old_test(static_cast<type>(initial), buf,            \
                            static_cast<uint32_t>(loop));               \
    }

ATOMIC_DEC_OLD_TEST_DECL(uint8, UINT8_MAX, UINT8_MAX);
ATOMIC_DEC_OLD_TEST_DECL(uint16, UINT16_MAX, UINT16_MAX);
ATOMIC_DEC_OLD_TEST_DECL(uint32, UINT32_MAX, 100000);
ATOMIC_DEC_OLD_TEST_DECL(uint64, UINT64_MAX, 100000);

/*
 * Body of pfc_atomic_dec_XXX_old() race tests.
 */
template <class T>
class DecOldThread
    : public SubThread<T>
{
public:
    DecOldThread<T>(SubThreadEnv &env) : SubThread<T>(env) {}

protected:
    void
    doLoop(void)
    {
        SubThreadEnv	*env(dynamic_cast<SubThreadEnv *>(this->_env));
        T	*addr;
        env->getAddr(addr);

        while (this->checkState()) {
            (void)atomic_dec_old(addr);
            this->incCount();
            this->checkQuantum();
        }

        this->setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_dec_XXX_old().
 */
#define	ATOMIC_DEC_OLD_RACE_TEST_DECL(type, initial)                    \
    ATOMIC_SUB_RACE_TEST_COMMON(dec_old, type, DecOldThread, initial, 1)

ATOMIC_DEC_OLD_RACE_TEST_DECL(uint8, UINT8_MAX);
ATOMIC_DEC_OLD_RACE_TEST_DECL(uint16, UINT16_MAX);
ATOMIC_DEC_OLD_RACE_TEST_DECL(uint32, UINT32_MAX);
ATOMIC_DEC_OLD_RACE_TEST_DECL(uint64, UINT64_MAX);

/*
 * Body of pfc_atomic_and_XXX() tests.
 */
template <class T, class A>
static void
atomic_and_test(T unused, A &buf)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = static_cast<T>(-1);
    const uint32_t  nbits(PFC_NBITS(sizeof(T)));

    T current(buf.target);
    T mask(1);
    for (uint32_t i(0); i < nbits; i++, mask <<= 1) {
        atomic_and(&buf.target, ~mask);
        current &= ~mask;
        ASSERT_EQ(current, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(static_cast<T>(0), buf.target);
}

/*
 * Test case for pfc_atomic_and_XXX().
 */
#define	ATOMIC_AND_TEST_DECL(type)                                      \
    TEST(atomic, and_##type)                                            \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_and_test(static_cast<type>(0), buf);                     \
    }

ATOMIC_AND_TEST_DECL(uint8);
ATOMIC_AND_TEST_DECL(uint16);
ATOMIC_AND_TEST_DECL(uint32);
ATOMIC_AND_TEST_DECL(uint64);

/*
 * Body of pfc_atomic_or_XXX() tests.
 */
template <class T, class A>
static void
atomic_or_test(T unused, A &buf)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = 0U;
    const uint32_t  nbits(PFC_NBITS(sizeof(T)));

    T current(buf.target);
    T mask(1);
    for (uint32_t i(0); i < nbits; i++, mask <<= 1) {
        atomic_or(&buf.target, mask);
        current |= mask;
        ASSERT_EQ(current, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    ASSERT_EQ(static_cast<T>(-1), buf.target);
}

/*
 * Test case for pfc_atomic_or_XXX().
 */
#define	ATOMIC_OR_TEST_DECL(type)                                       \
    TEST(atomic, or_##type)                                             \
    {                                                                   \
        ATOMIC_BUFFER_##type  buf;                                      \
                                                                        \
        atomic_or_test(static_cast<type>(0), buf);                      \
    }

ATOMIC_OR_TEST_DECL(uint8);
ATOMIC_OR_TEST_DECL(uint16);
ATOMIC_OR_TEST_DECL(uint32);
ATOMIC_OR_TEST_DECL(uint64);

/*
 * Body of pfc_atomic_cas_XXX() tests.
 */
template <class T, class A>
static void
atomic_cas_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	newval(prev);
        add_value(newval, 1);
        T	ret(atomic_cas(&buf.target, newval, prev));
        ASSERT_EQ(prev, ret);
        ASSERT_EQ(newval, buf.target);
        REDZONE_ASSERT(buf, saved);

        T	dummy(newval);
        add_value(dummy, 10);
        ret = atomic_cas(&buf.target, dummy, prev);
        ASSERT_EQ(newval, ret);
        ASSERT_EQ(newval, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    T	required(initial);
    add_value(required, loop);
    ASSERT_EQ(required, buf.target);
}

/*
 * Test case for pfc_atomic_cas_XXX().
 */
#define	ATOMIC_CAS_TEST_DECL(tname, type, initial, loop)                \
    TEST(atomic, cas_##tname)                                           \
    {                                                                   \
        ATOMIC_BUFFER_##tname  buf;                                     \
                                                                        \
        atomic_cas_test(static_cast<type>(initial), buf,                \
                        static_cast<uint32_t>(loop));                   \
    }

ATOMIC_CAS_TEST_DECL(uint8, uint8, 0, UINT8_MAX);
ATOMIC_CAS_TEST_DECL(uint16, uint16, 0, UINT16_MAX);
ATOMIC_CAS_TEST_DECL(uint32, uint32, UINT16_MAX - 100, 100000);
ATOMIC_CAS_TEST_DECL(uint64, uint64, UINT32_MAX - 100, 100000);
ATOMIC_CAS_TEST_DECL(ptr, pfc_ptr_t, NULL, 100000);

/*
 * Body of pfc_atomic_cas_XXX() race tests.
 */
class CasThreadEnv
    : public AtomicThreadEnv
{
public:
    template <class T>
    CasThreadEnv(T initial) : AtomicThreadEnv(initial) {}

#define	TYPE_OVERLOAD_DECL(type)                             \
    void                                                     \
    getRequired(type &value)                                 \
    {                                                        \
        getInitialValue(value);                              \
        add_value(value, getCount());                        \
    }

    TYPE_OVERLOAD_DECL(uint8_t);
    TYPE_OVERLOAD_DECL(uint16_t);
    TYPE_OVERLOAD_DECL(uint32_t);
    TYPE_OVERLOAD_DECL(uint64_t);
    TYPE_OVERLOAD_DECL(pfc_ptr_t);

#undef	TYPE_OVERLOAD_DECL
};

template <class T>
class CasThread
    : public AtomicThread
{
public:
    CasThread<T>(CasThreadEnv &env) : AtomicThread(env) {}

protected:
    void
    doLoop(void)
    {
        T	*addr;
        _env->getAddr(addr);

        while (checkState()) {
            T	oldval(*addr);
            T	newval(oldval);
            add_value(newval, 1);
            T	ret(atomic_cas(addr, newval, oldval));
            if (ret == oldval) {
                this->incCount();
            }

            checkQuantum();
        }

        setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_cas_XXX().
 */
#define	ATOMIC_CAS_RACE_TEST_DECL(tname, type, initial)                 \
    TEST(atomic, cas_##tname##_race)                                    \
    {                                                                   \
        type  ini(initial);                                             \
        CasThreadEnv  env(ini);                                         \
                                                                        \
        {                                                               \
            const int  nthreads(ATOMIC_NTHREADS);                       \
            CasThread<type>	thr[] = {                               \
                CasThread<type>(env),                                   \
                CasThread<type>(env),                                   \
                CasThread<type>(env),                                   \
                CasThread<type>(env),                                   \
            };                                                          \
            AtomicThread	*threads[] = {                          \
                &thr[0], &thr[1], &thr[2], &thr[3],                     \
            };                                                          \
                                                                        \
            env.runTest(threads, nthreads, ini);                        \
        }                                                               \
    }

ATOMIC_CAS_RACE_TEST_DECL(uint8, uint8, 0);
ATOMIC_CAS_RACE_TEST_DECL(uint16, uint16, 0);
ATOMIC_CAS_RACE_TEST_DECL(uint32, uint32, 0xf000);
ATOMIC_CAS_RACE_TEST_DECL(uint64, uint64, 0xfffff000);
ATOMIC_CAS_RACE_TEST_DECL(ptr, pfc_ptr_t, NULL);

/*
 * Body of pfc_atomic_cas_acq_XXX() tests.
 */
template <class T, class A>
static void
atomic_cas_acq_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	newval(prev);
        add_value(newval, 1);
        T	ret(atomic_cas_acq(&buf.target, newval, prev));
        ASSERT_EQ(prev, ret);
        ASSERT_EQ(newval, buf.target);
        REDZONE_ASSERT(buf, saved);

        T	dummy(newval);
        add_value(dummy, 10);
        ret = atomic_cas_acq(&buf.target, dummy, prev);
        ASSERT_EQ(newval, ret);
        ASSERT_EQ(newval, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    T	required(initial);
    add_value(required, loop);
    ASSERT_EQ(required, buf.target);
}

/*
 * Test case for pfc_atomic_cas_acq_XXX().
 */
#define	ATOMIC_CAS_ACQ_TEST_DECL(tname, type, initial, loop)            \
    TEST(atomic, cas_acq_##tname)                                       \
    {                                                                   \
        ATOMIC_BUFFER_##tname  buf;                                     \
                                                                        \
        atomic_cas_acq_test(static_cast<type>(initial), buf,            \
                            static_cast<uint32_t>(loop));               \
    }

ATOMIC_CAS_ACQ_TEST_DECL(uint8, uint8, 0, UINT8_MAX);
ATOMIC_CAS_ACQ_TEST_DECL(uint16, uint16, 0, UINT16_MAX);
ATOMIC_CAS_ACQ_TEST_DECL(uint32, uint32, UINT16_MAX - 3000, 100000);
ATOMIC_CAS_ACQ_TEST_DECL(uint64, uint64, UINT32_MAX - 3000, 100000);

/*
 * Body of pfc_atomic_swap_XXX() tests.
 */
template <class T, class A>
static void
atomic_swap_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	newval(prev);
        add_value(newval, 1);
        T	ret(atomic_swap(&buf.target, newval));
        ASSERT_EQ(prev, ret);
        ASSERT_EQ(newval, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    T	required(initial);
    add_value(required, loop);
    ASSERT_EQ(required, buf.target);
}

/*
 * Test case for pfc_atomic_swap_XXX().
 */
#define	ATOMIC_SWAP_TEST_DECL(tname, type, initial, loop)               \
    TEST(atomic, swap_##tname)                                          \
    {                                                                   \
        ATOMIC_BUFFER_##tname  buf;                                     \
                                                                        \
        atomic_swap_test(static_cast<type>(initial), buf,               \
                         static_cast<uint32_t>(loop));                  \
    }

ATOMIC_SWAP_TEST_DECL(uint8, uint8, 0, UINT8_MAX);
ATOMIC_SWAP_TEST_DECL(uint16, uint16, 0, UINT16_MAX);
ATOMIC_SWAP_TEST_DECL(uint32, uint32, UINT16_MAX - 100, 100000);
ATOMIC_SWAP_TEST_DECL(uint64, uint64, UINT32_MAX - 100, 100000);
ATOMIC_SWAP_TEST_DECL(ptr, pfc_ptr_t, NULL, 100000);

/*
 * Body of pfc_atomic_swap_XXX() race tests.
 */
class SwapThreadEnv
    : public LockThreadEnv
{
public:
    template <class T>
    SwapThreadEnv(T initial) : LockThreadEnv(initial) {}

#define	TYPE_OVERLOAD_DECL(type)                             \
    void                                                     \
    getRequired(type &value)                                 \
    {                                                        \
        getInitialValue(value);                              \
        add_value(value, getCount());                        \
    }

    TYPE_OVERLOAD_DECL(uint8_t);
    TYPE_OVERLOAD_DECL(uint16_t);
    TYPE_OVERLOAD_DECL(uint32_t);
    TYPE_OVERLOAD_DECL(uint64_t);
    TYPE_OVERLOAD_DECL(pfc_ptr_t);

#undef	TYPE_OVERLOAD_DECL
};

template <class T>
class SwapThread
    : public AtomicThread
{
public:
    SwapThread<T>(SwapThreadEnv &env)
      : AtomicThread(env) {}

protected:
    void
    doLoop(void)
    {
        SwapThreadEnv	*env(dynamic_cast<SwapThreadEnv *>(_env));
        T	*addr, locked, unlocked;
        env->getAddr(addr);
        env->getLockedValue(locked);
        env->getUnlockedValue(unlocked);

        while (checkState()) {
            T	ret;
            for (;;) {
                ret = env->swapLock(locked);
                if (ret == unlocked) {
                    break;
                }

                // Lost the race.
                ASSERT_EQ(locked, ret);
                clearQuantum();
                if (!checkState()) {
                    setSucceeded();
                    return;
                }
                sched_yield();
            }

            T	oldval(*addr);
            T	newval(oldval);
            add_value(newval, 1);
            ret = atomic_swap(addr, newval);
            ASSERT_EQ(oldval, ret);

            ret = env->swapLock(unlocked);
            ASSERT_EQ(locked, ret) << "unlocked = " << unlocked;

            incCount();
            checkQuantum();
        }

        setSucceeded();
    }
};

/*
 * Thread race test for pfc_atomic_swap_XXX().
 */
#define	ATOMIC_SWAP_RACE_TEST_DECL(tname, type, initial)                \
    TEST(atomic, swap_##tname##_race)                                   \
    {                                                                   \
        type  ini(initial);                                             \
        SwapThreadEnv   env(ini);                                       \
        if (PFC_EXPECT_FALSE(HasFatalFailure())) {                      \
            return;                                                     \
        }                                                               \
                                                                        \
        {                                                               \
            const int  nthreads(ATOMIC_NTHREADS);                       \
            SwapThread<type>  thr[] = {                                 \
                SwapThread<type>(env),                                  \
                SwapThread<type>(env),                                  \
                SwapThread<type>(env),                                  \
                SwapThread<type>(env),                                  \
            };                                                          \
            AtomicThread	*threads[] = {                          \
                &thr[0], &thr[1], &thr[2], &thr[3],                     \
            };                                                          \
                                                                        \
            env.runTest(threads, nthreads, ini);                        \
        }                                                               \
    }

ATOMIC_SWAP_RACE_TEST_DECL(uint8, uint8, 0);
ATOMIC_SWAP_RACE_TEST_DECL(uint16, uint16, 0);
ATOMIC_SWAP_RACE_TEST_DECL(uint32, uint32, 0xf000);
ATOMIC_SWAP_RACE_TEST_DECL(uint64, uint64, 0xfffff000);
ATOMIC_SWAP_RACE_TEST_DECL(ptr, pfc_ptr_t, NULL);

/*
 * Body of pfc_atomic_swap_rel_XXX() tests.
 */
template <class T, class A>
static void
atomic_swap_rel_test(T initial, A &buf, uint32_t loop)
{
    A  saved;

    REDZONE_SETUP(buf, saved);
    buf.target = initial;

    for (uint32_t i(0); i < loop; i++) {
        T	prev(buf.target);
        T	newval(prev);
        add_value(newval, 1);
        T	ret(atomic_swap_rel(&buf.target, newval));
        ASSERT_EQ(prev, ret);
        ASSERT_EQ(newval, buf.target);
        REDZONE_ASSERT(buf, saved);
    }

    T	required(initial);
    add_value(required, loop);
    ASSERT_EQ(required, buf.target);
}

/*
 * Test case for pfc_atomic_swap_rel_XXX().
 */
#define	ATOMIC_SWAP_REL_TEST_DECL(tname, type, initial, loop)           \
    TEST(atomic, swap_rel_##tname)                                      \
    {                                                                   \
        ATOMIC_BUFFER_##tname  buf;                                     \
                                                                        \
        atomic_swap_rel_test(static_cast<type>(initial), buf,           \
                             static_cast<uint32_t>(loop));              \
    }

ATOMIC_SWAP_REL_TEST_DECL(uint8, uint8, 0, UINT8_MAX);
ATOMIC_SWAP_REL_TEST_DECL(uint16, uint16, 0, UINT16_MAX);
ATOMIC_SWAP_REL_TEST_DECL(uint32, uint32, UINT16_MAX - 1000, 100000);
ATOMIC_SWAP_REL_TEST_DECL(uint64, uint64, UINT32_MAX - 1000, 100000);

/*
 * Body of pfc_atomic_read_uint64().
 */
class Read64Thread
    : public AtomicThread
{
public:
    static const uint32_t	MAX_LOOP = 0x80000;

    Read64Thread(AtomicThreadEnv &env) : AtomicThread(env) {}

    uint64_t
    createValue(uint32_t value)
    {
        return (static_cast<uint64_t>(value) << 32 | value);
    }

protected:
    void
    doLoop(void)
    {
        uint64_t	*addr;
        _env->getAddr(addr);

        // Update upper and lower value on another thread.
        for (uint32_t i(0); checkState() && i <= MAX_LOOP; i++) {
            uint64_t	newval(createValue(i));
            uint64_t	prev(*addr);
            uint64_t	ret(atomic_swap(addr, newval));
            ASSERT_EQ(prev, ret);
        }

        setSucceeded();
    }
};

/*
 * Test case for pfc_atomic_read_uint64().
 */
TEST(atomic, read_uint64)
{
    AtomicThreadEnv	env(static_cast<uint64_t>(0));

    {
        Read64Thread	thr(env);
        ASSERT_EQ(0, thr.start());
        SHORT_DELAY();

        uint64_t	*addr;
        env.getAddr(addr);
        uint32_t	prev(0);

        // Read value using pfc_atomic_read_uint64().
        env.startTest();
        for (;;) {
            uint64_t	ret(pfc_atomic_read_uint64(addr));
            uint32_t	upper(static_cast<uint32_t>(ret >> 32));
            uint32_t	lower(static_cast<uint32_t>(ret & 0xffffffffU));
            ASSERT_EQ(upper, lower);
            ASSERT_GE(upper, prev);
            prev = upper;
            if (upper == Read64Thread::MAX_LOOP) {
                break;
            }
        }

        ASSERT_EQ(0, thr.join());
    }
}

/*
 * Body of spinlock test.
 */
class SpinLockThreadEnv
    : public LockThreadEnv
{
public:
    static const uint64_t	INITIAL = UINT32_MAX - 0x100;
    static const uint64_t	DELTA = 3U;

    template <class T>
    SpinLockThreadEnv(T initial)
        : LockThreadEnv(initial), _value1(0), _value2(INITIAL) {}

    void
    update(void)
    {
        _value1++;
        _value2 += DELTA;
    }

    template <class T>
    void
    runTest(AtomicThread **threads, const int nthreads, T unused)
    {
        AtomicThreadEnv::runTest(threads, nthreads, unused);
        checkValue();
    }

    uint32_t
    getValue1(void) const
    {
        return _value1;
    }

    uint64_t
    getValue2(void) const
    {
        return _value2;
    }

private:
    void	checkValue(void);
    uint32_t	_value1;
    uint64_t	_value2;

};

void
SpinLockThreadEnv::checkValue(void)
{
    if (PFC_EXPECT_TRUE(!::testing::Test::HasFatalFailure())) {
        uint32_t	count(getCount());
        ASSERT_EQ(count, _value1);

        uint64_t	req64(INITIAL + (count * DELTA));
        ASSERT_EQ(req64, _value2);
    }
}

template <class T>
class SpinLockThread
    : public AtomicThread
{
public:
    SpinLockThread<T>(SpinLockThreadEnv &env) : AtomicThread(env) {}

protected:
    void
    doLoop(void)
    {
        SpinLockThreadEnv	*env(dynamic_cast<SpinLockThreadEnv *>(_env));
        T	locked, unlocked;
        env->getLockedValue(locked);
        env->getUnlockedValue(unlocked);

        while (checkState()) {
            T	ret;
            for (;;) {
                ret = env->casAcqLock(locked, unlocked);
                if (ret == unlocked) {
                    break;
                }

                // Lost the race.
                ASSERT_EQ(locked, ret);
                clearQuantum();
                if (!checkState()) {
                    setSucceeded();
                    return;
                }
                sched_yield();
            }
            env->update();

            ret = env->swapRelLock(unlocked);
            ASSERT_EQ(locked, ret);

            incCount();
            checkQuantum();
        }

        setSucceeded();
    }
};

/*
 * Spin lock test using pfc_atomic_cas_acq_XXX() and pfc_atomic_swap_rel_XXX().
 */
#define	ATOMIC_SPINLOCK_TEST_DECL(type)                                 \
    TEST(atomic, spinlock_##type)                                       \
    {                                                                   \
        type  ini(0);                                                   \
        SpinLockThreadEnv  env(ini);                                    \
        if (PFC_EXPECT_FALSE(HasFatalFailure())) {                      \
            return;                                                     \
        }                                                               \
                                                                        \
        {                                                               \
            const int  nthreads(ATOMIC_NTHREADS);                       \
            SpinLockThread<type>  thr[] = {                             \
                SpinLockThread<type>(env),                              \
                SpinLockThread<type>(env),                              \
                SpinLockThread<type>(env),                              \
                SpinLockThread<type>(env),                              \
            };                                                          \
            AtomicThread	*threads[] = {                          \
                &thr[0], &thr[1], &thr[2], &thr[3],                     \
            };                                                          \
                                                                        \
            env.runTest(threads, nthreads, ini);                        \
        }                                                               \
    }

ATOMIC_SPINLOCK_TEST_DECL(uint8);
ATOMIC_SPINLOCK_TEST_DECL(uint16);
ATOMIC_SPINLOCK_TEST_DECL(uint32);
ATOMIC_SPINLOCK_TEST_DECL(uint64);

/*
 * Body of atomic counter test.
 */
template <class T>
class CounterThread
    : public AtomicThread
{
public:
    CounterThread<T>(AtomicThreadEnv &env) : AtomicThread(env) {}

protected:
    void
    doLoop(void)
    {
        T	*addr;
        _env->getAddr(addr);

        while (checkState()) {
            // Increment and decrement counter, but keeps sum of differences
            // as zero.
            atomic_add(addr, 10);
            atomic_inc(addr);
            (void)atomic_add_old(addr, static_cast<T>(-1));
            (void)atomic_dec_old(addr);
            atomic_sub(addr, 123);
            (void)atomic_sub_old(addr, 3U);
            atomic_add(addr, static_cast<T>(-5));
            atomic_sub(addr, static_cast<T>(-1));
            atomic_add(addr, 122);
            atomic_dec(addr);
            atomic_inc(addr);
            atomic_dec(addr);
            (void)atomic_inc_old(addr);
            atomic_sub(addr, 6U);
            atomic_dec(addr);
            (void)atomic_sub_old(addr, static_cast<T>(-5));
            (void)atomic_inc_old(addr);

            incCount();
            checkQuantum();
        }

        setSucceeded();
    }
};

/*
 * Atomic counter test.
 */
#define	ATOMIC_COUNTER_TEST_DECL(type)                                  \
    TEST(atomic, counter_##type)                                        \
    {                                                                   \
        const type  ini(static_cast<type>(-1) >>                        \
                        (PFC_NBITS(sizeof(type)) >> 1));                \
        AtomicThreadEnv  env(ini);                                      \
                                                                        \
        {                                                               \
            const int  nthreads(ATOMIC_NTHREADS);                       \
            CounterThread<type>	thr[] = {                               \
                CounterThread<type>(env),                               \
                CounterThread<type>(env),                               \
                CounterThread<type>(env),                               \
                CounterThread<type>(env),                               \
            };                                                          \
            AtomicThread	*threads[] = {                          \
                &thr[0], &thr[1], &thr[2], &thr[3],                     \
            };                                                          \
                                                                        \
            env.runTest(threads, nthreads, ini);                        \
        }                                                               \
    }

ATOMIC_COUNTER_TEST_DECL(uint8);
ATOMIC_COUNTER_TEST_DECL(uint16);
ATOMIC_COUNTER_TEST_DECL(uint32);
ATOMIC_COUNTER_TEST_DECL(uint64);

/*
 * Body of bitmask test.
 */
class BitMaskThreadEnv
    : public AtomicThreadEnv
{
public:
    BitMaskThreadEnv() : AtomicThreadEnv(static_cast<uint64_t>(0)) {}
};

template <class T>
class BitMaskThread
    : public AtomicThread
{
public:
    BitMaskThread<T>(BitMaskThreadEnv &env, T mask)
      : AtomicThread(env), _mask(mask) {}

protected:
    void
    doLoop(void)
    {
        BitMaskThreadEnv  *env(dynamic_cast<BitMaskThreadEnv *>(_env));
        T	*addr, mask(_mask);
        env->getAddr(addr);

        volatile T	*ptr(reinterpret_cast<volatile T *>(addr));

        while (checkState()) {
            // Set bits.
            // This routine assumes that another thread never modifies bits
            // in the given mask.
            atomic_or(addr, mask);
            sched_yield();

            T	value(*ptr);
            ASSERT_EQ(mask, value & mask) << "value = " << value;

            // Clear bits.
            atomic_and(addr, ~mask);
            sched_yield();

            value = *ptr;
            ASSERT_EQ(static_cast<T>(0), value & mask);
            incCount();
            checkQuantum();
        }

        setSucceeded();
    }

private:
    T	_mask;
};

/*
 * Atomic bitmask operation test using pfc_atomic_and_XXX() and
 * pfc_atomic_or_XXX().
 *
 * Remarks:
 *	All bits in mask1, mask2, mask3, and mask4 must be exclusive.
 */
#define	ATOMIC_BITMASK_TEST_DECL(type, mask1, mask2, mask3, mask4)	\
    TEST(atomic, bitmask_##type)                                        \
    {                                                                   \
        type	m1(mask1), m2(mask2), m3(mask3), m4(mask4);             \
        ASSERT_EQ(static_cast<type>(0), m1 & m2 & m3 & m4);             \
        BitMaskThreadEnv  env;                                          \
                                                                        \
        {                                                               \
            const int  nthreads(ATOMIC_NTHREADS);                       \
            BitMaskThread<type>	thr[] = {                               \
                BitMaskThread<type>(env, m1),                           \
                BitMaskThread<type>(env, m2),                           \
                BitMaskThread<type>(env, m3),                           \
                BitMaskThread<type>(env, m4),                           \
            };                                                          \
            AtomicThread	*threads[] = {                          \
                &thr[0], &thr[1], &thr[2], &thr[3],                     \
            };                                                          \
                                                                        \
            env.runTest(threads, nthreads, m1);                         \
        }                                                               \
    }

ATOMIC_BITMASK_TEST_DECL(uint8, 0x22, 0x48, 0x81, 0x14);
ATOMIC_BITMASK_TEST_DECL(uint16, 0x0468, 0x2980, 0xd002, 0x0215);
ATOMIC_BITMASK_TEST_DECL(uint32,
                         0x8a038041, 0x40ac2a00, 0x214004b4, 0x1410510a);
ATOMIC_BITMASK_TEST_DECL(uint64,
                         0x8164a08948042022ULL, 0x20911a2482120e00ULL,
                         0x1800450234491111ULL, 0x460a005001a0c0ccULL);
