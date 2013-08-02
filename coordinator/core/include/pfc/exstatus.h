/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_EXSTATUS_H
#define	_PFC_EXSTATUS_H

/*
 * Exit status codes for PFC commands.
 *
 * All PFC commands must specify one of the following exit status codes
 * to exit(2).
 */

#define	PFC_EX_OK		0	/* successfully completed */
#define	PFC_EX_FATAL		1	/* fatal error */
#define	PFC_EX_TEMPFAIL		2	/* temporary failure */
#define	PFC_EX_BUSY		3	/* resource busy */
#define	PFC_EX_LIBCRYPTO	88	/* unable to load libcrypto */
#define	PFC_EX_LIC_AUTH		99	/* license auth failed */

#endif	/* !_PFC_EXSTATUS_H */
