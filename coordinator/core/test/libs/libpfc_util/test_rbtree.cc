/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_rbtree.cc: Red-Black Tree test
 */

#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include "rbtree_test.h"
#include "random.hh"

#define	FULL_CHECK_THRESHOLD	2000
#define	MIN_TEST_NODES		2
#define	MAX_TEST_NODES		100000

#define	LOOP_CLEAR		32
#define	CLEAR_MIN_NODES		2
#define	CLEAR_MAX_NODES		10000

#define	LOOP_COMP		32
#define	COMP_MIN_NODES		2
#define	COMP_MAX_NODES		10000

#define	RBNODE_NULL		(reinterpret_cast<pfc_rbnode_t *>(NULL))

/*
 * Context to check tree state.
 */
struct rbtest_state {
    uint32_t	rs_min;			/* minimum node depth */
    uint32_t	rs_max;			/* maximum node depth */
    uint32_t	rs_black;		/* number of black node */
    uint32_t	rs_count;		/* number of nodes */

    rbtest_state()
        : rs_min(UINT32_MAX), rs_max(0), rs_black(0), rs_count(0)
    {}
};

/*
 * Data structure used for default comparator test.
 */
extern "C" {
    struct rbcomp_node;
    typedef struct rbcomp_node	rbcomp_node_t;

    struct rbcomp_node {
        union {
            int32_t	i32;
            uint32_t	u32;
            int64_t	i64;
            uint64_t	u64;
        } rn_value;
        pfc_rbnode_t	rn_node;
        rbcomp_node_t	*rn_next;
    };
}

#define	RBCOMP_NODE2PTR(node)                           \
    PFC_CAST_CONTAINER((node), rbcomp_node_t, rn_node)

/*
 * Functions to obtain key value of rbcomp_node_t.
 */

static pfc_cptr_t
rbcomp_getkey_int32(pfc_rbnode_t *node)
{
    rbcomp_node_t	*rnp = RBCOMP_NODE2PTR(node);

    return (pfc_cptr_t)(uintptr_t)rnp->rn_value.i32;
}

static pfc_cptr_t
rbcomp_getkey_uint32(pfc_rbnode_t *node)
{
    rbcomp_node_t	*rnp = RBCOMP_NODE2PTR(node);

    return (pfc_cptr_t)(uintptr_t)rnp->rn_value.u32;
}

static pfc_cptr_t
rbcomp_getkey_int64(pfc_rbnode_t *node)
{
    rbcomp_node_t	*rnp = RBCOMP_NODE2PTR(node);

    return PFC_RBTREE_KEY64(rnp->rn_value.i64);
}

static pfc_cptr_t
rbcomp_getkey_uint64(pfc_rbnode_t *node)
{
    rbcomp_node_t	*rnp = RBCOMP_NODE2PTR(node);

    return PFC_RBTREE_KEY64(rnp->rn_value.u64);
}

#define	RBTEST_ALLOC(rp, value)                  \
    do {                                         \
        (rp) = rbtest_alloc(&_nodes, value);     \
        ASSERT_NE((rbtest_t *)NULL, (rp));       \
    } while (0)

#define	RBTREE_CHECK()                                                  \
    do {                                                                \
        rbtest_state	__state;                                        \
                                                                        \
        checkState(&__state, _tree.rb_root);                            \
        if (PFC_EXPECT_FALSE(HasFatalFailure())) {                      \
            return;                                                     \
        }                                                               \
        ASSERT_GE(__state.rs_min * 2, __state.rs_max)                   \
            << "Maximum node depth must not exceed the twice of minimum: " \
            "max=" << __state.rs_max << ": min=" << __state.rs_min;     \
    } while (0)

#define	CHECK_NODE_ORDER(number)                        \
    do {                                                \
        checkOrder(number);                             \
        if (PFC_EXPECT_FALSE(HasFatalFailure())) {      \
            return;                                     \
        }                                               \
    } while (0)

#define	CHECK_SEQUENTIAL_NODE_ORDER(min, max)           \
    do {                                                \
        checkSequentialOrder((min), (max));             \
        if (PFC_EXPECT_FALSE(HasFatalFailure())) {      \
            return;                                     \
        }                                               \
    } while (0)

#define	RBTEST_LOOP	32

class RedBlackTreeTest
    : public testing::Test
{
protected:
    RedBlackTreeTest()
        : _rbtestTable(NULL), _rbcompList(NULL)
    {}
    virtual void	SetUp(void);
    virtual void	TearDown(void);

    void	sequentialAscTest(uint32_t number);
    void	sequentialDescTest(uint32_t number);
    void	randomTest(uint32_t number);
    void	clearTest(uint32_t number);
    template<class T>
    void	comparatorTest(pfc_rbtree_t *treep, uint32_t number,
                               bool key64 = false);

    void	checkState(rbtest_state *rsp, pfc_rbnode_t *node,
                           uint32_t depth = 1);
    void	checkOrder(uint32_t number);
    void	checkSequentialOrder(uint32_t min, uint32_t max);

    inline void
    setToTable(uint32_t index, rbtest_t *rp)
    {
        *(_rbtestTable + index) = rp;
    }

    inline void
    removeFromTable(uint32_t index)
    {
        setToTable(index, NULL);
    }

    inline rbtest_t *
    getFromTable(uint32_t index)
    {
        return *(_rbtestTable + index);
    }

    inline void
    addDtorCount(uint32_t count)
    {
        _dtorCount += count;
    }

    inline void
    resetDtorCount(void)
    {
        _dtorCount = 0;
    }

    inline uint32_t
    getDtorCount(void) const
    {
        return _dtorCount;
    }

    inline void
    getCompValue(rbcomp_node_t *rnp, int32_t &value)
    {
        value = rnp->rn_value.i32;
    }

    inline void
    getCompValue(rbcomp_node_t *rnp, uint32_t &value)
    {
        value = rnp->rn_value.u32;
    }

    inline void
    getCompValue(rbcomp_node_t *rnp, int64_t &value)
    {
        value = rnp->rn_value.i64;
    }

    inline void
    getCompValue(rbcomp_node_t *rnp, uint64_t &value)
    {
        value = rnp->rn_value.u64;
    }

    inline void
    setCompValue(rbcomp_node_t *rnp, int32_t value)
    {
        rnp->rn_value.i32 = value;
    }

    inline void
    setCompValue(rbcomp_node_t *rnp, uint32_t value)
    {
        rnp->rn_value.u32 = value;
    }

    inline void
    setCompValue(rbcomp_node_t *rnp, int64_t value)
    {
        rnp->rn_value.i64 = value;
    }

    inline void
    setCompValue(rbcomp_node_t *rnp, uint64_t value)
    {
        rnp->rn_value.u64 = value;
    }

#ifndef	PFC_LP64
    inline void
    getCompValue(rbcomp_node_t *rnp, pfc_long_t &value)
    {
        value = (pfc_long_t)rnp->rn_value.i32;
    }

    inline void
    getCompValue(rbcomp_node_t *rnp, pfc_ulong_t &value)
    {
        value = (pfc_ulong_t)rnp->rn_value.u32;
    }

    inline void
    setCompValue(rbcomp_node_t *rnp, pfc_long_t value)
    {
        rnp->rn_value.i32 = (int32_t)value;
    }

    inline void
    setCompValue(rbcomp_node_t *rnp, pfc_ulong_t value)
    {
        rnp->rn_value.u32 = (uint32_t)value;
    }
#endif	/* !PFC_LP64 */

    inline void
    rbcompClear(void)
    {
        rbcomp_node_t	*rnp, *next;
        for (rnp = _rbcompList; rnp != NULL; rnp = next) {
            next = rnp->rn_next;
            free(rnp);
        }
        _rbcompList = NULL;
    }

    static void		dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
    static void		dtorFree(pfc_rbnode_t *node, pfc_ptr_t arg);

    RandomGenerator	_rand;
    pfc_list_t		_nodes;
    pfc_rbtree_t	_tree;
    uint32_t		_dtorCount;
    rbtest_t		**_rbtestTable;
    rbcomp_node_t	*_rbcompList;
};

#define	RANDOM_UINT32(value, max)                                       \
    RANDOM_INTEGER_MAX(_rand, value, static_cast<uint32_t>(max))

void
RedBlackTreeTest::dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
    RedBlackTreeTest	*rbtest = (RedBlackTreeTest *)arg;

    rbtest->addDtorCount(1);
}

void
RedBlackTreeTest::dtorFree(pfc_rbnode_t *node, pfc_ptr_t arg)
{
    RedBlackTreeTest	*rbtest = (RedBlackTreeTest *)arg;

    rbtest->addDtorCount(1);
    rbtest_node_free(node);
}

/*
 * void
 * RedBlackTreeTest::SetUp(void)
 *	Set up Red-Black Tree test.
 */
void
RedBlackTreeTest::SetUp(void)
{
    pfc_rbtree_init(&_tree, rbtest_compare, rbtest_getkey);
    pfc_list_init(&_nodes);
    _dtorCount = 0;
    _rbcompList = NULL;
}

/*
 * void
 * RedBlackTreeTest::TearDown(void)
 *	Clean up Red-Black Tree test.
 */
void
RedBlackTreeTest::TearDown(void)
{
    pfc_rbtree_clear(&_tree, NULL, NULL);
    free(_rbtestTable);
    rbtest_cleanup(&_nodes);
    rbcompClear();
}

/*
 * void
 * RedBlackTreeTest::sequentialAscTest(uint32_t number)
 *	Test for ascending sequential data.
 */
void
RedBlackTreeTest::sequentialAscTest(uint32_t number)
{
    pfc_rbnode_t	*node;
    const pfc_bool_t	do_check = (number < FULL_CHECK_THRESHOLD)
        ? PFC_TRUE : PFC_FALSE;

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));

    // Add tree nodes.
    for (uint32_t value = 1; value <= number; value++) {
        rbtest_t	*rp;

        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        ASSERT_EQ(0, pfc_rbtree_put(&_tree, node));
        if (do_check) {
            RBTREE_CHECK();
            CHECK_SEQUENTIAL_NODE_ORDER(1, value);

            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);
            ASSERT_EQ(node, pfc_rbtree_get(&_tree, key));

            key = (pfc_cptr_t)(uintptr_t)(value + 1);
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
        }
    }
    ASSERT_FALSE(pfc_rbtree_isempty(&_tree));

    pfc_cptr_t	zerokey((pfc_cptr_t)(uintptr_t)0);
    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, zerokey));

    pfc_rbnode_t	*ceil(pfc_rbtree_get_ceil(&_tree, zerokey));
    ASSERT_NE(RBNODE_NULL, ceil);
    ASSERT_EQ(1U, RBTEST_GETKEY(ceil));

    pfc_rbnode_t	*floor(pfc_rbtree_get_floor(&_tree, zerokey));
    ASSERT_EQ(RBNODE_NULL, floor);

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Ensure that all tree nodes can be found.
    for (uint32_t value = 1; value <= number; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_get(&_tree, key);
        ASSERT_NE(RBNODE_NULL, node);
        ASSERT_EQ(value, RBTEST_GETKEY(node));
        ASSERT_EQ(node, pfc_rbtree_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_get_floor(&_tree, key));

        // Update test.
        rbtest_t	*rp2;
        RBTEST_ALLOC(rp2, value);
        pfc_rbnode_t	*node2(RBTEST_PTR2NODE(rp2));
        ASSERT_EQ(EEXIST, pfc_rbtree_put(&_tree, node2));
        ASSERT_EQ(node, pfc_rbtree_update(&_tree, node2));
        rbtest_node_free(node);
    }
    for (uint32_t value = number + 1; value < number + RBTEST_LOOP; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
    }

    pfc_cptr_t	maxkey((pfc_cptr_t)(uintptr_t)(number + 1));
    ceil = pfc_rbtree_get_ceil(&_tree, maxkey);
    ASSERT_EQ(RBNODE_NULL, ceil);

    floor = pfc_rbtree_get_floor(&_tree, maxkey);
    ASSERT_NE(RBNODE_NULL, floor);
    ASSERT_EQ(number, RBTEST_GETKEY(floor));

    // Verify order of tree nodes.
    CHECK_SEQUENTIAL_NODE_ORDER(1, number);

    // Try to remove root node.
    pfc_rbnode_t	*rnode(_tree.rb_root);
    uint32_t		rvalue(RBTEST_GETKEY(rnode));
    pfc_rbtree_remove_node(&_tree, rnode);
    ASSERT_EQ(RBNODE_NULL,
              pfc_rbtree_get(&_tree, (pfc_cptr_t)(uintptr_t)rvalue));
    uint32_t	count(number - 1);
    RBTREE_CHECK();
    CHECK_NODE_ORDER(count);

    // Remove tree nodes.
    for (uint32_t value = 1; value <= number; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_remove(&_tree, key);
        if (value == rvalue) {
            ASSERT_EQ(RBNODE_NULL, node);
        }
        else {
            ASSERT_NE(RBNODE_NULL, node);
            ASSERT_EQ(value, RBTEST_GETKEY(node));
            count--;

            if (do_check) {
                RBTREE_CHECK();
                CHECK_NODE_ORDER(count);
                ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
            }
        }
    }

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));
    ASSERT_EQ(RBNODE_NULL, _tree.rb_root);
}

/*
 * void
 * RedBlackTreeTest::sequentialDescTest(uint32_t number)
 *	Test for descending sequential data.
 */
void
RedBlackTreeTest::sequentialDescTest(uint32_t number)
{
    pfc_rbnode_t	*node;
    const pfc_bool_t	do_check = (number < FULL_CHECK_THRESHOLD)
        ? PFC_TRUE : PFC_FALSE;

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));

    // Add tree nodes.
    for (uint32_t value = number; value > 0; value--) {
        rbtest_t	*rp;

        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        ASSERT_EQ(0, pfc_rbtree_put(&_tree, node));
        if (do_check) {
            RBTREE_CHECK();
            CHECK_SEQUENTIAL_NODE_ORDER(value, number);

            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);
            ASSERT_EQ(node, pfc_rbtree_get(&_tree, key));

            key = (pfc_cptr_t)(uintptr_t)(value - 1);
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
        }
    }

    ASSERT_FALSE(pfc_rbtree_isempty(&_tree));

    pfc_cptr_t	zerokey((pfc_cptr_t)(uintptr_t)0);
    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, zerokey));

    pfc_rbnode_t	*ceil(pfc_rbtree_get_ceil(&_tree, zerokey));
    ASSERT_NE(RBNODE_NULL, ceil);
    ASSERT_EQ(1U, RBTEST_GETKEY(ceil));

    pfc_rbnode_t	*floor(pfc_rbtree_get_floor(&_tree, zerokey));
    ASSERT_EQ(RBNODE_NULL, floor);

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Ensure that all tree nodes can be found.
    for (uint32_t value = number; value > 0; value--) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_get(&_tree, key);
        ASSERT_NE(RBNODE_NULL, node);
        ASSERT_EQ(value, RBTEST_GETKEY(node));
        ASSERT_EQ(node, pfc_rbtree_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_get_floor(&_tree, key));

        // Update test.
        rbtest_t	*rp2;
        RBTEST_ALLOC(rp2, value);
        pfc_rbnode_t	*node2(RBTEST_PTR2NODE(rp2));
        ASSERT_EQ(EEXIST, pfc_rbtree_put(&_tree, node2));
        ASSERT_EQ(node, pfc_rbtree_update(&_tree, node2));
        rbtest_node_free(node);
    }
    for (uint32_t value = number + 1; value < number + RBTEST_LOOP; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
    }

    pfc_cptr_t	maxkey((pfc_cptr_t)(uintptr_t)(number + 1));
    ceil = pfc_rbtree_get_ceil(&_tree, maxkey);
    ASSERT_EQ(RBNODE_NULL, ceil);

    floor = pfc_rbtree_get_floor(&_tree, maxkey);
    ASSERT_NE(RBNODE_NULL, floor);
    ASSERT_EQ(number, RBTEST_GETKEY(floor));

    // Verify order of tree nodes.
    CHECK_SEQUENTIAL_NODE_ORDER(1, number);

    // Try to remove root node.
    pfc_rbnode_t	*rnode(_tree.rb_root);
    uint32_t		rvalue(RBTEST_GETKEY(rnode));
    pfc_rbtree_remove_node(&_tree, rnode);
    ASSERT_EQ(RBNODE_NULL,
              pfc_rbtree_get(&_tree, (pfc_cptr_t)(uintptr_t)rvalue));
    uint32_t	count(number - 1);
    RBTREE_CHECK();
    CHECK_NODE_ORDER(count);

    // Remove tree nodes.
    for (uint32_t value = number; value > 0; value--) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_remove(&_tree, key);
        if (value == rvalue) {
            ASSERT_EQ(RBNODE_NULL, node);
        }
        else {
            ASSERT_NE(RBNODE_NULL, node);
            ASSERT_EQ(value, RBTEST_GETKEY(node));
            count--;

            if (do_check) {
                RBTREE_CHECK();
                CHECK_NODE_ORDER(count);
                ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
            }
        }
    }

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));
    ASSERT_EQ(RBNODE_NULL, _tree.rb_root);

    pfc_rbtree_clear(&_tree, dtor, (pfc_ptr_t)this);
    ASSERT_EQ(0U, getDtorCount());
}

/*
 * void
 * RedBlackTreeTest::randomTest(uint32_t number)
 *	Test for random data.
 */
void
RedBlackTreeTest::randomTest(uint32_t number)
{
    pfc_rbnode_t	*node;
    const pfc_bool_t	do_check = (number < FULL_CHECK_THRESHOLD)
        ? PFC_TRUE : PFC_FALSE;

    free(_rbtestTable);
    _rbtestTable = (rbtest_t **)malloc(sizeof(rbtest_t *) * number);
    ASSERT_NE((rbtest_t **)NULL, _rbtestTable);

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));

    uint32_t	count(0);
    uint32_t	max(0), min(UINT32_MAX);
    rbtest_t	**table = _rbtestTable;

    // Add tree nodes which has random value.
    do {
        rbtest_t	*rp;
        uint32_t	value;

        RANDOM_UINT32(value, 0);
        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        int	err(pfc_rbtree_put(&_tree, node));
        if (err != 0) {
            // Duplicated key.
            ASSERT_EQ(EEXIST, err);
            rbtest_free(rp);
            continue;
        }

        rp->rt_seq = count;
        count++;
        if (do_check) {
            RBTREE_CHECK();
            CHECK_NODE_ORDER(count);

            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);
            ASSERT_EQ(node, pfc_rbtree_get(&_tree, key));
            ASSERT_EQ(node, pfc_rbtree_get_ceil(&_tree, key));
            ASSERT_EQ(node, pfc_rbtree_get_floor(&_tree, key));
        }
        if (value > max) {
            max = value;
        }
        if (value < min) {
            min = value;
        }

        ASSERT_EQ((uint32_t)(table - _rbtestTable), rp->rt_seq);
        *table = rp;
        table++;
    } while (count < number);

    ASSERT_FALSE(pfc_rbtree_isempty(&_tree));

    for (uint32_t value = min - 1, loop = 0;
         loop < RBTEST_LOOP && value != UINT32_MAX; value--, loop++) {
        pfc_cptr_t	k((pfc_cptr_t)(uintptr_t)value);
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, k));

        pfc_rbnode_t	*ceil(pfc_rbtree_get_ceil(&_tree, k));
        ASSERT_NE(RBNODE_NULL, ceil);
        ASSERT_EQ(min, RBTEST_GETKEY(ceil));

        pfc_rbnode_t	*floor(pfc_rbtree_get_floor(&_tree, k));
        ASSERT_EQ(RBNODE_NULL, floor);
    }

    for (uint32_t value = max + 1, loop = 0;
         loop < RBTEST_LOOP && value != 0; value++, loop++) {
        pfc_cptr_t	k((pfc_cptr_t)(uintptr_t)value);
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, k));

        pfc_rbnode_t	*ceil(pfc_rbtree_get_ceil(&_tree, k));
        ASSERT_EQ(RBNODE_NULL, ceil);

        pfc_rbnode_t	*floor(pfc_rbtree_get_floor(&_tree, k));
        ASSERT_NE(RBNODE_NULL, floor);
        ASSERT_EQ(max, RBTEST_GETKEY(floor));
    }

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Ensure that all tree nodes can be found.
    for (uint32_t index = 0; index < number; index++) {
        rbtest_t	*rp(getFromTable(index));
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)rp->rt_value);

        node = pfc_rbtree_get(&_tree, key);
        ASSERT_EQ(RBTEST_PTR2NODE(rp), node);
        ASSERT_EQ(node, pfc_rbtree_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_get_floor(&_tree, key));

        // Update test.
        rbtest_t	*rp2;
        RBTEST_ALLOC(rp2, rp->rt_value);
        pfc_rbnode_t	*node2(RBTEST_PTR2NODE(rp2));
        ASSERT_EQ(EEXIST, pfc_rbtree_put(&_tree, node2));
        ASSERT_EQ(node, pfc_rbtree_update(&_tree, node2));

        rp2->rt_seq = rp->rt_seq;
        setToTable(rp->rt_seq, rp2);

        rbtest_free(rp);
    }

    // Verify order of tree nodes.
    CHECK_NODE_ORDER(number);

    // Try to remove root node.
    pfc_rbnode_t	*rnode(_tree.rb_root);
    uint32_t		rvalue(RBTEST_GETKEY(rnode));
    pfc_rbtree_remove_node(&_tree, rnode);
    ASSERT_EQ(RBNODE_NULL,
              pfc_rbtree_get(&_tree, (pfc_cptr_t)(uintptr_t)rvalue));
    count = number - 1;
    RBTREE_CHECK();
    CHECK_NODE_ORDER(count);

    if (number > RBTEST_LOOP) {
        // Choose random node and remove it.
        rbtest_t	*rp;
        uint32_t	idx;
        do {

            RANDOM_UINT32(idx, count);
            rp = getFromTable(idx);
        } while (rp == NULL || RBTEST_PTR2NODE(rp) == rnode);
        removeFromTable(idx);

        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)rp->rt_value);
        node = RBTEST_PTR2NODE(rp);
        ASSERT_EQ(node, pfc_rbtree_remove(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
        count--;
        rbtest_free(rp);
        if (do_check) {
            RBTREE_CHECK();
            CHECK_NODE_ORDER(count);
        }
    }

    // Remove all tree nodes.
    for (uint32_t idx = 0; idx < number; idx++) {
        rbtest_t	*rp(getFromTable(idx));
        if (rp == NULL) {
            continue;
        }

        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)rp->rt_value);
        node = RBTEST_PTR2NODE(rp);
        if (rp->rt_value == rvalue) {
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_remove(&_tree, key));
        }
        else {
            ASSERT_EQ(node, pfc_rbtree_remove(&_tree, key));
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
            count--;
            rbtest_free(rp);
            if (do_check) {
                RBTREE_CHECK();
                CHECK_NODE_ORDER(count);
            }
        }
    }

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));
    ASSERT_EQ(RBNODE_NULL, _tree.rb_root);

    pfc_rbtree_clear(&_tree, dtor, (pfc_ptr_t)this);
    ASSERT_EQ(0U, getDtorCount());
}

/*
 * void
 * RedBlackTreeTest::clearTest(uint32_t number)
 *	Test for pfc_rbtree_clear().
 */
void
RedBlackTreeTest::clearTest(uint32_t number)
{
    pfc_rbnode_t	*node;

    resetDtorCount();
    free(_rbtestTable);
    _rbtestTable = (rbtest_t **)malloc(sizeof(rbtest_t *) * number);
    ASSERT_NE((rbtest_t **)NULL, _rbtestTable);

    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));

    uint32_t	count(0);
    uint32_t	max(0), min(UINT32_MAX);
    rbtest_t	**table = _rbtestTable;

    // Add tree nodes which has random value.
    do {
        rbtest_t	*rp;
        uint32_t	value;

        RANDOM_UINT32(value, 0);
        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        int	err(pfc_rbtree_put(&_tree, node));
        if (err != 0) {
            // Duplicated key.
            ASSERT_EQ(EEXIST, err);
            rbtest_free(rp);
            continue;
        }

        rp->rt_seq = count;
        count++;
        if (value > max) {
            max = value;
        }
        if (value < min) {
            min = value;
        }

        ASSERT_EQ((uint32_t)(table - _rbtestTable), rp->rt_seq);
        *table = rp;
        table++;
    } while (count < number);

    ASSERT_FALSE(pfc_rbtree_isempty(&_tree));

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Clean up tree nodes.
    pfc_rbtree_clear(&_tree, dtorFree, (pfc_ptr_t)this);
    ASSERT_EQ(count, getDtorCount());
    ASSERT_TRUE(pfc_rbtree_isempty(&_tree));
    ASSERT_EQ(RBNODE_NULL, _tree.rb_root);
    ASSERT_TRUE(pfc_list_is_empty(&_nodes));
}

/*
 * template<class T>
 * void
 * RedBlackTreeTest::comparatorTest(pfc_rbtree_t *treep, uint32_t number,
 *				    bool key64)
 *	Test for default comparators.
 */
template<class T>
void
RedBlackTreeTest::comparatorTest(pfc_rbtree_t *treep, uint32_t number,
                                 bool key64)
{
    rbcomp_node_t	*rnp;
    pfc_rbnode_t	*node;

    rbcompClear();

    ASSERT_TRUE(pfc_rbtree_isempty(treep));

    // Allocate test nodes.
    for (uint32_t i(0); i < number; i++) {
        rnp = (rbcomp_node_t *)malloc(sizeof(*rnp));
        ASSERT_NE((rbcomp_node_t *)NULL, rnp);

        rnp->rn_next = _rbcompList;
        _rbcompList = rnp;

        for (;;) {
            // Determine node value.
            T value;
            RANDOM_INTEGER(_rand, value);
            setCompValue(rnp, value);

            int	err(pfc_rbtree_put(treep, &rnp->rn_node));
            if (err == 0) {
                if (key64) {
                    node = pfc_rbtree_get(treep, PFC_RBTREE_KEY64(value));
                }
                else {
                    node = pfc_rbtree_get(treep, (pfc_cptr_t)(uintptr_t)value);
                }
                ASSERT_EQ(&rnp->rn_node, node);

                break;
            }
            // Duplicated key.
            ASSERT_EQ(EEXIST, err);
        }
    }

    ASSERT_FALSE(pfc_rbtree_isempty(treep));

    // Ensure that all nodes are visible.
    for (rnp = _rbcompList; rnp != NULL; rnp = rnp->rn_next) {
        T key;
        getCompValue(rnp, key);
        if (key64) {
            node = pfc_rbtree_get(treep, PFC_RBTREE_KEY64(key));
        }
        else {
            node = pfc_rbtree_get(treep, (pfc_cptr_t)(uintptr_t)key);
        }
        ASSERT_EQ(&rnp->rn_node, node);
    }

    // Ensure that all nodes are sorted in ascending order.
    node = NULL;
    uint32_t	count(0);
    T prev(0);
    while ((node = pfc_rbtree_next(treep, node)) != NULL) {
        rbcomp_node_t	*rnp = RBCOMP_NODE2PTR(node);

        T key;
        getCompValue(rnp, key);
        if (count != 0) {
            ASSERT_LT(prev, key);
        }
        prev = key;
        count++;
    }
    ASSERT_EQ(number, count);

    pfc_rbtree_clear(treep, NULL, NULL);
    ASSERT_TRUE(pfc_rbtree_isempty(treep));
}

/*
 * void
 * RedBlackTreeTest::checkState(rbtest_state *rsp, pfc_rbnode_t *node,
 *				uint32_t depth)
 *	Ensure that the tree state is valid.
 */
void
RedBlackTreeTest::checkState(rbtest_state *rsp, pfc_rbnode_t *node,
                             uint32_t depth)
{
    if (node == NULL) {
        return;
    }

    rsp->rs_count++;

    pfc_rbnode_t	*left(node->rbn_children[PFC_RBNODE_IDX_LEFT]);
    pfc_rbnode_t	*right(node->rbn_children[PFC_RBNODE_IDX_RIGHT]);
    pfc_rbnode_t	*parent(node->rbn_parent);

    if (parent == NULL) {
        ASSERT_EQ(PFC_FALSE, node->rbn_red) << "Root node must be black.";
    }

    if (left != NULL) {
        // Check parent of left node.
        ASSERT_EQ(node, left->rbn_parent)
            << "Parent of left child is invalid.";

        ASSERT_GT(RBTEST_GETKEY(node), RBTEST_GETKEY(left))
            << "Left child must be less than parent.";
    }
    if (right != NULL) {
        // Check parent of right node.
        ASSERT_EQ(node, right->rbn_parent)
            << "Parent of right child is invalid";

        ASSERT_LT(RBTEST_GETKEY(node), RBTEST_GETKEY(right))
            << "Right child must be greater than parent.";
    }

    if (node->rbn_red) {
        ASSERT_EQ(PFC_FALSE, parent->rbn_red)
            << "Parent of red node must be black.";

        // All children of red node must be black.
        if (left != NULL) {
            ASSERT_EQ(PFC_FALSE, left->rbn_red)
                << "Left child of red node must be black.";
        }
        if (right != NULL) {
            ASSERT_EQ(PFC_FALSE, right->rbn_red)
                << "Right child of red node must be black.";
        }
    }

    if (left == NULL && right == NULL) {
        // This node is a leaf node.
        uint32_t	cnt(1);	// count NIL.
        pfc_rbnode_t	*np(node);

        if (depth < rsp->rs_min) {
            rsp->rs_min = depth;
        }
        if (depth > rsp->rs_max) {
            rsp->rs_max = depth;
        }

        // Count number of black nodes from root.
        do {
            if (np->rbn_red == PFC_FALSE) {
                cnt++;
            }
            np = np->rbn_parent;
        } while (np != NULL);

        if (rsp->rs_black == 0) {
            rsp->rs_black = cnt;
        }
        else {
            ASSERT_EQ(cnt, rsp->rs_black)
                << "Unexpected number of black nodes from root: " << cnt
                << ": expected=" << rsp->rs_black;
        }
    }
    else {
	depth++;
	checkState(rsp, left, depth);
	checkState(rsp, right, depth);
    }
}

/*
 * void
 * RedBlackTreeTest::checkOrder(uint32_t number)
 *	Ensure that all tree nodes are sorted correctly.
 */
void
RedBlackTreeTest::checkOrder(uint32_t number)
{
    uint32_t	prev(0), count(0);
    pfc_rbnode_t	*node(NULL), *next_ceil(NULL), *next_floor(NULL);

    while ((node = pfc_rbtree_next(&_tree, node)) != NULL) {
        uint32_t	v(RBTEST_GETKEY(node));

        if (count != 0) {
            ASSERT_LT(prev, v)
                << "Invalid tree order: value=" << v << ": prev=" << prev;
        }
        ASSERT_EQ(EEXIST, pfc_rbtree_put(&_tree, node));

        if (v != 0) {
            uint32_t	minus1(v - 1);
            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)minus1);

            pfc_rbnode_t	*ceil(pfc_rbtree_get_ceil(&_tree, key));
            if (ceil != node) {
                uint32_t	val(RBTEST_GETKEY(ceil));
                ASSERT_EQ(prev, val)
                    << "Invalid ceil node: value=" << val
                    << ": expected=" << prev;
            }

            pfc_rbnode_t	*floor(pfc_rbtree_get_floor(&_tree, key));
            if (count == 0) {
                ASSERT_EQ(RBNODE_NULL, floor)
                    << "Floor of the minimum node must be NULL: value="
                    << RBTEST_GETKEY(floor);
            }
            else {
                uint32_t	val(RBTEST_GETKEY(floor));
                ASSERT_EQ(prev, val)
                    << "Invalid floor node: value=" << val
                    << ": expected=" << prev;
            }
        }

        if (next_ceil != NULL) {
            ASSERT_EQ(next_ceil, node)
                << "Invalid ceil node: value=" << v
                << ": expected=" << RBTEST_GETKEY(next_ceil);
            next_ceil = NULL;
        }
        if (next_floor != NULL) {
            ASSERT_EQ(next_floor, node)
                << "Invalid floor node: value=" << v
                << ": expected=" << RBTEST_GETKEY(next_floor);
            next_floor = NULL;
        }

        if (v != UINT_MAX) {
            uint32_t	plus1(v + 1);
            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)plus1);

            pfc_rbnode_t	*ceil(pfc_rbtree_get_ceil(&_tree, key));
            if (count == number - 1) {
                ASSERT_EQ(RBNODE_NULL, ceil)
                    << "Ceil of the maximum node must be NULL: value="
                    << RBTEST_GETKEY(ceil);
            }
            else {
                next_ceil = ceil;
            }

            pfc_rbnode_t	*floor(pfc_rbtree_get_floor(&_tree, key));
            if (floor != node) {
                next_floor = floor;
            }
        }

        count++;
        prev = v;
    }

    ASSERT_EQ(number, count)
        << "Unexpected node count: " << count << ": expected=" << number;
}

/*
 * void
 * RedBlackTreeTest::checkSequentialOrder(uint32_t min, uint32_t max)
 *	Ensure that all tree nodes are sorted correctly.
 *	This methods must be used for sequential node test.
 */
void
RedBlackTreeTest::checkSequentialOrder(uint32_t min, uint32_t max)
{
    uint32_t	count(0);
    pfc_rbnode_t	*node(NULL);

    while ((node = pfc_rbtree_next(&_tree, node)) != NULL) {
        uint32_t	v(RBTEST_GETKEY(node));
        uint32_t	expected(min + count);
        ASSERT_EQ(expected, v)
            << "Invalid tree order: value=" << v << ": expected=" << expected;
        ASSERT_EQ(EEXIST, pfc_rbtree_put(&_tree, node));
        count++;
    }

    uint32_t	number(max - min + 1);
    ASSERT_EQ(number, count)
        << "Unexpected node count: " << count << ": expected=" << number;
}

/*
 * Below are test cases.
 */

/*
 * Empty tree test.
 */
TEST_F(RedBlackTreeTest, empty)
{
    ASSERT_EQ(RBNODE_NULL, _tree.rb_root);
    ASSERT_EQ((pfc_ptr_t)rbtest_compare, (pfc_ptr_t)_tree.rb_comp);
    ASSERT_EQ((pfc_ptr_t)rbtest_getkey, (pfc_ptr_t)_tree.rb_keyfunc);

    for (int i = 0; i < RBTEST_LOOP; i++) {
        uint32_t	value;

        // Any get method must return NULL.
        RANDOM_UINT32(value, 0);
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get_ceil(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get_floor(&_tree, key));
    }

    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_next(&_tree, NULL));

    pfc_rbtree_clear(&_tree, dtor, (pfc_ptr_t)this);
    ASSERT_EQ(0U, getDtorCount());
}

/*
 * Test in the case of only one node.
 */
TEST_F(RedBlackTreeTest, one)
{
    for (int i = 0; i < RBTEST_LOOP; i++) {
        rbtest_t	*rp, *rp2;
        pfc_rbnode_t	*node;
        uint32_t	value;
        pfc_cptr_t	key, key_gt, key_le;

        do {
            RANDOM_UINT32(value, 0);
        } while (value == 0 || value == UINT_MAX);
        key = (pfc_cptr_t)(uintptr_t)value;
        key_gt = (pfc_cptr_t)(uintptr_t)(value + 1);
        key_le = (pfc_cptr_t)(uintptr_t)(value - 1);

        RBTEST_ALLOC(rp, value);
        RBTEST_ALLOC(rp2, value);
        node = RBTEST_PTR2NODE(rp);

        // Put one node.
        ASSERT_EQ(0, pfc_rbtree_put(&_tree, node));
        ASSERT_EQ(EEXIST, pfc_rbtree_put(&_tree, node));
        ASSERT_EQ(node, pfc_rbtree_get(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_get_floor(&_tree, key));

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key_gt));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get(&_tree, key_le));

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get_ceil(&_tree, key_gt));
        ASSERT_EQ(node, pfc_rbtree_get_ceil(&_tree, key_le));

        ASSERT_EQ(node, pfc_rbtree_get_floor(&_tree, key_gt));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_get_floor(&_tree, key_le));

        ASSERT_EQ(node, pfc_rbtree_next(&_tree, NULL));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_next(&_tree, node));

        RBTREE_CHECK();

        // Remove node.
        ASSERT_EQ(node, pfc_rbtree_remove(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_remove(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_next(&_tree, NULL));

        // Update test.
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_update(&_tree, node));
        ASSERT_EQ(node, pfc_rbtree_get(&_tree, key));

        pfc_rbnode_t	*node2 = RBTEST_PTR2NODE(rp2);
        ASSERT_EQ(node, pfc_rbtree_update(&_tree, node2));
        ASSERT_EQ(node2, pfc_rbtree_get(&_tree, key));

        ASSERT_EQ(node2, pfc_rbtree_next(&_tree, NULL));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_next(&_tree, node2));

        RBTREE_CHECK();

        if (i & 1) {
            pfc_rbtree_clear(&_tree, dtor, (pfc_ptr_t)this);
            ASSERT_EQ(1U, getDtorCount());
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_remove(&_tree, key));
            resetDtorCount();
        }
        else {
            ASSERT_EQ(node2, pfc_rbtree_remove(&_tree, key));
        }
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_remove(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_next(&_tree, NULL));

        rbtest_free(rp);
        rbtest_free(rp2);
    }
}

/*
 * Sequential tree node test. (ascending order)
 */
#define	RBTEST_NAME_SEQ_ASC(suffix)	sequential_asc_##suffix
#define	RBTEST_DECL_SEQ_ASC(num)                        \
    TEST_F(RedBlackTreeTest, RBTEST_NAME_SEQ_ASC(num))  \
    {                                                   \
        sequentialAscTest(num);                         \
    }

RBTEST_DECL_SEQ_ASC(2);
RBTEST_DECL_SEQ_ASC(4);
RBTEST_DECL_SEQ_ASC(8);
RBTEST_DECL_SEQ_ASC(16);
RBTEST_DECL_SEQ_ASC(256);
RBTEST_DECL_SEQ_ASC(512);
RBTEST_DECL_SEQ_ASC(1024);
RBTEST_DECL_SEQ_ASC(4096);
RBTEST_DECL_SEQ_ASC(16384);
RBTEST_DECL_SEQ_ASC(131072);

/*
 * Test cases with random number of tree nodes in sequential ascending order.
 */
TEST_F(RedBlackTreeTest, RBTEST_NAME_SEQ_ASC(random))
{
    uint32_t	number;

    do {
        RANDOM_UINT32(number, MAX_TEST_NODES);
    } while (number < MIN_TEST_NODES);

    sequentialAscTest(number);
}

/*
 * Sequential tree node test. (descending order)
 */
#define	RBTEST_NAME_SEQ_DESC(suffix)	sequential_desc_##suffix
#define	RBTEST_DECL_SEQ_DESC(num)                       \
    TEST_F(RedBlackTreeTest, RBTEST_NAME_SEQ_DESC(num)) \
    {                                                   \
        sequentialDescTest(num);                        \
    }

RBTEST_DECL_SEQ_DESC(3);
RBTEST_DECL_SEQ_DESC(5);
RBTEST_DECL_SEQ_DESC(7);
RBTEST_DECL_SEQ_DESC(9);
RBTEST_DECL_SEQ_DESC(10);
RBTEST_DECL_SEQ_DESC(100);
RBTEST_DECL_SEQ_DESC(1000);
RBTEST_DECL_SEQ_DESC(3000);
RBTEST_DECL_SEQ_DESC(10000);
RBTEST_DECL_SEQ_DESC(100000);

/*
 * Test cases with random number of tree nodes in sequential descending order.
 */
TEST_F(RedBlackTreeTest, RBTEST_NAME_SEQ_DESC(random))
{
    uint32_t	number;

    do {
        RANDOM_UINT32(number, MAX_TEST_NODES);
    } while (number < MIN_TEST_NODES);

    sequentialDescTest(number);
}

/*
 * Random tree node test.
 */
#define	RBTEST_NAME_RANDOM(suffix)	random_##suffix
#define	RBTEST_DECL_RANDOM(num)                         \
    TEST_F(RedBlackTreeTest, RBTEST_NAME_RANDOM(num))   \
    {                                                   \
        randomTest(num);                                \
    }

RBTEST_DECL_RANDOM(3);
RBTEST_DECL_RANDOM(5);
RBTEST_DECL_RANDOM(7);
RBTEST_DECL_RANDOM(13);
RBTEST_DECL_RANDOM(41);
RBTEST_DECL_RANDOM(103);
RBTEST_DECL_RANDOM(1013);
RBTEST_DECL_RANDOM(3209);
RBTEST_DECL_RANDOM(10037);
RBTEST_DECL_RANDOM(100069);

/*
 * Test cases with random number of tree nodes which have random value.
 */
TEST_F(RedBlackTreeTest, RBTEST_NAME_RANDOM(random))
{
    uint32_t	number;

    do {
        RANDOM_UINT32(number, MAX_TEST_NODES);
    } while (number < MIN_TEST_NODES);

    randomTest(number);
}

/*
 * Test cases for pfc_rbtree_clear().
 */
TEST_F(RedBlackTreeTest, clear)
{
    for (int loop = 0; loop < LOOP_CLEAR; loop++) {
        uint32_t	number;

        do {
            RANDOM_UINT32(number, CLEAR_MAX_NODES);
        } while (number < CLEAR_MIN_NODES);

        clearTest(number);
        if (PFC_EXPECT_FALSE(HasFatalFailure())) {
            break;
        }
    }
}

/*
 * Test cases for default comparators.
 */
#define	RBTEST_DECL_COMP(name, type, key64)                             \
    TEST_F(RedBlackTreeTest, comp_##name)                               \
    {                                                                   \
        for (int loop = 0; loop < LOOP_COMP; loop++) {                  \
            uint32_t	number;                                         \
                                                                        \
            do {                                                        \
                RANDOM_UINT32(number, COMP_MAX_NODES);                  \
            } while (number < COMP_MIN_NODES);                          \
                                                                        \
            pfc_rbtree_t	tree;                                   \
            pfc_rbtree_init(&tree, pfc_rbtree_##name##_compare,         \
                            rbcomp_getkey_##name);                      \
                                                                        \
            comparatorTest<type>(&tree, number, key64);                 \
            if (PFC_EXPECT_FALSE(HasFatalFailure())) {                  \
                break;                                                  \
            }                                                           \
        }                                                               \
    }

#define	RBTEST_DECL_COMP_STDINT(name)                                   \
    RBTEST_DECL_COMP(name, name##_t, (sizeof(name##_t) == sizeof(int64_t)))

RBTEST_DECL_COMP_STDINT(int32);
RBTEST_DECL_COMP_STDINT(uint32);
RBTEST_DECL_COMP_STDINT(int64);
RBTEST_DECL_COMP_STDINT(uint64);

#ifdef	PFC_LP64
#define	rbcomp_getkey_long		rbcomp_getkey_int64
#define	rbcomp_getkey_ulong		rbcomp_getkey_uint64
#else	/* !PFC_LP64 */
#define	rbcomp_getkey_long		rbcomp_getkey_int32
#define	rbcomp_getkey_ulong		rbcomp_getkey_uint32
#endif	/* PFC_LP64 */

#define	RBTEST_DECL_COMP_LONG(name, type)	\
    RBTEST_DECL_COMP(name, type, false)

RBTEST_DECL_COMP_LONG(long, pfc_long_t);
RBTEST_DECL_COMP_LONG(ulong, pfc_ulong_t);
