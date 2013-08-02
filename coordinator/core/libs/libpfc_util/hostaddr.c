/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * hostaddr.c - Abstract host address.
 */

#include <stddef.h>
#include <string.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pfc/util.h>
#include <pfc/socket.h>
#include "hostaddr_impl.h"

/*
 * Data type shorthands.
 */
typedef struct sockaddr_un	sockaddr_un_t;
typedef struct sockaddr_in	sockaddr_in_t;
typedef struct sockaddr_in6	sockaddr_in6_t;

/*
 * Separator of host address and address type.
 */
#define	HOSTADDR_SEP_CHAR		'/'

/*
 * Separator of IPv6 scope ID.
 */
#define	HOSTADDR_SCOPE_SEP_CHAR		'%'

#ifdef	PFC_LP64
#define	HOSTADDR_PAD_INIT(hap)			\
	do {					\
		(hap)->ha_pad = 0;		\
	} while (0)
#else	/* !PFC_LP64 */
#define	HOSTADDR_PAD_INIT(hap)		((void)0)
#endif	/* PFC_LP64 */

/*
 * Initialize host address type.
 */
#define	HOSTADDR_INIT_SCOPE(hap, attr, scope)			\
	do {							\
		(hap)->ha_attr = (attr);			\
		(hap)->ha_scope = (scope);			\
		HOSTADDR_PAD_INIT(hap);				\
		(hap)->ha_addr.inet6[0] = 0;			\
		(hap)->ha_addr.inet6[1] = 0;			\
	} while (0)

#define	HOSTADDR_INIT(hap, attr)	HOSTADDR_INIT_SCOPE(hap, attr, 0)

/*
 * Valid flag bits for pfc_hostaddr_tostring().
 */
#define	HA2STR_FLAGS		(PFC_HA2STR_TYPE)
#define	HA2STR_FLAGS_IS_VALID(flags)			\
	PFC_EXPECT_TRUE(((flags) & ~HA2STR_FLAGS) == 0)

#define	HOSTADDR_IN_SOCKADDR(attr, saddr)		\
	((uint8_t *)(saddr) + (attr)->hat_addroff)

/*
 * Declare host address type.
 */
#define	HOSTADDR_LOCAL_DECL()					\
	{							\
		.hat_family	= AF_UNIX,			\
		.hat_name	= "LOCAL",			\
		.hat_addroff	= 0,				\
		.hat_addrlen	= 0,				\
		.hat_compare	= NULL,				\
	}

#define	HOSTADDR_INET_DECL(family, name, addrtype, addrmember)		\
	{								\
		.hat_family	= (family),				\
		.hat_name	= (name),				\
		.hat_addroff	= offsetof(addrtype, addrmember),	\
		.hat_addrlen	= sizeof(((addrtype *)0)->addrmember),	\
		.hat_compare	= hostaddr_compare_##family,		\
	}

/*
 * Internal prototypes.
 */
static int	hostaddr_init_network(hostaddr_t *PFC_RESTRICT hap, int family,
				      const char *PFC_RESTRICT addr);
static int	hostaddr_tostring(const hostaddr_t *PFC_RESTRICT hap,
				  haddr_cattr_t *PFC_RESTRICT attr,
				  char *PFC_RESTRICT dst, size_t size,
				  uint32_t flags);
static int	hostaddr_compare_AF_INET(const hostaddr_t *hap1,
					 const hostaddr_t *hap2);
static int	hostaddr_compare_AF_INET6(const hostaddr_t *hap1,
					  const hostaddr_t *hap2);

static int	sockaddr_init_local(pfc_sockaddr_t *PFC_RESTRICT sap,
				    const char *path,
				    const struct addrinfo *PFC_RESTRICT hints);
static int	sockaddr_init_addrinfo(pfc_sockaddr_t *PFC_RESTRICT sap,
				       struct addrinfo *PFC_RESTRICT aip,
				       int family, uint32_t scope);

/*
 * Host address types.
 * Array index must be identical to haddrtype_t.
 */
static haddr_cattr_t	hostaddr_types[] = {
	/* HOSTADDR_LOCAL */
	HOSTADDR_LOCAL_DECL(),

	/* HOSTADDR_INET4 */
	HOSTADDR_INET_DECL(AF_INET, "INET4", sockaddr_in_t, sin_addr),

	/* HOSTADDR_INET6 */
	HOSTADDR_INET_DECL(AF_INET6, "INET6", sockaddr_in6_t, sin6_addr),
};

#define	HOSTADDR_TYPE2ATTR(type)	(&hostaddr_types[(uint32_t)(type)])
#define	HOSTADDR_ATTR_IS_ALIGNED(attr)					\
	PFC_EXPECT_TRUE(&hostaddr_types[((attr) - hostaddr_types)] == (attr))
#define	HOSTADDR_ATTR_IS_VALID(attr)					\
	PFC_EXPECT_TRUE((attr) >= hostaddr_types &&			\
			(attr) < PFC_ARRAY_LIMIT(hostaddr_types) &&	\
			HOSTADDR_ATTR_IS_ALIGNED(attr))

/*
 * static inline haddr_cattr_t *
 * hostaddr_getbyfamily(int family)
 *	Return a non-NULL pointer to host address type associated with
 *	the given socket address family. NULL is returned if not found.
 */
static inline haddr_cattr_t *
hostaddr_getbyfamily(int family)
{
	haddr_cattr_t	*attr;

	for (attr = hostaddr_types; attr < PFC_ARRAY_LIMIT(hostaddr_types);
	     attr++) {
		if (attr->hat_family == family) {
			return attr;
		}
	}

	return NULL;
}

/*
 * static inline haddr_cattr_t *
 * hostaddr_getbyname(const char *typename)
 *	Return a non-NULL pointer to host address type associated with
 *	the given address type name. NULL is returned if not found.
 */
static inline haddr_cattr_t *
hostaddr_getbyname(const char *typename)
{
	haddr_cattr_t	*attr;

	for (attr = hostaddr_types; attr < PFC_ARRAY_LIMIT(hostaddr_types);
	     attr++) {
		if (strcasecmp(attr->hat_name, typename) == 0) {
			return attr;
		}
	}

	return NULL;
}

/*
 * int
 * pfc_hostaddr_init(pfc_hostaddr_t *PFC_RESTRICT haddr,
 *		     int family, const char *PFC_RESTRICT addr)
 *	Initialize host address.
 *
 *	if AF_UNIX is specified to `family', the host address is considered
 *	as local host. In this case, `addr' is never used.
 *
 *	Otherwise, it is considered as network host address. `family' is used
 *	as a hint to determine address type:
 *
 *	  - If AF_INET is specified to `family', the host address is
 *	    considered as IPv4 address.
 *	  - If AF_INET6 is specified to `family', the host address is
 *	    considered as IPv6 address.
 *	  - If AF_UNSPEC is specified to `family', the host address is
 *	    considered as IPv4 or IPv6 address.
 *
 *	If the string specified to `addr' is not numeric network address, and
 *	two or more addresses is assigned to `addr', this function chooses the
 *	first address returned by getaddrinfo(3).
 *
 *	Note that the host address is initialized as loopback address if
 *	NULL is specified to `addr'.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	Otherwise error code returned by getaddrinfo(3) is returned.
 *	So gai_strerror() can be used to convert error code to human readable
 *	string. If EAI_SYSTEM is returned, error number defined in errno.h
 *	is set to errno.
 */
int
pfc_hostaddr_init(pfc_hostaddr_t *PFC_RESTRICT haddr, int family,
		  const char *PFC_RESTRICT addr)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);

	/* Assertion for alignment of structure fields. */
	PFC_ASSERT(offsetof(hostaddr_t, ha_addr) ==
		   offsetof(hostaddr_t, ha_addr.inet4));
	PFC_ASSERT(offsetof(hostaddr_t, ha_addr) ==
		   offsetof(hostaddr_t, ha_addr.inet6));

	if (PFC_EXPECT_FALSE(hap == NULL)) {
		errno = EINVAL;

		return EAI_SYSTEM;
	}

	if (family == AF_UNIX) {
		/* Local address. */
		HOSTADDR_INIT(hap, &hostaddr_types[HOSTADDR_LOCAL]);

		return 0;
	}

	return hostaddr_init_network(hap, family, addr);
}

/*
 * int
 * pfc_hostaddr_init_local(pfc_hostaddr_t *haddr)
 *	Initialize host address as local host, which is bound to UNIX domain
 *	socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, the host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	EINVAL is returned if NULL is passed to `haddr'.
 */
int
pfc_hostaddr_init_local(pfc_hostaddr_t *haddr)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);

	if (PFC_EXPECT_FALSE(hap == NULL)) {
		return EINVAL;
	}

	HOSTADDR_INIT(hap, &hostaddr_types[HOSTADDR_LOCAL]);

	return 0;
}

/*
 * int
 * pfc_hostaddr_init_inet4(pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			   const struct in_addr *PFC_RESTRICT addr)
 *	Initialize host address as IPv4 address.
 *
 *	`addr' must points IPv4 address in network byte order.
 *	If `addr' is NULL, the host address is initialized as loopback address.
 *
 * Calling/Exit State:
 *	Upon successful completion, the host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	EINVAL is returned if NULL is passed to `haddr'.
 */
int
pfc_hostaddr_init_inet4(pfc_hostaddr_t *PFC_RESTRICT haddr,
			const struct in_addr *PFC_RESTRICT addr)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	struct in_addr	laddr;

	if (PFC_EXPECT_FALSE(hap == NULL)) {
		return EINVAL;
	}

	HOSTADDR_INIT(hap, &hostaddr_types[HOSTADDR_INET4]);

	if (addr == NULL) {
		/* Initialize as loopback address. */
		laddr.s_addr = htonl(INADDR_LOOPBACK);
		addr = &laddr;
	}

	memcpy(&hap->ha_addr, addr, sizeof(*addr));

	return 0;
}

/*
 * int
 * pfc_hostaddr_init_inet6(pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			   const struct in6_addr *PFC_RESTRICT addr,
 *			   uint32_t scope)
 *	Initialize host address as IPv6 address.
 *
 *	`addr' must points IPv6 address in network byte order.
 *	If `addr' is NULL, the host address is initialized as loopback address.
 *	`scope' is a scoped network interface ID. Zero means that no network
 *	interface is specified.
 *
 * Calling/Exit State:
 *	Upon successful completion, the host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	EINVAL is returned if NULL is passed to `haddr'.
 */
int
pfc_hostaddr_init_inet6(pfc_hostaddr_t *PFC_RESTRICT haddr,
			const struct in6_addr *PFC_RESTRICT addr,
			uint32_t scope)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);

	if (PFC_EXPECT_FALSE(hap == NULL)) {
		return EINVAL;
	}

	HOSTADDR_INIT_SCOPE(hap, &hostaddr_types[HOSTADDR_INET6], scope);

	if (addr == NULL) {
		/* Initialize as loopback address. */
		addr = &in6addr_loopback;
	}

	memcpy(&hap->ha_addr, addr, sizeof(*addr));

	return 0;
}

/*
 * int
 * pfc_hostaddr_fromsockaddr(pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			     const struct sockaddr *PFC_RESTRICT saddr)
 *	Get address part of the socket address specified by `saddr'.
 *
 * Calling/Exit State:
 *	Upon successful completion, the host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hostaddr_fromsockaddr(pfc_hostaddr_t *PFC_RESTRICT haddr,
			  const struct sockaddr *PFC_RESTRICT saddr)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;
	int		family;

	if (PFC_EXPECT_FALSE(hap == NULL || saddr == NULL)) {
		return EINVAL;
	}

	/* Determine host address type. */
	family = saddr->sa_family;
	attr = hostaddr_getbyfamily(family);
	if (PFC_EXPECT_FALSE(attr == NULL)) {
		return EAFNOSUPPORT;
	}

	HOSTADDR_INIT(hap, attr);
	if (family != AF_UNIX) {
		memcpy(&hap->ha_addr, HOSTADDR_IN_SOCKADDR(attr, saddr),
		       attr->hat_addrlen);
		if (family == AF_INET6) {
			const sockaddr_in6_t	*sin6 =
				(const sockaddr_in6_t *)saddr;

			/* Copy scope ID. */
			hap->ha_scope = sin6->sin6_scope_id;
		}
	}

	return 0;
}

/*
 * int
 * pfc_hostaddr_fromstring(pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			   const char *PFC_RESTRICT addr)
 *	Construct host address from string representation of pfc_hostaddr_t.
 *
 *	`haddr' is initializes as local address in the following cases:
 *
 *	- `addr' is NULL, or empty string, or "LOCAL" (case insensitive).
 *	- `addr' ends with "/LOCAL" (case insensitive).
 *
 *	Otherwise `haddr' is initialized as network host address.
 *
 *	- If `addr' ends with "/INET4" (case insensitive), the host address
 *	  is initialized as IPv4 address.
 *	- If `addr' ends with "/INET6" (case insensitive), the host address
 *	  is initialized as IPv6 address.
 *	- Otherwise, the host address is considered as IPv4 or IPv6 address.
 *
 *	If the string specified to `addr', excluding network type suffix,
 *	is not numeric network address, and two or more addresses is assigned
 *	to it, this function chooses the first address returned by
 *	getaddrinfo(3).
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	Otherwise error code returned by getaddrinfo(3) is returned.
 *	So gai_strerror() can be used to convert error code to human readable
 *	string. If EAI_SYSTEM is returned, error number defined in errno.h
 *	is set to errno.
 */
int
pfc_hostaddr_fromstring(pfc_hostaddr_t *PFC_RESTRICT haddr,
			const char *PFC_RESTRICT addr)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;
	char		*textaddr;
	const char	*typestr;
	int		family, gerr;

	if (PFC_EXPECT_FALSE(hap == NULL)) {
		errno = EINVAL;

		return EAI_SYSTEM;
	}

	attr = &hostaddr_types[HOSTADDR_LOCAL];

	if (addr == NULL || *addr == '\0' ||
	    strcasecmp(addr, attr->hat_name) == 0) {
		/* Initialize the host address as local address. */
		HOSTADDR_INIT(hap, attr);

		return 0;
	}

	textaddr = NULL;

	/* Find address type part in the string. */
	typestr = strrchr(addr, HOSTADDR_SEP_CHAR);
	if (typestr != NULL) {
		size_t	len;

		attr = hostaddr_getbyname(typestr + 1);
		if (PFC_EXPECT_FALSE(attr == NULL)) {
			return EAI_FAMILY;
		}

		family = attr->hat_family;
		if (family == AF_UNIX) {
			HOSTADDR_INIT(hap, attr);

			return 0;
		}

		/* Copy address part of the string. */
		len = typestr - addr;
		if (len == 0) {
			addr = NULL;
		}
		else {
			textaddr = (char *)malloc(len + 1);
			if (PFC_EXPECT_FALSE(textaddr == NULL)) {
				return EAI_MEMORY;
			}

			memcpy(textaddr, addr, len);
			*(textaddr + len) = '\0';
			addr = textaddr;
		}
	}
	else {
		family = AF_UNSPEC;
	}

	gerr = hostaddr_init_network(hap, family, addr);
	if (textaddr != NULL) {
		free(textaddr);
	}

	return gerr;
}

/*
 * int
 * pfc_hostaddr_tostring(const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			 char *PFC_RESTRICT dst, size_t size, uint32_t flags)
 *	Convert host address specified by `haddr' to string representation.
 *
 *	`dst' must points the buffer to store string, and `size' must be the
 *	number of available bytes in the buffer pointed by `dst'.
 *
 *	If PFC_HA2STR_TYPE bit is set in `flags', the symbolic name of the
 *	host address type is appended to the string representation of the host
 *	address.
 *
 * Calling/Exit State:
 *	Upon successful completion, the string representation of the specified
 *	host address is stored to the buffer pointed by `dst', and zero is
 *	returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hostaddr_tostring(const pfc_hostaddr_t *PFC_RESTRICT haddr,
		      char *PFC_RESTRICT dst, size_t size, uint32_t flags)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;

	if (PFC_EXPECT_FALSE(hap == NULL || dst == NULL ||
			     !HA2STR_FLAGS_IS_VALID(flags))) {
		return EINVAL;
	}

	attr = hap->ha_attr;
	if (!HOSTADDR_ATTR_IS_VALID(attr)) {
		return EINVAL;
	}

	return hostaddr_tostring(hap, attr, dst, size, flags);
}

/*
 * int
 * pfc_hostaddr_compare(const pfc_hostaddr_t *haddr1,
 *			const pfc_hostaddr_t *haddr2)
 *	Compare the given two host addresses.
 *
 * Calling/Exit State:
 *	If the given two addresses are identical, zero is returned.
 *	If the first argument is less than the second, a negative value is
 *	returned.
 *	If the first argument is greater than the second, a positive value
 *	is returned.
 *
 * Remarks:
 *	Specifying invalid host address, including NULL, to any argument
 *	results in undefined behavior.
 */
int
pfc_hostaddr_compare(const pfc_hostaddr_t *haddr1,
		     const pfc_hostaddr_t *haddr2)
{
	hostaddr_t	*hap1 = HOSTADDR_PTR(haddr1);
	hostaddr_t	*hap2 = HOSTADDR_PTR(haddr2);
	haddr_cattr_t	*attr1 = hap1->ha_attr;
	haddr_cattr_t	*attr2 = hap2->ha_attr;
	int		ret;

	/* Compare address type. */
	ret = (int)(attr1 - attr2);
	if (ret != 0) {
		return ret;
	}

	if (attr1->hat_compare == NULL) {
		/* No address part. */
		return 0;
	}

	return attr1->hat_compare(hap1, hap2);
}

/*
 * int
 * pfc_hostaddr_gettype(const pfc_hostaddr_t *haddr)
 *	Return socket address family associated with the given host address.
 *
 * Calling/Exit State:
 *	AF_UNIX is returned if the specified address is local address.
 *	AF_INET is returned if the specified address is IPv4 address.
 *	AF_INET6 is returned if the specified address is IPv6 address.
 *
 *	Otherwise AF_UNSPEC is returned.
 */
int
pfc_hostaddr_gettype(const pfc_hostaddr_t *haddr)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;

	if (PFC_EXPECT_FALSE(hap == NULL)) {
		return AF_UNSPEC;
	}

	attr = hap->ha_attr;
	if (!HOSTADDR_ATTR_IS_VALID(attr)) {
		return AF_UNSPEC;
	}

	return attr->hat_family;
}

/*
 * int
 * pfc_hostaddr_getaddr(const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			void *PFC_RESTRICT dst, size_t *PFC_RESTRICT sizep)
 *	Copy network address in the host address `haddr' to the specified
 *	buffer.
 *
 *	if the host address specified by `haddr' contains network address,
 *	it is copied to the buffer pointed by `dst'. The caller must set the
 *	number of available bytes in `dst' to the buffer pointed by `sizep'.
 *	Upon successful return, actual network address size is set to the
 *	buffer pointed by `sizep'.
 *
 *	This function copies the network address in network byte order.
 *
 * Calling/Exit State:
 *	Upon successful completion, the buffer pointed by `dst' and `sizep'
 *	are updated as described above, and then zero is returned.
 *
 *	If `haddr' is local address, zero is set to the buffer pointed by
 *	`buf'. Any data is not copied to the buffer pointed by `dst'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hostaddr_getaddr(const pfc_hostaddr_t *PFC_RESTRICT haddr,
		     void *PFC_RESTRICT dst, size_t *PFC_RESTRICT sizep)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;
	int		family;
	const void	*src;
	size_t		size, reqsize;

	if (PFC_EXPECT_FALSE(hap == NULL || dst == NULL || sizep == NULL)) {
		return EINVAL;
	}

	attr = hap->ha_attr;
	if (!HOSTADDR_ATTR_IS_VALID(attr)) {
		return EINVAL;
	}

	family = attr->hat_family;
	if (family == AF_UNIX) {
		*sizep = 0;

		return 0;
	}

	if (family == AF_INET) {
		/* IPv4 address. */
		src = (const void *)&hap->ha_addr.inet4;
		reqsize = sizeof(hap->ha_addr.inet4);
	}
	else {
		/* IPv6 address. */
		PFC_ASSERT(family == AF_INET6);
		src = (const void *)hap->ha_addr.inet6;
		reqsize = sizeof(hap->ha_addr.inet6);
	}

	size = *sizep;
	if (PFC_EXPECT_FALSE(size < reqsize)) {
		return ENOSPC;
	}

	/* Copy network address. */
	memcpy(dst, src, reqsize);
	*sizep = reqsize;

	return 0;
}

/*
 * int
 * pfc_hostaddr_getscope(const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			 uint32_t *PFC_RESTRICT scopep)
 *	Get IPv6 scope ID in the specified host address.
 *
 *	This function is provided only for IPv6 address.
 *	If a non IPv6 address is specified to `haddr', zero is always set
 *	to the buffer pointed by `scopep'.
 *
 * Calling/Exit State:
 *	Upon successful completion, IPv6 scope ID is set to the buffer
 *	pointed by `scopep', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hostaddr_getscope(const pfc_hostaddr_t *PFC_RESTRICT haddr,
		      uint32_t *PFC_RESTRICT scopep)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;

	if (PFC_EXPECT_FALSE(hap == NULL || scopep == NULL)) {
		return EINVAL;
	}

	attr = hap->ha_attr;
	if (!HOSTADDR_ATTR_IS_VALID(attr)) {
		return EINVAL;
	}

	*scopep = hap->ha_scope;

	return 0;
}

/*
 * int
 * pfc_sockaddr_init(pfc_sockaddr_t *PFC_RESTRICT sap,
 *		     const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *		     const char *PFC_RESTRICT service,
 *		     const struct addrinfo *PFC_RESTRICT hints)
 *	Initialize abstract socket address.
 *
 *	Host address in the socket address is determine by pfc_hostaddr_t
 *	pointed by `haddr'.
 *	`service' is a pointer to string which represents service on the host.
 *	`hints' is a hint to resolve service.
 *
 *	- If the host address specified by `haddr' is local address, the socket
 *	  address is initialized as UNIX domain socket address.
 *	  + `service' must be a pointer to UNIX domain socket file path.
 *	  + If a non-NULL pointer is specified to `hints' and ai_socktype in
 *	    `hints' is not zero, its value is set to sa_socktype in `saddr'.
 *	    Otherwise SOCK_STREAM is set to sa_socktype in `saddr'.
 *
 *	- If the host address specified by `haddr' is network address, the
 *	  socket address is initialized as Internet Protocol socket address.
 *	  + Service, as known as port number, is determined by getaddrinfo(3).
 *        + Valid parameters in `hints' are passed to getaddrinfo(3).
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Note that pfc_sockaddr_destroy() must be called to free up resources
 *	held by this function.
 *
 *	Otherwise error code returned by getaddrinfo(3) is returned.
 *	So gai_strerror() can be used to convert error code to human readable
 *	string. If EAI_SYSTEM is returned, error number defined in errno.h
 *	is set to errno.
 *
 * Remarks:
 *	AI_CANONNAME flag in hints->ai_flags is always ignored.
 */
int
pfc_sockaddr_init(pfc_sockaddr_t *PFC_RESTRICT sap,
		  const pfc_hostaddr_t *PFC_RESTRICT haddr,
		  const char *PFC_RESTRICT service,
		  const struct addrinfo *PFC_RESTRICT hints)
{
	hostaddr_t	*hap = HOSTADDR_PTR(haddr);
	haddr_cattr_t	*attr;
	const char	*host;
	char		hostbuf[PFC_HOSTADDR_STRSIZE_INET6];
	struct addrinfo	ai, *result;
	int		gerr, family;

	if (PFC_EXPECT_FALSE(sap == NULL || hap == NULL)) {
		errno = EINVAL;

		return EAI_SYSTEM;
	}

	attr = hap->ha_attr;
	if (!HOSTADDR_ATTR_IS_VALID(attr)) {
		errno = EINVAL;

		return EAI_SYSTEM;
	}

	family = attr->hat_family;
	if (family == AF_UNIX) {
		/* UNIX domain socket address is required. */
		return sockaddr_init_local(sap, service, hints);
	}

	/* Use getaddrinfo() to resolve port number. */
	host = NULL;
	if (hints == NULL) {
		memset(&ai, 0, sizeof(ai));
	}
	else {
		ai = *hints;
		ai.ai_flags &= ~AI_CANONNAME;

		if (ai.ai_socktype == SOCK_RAW) {
			/* Need to specify host address. */
			PFC_ASSERT_INT(hostaddr_tostring(hap, attr, hostbuf,
							 sizeof(hostbuf), 0),
				       0);
			host = hostbuf;
		}
	}
	ai.ai_family = family;
	ai.ai_flags |= AI_PASSIVE;

	gerr = getaddrinfo(host, service, &ai, &result);
	if (PFC_EXPECT_TRUE(gerr == 0)) {
		gerr = sockaddr_init_addrinfo(sap, result, family,
					      hap->ha_scope);
		if (PFC_EXPECT_TRUE(gerr == 0)) {
			/* Copy host address. */
			memcpy(HOSTADDR_IN_SOCKADDR(attr, sap->sa_addr),
			       &hap->ha_addr, attr->hat_addrlen);
		}

		freeaddrinfo(result);
	}

	return gerr;
}

/*
 * static int
 * hostaddr_init_network(hostaddr_t *PFC_RESTRICT hap, int family,
 *			 const char *PFC_RESTRICT addr)
 *	Initialize host address as network address.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed host address is stored to the buffer
 *	pointed by `haddr', and zero is returned.
 *
 *	Otherwise error code returned by getaddrinfo(3) is returned.
 *	So gai_strerror() can be used to convert error code to human readable
 *	string. If EAI_SYSTEM is returned, error number defined in errno.h
 *	is set to errno.
 */
static int
hostaddr_init_network(hostaddr_t *PFC_RESTRICT hap, int family,
		      const char *PFC_RESTRICT addr)
{
	struct addrinfo	hints, *result, *aip;
	int		gerr;

	/*
	 * Convert string representation of host address into numeric address
	 * using getaddrinfo().
	 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;

	gerr = getaddrinfo(addr, "23", &hints, &result);
	if (PFC_EXPECT_FALSE(gerr != 0)) {
		return gerr;
	}

	/* Choose the first result. */
	gerr = EAI_FAMILY;
	for (aip = result; aip != NULL; aip = aip->ai_next) {
		struct sockaddr	*saddr = aip->ai_addr;
		haddr_cattr_t	*attr;

		for (attr = HOSTADDR_TYPE2ATTR(HOSTADDR_INET4);
		     attr < PFC_ARRAY_LIMIT(hostaddr_types); attr++) {
			if (attr->hat_family != saddr->sa_family) {
				continue;
			}

			HOSTADDR_INIT(hap, attr);
			memcpy(&hap->ha_addr,
			       HOSTADDR_IN_SOCKADDR(attr, saddr),
			       attr->hat_addrlen);
			gerr = 0;

			if (attr->hat_family == AF_INET6) {
				sockaddr_in6_t	*sin6 =
					(sockaddr_in6_t *)saddr;

				/* Preserve scope ID. */
				hap->ha_scope = sin6->sin6_scope_id;
			}
			goto out;
		}
	}

out:
	freeaddrinfo(result);

	return gerr;
}

/*
 * static int
 * hostaddr_tostring(const hostaddr_t *PFC_RESTRICT hap,
 *		     haddr_cattr_t *PFC_RESTRICT attr,
 *		     char *PFC_RESTRICT dst, size_t size, uint32_t flags)
 *	Convert host address specified by `hap' to string representation.
 *
 * Calling/Exit State:
 *	Upon successful completion, the string representation of the specified
 *	host address is stored to the buffer pointed by `dst', and zero is
 *	returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
hostaddr_tostring(const hostaddr_t *PFC_RESTRICT hap,
		  haddr_cattr_t *PFC_RESTRICT attr, char *PFC_RESTRICT dst,
		  size_t size, uint32_t flags)
{
	const char	*ret;
	size_t		sz;
	int		family = attr->hat_family;

	PFC_ASSERT(hap->ha_attr == attr);
	if (family == AF_UNIX) {
		int	err;

		/* Local address */
		if (flags & PFC_HA2STR_TYPE) {
			/* Copy "LOCAL" to the destination buffer. */
			sz = pfc_strlcpy(dst, attr->hat_name, size);
			err = (PFC_EXPECT_FALSE(sz >= size)) ? ENOSPC : 0;
		}
		else {
			/* Set empty string. */
			if (PFC_EXPECT_FALSE(size == 0)) {
				err = ENOSPC;
			}
			else {
				*dst = '\0';
				err = 0;
			}
		}

		return err;
	}

	/* Convert network address into string. */
	ret = inet_ntop(family, &hap->ha_addr, dst, size);
	if (PFC_EXPECT_FALSE(ret == NULL)) {
		return errno;
	}

	if (family == AF_INET6 && hap->ha_scope != 0) {
		char	scope[32];

		/* Append scope ID. */
		sz = strlen(dst);
		PFC_ASSERT(sz < size);
		size -= sz;
		dst += sz;
		if (PFC_EXPECT_FALSE(size <= 1)) {
			return ENOSPC;
		}

		*dst = HOSTADDR_SCOPE_SEP_CHAR;
		dst++;
		size--;

		(void)snprintf(scope, sizeof(scope), "%u", hap->ha_scope);
		sz = pfc_strlcpy(dst, scope, size);
		if (PFC_EXPECT_FALSE(sz >= size)) {
			return ENOSPC;
		}
		dst += sz;
		size -= sz;
	}

	if (flags & PFC_HA2STR_TYPE) {
		/* Append symbolic name of the address type. */
		if (dst == ret) {
			sz = strlen(dst);
			PFC_ASSERT(sz < size);
			size -= sz;
			dst += sz;
		}

		if (PFC_EXPECT_FALSE(size <= 1)) {
			return ENOSPC;
		}

		*dst = HOSTADDR_SEP_CHAR;
		dst++;
		size--;

		sz = pfc_strlcpy(dst, attr->hat_name, size);
		if (PFC_EXPECT_FALSE(sz >= size)) {
			return ENOSPC;
		}
	}

	return 0;
}

/*
 * static int
 * hostaddr_compare_AF_INET(const hostaddr_t *hap1, const hostaddr_t *hap2)
 *	Compare the given two IPv4 addresses.
 *
 * Calling/Exit State:
 *	If the given two addresses are identical, zero is returned.
 *	If the first argument is less than the second, a negative value is
 *	returned.
 *	If the first argument is greater than the second, a positive value
 *	is returned.
 */
static int
hostaddr_compare_AF_INET(const hostaddr_t *hap1, const hostaddr_t *hap2)
{
	uint32_t	addr1 = hap1->ha_addr.inet4;
	uint32_t	addr2 = hap2->ha_addr.inet4;

	if (addr1 == addr2) {
		return 0;
	}

	addr1 = ntohl(addr1);
	addr2 = ntohl(addr2);

	return (addr1 < addr2) ? -1 : 1;
}

/*
 * static int
 * hostaddr_compare_AF_INET6(const hostaddr_t *hap1, const hostaddr_t *hap2)
 *	Compare the given two IPv6 addresses.
 *
 * Calling/Exit State:
 *	If the given two addresses are identical, zero is returned.
 *	If the first argument is less than the second, a negative value is
 *	returned.
 *	If the first argument is greater than the second, a positive value
 *	is returned.
 */
static int
hostaddr_compare_AF_INET6(const hostaddr_t *hap1, const hostaddr_t *hap2)
{
	uint64_t	addr1 = hap1->ha_addr.inet6[0];
	uint64_t	addr2 = hap2->ha_addr.inet6[0];
	uint32_t	scope1, scope2;

	if (addr1 != addr2) {
		addr1 = ntohll(addr1);
		addr2 = ntohll(addr2);

		return (addr1 < addr2) ? -1 : 1;
	}

	addr1 = hap1->ha_addr.inet6[1];
	addr2 = hap2->ha_addr.inet6[1];
	if (addr1 != addr2) {
		addr1 = ntohll(addr1);
		addr2 = ntohll(addr2);

		return (addr1 < addr2) ? -1 : 1;
	}

	scope1 = hap1->ha_scope;
	scope2 = hap2->ha_scope;

	if (scope1 == scope2) {
		return 0;
	}

	return (scope1 < scope2) ? -1 : 1;
}

/*
 * static int
 * sockaddr_init_local(pfc_sockaddr_t *PFC_RESTRICT sap, const char *path,
 *		       const struct addrinfo *PFC_RESTRICT hints)
 *	Initialize the socket address for UNIX domain socket.
 *
 *	`hints' is used to determine socket type. If ai_socktype is not
 *	specified by `hints', SOCK_STREAM is used as socket type.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error code returned by getaddrinfo(3) is returned.
 *	So gai_strerror() can be used to convert error code to human readable
 *	string. If EAI_SYSTEM is returned, error number defined in errno.h
 *	is set to errno.
 */
static int
sockaddr_init_local(pfc_sockaddr_t *PFC_RESTRICT sap, const char *path,
		    const struct addrinfo *PFC_RESTRICT hints)
{
	socklen_t	plen, reqlen;
	sockaddr_un_t	*un;
	int		socktype;

	if (PFC_EXPECT_FALSE(path == NULL)) {
		return EAI_NONAME;
	}

	plen = strlen(path);
	if (PFC_EXPECT_FALSE(plen == 0)) {
		return EAI_NONAME;
	}

	reqlen = PFC_SOCK_UNADDR_SIZE(plen);
	if (PFC_EXPECT_FALSE(reqlen > sizeof(struct sockaddr_un))) {
		/* Too long socket file path. */
		errno = ENAMETOOLONG;

		return EAI_SYSTEM;
	}

	if (hints == NULL || hints->ai_socktype == 0) {
		socktype = SOCK_STREAM;
	}
	else {
		socktype = hints->ai_socktype;
		if (!IS_SOCKTYPE_VALID_UN(socktype)) {
			return EAI_SOCKTYPE;
		}
	}

	un = (sockaddr_un_t *)malloc(reqlen + 1);
	if (PFC_EXPECT_FALSE(un == NULL)) {
		return EAI_MEMORY;
	}

	un->sun_family = AF_UNIX;
	memcpy(un->sun_path, path, plen + 1);

	sap->sa_addr = (struct sockaddr *)un;
	sap->sa_addrlen = reqlen;
	sap->sa_protocol = 0;
	sap->sa_socktype = socktype;

	return 0;
}

/*
 * static int
 * sockaddr_init_addrinfo(pfc_sockaddr_t *PFC_RESTRICT sap,
 *			  struct addrinfo *PFC_RESTRICT aip, int family,
 *			  uint32_t scope)
 *	Initialize the socket address using address information returned by
 *	getaddrinfo(3).
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error code returned by getaddrinfo(3) is returned.
 *	So gai_strerror() can be used to convert error code to human readable
 *	string. If EAI_SYSTEM is returned, error number defined in errno.h
 *	is set to errno.
 *
 * Remarks:
 *	This function never initializes network address in saddr->sa_addr.
 *	It must be done by the caller.
 */
static int
sockaddr_init_addrinfo(pfc_sockaddr_t *PFC_RESTRICT sap,
		       struct addrinfo *PFC_RESTRICT aip, int family,
		       uint32_t scope)
{

	PFC_ASSERT(aip != NULL);

	/* Find socket address that matches required type. */
	do {
		struct sockaddr	*saddr = aip->ai_addr;
		struct sockaddr	*newaddr;
		socklen_t	addrlen;
		int		gerr;

		if (PFC_EXPECT_FALSE(saddr->sa_family != family)) {
			aip = aip->ai_next;
			continue;
		}

		/* Use this address. */
		addrlen = aip->ai_addrlen;
		newaddr = (struct sockaddr *)malloc(addrlen);
		if (PFC_EXPECT_FALSE(newaddr == NULL)) {
			gerr = EAI_MEMORY;
		}
		else {
			memcpy(newaddr, saddr, addrlen);
			sap->sa_addr = newaddr;
			sap->sa_addrlen = addrlen;
			sap->sa_socktype = aip->ai_socktype;
			sap->sa_protocol = aip->ai_protocol;
			if (family == AF_INET6) {
				sockaddr_in6_t	*new6 =
					(sockaddr_in6_t *)newaddr;

				/* Copy scope ID. */
				new6->sin6_scope_id = scope;
			}
			gerr = 0;
		}

		return gerr;
	} while (aip != NULL);

	return EAI_FAMILY;
}
