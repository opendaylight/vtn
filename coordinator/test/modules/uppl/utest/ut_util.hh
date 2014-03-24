/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_UPPL_UT_UTIL_HH
#define _TEST_UPPL_UT_UTIL_HH

/*
 * Miscellaneous utility.
 */
#include <stdlib.h>
#include <string.h>
#include <gtest/gtest.h>

namespace unc {
namespace uppl {
namespace test {

/*
 * Base class for test environment.
 */
class UpplTestEnv
  : public ::testing::Test {
 protected:
  virtual void  SetUp();
  virtual void  TearDown();
};

}  // namespace test
}  // namespace uppl
}  // namespace unc

#endif /* !_TEST_UPPL_UT_UTIL_HH */
