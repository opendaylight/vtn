/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_HOSTADDR_H
#define	_PFC_HOSTADDR_H

/*
 * Abstract host address.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pfc/socket.h>

PFC_C_BEGIN_DECL

/*
 * String length enough to store symbolic name of host address type,
 * including terminator.
 */
#define	PFC_HOSTADDR_STRSIZE_TYPE	PFC_CONST_U(6)

/*
 * String length enough to store numeric IP address, including terminator.
 */
#define	PFC_HOSTADDR_STRSIZE_INET4	PFC_CONST_U(16)
#define	PFC_HOSTADDR_STRSIZE_INET6	PFC_CONST_U(52)
#define	PFC_HOSTADDR_STRSIZE_INET	PFC_HOSTADDR_STRSIZE_INET6

/*
 * String length enough to store string representation of host address,
 * including terminator.
 */
#define	PFC_HOSTADDR_STRSIZE					\
	(PFC_HOSTADDR_STRSIZE_TYPE + PFC_HOSTADDR_STRSIZE_INET)

/*
 * Abstract host address.
 */
#ifdef	PFC_LP64
#define	__PFC_HOSTADDR_SIZE	PFC_CONST_U(32)
#else	/* !PFC_LP64 */
#define	__PFC_HOSTADDR_SIZE	PFC_CONST_U(24)
#endif	/* PFC_LP64 */

typedef union {
	uint8_t		addr[__PFC_HOSTADDR_SIZE];
	uint64_t	align;
} pfc_hostaddr_t;

/*
 * Abstract socket address.
 */
typedef struct {
	const struct sockaddr	*sa_addr;	/* socket address */
	socklen_t		sa_addrlen;	/* length of socket address */
	int			sa_socktype;	/* socket type (SOCK_XXX) */
	int			sa_protocol;	/* protocol */
} pfc_sockaddr_t;

/*
 * Flags for pfc_hostaddr_tostring().
 */
#define	PFC_HA2STR_TYPE		PFC_CONST_U(0x1)	/* include addr type */

/*
 * Prototypes.
 */
extern int	pfc_hostaddr_init(pfc_hostaddr_t *PFC_RESTRICT haddr,
				  int family, const char *PFC_RESTRICT addr);
extern int	pfc_hostaddr_init_local(pfc_hostaddr_t *haddr);
extern int	pfc_hostaddr_init_inet4(pfc_hostaddr_t *PFC_RESTRICT haddr,
					const struct in_addr *PFC_RESTRICT
					addr);
extern int	pfc_hostaddr_init_inet6(pfc_hostaddr_t *PFC_RESTRICT haddr,
					const struct in6_addr *PFC_RESTRICT
					addr, uint32_t scope);
extern int	pfc_hostaddr_fromsockaddr(pfc_hostaddr_t *PFC_RESTRICT haddr,
					  const struct sockaddr *PFC_RESTRICT
					  saddr);
extern int	pfc_hostaddr_fromstring(pfc_hostaddr_t *PFC_RESTRICT haddr,
					const char *PFC_RESTRICT addr);
extern int	pfc_hostaddr_tostring(const pfc_hostaddr_t *PFC_RESTRICT haddr,
				      char *PFC_RESTRICT dst,
				      size_t size, uint32_t flags);
extern int	pfc_hostaddr_compare(const pfc_hostaddr_t *haddr1,
				     const pfc_hostaddr_t *haddr2);
extern int	pfc_hostaddr_gettype(const pfc_hostaddr_t *haddr);
extern int	pfc_hostaddr_getaddr(const pfc_hostaddr_t *PFC_RESTRICT haddr,
				     void *PFC_RESTRICT dst,
				     size_t *PFC_RESTRICT sizep);
extern int	pfc_hostaddr_getscope(const pfc_hostaddr_t *PFC_RESTRICT haddr,
				      uint32_t *PFC_RESTRICT scopep);

extern int	pfc_sockaddr_init(pfc_sockaddr_t *PFC_RESTRICT sap,
				  const pfc_hostaddr_t *PFC_RESTRICT haddr,
				  const char *PFC_RESTRICT service,
				  const struct addrinfo *PFC_RESTRICT hints);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_sockaddr_destroy(pfc_sockaddr_t *sap)
 *	Free up resources held by the call of pfc_sockaddr_init() or
 *	pfc_sockaddr_fromhostaddr().
 *
 * Remarks:
 *	Specifying invalid pointer to pfc_sockaddr_t, including NULL, results
 *	in undefined behavior.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_sockaddr_destroy(pfc_sockaddr_t *sap)
{
	free((void *)sap->sa_addr);
	sap->sa_addr = NULL;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_sockaddr_open(const pfc_sockaddr_t *sap, int flags)
 *	Create a socket suitable for the specified socket address.
 *
 *	`flags' is a flag bits to be passed to pfc_sock_open().
 *
 * Calling/Exit State:
 *	Upon successful completion, valid socket descriptor that is not -1 is
 *	returned.
 *	On error, an appropriate error number is set to errno, and -1 is
 *	returned.
 *
 * Remarks:
 *	Specifying NULL to `sap' results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_sockaddr_open(const pfc_sockaddr_t *sap, int flags)
{
	return pfc_sock_open(sap->sa_addr->sa_family, sap->sa_socktype,
			     sap->sa_protocol, flags);
}

PFC_C_END_DECL

#endif	/* !_PFC_HOSTADDR_H */
