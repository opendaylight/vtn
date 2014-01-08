/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>

#include <pfc/log.h>
#include <pfcxx/thread.hh>
#include <pfcxx/event.hh>
#include <pfcxx/synch.hh>

using namespace pfc::core;

enum EventMaskType {
	EM_TYPE1,
	EM_TYPE2,
	EM_TYPE3
};

TEST(EventMask, EventMask1)
{
	pfc_log_debug("EventMask_EventMask1() is called");

	// test1:
	EventMask *em = NULL;
	em = new EventMask();
	ASSERT_NE((EventMask *)NULL, em);
	delete em;
}

TEST(EventMask, EventMask2)
{
	pfc_log_debug("EventMask_EventMask2() is called");

	// normal test1:
	{
		EventMask *em = NULL;
		pfc_evtype_t evtype = 1;
		em = new EventMask(evtype);
		ASSERT_NE((EventMask *)NULL, em);
		// = 0...0010
		ASSERT_EQ(static_cast<pfc_evmask_t>(2), em->getValue());
		delete em;
	}

	// abnormal test1:
	{
		EventMask *em = NULL;
		pfc_evtype_t evtype = -1;
		em = new EventMask(evtype);
		ASSERT_NE((EventMask *)NULL, em);
		// = 0...00
		ASSERT_EQ(static_cast<pfc_evmask_t>(0), em->getValue());
		delete em;
	}
}

TEST(EventMask, EventMask3)
{
	pfc_log_debug("EventMask_EventMask3() is called");

	// normal test1:
	{
		EventMask em_org(1);
		EventMask *em = NULL;
		em = new EventMask(em_org);
		ASSERT_NE((EventMask *)NULL, em);
		ASSERT_EQ(static_cast<pfc_evmask_t>(2), em->getValue());
		delete em;
	}
}

TEST(EventMask, fill)
{
	pfc_log_debug("EventMask_fill() is called");

	// test1:
	// for EventMask1
	{
		EventMask *em = new EventMask();
		em->fill();
		pfc_evmask_t evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(-1),
                          evmask);  // = 0xffffffffffffffff
		delete em;
	}

	// test2:
	// for EventMask2:
	{
		EventMask *em = new EventMask(static_cast<pfc_evtype_t>(1));
		em->fill();
		pfc_evmask_t evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(-1),
                          evmask);	// = 0xffffffffffffffff
		delete em;
	}

	// test3:
	// for EventMask3:
	{
		EventMask em_org;
		EventMask *em = new EventMask(em_org);
		em->fill();
		pfc_evmask_t evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(-1),
                          evmask);	// = 0xffffffffffffffff
		delete em;
	}
}

TEST(EventMask, empty)
{
	pfc_log_debug("EventMask_empty() is called");

	// test1:
	// for EventMask1
	{
		EventMask *em = new EventMask();
		em->empty();
		pfc_evmask_t evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);
		delete em;
	}

	// test2:
	// for EventMask2:
	{
		EventMask *em = new EventMask(static_cast<pfc_evtype_t>(1));
		em->empty();
		pfc_evmask_t evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);
		delete em;
	}

	// test3:
	// for EventMask3:
	{
		EventMask em_org;
		EventMask *em = new EventMask(em_org);
		em->empty();
		pfc_evmask_t evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);
		delete em;
	}
}

TEST(EventMask, add)
{
	pfc_log_debug("EventMask_add() is called");

	// test1:
	// for EventMask1
	{
		pfc_evmask_t evmask;
		EventMask *em = new EventMask();
		em->empty();

		// normal test
		em->add((pfc_evtype_t)2);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(4), evmask);

		// overwrite test
		em->add((pfc_evtype_t)0);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(5), evmask);

		// abnormal test
		em->empty();
		em->add((pfc_evtype_t)32);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);

		delete em;
	}

	// test2:
	// for EventMask2:
	{
		pfc_evmask_t evmask;
		EventMask *em = new EventMask(static_cast<pfc_evtype_t>(1));
		em->empty();

		// normal test
		em->add((pfc_evtype_t)2);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(4), evmask);

		// overwrite test
		em->add((pfc_evtype_t)0);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(5), evmask);

		// abnormal test
		em->empty();
		em->add((pfc_evtype_t)32);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);

		delete em;
	}

	// test3:
	// for EventMask3:
	{
		pfc_evmask_t evmask;
		EventMask em_org;
		EventMask *em = new EventMask(em_org);
		em->empty();

		// normal test
		em->add((pfc_evtype_t)2);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(4), evmask);

		// overwrite test
		em->add((pfc_evtype_t)0);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(5), evmask);

		// abnormal test
		em->empty();
		em->add((pfc_evtype_t)32);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);

		delete em;
	}

}

TEST(EventMask, remove)
{
	pfc_log_debug("EventMask_remove() is called");

	// test1:
	// for EventMask1
	{
		pfc_evmask_t evmask;
		EventMask *em = new EventMask();
		em->fill();

		// normal test
		em->remove((pfc_evtype_t)31);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(2147483647), evmask);

		// overwrite test
		em->remove((pfc_evtype_t)29);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(1610612735), evmask);

		// abnormal test
		em->empty();
		em->remove((pfc_evtype_t)32);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);

		delete em;
	}

	// test2:
	// for EventMask2:
	{
		pfc_evmask_t evmask;
		EventMask *em = new EventMask(static_cast<pfc_evtype_t>(1));
		em->fill();

		// normal test
		em->remove((pfc_evtype_t)31);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(2147483647), evmask);

		// overwrite test
		em->remove((pfc_evtype_t)29);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(1610612735), evmask);

		// abnormal test
		em->empty();
		em->remove((pfc_evtype_t)32);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);

		delete em;
	}

	// test3:
	// for EventMask3:
	{
		pfc_evmask_t evmask;
		EventMask em_org;
		EventMask *em = new EventMask(em_org);
		em->fill();

		// normal test
		em->remove((pfc_evtype_t)31);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(2147483647), evmask);

		// overwrite test
		em->remove((pfc_evtype_t)29);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(1610612735), evmask);

		// abnormal test
		em->empty();
		em->remove((pfc_evtype_t)32);
		evmask = em->getValue();
		EXPECT_EQ(static_cast<pfc_evmask_t>(0), evmask);

		delete em;
	}

}

TEST(EventMask, test)
{
	pfc_log_debug("EventMask_test() is called");

	// test1:
	// for EventMask1
	{
		EventMask *em = new EventMask();

		// normal test to match with the mask
		em->fill();
		EXPECT_EQ(PFC_TRUE, em->test((pfc_evtype_t)1));

		// normal test to unmatch with the mask
		em->empty();
		EXPECT_EQ(PFC_FALSE, em->test((pfc_evtype_t)1));

		// abnormal test
		em->fill();
		EXPECT_EQ(PFC_FALSE, em->test((pfc_evtype_t)32));

		delete em;
	}

	// test2:
	// for EventMask2:
	{
		EventMask *em = new EventMask(static_cast<pfc_evtype_t>(1));

		// normal test to match with the mask
		em->fill();
		EXPECT_EQ(PFC_TRUE, em->test((pfc_evtype_t)1));

		// normal test to unmatch with the mask
		em->empty();
		EXPECT_EQ(PFC_FALSE, em->test((pfc_evtype_t)1));

		// abnormal test
		em->fill();
		EXPECT_EQ(PFC_FALSE, em->test((pfc_evtype_t)32));

		delete em;
	}

	// test3:
	// for EventMask3:
	{
		EventMask em_org;
		EventMask *em = new EventMask(em_org);

		// normal test to match with the mask
		em->fill();
		EXPECT_EQ(PFC_TRUE, em->test((pfc_evtype_t)1));

		// normal test to unmatch with the mask
		em->empty();
		EXPECT_EQ(PFC_FALSE, em->test((pfc_evtype_t)1));

		// abnormal test
		em->fill();
		EXPECT_EQ(PFC_FALSE, em->test((pfc_evtype_t)32));

		delete em;
	}
}

TEST(EventMask, getValue)
{
	pfc_log_debug("EventMask_getValue() is called");

	// This test has been executed by the above tests.
}
