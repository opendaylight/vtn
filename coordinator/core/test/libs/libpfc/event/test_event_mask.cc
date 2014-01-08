/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_event_common.h"

TEST(pfc_event_mask_fill, mask)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_fill_mask() is called");

	// Fill
	pfc_event_mask_fill(&mask);
	EXPECT_EQ((pfc_evmask_t)-1, mask);

	// Overwrite
	mask = (pfc_evmask_t)2;
	pfc_event_mask_fill(&mask);
	EXPECT_EQ((pfc_evmask_t)-1, mask);
}

TEST(pfc_event_mask_empty, mask)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_empty_mask() is called");

	// Empty
	pfc_event_mask_empty(&mask);
	EXPECT_EQ(static_cast<pfc_evmask_t>(0), mask);

	// Overwrite
	mask = (pfc_evmask_t)2;
	pfc_event_mask_empty(&mask);
	EXPECT_EQ(static_cast<pfc_evmask_t>(0), mask);
}

TEST(pfc_event_mask_add, mask)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_add_mask() is called");

	// Setup
	pfc_event_mask_empty(&mask);

	// Add
	EXPECT_EQ(0, pfc_event_mask_add(&mask, 1));
	EXPECT_EQ(static_cast<pfc_evmask_t>(2), mask);	// 0.....010

	// Re add
	EXPECT_EQ(0, pfc_event_mask_add(&mask, 3));
	EXPECT_EQ(static_cast<pfc_evmask_t>(10), mask);	// 0...1010
}

TEST(pfc_event_mask_add, type)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_add_type() is called");

	// Setup
	pfc_event_mask_empty(&mask);

	// Add normal value
	EXPECT_EQ(0, pfc_event_mask_add(&mask, 1));
	EXPECT_EQ(static_cast<pfc_evmask_t>(2), mask);

	// Add an invalid value
	EXPECT_EQ(EINVAL, pfc_event_mask_add(&mask, 32));
	EXPECT_EQ(static_cast<pfc_evmask_t>(2), mask);
}

TEST(pfc_event_mask_delete, mask)
{

	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_delete_mask() is called");

	// Setup
	pfc_event_mask_fill(&mask);

	// Delete
	pfc_event_mask_delete(&mask, 31);
	EXPECT_EQ(PFC_CONST_U(2147483647), mask);   // 01.....1

	// Re delete
	pfc_event_mask_delete(&mask, 29);
	EXPECT_EQ(PFC_CONST_U(1610612735), mask); // 0101.....1
}

TEST(pfc_event_mask_delete, type)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_delete_type() is called");

	// Setup
	pfc_event_mask_fill(&mask);

	// Valid
	EXPECT_EQ(0, pfc_event_mask_delete(&mask, 31));
	EXPECT_EQ(PFC_CONST_U(2147483647), mask);

	// Invalid
	EXPECT_EQ(EINVAL, pfc_event_mask_delete(&mask, -1));
	EXPECT_EQ(PFC_CONST_U(2147483647), mask);
}

TEST(pfc_event_mask_test, mask)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_test_mask() is called");

	// Setup
	pfc_event_mask_fill(&mask);

	// match
	EXPECT_EQ(PFC_TRUE, pfc_event_mask_test(&mask, 31));

	// Setup
	EXPECT_EQ(0, pfc_event_mask_delete(&mask, 31));

	// unmatch
	EXPECT_EQ(PFC_FALSE, pfc_event_mask_test(&mask, 31));
}

TEST(pfc_event_mask_test, type)
{
	pfc_evmask_t mask;

	pfc_log_debug("pfc_event_mask_test_type() is called");

	// Setup
	pfc_event_mask_fill(&mask);

	// valid
	EXPECT_EQ(PFC_TRUE, pfc_event_mask_test(&mask, 31));

	// invalid
	EXPECT_EQ(PFC_FALSE, pfc_event_mask_test(&mask, 32));
}
