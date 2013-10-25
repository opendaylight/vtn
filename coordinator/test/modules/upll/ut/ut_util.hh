/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_TEST_UPLL_UT_UTIL_HH
#define	_TEST_UPLL_UT_UTIL_HH

/*
 * Miscellaneous utility.
 */
#include <stdlib.h>
#include <string.h>
#include <gtest/gtest.h>
#include <ipc_util.hh>
#include <dal_odbc_mgr.hh>

namespace unc {
namespace upll {
namespace test {

/*
 * Allocate zeroed buffer.
 */
#define	ZALLOC_TYPE(type)					\
	(reinterpret_cast<type *>(calloc(1, sizeof(type))))
#define	ZALLOC_ARRAY(type, nelems)					\
	(reinterpret_cast<type *>(calloc((nelems), sizeof(type))))

/*
 * Create a shallow copy.
 */
static inline void *
ut_clone(void *addr, size_t size) {
	void	*newbuf(calloc(1, size));

	if (PFC_EXPECT_TRUE(newbuf != NULL)) {
		memcpy(newbuf, addr, size);
	}
	return newbuf;
}

#define	UT_CLONE(type, addr)						\
	(reinterpret_cast<type *>(ut_clone(addr, sizeof(type))))

/*
 * Base class for test environment.
 */
class UpllTestEnv
  : public ::testing::Test
{
protected:
  virtual void  SetUp();
  virtual void  TearDown();

  inline unc::upll::dal::DalDmlIntf *
  getDalDmlIntf(void)
  {
    return dynamic_cast<unc::upll::dal::DalDmlIntf *>(&_dalOdbcMgr);
  }

private:
  unc::upll::dal::DalOdbcMgr  _dalOdbcMgr;
};

/*
 * Declare IpcReqRespHeader as local variable.
 */
#define	IPC_REQ_RESP_HEADER_DECL(name)          \
    IpcReqRespHeader  __buf_##name;             \
    IpcReqRespHeader  *name(&__buf_##name);     \
    memset(name, 0, sizeof(*name))

/*
 * Declare IpcResponse as local variable.
 */
#define	IPC_RESPONSE_DECL(name)                 \
    IpcResponse  __buf_##name;                  \
    IpcResponse  *name(&__buf_##name);          \
    memset(name, 0, sizeof(*name))

}  // test
}  // upll
}  // unc

#endif	/* !_TEST_UPLL_UT_UTIL_HH */
