/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODCDRIVER_INCLUDE_H_
#define _ODCDRIVER_INCLUDE_H_

#define ODCDRIVER_CHANNEL_NAME         "drvodcd"    /* Channel Name for ODC Driver */
#define ODCDRIVER_SERVICE_NAME         "vtndrvintf"  /* Service Name for normal IPC operation like request/response for ODC Driver */

typedef enum {
  ODCDRV_SVID_PLATFORM = 0              /* Platform Service ID  */
} OdcdrvSvid;

#endif
