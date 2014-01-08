/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_rbtree_ex.cc: Extended Red-Black Tree test
 */

#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <pfc/rbtree_ex.h>
#include <pfc/clock.h>
#include "rbtree_test.h"
#include "random.hh"

#define	FULL_CHECK_THRESHOLD	2000
#define	MIN_TEST_NODES		2
#define	MAX_TEST_NODES		100000

#define	LOOP_CLEAR		32
#define	CLEAR_MIN_NODES		2
#define	CLEAR_MAX_NODES		10000

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

#define	RBTEST_ALLOC(rp, value)                  \
    do {                                         \
        (rp) = rbtest_alloc(&_nodes, value);     \
        ASSERT_NE((rbtest_t *)NULL, (rp));       \
    } while (0)

#define	RBTREE_CHECK()                                                  \
    do {                                                                \
        rbtest_state	__state;                                        \
                                                                        \
        checkState(&__state, _tree.rbx_tree.rb_root);                   \
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

class RedBlackTreeExTest
    : public testing::Test
{
protected:
    RedBlackTreeExTest()
        : _rbtestTable(NULL), _wrlocked(false)
    {}
    ~RedBlackTreeExTest()
    {
        if (_wrlocked) {
            pfc_rbtree_ex_unlock(&_tree);
        }
    }

    virtual void	SetUp(void);
    virtual void	TearDown(void);

    void	sequentialAscTest(uint32_t number);
    void	sequentialDescTest(uint32_t number);
    void	randomTest(uint32_t number);
    void	clearTest(uint32_t number);
    void	checkState(rbtest_state *rsp, pfc_rbnode_t *node,
                           uint32_t depth = 1);
    void	checkOrder(uint32_t number);
    void	checkSequentialOrder(uint32_t min, uint32_t max);

    inline void
    writeLock(void)
    {
        pfc_rbtree_ex_wrlock(&_tree);
        _wrlocked = true;
    }

    inline void
    unlock(void)
    {
        _wrlocked = false;
        pfc_rbtree_ex_unlock(&_tree);
    }

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

    static void		dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
    static void		dtorFree(pfc_rbnode_t *node, pfc_ptr_t arg);

    RandomGenerator	_rand;
    pfc_list_t		_nodes;
    pfc_rbtree_ex_t	_tree;
    uint32_t		_dtorCount;
    rbtest_t		**_rbtestTable;
    bool		_wrlocked;
};

#define	RANDOM_UINT32(value, max)                                       \
    RANDOM_INTEGER_MAX(_rand, value, static_cast<uint32_t>(max))

void
RedBlackTreeExTest::dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
    RedBlackTreeExTest	*rbtest = (RedBlackTreeExTest *)arg;

    rbtest->addDtorCount(1);
}

void
RedBlackTreeExTest::dtorFree(pfc_rbnode_t *node, pfc_ptr_t arg)
{
    RedBlackTreeExTest	*rbtest = (RedBlackTreeExTest *)arg;

    rbtest->addDtorCount(1);
    rbtest_node_free(node);
}

/*
 * void
 * RedBlackTreeExTest::SetUp(void)
 *	Set up Red-Black Tree test.
 */
void
RedBlackTreeExTest::SetUp(void)
{
    pfc_rbtree_ex_init(&_tree, rbtest_compare, rbtest_getkey);
    pfc_list_init(&_nodes);
    _dtorCount = 0;
}

/*
 * void
 * RedBlackTreeExTest::TearDown(void)
 *	Clean up Red-Black Tree test.
 */
void
RedBlackTreeExTest::TearDown(void)
{
    pfc_rbtree_ex_clear(&_tree, NULL, NULL);
    free(_rbtestTable);
    rbtest_cleanup(&_nodes);
}

/*
 * void
 * RedBlackTreeExTest::sequentialAscTest(uint32_t number)
 *	Test for ascending sequential data.
 */
void
RedBlackTreeExTest::sequentialAscTest(uint32_t number)
{
    pfc_rbnode_t	*node;
    const pfc_bool_t	do_check = (number < FULL_CHECK_THRESHOLD)
        ? PFC_TRUE : PFC_FALSE;

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    // Add tree nodes.
    for (uint32_t value = 1; value <= number; value++) {
        rbtest_t	*rp;

        ASSERT_EQ(value - 1, pfc_rbtree_ex_get_size(&_tree));
        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        ASSERT_EQ(0, pfc_rbtree_ex_put(&_tree, node));
        if (do_check) {
            RBTREE_CHECK();
            CHECK_SEQUENTIAL_NODE_ORDER(1, value);

            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);
            ASSERT_EQ(node, pfc_rbtree_ex_get(&_tree, key));

            key = (pfc_cptr_t)(uintptr_t)(value + 1);
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
        }
    }

    ASSERT_FALSE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_FALSE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));

    pfc_cptr_t	zerokey((pfc_cptr_t)(uintptr_t)0);
    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, zerokey));

    pfc_rbnode_t	*ceil(pfc_rbtree_ex_get_ceil(&_tree, zerokey));
    ASSERT_NE(RBNODE_NULL, ceil);
    ASSERT_EQ(1U, RBTEST_GETKEY(ceil));

    pfc_rbnode_t	*floor(pfc_rbtree_ex_get_floor(&_tree, zerokey));
    ASSERT_EQ(RBNODE_NULL, floor);

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Ensure that all tree nodes can be found.
    for (uint32_t value = 1; value <= number; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_ex_get(&_tree, key);
        ASSERT_NE(RBNODE_NULL, node);
        ASSERT_EQ(value, RBTEST_GETKEY(node));
        ASSERT_EQ(node, pfc_rbtree_ex_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_ex_get_floor(&_tree, key));

        // Update test.
        rbtest_t	*rp2;
        RBTEST_ALLOC(rp2, value);
        pfc_rbnode_t	*node2(RBTEST_PTR2NODE(rp2));
        ASSERT_EQ(EEXIST, pfc_rbtree_ex_put(&_tree, node2));
        ASSERT_EQ(node, pfc_rbtree_ex_update(&_tree, node2));
        rbtest_node_free(node);
    }
    for (uint32_t value = number + 1; value < number + RBTEST_LOOP; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
    }

    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));

    pfc_cptr_t	maxkey((pfc_cptr_t)(uintptr_t)(number + 1));
    ceil = pfc_rbtree_ex_get_ceil(&_tree, maxkey);
    ASSERT_EQ(RBNODE_NULL, ceil);

    floor = pfc_rbtree_ex_get_floor(&_tree, maxkey);
    ASSERT_NE(RBNODE_NULL, floor);
    ASSERT_EQ(number, RBTEST_GETKEY(floor));

    // Verify order of tree nodes.
    CHECK_SEQUENTIAL_NODE_ORDER(1, number);

    // Try to remove root node.
    pfc_rbnode_t	*rnode(_tree.rbx_tree.rb_root);
    uint32_t		rvalue(RBTEST_GETKEY(rnode));
    pfc_rbtree_ex_remove_node(&_tree, rnode);
    ASSERT_EQ(RBNODE_NULL,
              pfc_rbtree_ex_get(&_tree, (pfc_cptr_t)(uintptr_t)rvalue));
    uint32_t	count(number - 1);
    RBTREE_CHECK();
    CHECK_NODE_ORDER(count);

    // Remove tree nodes.
    for (uint32_t value = 1; value <= number; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_ex_remove(&_tree, key);
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
                ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
            }
        }
    }

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(RBNODE_NULL, _tree.rbx_tree.rb_root);
}

/*
 * void
 * RedBlackTreeExTest::sequentialDescTest(uint32_t number)
 *	Test for descending sequential data.
 */
void
RedBlackTreeExTest::sequentialDescTest(uint32_t number)
{
    pfc_rbnode_t	*node;
    const pfc_bool_t	do_check = (number < FULL_CHECK_THRESHOLD)
        ? PFC_TRUE : PFC_FALSE;

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    // Add tree nodes.
    for (uint32_t value = number; value > 0; value--) {
        rbtest_t	*rp;

        ASSERT_EQ(number - value, pfc_rbtree_ex_get_size(&_tree));
        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        ASSERT_EQ(0, pfc_rbtree_ex_put(&_tree, node));
        if (do_check) {
            RBTREE_CHECK();
            CHECK_SEQUENTIAL_NODE_ORDER(value, number);

            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);
            ASSERT_EQ(node, pfc_rbtree_ex_get(&_tree, key));

            key = (pfc_cptr_t)(uintptr_t)(value - 1);
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
        }
    }

    ASSERT_FALSE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_FALSE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));

    pfc_cptr_t	zerokey((pfc_cptr_t)(uintptr_t)0);
    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, zerokey));

    pfc_rbnode_t	*ceil(pfc_rbtree_ex_get_ceil(&_tree, zerokey));
    ASSERT_NE(RBNODE_NULL, ceil);
    ASSERT_EQ(1U, RBTEST_GETKEY(ceil));

    pfc_rbnode_t	*floor(pfc_rbtree_ex_get_floor(&_tree, zerokey));
    ASSERT_EQ(RBNODE_NULL, floor);

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Ensure that all tree nodes can be found.
    for (uint32_t value = number; value > 0; value--) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_ex_get(&_tree, key);
        ASSERT_NE(RBNODE_NULL, node);
        ASSERT_EQ(value, RBTEST_GETKEY(node));
        ASSERT_EQ(node, pfc_rbtree_ex_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_ex_get_floor(&_tree, key));

        // Update test.
        rbtest_t	*rp2;
        RBTEST_ALLOC(rp2, value);
        pfc_rbnode_t	*node2(RBTEST_PTR2NODE(rp2));
        ASSERT_EQ(EEXIST, pfc_rbtree_ex_put(&_tree, node2));
        ASSERT_EQ(node, pfc_rbtree_ex_update(&_tree, node2));
        rbtest_node_free(node);
    }
    for (uint32_t value = number + 1; value < number + RBTEST_LOOP; value++) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
    }

    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, zerokey));

    pfc_cptr_t	maxkey((pfc_cptr_t)(uintptr_t)(number + 1));
    ceil = pfc_rbtree_ex_get_ceil(&_tree, maxkey);
    ASSERT_EQ(RBNODE_NULL, ceil);

    floor = pfc_rbtree_ex_get_floor(&_tree, maxkey);
    ASSERT_NE(RBNODE_NULL, floor);
    ASSERT_EQ(number, RBTEST_GETKEY(floor));

    // Verify order of tree nodes.
    CHECK_SEQUENTIAL_NODE_ORDER(1, number);

    // Try to remove root node.
    pfc_rbnode_t	*rnode(_tree.rbx_tree.rb_root);
    uint32_t		rvalue(RBTEST_GETKEY(rnode));
    pfc_rbtree_ex_remove_node(&_tree, rnode);
    ASSERT_EQ(RBNODE_NULL,
              pfc_rbtree_ex_get(&_tree, (pfc_cptr_t)(uintptr_t)rvalue));

    uint32_t	count(number - 1);
    ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));
    RBTREE_CHECK();
    CHECK_NODE_ORDER(count);

    // Remove tree nodes.
    for (uint32_t value = number; value > 0; value--) {
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        node = pfc_rbtree_ex_remove(&_tree, key);
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
                ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
            }
        }
        ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));
    }

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(RBNODE_NULL, _tree.rbx_tree.rb_root);
    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));

    pfc_rbtree_ex_clear(&_tree, dtor, (pfc_ptr_t)this);
    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
    ASSERT_EQ(0U, getDtorCount());
}

/*
 * void
 * RedBlackTreeExTest::randomTest(uint32_t number)
 *	Test for random data.
 */
void
RedBlackTreeExTest::randomTest(uint32_t number)
{
    pfc_rbnode_t	*node;
    const pfc_bool_t	do_check = (number < FULL_CHECK_THRESHOLD)
        ? PFC_TRUE : PFC_FALSE;

    free(_rbtestTable);
    _rbtestTable = (rbtest_t **)malloc(sizeof(rbtest_t *) * number);
    ASSERT_NE((rbtest_t **)NULL, _rbtestTable);

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    uint32_t	count(0);
    uint32_t	max(0), min(UINT32_MAX);
    rbtest_t	**table = _rbtestTable;

    // Add tree nodes which has random value.
    do {
        rbtest_t	*rp;
        uint32_t	value;

        ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));
        RANDOM_UINT32(value, 0);
        RBTEST_ALLOC(rp, value);

        node = RBTEST_PTR2NODE(rp);
        int	err(pfc_rbtree_ex_put(&_tree, node));
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
            ASSERT_EQ(node, pfc_rbtree_ex_get(&_tree, key));
            ASSERT_EQ(node, pfc_rbtree_ex_get_ceil(&_tree, key));
            ASSERT_EQ(node, pfc_rbtree_ex_get_floor(&_tree, key));
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

    ASSERT_FALSE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_FALSE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));

    for (uint32_t value = min - 1, loop = 0;
         loop < RBTEST_LOOP && value != UINT32_MAX; value--, loop++) {
        pfc_cptr_t	k((pfc_cptr_t)(uintptr_t)value);
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, k));

        pfc_rbnode_t	*ceil(pfc_rbtree_ex_get_ceil(&_tree, k));
        ASSERT_NE(RBNODE_NULL, ceil);
        ASSERT_EQ(min, RBTEST_GETKEY(ceil));

        pfc_rbnode_t	*floor(pfc_rbtree_ex_get_floor(&_tree, k));
        ASSERT_EQ(RBNODE_NULL, floor);
    }

    for (uint32_t value = max + 1, loop = 0;
         loop < RBTEST_LOOP && value != 0; value++, loop++) {
        pfc_cptr_t	k((pfc_cptr_t)(uintptr_t)value);
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, k));

        pfc_rbnode_t	*ceil(pfc_rbtree_ex_get_ceil(&_tree, k));
        ASSERT_EQ(RBNODE_NULL, ceil);

        pfc_rbnode_t	*floor(pfc_rbtree_ex_get_floor(&_tree, k));
        ASSERT_NE(RBNODE_NULL, floor);
        ASSERT_EQ(max, RBTEST_GETKEY(floor));
    }

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Ensure that all tree nodes can be found.
    for (uint32_t index = 0; index < number; index++) {
        rbtest_t	*rp(getFromTable(index));
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)rp->rt_value);

        node = pfc_rbtree_ex_get(&_tree, key);
        ASSERT_EQ(RBTEST_PTR2NODE(rp), node);
        ASSERT_EQ(node, pfc_rbtree_ex_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_ex_get_floor(&_tree, key));

        // Update test.
        rbtest_t	*rp2;
        RBTEST_ALLOC(rp2, rp->rt_value);
        pfc_rbnode_t	*node2(RBTEST_PTR2NODE(rp2));
        ASSERT_EQ(EEXIST, pfc_rbtree_ex_put(&_tree, node2));
        ASSERT_EQ(node, pfc_rbtree_ex_update(&_tree, node2));

        rp2->rt_seq = rp->rt_seq;
        setToTable(rp->rt_seq, rp2);

        rbtest_free(rp);
    }

    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));

    // Verify order of tree nodes.
    CHECK_NODE_ORDER(number);

    // Try to remove root node.
    pfc_rbnode_t	*rnode(_tree.rbx_tree.rb_root);
    uint32_t		rvalue(RBTEST_GETKEY(rnode));
    pfc_rbtree_ex_remove_node(&_tree, rnode);
    ASSERT_EQ(RBNODE_NULL,
              pfc_rbtree_ex_get(&_tree, (pfc_cptr_t)(uintptr_t)rvalue));
    count = number - 1;
    RBTREE_CHECK();
    CHECK_NODE_ORDER(count);
    ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));

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
        ASSERT_EQ(node, pfc_rbtree_ex_remove(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
        count--;
        rbtest_free(rp);
        ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));
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

        ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));

        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)rp->rt_value);
        node = RBTEST_PTR2NODE(rp);
        if (rp->rt_value == rvalue) {
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_remove(&_tree, key));
        }
        else {
            ASSERT_EQ(node, pfc_rbtree_ex_remove(&_tree, key));
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
            count--;
            rbtest_free(rp);
            if (do_check) {
                RBTREE_CHECK();
                CHECK_NODE_ORDER(count);
            }
        }
    }

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(RBNODE_NULL, _tree.rbx_tree.rb_root);
    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));

    pfc_rbtree_ex_clear(&_tree, dtor, (pfc_ptr_t)this);
    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
    ASSERT_EQ(0U, getDtorCount());
}

/*
 * void
 * RedBlackTreeExTest::clearTest(uint32_t number)
 *	Test for pfc_rbtree_ex_clear().
 */
void
RedBlackTreeExTest::clearTest(uint32_t number)
{
    pfc_rbnode_t	*node;

    resetDtorCount();
    free(_rbtestTable);
    _rbtestTable = (rbtest_t **)malloc(sizeof(rbtest_t *) * number);
    ASSERT_NE((rbtest_t **)NULL, _rbtestTable);

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

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
        int	err(pfc_rbtree_ex_put(&_tree, node));
        if (err != 0) {
            // Duplicated key.
            ASSERT_EQ(EEXIST, err);
            rbtest_free(rp);
            continue;
        }

        rp->rt_seq = count;
        count++;
        ASSERT_EQ(count, pfc_rbtree_ex_get_size(&_tree));
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

    ASSERT_FALSE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_FALSE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    // Ensure that whole tree state is valid.
    RBTREE_CHECK();

    // Clean up tree nodes.
    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));
    pfc_rbtree_ex_clear(&_tree, dtorFree, (pfc_ptr_t)this);

    ASSERT_TRUE(pfc_rbtree_ex_isempty(&_tree));
    pfc_rbtree_ex_rdlock(&_tree);
    ASSERT_TRUE(pfc_rbtree_ex_isempty_l(&_tree));
    pfc_rbtree_ex_unlock(&_tree);

    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
    ASSERT_EQ(count, getDtorCount());
    ASSERT_EQ(RBNODE_NULL, _tree.rbx_tree.rb_root);
    ASSERT_TRUE(pfc_list_is_empty(&_nodes));
}

/*
 * void
 * RedBlackTreeExTest::checkState(rbtest_state *rsp, pfc_rbnode_t *node,
 *				uint32_t depth)
 *	Ensure that the tree state is valid.
 */
void
RedBlackTreeExTest::checkState(rbtest_state *rsp, pfc_rbnode_t *node,
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
 * RedBlackTreeExTest::checkOrder(uint32_t number)
 *	Ensure that all tree nodes are sorted correctly.
 */
void
RedBlackTreeExTest::checkOrder(uint32_t number)
{
    uint32_t	prev(0), count(0);
    pfc_rbnode_t	*node(NULL), *next_ceil(NULL), *next_floor(NULL);

    writeLock();
    while ((node = pfc_rbtree_ex_next_l(&_tree, node)) != NULL) {
        uint32_t	v(RBTEST_GETKEY(node));

        if (count != 0) {
            ASSERT_LT(prev, v)
                << "Invalid tree order: value=" << v << ": prev=" << prev;
        }
        ASSERT_EQ(EEXIST, pfc_rbtree_ex_put_l(&_tree, node));

        if (v != 0) {
            uint32_t	minus1(v - 1);
            pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)minus1);

            pfc_rbnode_t	*ceil(pfc_rbtree_ex_get_ceil_l(&_tree, key));
            if (ceil != node) {
                uint32_t	val(RBTEST_GETKEY(ceil));
                ASSERT_EQ(prev, val)
                    << "Invalid ceil node: value=" << val
                    << ": expected=" << prev;
            }

            pfc_rbnode_t	*floor(pfc_rbtree_ex_get_floor_l(&_tree, key));
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

            pfc_rbnode_t	*ceil(pfc_rbtree_ex_get_ceil_l(&_tree, key));
            if (count == number - 1) {
                ASSERT_EQ(RBNODE_NULL, ceil)
                    << "Ceil of the maximum node must be NULL: value="
                    << RBTEST_GETKEY(ceil);
            }
            else {
                next_ceil = ceil;
            }

            pfc_rbnode_t	*floor(pfc_rbtree_ex_get_floor_l(&_tree, key));
            if (floor != node) {
                next_floor = floor;
            }
        }

        count++;
        prev = v;
    }
    unlock();

    ASSERT_EQ(number, count)
        << "Unexpected node count: " << count << ": expected=" << number;
    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));
}

/*
 * void
 * RedBlackTreeExTest::checkSequentialOrder(uint32_t min, uint32_t max)
 *	Ensure that all tree nodes are sorted correctly.
 *	This methods must be used for sequential node test.
 */
void
RedBlackTreeExTest::checkSequentialOrder(uint32_t min, uint32_t max)
{
    uint32_t	count(0);
    pfc_rbnode_t	*node(NULL);

    writeLock();
    while ((node = pfc_rbtree_ex_next_l(&_tree, node)) != NULL) {
        uint32_t	v(RBTEST_GETKEY(node));
        uint32_t	expected(min + count);
        ASSERT_EQ(expected, v)
            << "Invalid tree order: value=" << v << ": expected=" << expected;
        ASSERT_EQ(EEXIST, pfc_rbtree_ex_put_l(&_tree, node));
        count++;
    }
    unlock();

    uint32_t	number(max - min + 1);
    ASSERT_EQ(number, count)
        << "Unexpected node count: " << count << ": expected=" << number;
    ASSERT_EQ(number, pfc_rbtree_ex_get_size(&_tree));
}

/*
 * Thread which is used to verify that wrapper function acquires appropriate
 * lock.
 */
class TreeThread
{
public:
    TreeThread();
    virtual ~TreeThread();

    void	readLock(void);
    void	writeLock(void);
    int		tryWriteLock(void);
    void	unlock(void);
    int		start(void);
    int		join(void);
    void	delay(void);

    const char *
    getError(void) const
    {
        return _error.c_str();
    }

    bool
    hasError(void) const
    {
        return (_error.length() != 0);
    }  

    bool
    isDone(void) const
    {
        return _done;
    }

protected:
    pfc_rbtree_ex_t *
    getTree(void)
    {
        return &_tree;
    }

    rbtest_t		*allocNode(void);
    void		setError(const char *msg, int err=0);
    virtual int		doTest(void) = 0;

private:
    static void		*thread_main(void *arg);

    pthread_t		_thread;
    bool		_done;
    bool		_locked;
    rbtest_t		*_node;
    pfc_rbtree_ex_t	_tree;
    std::string		_error;
};

TreeThread::TreeThread()
    : _thread(PFC_PTHREAD_INVALID_ID), _done(false), _locked(false),
      _node(NULL)
{
    pfc_rbtree_ex_init(&_tree, rbtest_compare, rbtest_getkey);
}

TreeThread::~TreeThread()
{
    if (_locked) {
        unlock();
    }
    if (_thread != PFC_PTHREAD_INVALID_ID) {
        (void)pthread_join(_thread, NULL);
    }

    free(_node);
}

void
TreeThread::readLock(void)
{
    pfc_rbtree_ex_rdlock(&_tree);
    _locked = true;
}

void
TreeThread::writeLock(void)
{
    pfc_rbtree_ex_wrlock(&_tree);
    _locked = true;
}

int
TreeThread::tryWriteLock(void)
{
    int	err(pfc_rwlock_trywrlock(pfc_rbtree_ex_get_lock(&_tree)));
    if (err == 0) {
        _locked = true;
    }

    return err;
}

void
TreeThread::unlock(void)
{
    _locked = false;
    pfc_rbtree_ex_unlock(&_tree);
}

int
TreeThread::start(void)
{
    return pthread_create(&_thread, NULL, &TreeThread::thread_main, this);
}

int
TreeThread::join(void)
{
    int	err(pthread_join(_thread, NULL));
    if (err == 0) {
        _thread = PFC_PTHREAD_INVALID_ID;
    }

    return err;
}

#define	RBTEST_EX_LOCK_DELAY	5000000

void
TreeThread::delay(void)
{
    struct timespec	ts;

    ts.tv_sec = 0;
    ts.tv_nsec = RBTEST_EX_LOCK_DELAY;
    nanosleep(&ts, NULL);
}

rbtest_t *
TreeThread::allocNode(void)
{
    rbtest_t	*rp((rbtest_t *)malloc(sizeof(rbtest_t)));
    if (rp == NULL) {
        setError("Failed to allocate test node.");
    }

    memset(rp, 0, sizeof(*rp));
    _node = rp;

    return rp;
}

void
TreeThread::setError(const char *msg, int err)
{
    std::ostringstream	stream;

    stream << "ERROR: " << msg;
    if (err != 0) {
        stream << "errno=" << err << " (" << strerror(err) << ")";
    }

    _error = stream.str();
}

void *
TreeThread::thread_main(void *arg)
{
    TreeThread	*me(reinterpret_cast<TreeThread *>(arg));
    
    int	err(me->doTest());
    me->_done = true;
    if (err != 0) {
        me->setError("Test failed.", err);
    }

    return NULL;
}

class TreePutThread
    : public TreeThread
{
public:
    TreePutThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreePutThread::doTest(void)
{
    rbtest_t	*rp(allocNode());

    return pfc_rbtree_ex_put(getTree(), RBTEST_PTR2NODE(rp));
}

class TreeUpdateThread
    : public TreeThread
{
public:
    TreeUpdateThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeUpdateThread::doTest(void)
{
    rbtest_t	*rp(allocNode());

    pfc_rbtree_ex_t	*tree(getTree());
    pfc_rbnode_t  *n(pfc_rbtree_ex_update(tree, RBTEST_PTR2NODE(rp)));
    if (n != NULL) {
        return EINVAL;
    }

    return (pfc_rbtree_ex_get_size(tree) == 1) ? 0 : EINVAL;
}

class TreeGetThread
    : public TreeThread
{
public:
    TreeGetThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeGetThread::doTest(void)
{
    pfc_cptr_t    key((pfc_cptr_t)(uintptr_t)0);
    pfc_rbnode_t  *n(pfc_rbtree_ex_get(getTree(), key));

    return (n == NULL) ? 0 : EINVAL;
}

class TreeGetCeilThread
    : public TreeThread
{
public:
    TreeGetCeilThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeGetCeilThread::doTest(void)
{
    pfc_cptr_t    key((pfc_cptr_t)(uintptr_t)0);
    pfc_rbnode_t  *n(pfc_rbtree_ex_get_ceil(getTree(), key));

    return (n == NULL) ? 0 : EINVAL;
}

class TreeGetFloorThread
    : public TreeThread
{
public:
    TreeGetFloorThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeGetFloorThread::doTest(void)
{
    pfc_cptr_t    key((pfc_cptr_t)(uintptr_t)0);
    pfc_rbnode_t  *n(pfc_rbtree_ex_get_floor(getTree(), key));

    return (n == NULL) ? 0 : EINVAL;
}

class TreeRemoveThread
    : public TreeThread
{
public:
    TreeRemoveThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeRemoveThread::doTest(void)
{
    pfc_cptr_t    key((pfc_cptr_t)(uintptr_t)0);
    pfc_rbnode_t  *n(pfc_rbtree_ex_remove(getTree(), key));

    return (n == NULL) ? 0 : EINVAL;
}

class TreeRemoveNodeThread
    : public TreeThread
{
public:
    TreeRemoveNodeThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeRemoveNodeThread::doTest(void)
{
    rbtest_t	*rp(allocNode());

    pfc_rbnode_t  *node(RBTEST_PTR2NODE(rp));
    int	err(pfc_rbtree_ex_put(getTree(), node));
    if (err != 0) {
        return err;
    }

    pfc_rbtree_ex_remove_node(getTree(), node);

    return 0;
}

class TreeNextThread
    : public TreeThread
{
public:
    TreeNextThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeNextThread::doTest(void)
{
    pfc_rbnode_t  *node(NULL);
    pfc_rbnode_t  *next(pfc_rbtree_ex_next(getTree(), node));

    return (next == NULL) ? 0 : EINVAL;
}

class TreeClearThread
    : public TreeThread
{
public:
    TreeClearThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeClearThread::doTest(void)
{
    pfc_rbtree_ex_clear(getTree(), NULL, NULL);

    return 0;
}

class TreeGetSizeThread
    : public TreeThread
{
public:
    TreeGetSizeThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeGetSizeThread::doTest(void)
{
    return (pfc_rbtree_ex_get_size(getTree()) == 0) ? 0 : EINVAL;
}

class TreeIsEmptyThread
    : public TreeThread
{
public:
    TreeIsEmptyThread() : TreeThread() {}

protected:
    int	doTest(void);
};

int
TreeIsEmptyThread::doTest(void)
{
    return (pfc_rbtree_ex_isempty(getTree())) ? 0 : EINVAL;
}

/*
 * Thread which is used for race condition test.
 */
class RaceThread
{
public:
    RaceThread(pfc_rbtree_ex_t *tree, pfc_timespec_t *abstime, uint32_t base,
               uint32_t nitems)
        : _tree(tree), _abstime(abstime), _nodeArray(NULL), _base(base),
          _nitems(nitems), _thread(PFC_PTHREAD_INVALID_ID)
    {
    }

    ~RaceThread()
    {
        if (_thread != PFC_PTHREAD_INVALID_ID) {
            (void)pthread_join(_thread, NULL);
        }

        free(_nodeArray);
        free(_nodeArray2);
    }

    int	start(void);
    int	join(void);

    const char *
    getError(void) const
    {
        return _error.c_str();
    }

    bool
    hasError(void) const
    {
        return (_error.length() != 0);
    }  

private:
    static void		*thread_main(void *arg);
    void		setError(const char *msg, int err=0);

    pfc_rbtree_ex_t	*_tree;
    pfc_timespec_t	*_abstime;
    rbtest_t		*_nodeArray;
    rbtest_t		*_nodeArray2;
    uint32_t		_base;
    uint32_t		_nitems;
    pthread_t		_thread;
    std::string		_error;
};

int
RaceThread::start(void)
{
    _nodeArray = (rbtest_t *)calloc(_nitems, sizeof(rbtest_t));
    if (_nodeArray == NULL) {
        return ENOMEM;
    }

    _nodeArray2 = (rbtest_t *)calloc(_nitems, sizeof(rbtest_t));
    if (_nodeArray2 == NULL) {
        return ENOMEM;
    }

    uint32_t base(_base);
    for (rbtest_t *rp(_nodeArray); rp < _nodeArray + _nitems; rp++, base++) {
        rp->rt_value = base;
    }

    base = _base;
    for (rbtest_t *rp(_nodeArray2); rp < _nodeArray2 + _nitems; rp++, base++) {
        rp->rt_value = base;
        rp->rt_seq = 1;
    }

    return pthread_create(&_thread, NULL, &RaceThread::thread_main, this);
}

int
RaceThread::join(void)
{
    int	err(pthread_join(_thread, NULL));
    if (err == 0) {
        _thread = PFC_PTHREAD_INVALID_ID;
    }

    return err;
}

void *
RaceThread::thread_main(void *arg)
{
    RaceThread	*me(reinterpret_cast<RaceThread *>(arg));

    pfc_timespec_t	remains;
    pfc_timespec_t	*abstime(me->_abstime);

    uint32_t	base(me->_base);
    uint32_t	nitems(me->_nitems);
    uint32_t	half(nitems >> 1);

    rbtest_t	*array1(me->_nodeArray);
    rbtest_t	*array2(me->_nodeArray2);

    pfc_rbnode_t	*node;
    pfc_rbtree_ex_t	*tree(me->_tree);

    do {
        int	err;

        for (rbtest_t *rp(array1); rp < array1 + half; rp++) {
            err = pfc_rbtree_ex_put(tree, RBTEST_PTR2NODE(rp));
            if (err != 0) {
                me->setError("pfc_rbtree_ex_put() failed", err);
                goto out;
            }
        }

        for (rbtest_t *rp(array1 + half); rp < array1 + nitems; rp++) {
            node = pfc_rbtree_ex_update(tree, RBTEST_PTR2NODE(rp));
            if (node != NULL) {
                me->setError("Unexpected return value of "
                             "pfc_rbtree_ex_update().");
                goto out;
            }
        }

        for (uint32_t key(base); key < base + nitems; key++) {
            pfc_cptr_t	ckey((pfc_cptr_t)(uintptr_t)key);
            node = pfc_rbtree_ex_get(tree, ckey);
            if (node == NULL) {
                me->setError("Node is not found.");
                goto out;
            }
            rbtest_t	*rp(RBTEST_NODE2PTR(node));
            if (rp->rt_value != key) {
                me->setError("Unexpected value.");
                goto out;
            }
            if (rp->rt_seq != 0) {
                me->setError("Unexpected node.");
                goto out;
            }

            node = pfc_rbtree_ex_get_ceil(tree, ckey);
            if (node == NULL) {
                me->setError("Ceil node is not found.");
                goto out;
            }
            rp = RBTEST_NODE2PTR(node);
            if (rp->rt_value != key) {
                me->setError("Unexpected ceil node value.");
                goto out;
            }
            if (rp->rt_seq != 0) {
                me->setError("Unexpected ceil node.");
                goto out;
            }

            node = pfc_rbtree_ex_get_floor(tree, ckey);
            if (node == NULL) {
                me->setError("Floor node is not found.");
                goto out;
            }
            rp = RBTEST_NODE2PTR(node);
            if (rp->rt_value != key) {
                me->setError("Unexpected floor node value.");
                goto out;
            }
            if (rp->rt_seq != 0) {
                me->setError("Unexpected floor node.");
                goto out;
            }

            // Another race thread may append node which is greater than
            // the last node.
            if (key != base + nitems - 1) {
                pfc_rbnode_t *next(pfc_rbtree_ex_next(tree, node));
                if (next == NULL) {
                    me->setError("Next node is not found.");
                    goto out;
                }

                rbtest_t  *nrp(RBTEST_NODE2PTR(next));
                if (nrp->rt_value != rp->rt_value + 1) {
                    me->setError("Unexpected next value.");
                    goto out;
                }
            }
        }

        if (pfc_rbtree_ex_get_size(tree) < nitems) {
            me->setError("Unexpected the number of tree nodes.");
            goto out;
        }

        for (rbtest_t *rp(array2); rp < array2 + nitems; rp++) {
            node = pfc_rbtree_ex_update(tree, RBTEST_PTR2NODE(rp));
            if (node == NULL) {
                me->setError("Unexpected return value of "
                             "pfc_rbtree_ex_update().");
                goto out;
            }
        }

        for (uint32_t key(base); key < base + nitems; key++) {
            pfc_cptr_t	ckey((pfc_cptr_t)(uintptr_t)key);
            node = pfc_rbtree_ex_get(tree, ckey);
            if (node == NULL) {
                me->setError("Node is not found.");
                goto out;
            }
            rbtest_t	*rp(RBTEST_NODE2PTR(node));
            if (rp->rt_seq != 1) {
                me->setError("Unexpected node.");
                goto out;
            }
        }

        if (pfc_rbtree_ex_get_size(tree) < nitems) {
            me->setError("Unexpected the number of tree nodes.");
            goto out;
        }

        for (uint32_t i(0); i < nitems; i++) {
            pfc_cptr_t	ckey((pfc_cptr_t)(uintptr_t)(base + i));
            if (i & 1) {
                node = pfc_rbtree_ex_remove(tree, ckey);
                if (node == NULL) {
                    me->setError("Node is not found.");
                    goto out;
                }
            }
            else {
                node = pfc_rbtree_ex_get(tree, ckey);
                if (node == NULL) {
                    me->setError("Node is not found.");
                    goto out;
                }
                pfc_rbtree_ex_remove_node(tree, node);
            }

            rbtest_t	*rp(RBTEST_NODE2PTR(node));
            if (rp->rt_seq != 1) {
                me->setError("Unexpected node.");
                goto out;
            }
        }

        for (uint32_t key(base); key < base + nitems; key++) {
            pfc_cptr_t	ckey((pfc_cptr_t)(uintptr_t)key);
            node = pfc_rbtree_ex_get(tree, ckey);
            if (node != NULL) {
                me->setError("Unexpected node is found.");
                goto out;
            }
        }
    } while (pfc_clock_isexpired(&remains, abstime) == 0);

out:
    return NULL;
}

void
RaceThread::setError(const char *msg, int err)
{
    std::ostringstream	stream;

    stream << "ERROR: " << msg;
    if (err != 0) {
        stream << "errno=" << err << " (" << strerror(err) << ")";
    }

    _error = stream.str();
}

/*
 * Below are test cases.
 */

/*
 * Empty tree test.
 */
TEST_F(RedBlackTreeExTest, empty)
{
    ASSERT_EQ(RBNODE_NULL, _tree.rbx_tree.rb_root);
    ASSERT_EQ((pfc_ptr_t)rbtest_compare, (pfc_ptr_t)_tree.rbx_tree.rb_comp);
    ASSERT_EQ((pfc_ptr_t)rbtest_getkey, (pfc_ptr_t)_tree.rbx_tree.rb_keyfunc);

    for (int i = 0; i < RBTEST_LOOP; i++) {
        uint32_t	value;

        // Any get method must return NULL.
        RANDOM_UINT32(value, 0);
        pfc_cptr_t	key((pfc_cptr_t)(uintptr_t)value);

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get_ceil(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get_floor(&_tree, key));
    }

    ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_next(&_tree, NULL));
    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));

    pfc_rbtree_ex_clear(&_tree, dtor, (pfc_ptr_t)this);
    ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
    ASSERT_EQ(0U, getDtorCount());
}

/*
 * Test in the case of only one node.
 */
TEST_F(RedBlackTreeExTest, one)
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
        ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
        ASSERT_EQ(0, pfc_rbtree_ex_put(&_tree, node));
        ASSERT_EQ(1U, pfc_rbtree_ex_get_size(&_tree));
        ASSERT_EQ(EEXIST, pfc_rbtree_ex_put(&_tree, node));
        ASSERT_EQ(node, pfc_rbtree_ex_get(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_ex_get_ceil(&_tree, key));
        ASSERT_EQ(node, pfc_rbtree_ex_get_floor(&_tree, key));

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key_gt));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get(&_tree, key_le));

        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get_ceil(&_tree, key_gt));
        ASSERT_EQ(node, pfc_rbtree_ex_get_ceil(&_tree, key_le));

        ASSERT_EQ(node, pfc_rbtree_ex_get_floor(&_tree, key_gt));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_get_floor(&_tree, key_le));

        ASSERT_EQ(node, pfc_rbtree_ex_next(&_tree, NULL));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_next(&_tree, node));

        RBTREE_CHECK();

        // Remove node.
        ASSERT_EQ(1U, pfc_rbtree_ex_get_size(&_tree));
        ASSERT_EQ(node, pfc_rbtree_ex_remove(&_tree, key));
        ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_remove(&_tree, key));
        ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_next(&_tree, NULL));

        // Update test.
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_update(&_tree, node));
        ASSERT_EQ(node, pfc_rbtree_ex_get(&_tree, key));
        ASSERT_EQ(1U, pfc_rbtree_ex_get_size(&_tree));

        pfc_rbnode_t	*node2 = RBTEST_PTR2NODE(rp2);
        ASSERT_EQ(node, pfc_rbtree_ex_update(&_tree, node2));
        ASSERT_EQ(node2, pfc_rbtree_ex_get(&_tree, key));
        ASSERT_EQ(1U, pfc_rbtree_ex_get_size(&_tree));

        ASSERT_EQ(node2, pfc_rbtree_ex_next(&_tree, NULL));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_next(&_tree, node2));

        RBTREE_CHECK();

        if (i & 1) {
            pfc_rbtree_ex_clear(&_tree, dtor, (pfc_ptr_t)this);
            ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
            ASSERT_EQ(1U, getDtorCount());
            ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_remove(&_tree, key));
            ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
            resetDtorCount();
        }
        else {
            ASSERT_EQ(1U, pfc_rbtree_ex_get_size(&_tree));
            ASSERT_EQ(node2, pfc_rbtree_ex_remove(&_tree, key));
            ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));
        }
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_remove(&_tree, key));
        ASSERT_EQ(RBNODE_NULL, pfc_rbtree_ex_next(&_tree, NULL));
        ASSERT_EQ(0U, pfc_rbtree_ex_get_size(&_tree));

        rbtest_free(rp);
        rbtest_free(rp2);
    }
}

/*
 * Sequential tree node test. (ascending order)
 */
#define	RBTEST_NAME_SEQ_ASC(suffix)	sequential_asc_##suffix
#define	RBTEST_DECL_SEQ_ASC(num)                        \
    TEST_F(RedBlackTreeExTest, RBTEST_NAME_SEQ_ASC(num))  \
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
TEST_F(RedBlackTreeExTest, RBTEST_NAME_SEQ_ASC(random))
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
    TEST_F(RedBlackTreeExTest, RBTEST_NAME_SEQ_DESC(num)) \
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
TEST_F(RedBlackTreeExTest, RBTEST_NAME_SEQ_DESC(random))
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
    TEST_F(RedBlackTreeExTest, RBTEST_NAME_RANDOM(num))   \
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
TEST_F(RedBlackTreeExTest, RBTEST_NAME_RANDOM(random))
{
    uint32_t	number;

    do {
        RANDOM_UINT32(number, MAX_TEST_NODES);
    } while (number < MIN_TEST_NODES);

    randomTest(number);
}

/*
 * Test cases for pfc_rbtree_ex_clear().
 */
TEST_F(RedBlackTreeExTest, clear)
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
 * Test cases which ensures that wrapper APIs hold appropriate tree lock.
 */

#define	RBTEST_EX_LOCK_EXCLUSIVE_DECL(name, cls)                \
    TEST_F(RedBlackTreeExTest, name)                            \
    {                                                           \
        cls	_t;                                             \
                                                                \
        /* Writer lock must be blocked by reader lock. */       \
        _t.readLock();                                          \
        ASSERT_EQ(0, _t.start());                               \
                                                                \
        _t.delay();                                             \
        ASSERT_FALSE(_t.isDone());                              \
                                                                \
        _t.unlock();                                            \
        ASSERT_EQ(0, _t.join());                                \
        ASSERT_TRUE(_t.isDone());                               \
        ASSERT_FALSE(_t.hasError()) << _t.getError();           \
                                                                \
        ASSERT_EQ(0, _t.tryWriteLock());                        \
    }

#define	RBTEST_EX_LOCK_SHARED_DECL(name, cls)                   \
    TEST_F(RedBlackTreeExTest, name)                            \
    {                                                           \
        /* Reader lock must be blocked by writer lock. */       \
        {                                                       \
            cls	_t;                                             \
                                                                \
            _t.writeLock();                                     \
            ASSERT_EQ(0, _t.start());                           \
                                                                \
            _t.delay();                                         \
            ASSERT_FALSE(_t.isDone());                          \
                                                                \
            _t.unlock();                                        \
            ASSERT_EQ(0, _t.join());                            \
            ASSERT_TRUE(_t.isDone());                           \
            ASSERT_FALSE(_t.hasError()) << _t.getError();       \
                                                                \
            ASSERT_EQ(0, _t.tryWriteLock());                    \
        }                                                       \
                                                                \
        /* Reader lock must be able to be shared. */            \
        {                                                       \
            cls	_t;                                             \
                                                                \
            _t.readLock();                                      \
            ASSERT_EQ(0, _t.start());                           \
                                                                \
            for (int i = 0; !_t.isDone() && i < 4; i++) {       \
                _t.delay();                                     \
            }                                                   \
            ASSERT_TRUE(_t.isDone());                           \
            ASSERT_EQ(0, _t.join());                            \
            ASSERT_FALSE(_t.hasError()) << _t.getError();       \
                                                                \
            _t.unlock();                                        \
                                                                \
            ASSERT_EQ(0, _t.tryWriteLock());                    \
        }                                                       \
    }

RBTEST_EX_LOCK_EXCLUSIVE_DECL(lock_put, TreePutThread);
RBTEST_EX_LOCK_EXCLUSIVE_DECL(lock_update, TreeUpdateThread);
RBTEST_EX_LOCK_SHARED_DECL(lock_get, TreeGetThread);
RBTEST_EX_LOCK_SHARED_DECL(lock_get_ceil, TreeGetCeilThread);
RBTEST_EX_LOCK_SHARED_DECL(lock_get_floor, TreeGetFloorThread);
RBTEST_EX_LOCK_EXCLUSIVE_DECL(lock_remove, TreeRemoveThread);
RBTEST_EX_LOCK_EXCLUSIVE_DECL(lock_remove_node, TreeRemoveNodeThread);
RBTEST_EX_LOCK_SHARED_DECL(lock_next, TreeNextThread);
RBTEST_EX_LOCK_EXCLUSIVE_DECL(lock_clear, TreeClearThread);
RBTEST_EX_LOCK_SHARED_DECL(lock_get_size, TreeGetSizeThread);
RBTEST_EX_LOCK_SHARED_DECL(lock_isempty, TreeIsEmptyThread);

#define	RBTEST_RACE_PERIOD_SEC		0L
#define	RBTEST_RACE_PERIOD_NSEC                         \
    (500 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC))
#define	RBTEST_RACE_NITEMS		1000U
#define	RBTEST_RACE_BASE(idx)		((idx) * RBTEST_RACE_NITEMS)

/*
 * Race condition test.
 */
TEST_F(RedBlackTreeExTest, race)
{
    pfc_rbtree_ex_t	tree = PFC_RBTREE_EX_INITIALIZER(rbtest_compare,
                                                         rbtest_getkey);
    pfc_timespec_t	period, abstime;

    period.tv_sec = RBTEST_RACE_PERIOD_SEC;
    period.tv_nsec = RBTEST_RACE_PERIOD_NSEC;
    ASSERT_EQ(0, pfc_clock_abstime(&abstime, &period));

    RaceThread	t0(&tree, &abstime, RBTEST_RACE_BASE(0), RBTEST_RACE_NITEMS);
    RaceThread	t1(&tree, &abstime, RBTEST_RACE_BASE(1), RBTEST_RACE_NITEMS);
    RaceThread	t2(&tree, &abstime, RBTEST_RACE_BASE(2), RBTEST_RACE_NITEMS);
    RaceThread	t3(&tree, &abstime, RBTEST_RACE_BASE(3), RBTEST_RACE_NITEMS);

    ASSERT_EQ(0, t0.start());
    ASSERT_EQ(0, t1.start());
    ASSERT_EQ(0, t2.start());
    ASSERT_EQ(0, t3.start());

    ASSERT_EQ(0, t0.join());
    ASSERT_EQ(0, t1.join());
    ASSERT_EQ(0, t2.join());
    ASSERT_EQ(0, t3.join());

    ASSERT_FALSE(t0.hasError()) << t0.getError();
    ASSERT_FALSE(t1.hasError()) << t1.getError();
    ASSERT_FALSE(t2.hasError()) << t2.getError();
    ASSERT_FALSE(t3.hasError()) << t3.getError();
}
