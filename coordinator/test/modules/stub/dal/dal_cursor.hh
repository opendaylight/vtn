/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_cusor.hh
 *   Contains definitions of DalCursor
 */

#ifndef __DAL_CURSOR_HH__
#define __DAL_CURSOR_HH__

#include <sql.h>
//#include "upll/upll_log.hh"
#include "dal_defines.hh"
#include "dal_bind_info.hh"
namespace unc {
namespace upll {
namespace dal {

class DalCursor {
public :
  DalCursor(const DalBindInfo *info)
    : _bindInfo1(info),
      _bindInfo2(reinterpret_cast<const DalBindInfo *>(NULL)) {}

  DalCursor(const DalBindInfo *info1, const DalBindInfo *info2)
    : _bindInfo1(info1), _bindInfo2(info2) {}

  DalCursor() {}
  ~DalCursor() {}

  inline DalResultCode
  GetNextRecord()
  {
    return kDalRcSuccess;
  }

  inline DalResultCode
  CloseCursor(bool delete_bind)
  {
    if (delete_bind) {
      delete _bindInfo1;
      delete _bindInfo2;
    }

    return kDalRcSuccess;
  }

private:
  const DalBindInfo *_bindInfo1;
  const DalBindInfo *_bindInfo2;
};  // class DalCursor

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_CURSOR_HH__
