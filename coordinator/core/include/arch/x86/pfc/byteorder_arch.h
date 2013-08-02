/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_ARCH_X86_PFC_BYTEORDER_ARCH_H
#define	_PFC_ARCH_X86_PFC_BYTEORDER_ARCH_H

/*
 * i386/x86_64 specific byte order definitions.
 */

#ifndef	_PFC_BYTEORDER_H
#error	Do NOT include byteorder_arch.h directly.
#endif	/* !_PFC_BYTEORDER_H */

/* Little endian system. */
#define	PFC_LITTLE_ENDIAN	1
#undef	PFC_BIG_ENDIAN

#endif	/* !_PFC_ARCH_X86_PFC_BYTEORDER_ARCH_H */
