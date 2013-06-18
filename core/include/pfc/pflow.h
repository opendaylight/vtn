/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_PFLOW_H
#define	_PFC_PFLOW_H

/*
 * Gateway header file for definition of PFLOW vendor specific message.
 *
 * Remarks:
 *	This is not public header file.
 */

#if	defined(__PFC_PFS_1_0_SOURCE)

/* Build object for PFS-1.0. */
#include <pfs-1_0/pf_nec.h>

#elif	defined(__PFC_PFS_2_0_SOURCE)

/* Build object for PFS-2.0. */
#include <pfs-2_0/pf_nec.h>

#elif	defined(__PFC_PFS_3_0_SOURCE)

/* Build object for PFS-3.0. */
#include <openflow/openflow.h>
#include <pfs-3_0/pf_nec.h>
#include <pfs-3_0/pf_nec_pfc.h>

#else	/* Default */

/* Build object for the latest PFS. */
#include <openflow/openflow.h>
#include <pfs/pf_nec.h>
#include <pfs/pf_nec_pfc.h>

#endif	/* defined(__PFC_PFS_1_0_SOURCE) */

#endif	/* !_PFC_PFLOW_H */
