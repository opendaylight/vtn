/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_COMPONENT_H
#define	_UNC_COMPONENT_H

/*
 * UNC Component ID definition.
 */

#include <unc/base.h>

UNC_C_BEGIN_DECL

/*
 * UNC Component ID is mainly used as apl_No of the alarm functionality.
 * UNC can use IDs from 64 to 127, since the range is not used by PFC.
 */
enum unc_component_id {
	UNCCID_NULL		= 0,		/* Invalid ID */

	UNCCID_CORE		= 64,		/* Core */
	UNCCID_TC		,		/* Transaction Coordinator */
	UNCCID_LOGICAL		,		/* Platform Logical Layer */
	UNCCID_PHYSICAL		,		/* Platform Physical Layer */
	UNCCID_PFCDRIVER	,		/* PFC Driver */
	UNCCID_SYSMG		,		/* System Management */
	UNCCID_NOMG		,		/* Node Management */
	UNCCID_MGMT		,		/* Operation Management */
	UNCCID_UNCSH		,		/* uncshell */
	UNCCID_UNCCMDGWD	,		/* unccmdgwd */
};

UNC_C_END_DECL

#endif	/* !_UNC_COMPONENT_H */
