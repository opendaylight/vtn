/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef   __UNC_TEST_TC_LOCK_HH__
#define   __UNC_TEST_TC_LOCK_HH__

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "tc_lock.hh"
#include <gtest/gtest.h>

using namespace std;
using namespace unc::tc;


/*class to test TcLock*/
class TestTcLock : public TcLock {

 public:
  TestTcLock():TcLock(){}

  uint32_t session_id;
  uint32_t config_id;
  TcConfigMode tc_mode;
  std::string vtn_name;

};

#endif
