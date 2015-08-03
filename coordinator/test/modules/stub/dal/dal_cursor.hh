/*
 * Copyright (c) 2013-2015 NEC Corporation
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
//#include "uncxx/upll_log.hh"
#include "dal_defines.hh"
#include "dal_bind_info.hh"
namespace unc {
namespace upll {
namespace dal {


class DalCursor {
  public :
    explicit DalCursor( ) {
    }
    ~DalCursor() {
    }

    DalResultCode GetNextRecord() {
    	return kDalRcSuccess;
    }

    DalResultCode CloseCursor(bool delete_bind) {
    	return kDalRcSuccess;
    }

};  // class DalCursor

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_CURSOR_HH__
