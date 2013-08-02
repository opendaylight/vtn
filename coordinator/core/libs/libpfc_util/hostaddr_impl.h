/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_HOSTADDR_IMPL_H
#define	_PFC_LIBPFC_UTIL_HOSTADDR_IMPL_H

/*
 * Internal definitions for abstract host address.
 */

#include <pfc/hostaddr.h>

/*
 * Type of host address.
 */
typedef enum {
	HOSTADDR_LOCAL	= 0,	/* local host */
	HOSTADDR_INET4,		/* IPv4 address */
	HOSTADDR_INET6,		/* IPv6 address */
} haddrtype_t;

struct hostaddr;
typedef struct hostaddr		hostaddr_t;

/*
 * Attributes of host address type.
 */
typedef struct {
	int		hat_family;	/* socket address family */
	socklen_t	hat_addroff;	/* offset ot address in sockaddr */
	socklen_t	hat_addrlen;	/* size of network address */
	const char	*hat_name;	/* symbolic name of address type */

	/*
	 * int
	 * hat_compare(const hostaddr_t *hap1, const hostaddr_t *hap2)
	 *	Compare the given two host addresses.
	 *	The address type of both addresses must be identical to
	 *	this address type.
	 *
	 * Calling/Exit State:
	 *	If the given two addresses are identical, zero is returned.
	 *	If the first argument is less than the second, a negative
	 *	value is returned.
	 *	If the first argument is greater than the second, a positive
	 *	value is returned.
	 */
	int	(*hat_compare)(const hostaddr_t *hap1, const hostaddr_t *hap2);
} haddr_attr_t;

typedef const haddr_attr_t	haddr_cattr_t;

/*
 * Layout of pfc_hostaddr_t.
 */
struct hostaddr {
	haddr_cattr_t		*ha_attr;	/* address type attributes */
	uint32_t		ha_scope;	/* IPv6 scope ID */
#ifdef	PFC_LP64
	uint32_t		ha_pad;
#endif	/* PFC_LP64 */

	/* Remarks: ha_addr keeps network address in network byte order. */
	union {
		uint32_t	inet4;		/* IPv4 address */
		uint64_t	inet6[2];	/* IPv6 address */
	} ha_addr;
};

PFC_TYPE_SIZE_ASSERT(hostaddr_t, __PFC_HOSTADDR_SIZE);

#define	HOSTADDR_PTR(addr)	((hostaddr_t *)(addr))

/*
 * Determine whether the specified socket type is valid for UNIX domain socket.
 */
#define	__IS_SOCKTYPE_VALID_UN(type)					\
	PFC_EXPECT_TRUE((type) == SOCK_STREAM || (type) == SOCK_DGRAM)
#ifdef	SOCK_SEQPACKET
#define	IS_SOCKTYPE_VALID_UN(type)			\
	PFC_EXPECT_TRUE(__IS_SOCKTYPE_VALID_UN(type) || \
			(type) == SOCK_SEQPACKET)
#else	/* !SOCK_SEQPACKET */
#define	IS_SOCKTYPE_VALID_UN(type)	__IS_SOCKTYPE_VALID_UN(type)
#endif	/* SOCK_SEQPACKET */

#endif	/* !_PFC_LIBPFC_UTIL_HOSTADDR_IMPL_H */
