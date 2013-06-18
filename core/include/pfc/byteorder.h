/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_BYTEORDER_H
#define	_PFC_BYTEORDER_H

/*
 * Byte order utilities.
 *
 * Remarks:
 *	This file is designed to be included from pfc/base.h.
 */

#ifndef	_PFC_BASE_H
#error	Do NOT include byteorder.h directly.
#endif	/* !_PFC_BASE_H */

/* Import architecture specific configuration. */
#include <pfc/byteorder_arch.h>

#ifdef	PFC_USE_GLIBC

/* Use byte swap macros provided by glibc. */

#include <byteswap.h>

#define	PFC_BSWAP_16(x)		bswap_16(x)
#define	PFC_BSWAP_32(x)		bswap_32(x)
#define	PFC_BSWAP_64(x)		bswap_64(x)

#else	/* !PFC_USE_GLIBC */
#error	Port me!
#endif	/* PFC_USE_GLIBC */

#ifndef	PFC_HAVE_NTOHLL

/*
 * uint64_t
 * ntohll(uint64_t x)
 *	Convert unsigned 64-bit integer from network byte order to host byte
 *	order.
 */

#ifdef	PFC_LITTLE_ENDIAN
#define	ntohll(x)		PFC_BSWAP_64(x)
#else	/* !PFC_LITTLE_ENDIAN */
#define	ntohll(x)		(x)
#endif	/* PFC_LITTLE_ENDIAN */

#endif	/* !PFC_HAVE_NTOHLL */

#ifndef	PFC_HAVE_HTONLL

/*
 * uint64_t
 * htonll(uint64_t x)
 *	Convert unsigned 64-bit integer from host byte order to network byte
 *	order.
 */

#ifdef	PFC_LITTLE_ENDIAN
#define	htonll(x)		PFC_BSWAP_64(x)
#else	/* !PFC_LITTLE_ENDIAN */
#define	htonll(x)		(x)
#endif	/* PFC_LITTLE_ENDIAN */

#endif	/* !PFC_HAVE_HTONLL */

#endif	/* !_PFC_BYTEORDER_H */
