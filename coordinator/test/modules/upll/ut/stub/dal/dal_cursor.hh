/*      Copyright (c) 2012 NEC Corporation                        */
/*      NEC CONFIDENTIAL AND PROPRIETARY                          */
/*      All rights reserved by NEC Corporation.                   */
/*      This program must be used solely for the purpose for      */
/*      which it was furnished by NEC Corporation.   No part      */
/*      of this program may be reproduced  or  disclosed  to      */
/*      others,  in  any form,  without  the  prior  written      */
/*      permission of NEC Corporation.    Use  of  copyright      */
/*      notice does not evidence publication of the program.      */

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
