/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_IPC_H
#define	_PFC_IPC_H

/*
 * Common definitions for the PFC Inter-Process Communication framework.
 */

#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Maximum string length defined by IPC framework.
 */
#define	PFC_IPC_CHANNEL_NAMELEN_MAX	PFC_CONST_U(15)	/* IPC channel */
#define	PFC_IPC_SERVICE_NAMELEN_MAX	PFC_CONST_U(31)	/* IPC service name */
#define	PFC_IPC_HOSTSET_NAMELEN_MAX	PFC_CONST_U(31) /* IPC host set name */

#ifndef	_PFC_IN_CFDEFC

#include <pfc/hostaddr.h>
#include <pfc/ipc_struct.h>

/*
 * Data type of protocol data unit.
 */
typedef enum {
	PFC_IPCTYPE_INT8	= 0,	/* signed 8-bit integer */
	PFC_IPCTYPE_UINT8,		/* unsigned 8-bit integer */
	PFC_IPCTYPE_INT16,		/* signed 16-bit integer */
	PFC_IPCTYPE_UINT16,		/* unsigned 16-bit integer */
	PFC_IPCTYPE_INT32,		/* signed 32-bit integer */
	PFC_IPCTYPE_UINT32,		/* unsigned 32-bit integer */
	PFC_IPCTYPE_INT64,		/* signed 64-bit integer */
	PFC_IPCTYPE_UINT64,		/* unsigned 64-bit integer */
	PFC_IPCTYPE_FLOAT,		/* single precision floating point */
	PFC_IPCTYPE_DOUBLE,		/* double precision floating point */
	PFC_IPCTYPE_IPV4,		/* IPv4 address */
	PFC_IPCTYPE_IPV6,		/* IPv6 address */
	PFC_IPCTYPE_STRING,		/* string terminated by NULL byte */
	PFC_IPCTYPE_BINARY,		/* raw binary image */
	PFC_IPCTYPE_NULL,		/* NULL */

	PFC_IPCTYPE_STRUCT	= 0xff,	/* user-defined struct */
} pfc_ipctype_t;

/*
 * Type of IPC service ID.
 */
typedef uint32_t	pfc_ipcid_t;

/*
 * Type of IPC service response code.
 */
typedef int32_t		pfc_ipcresp_t;

/*
 * IPC server session instance.
 */
struct __pfc_ipcsrv;
typedef struct __pfc_ipcsrv	pfc_ipcsrv_t;

/*
 * IPC client address, which identifies IPC client.
 */
typedef struct {
	pid_t		cla_pid;		/* process ID */
	pfc_hostaddr_t	cla_hostaddr;		/* host address */
} pfc_ipccladdr_t;

/*
 * Type of IPC server session callback.
 */
typedef enum {
	PFC_IPCSRVCB_CONNRESET	= 0,		/* session reset */

	__PFC_IPCSRVCB_MAX	= PFC_IPCSRVCB_CONNRESET,
} pfc_ipcsrvcb_type_t;

/*
 * IPC client session instance.
 */
struct __pfc_ipcsess;
typedef struct __pfc_ipcsess	pfc_ipcsess_t;

/*
 * Alternative IPC connection handle.
 */
typedef uint32_t		pfc_ipcconn_t;

/*
 * Response code which indicates that a fatal error occurred on the IPC
 * service.
 */
#define	PFC_IPCRESP_FATAL		(-1)

/*
 * IPC event type.
 * Each IPC event has an event type, integer in range of [0, 63].
 * Event generator can choose one event type for each event.
 */
typedef uint8_t		pfc_ipcevtype_t;

#define	PFC_IPC_EVTYPE_MIN		PFC_CONST_U(0)
#define	PFC_IPC_EVTYPE_MAX		PFC_CONST_U(63)

/*
 * Serial ID assigned to each IPC event.
 */
typedef uint32_t	pfc_ipcevid_t;

/*
 * Invalid event serial ID.
 */
#define	PFC_IPC_EVSERIAL_INVALID	PFC_CONST_U(0)

/*
 * Mask value for IPC event type.
 * One bit in event mask corresponds to one event type.
 */
typedef uint64_t	pfc_ipcevmask_t;

#define	PFC_IPC_EVENT_MASK_EMPTY	((pfc_ipcevmask_t)0)
#define	PFC_IPC_EVENT_MASK_FILL		((pfc_ipcevmask_t)-1)
#define	PFC_IPC_EVENT_MASK_BIT(type)	(PFC_CONST_ULL(1) << (type))

/*
 * Determine whether the given IPC event type is valid or not.
 */
#define	PFC_IPC_EVTYPE_IS_VALID(type)			\
	((pfc_ipcevtype_t)(type) <= PFC_IPC_EVTYPE_MAX)

/*
 * Stringify IPC struct name.
 * 'name' must be a literal which represents struct name defined in
 * IPC struct template file.
 */
#define	__PFC_IPC_STRUCT_STRINGIFY(name)	#name

/*
 * Determine signature of IPC struct layout.
 * 'name' must be a literal which represents struct name defined in
 * IPC struct template file.
 */
#define	__PFC_IPC_STRUCT_SIGNATURE(name)	__PFC_IPCTMPL_SIG_##name

/*
 * Determine size of IPC struct.
 * 'name' must be a literal which represents struct name defined in
 * IPC struct template file.
 */
#ifdef	__cplusplus
#define	__PFC_IPC_STRUCT_SIZE(name)		sizeof(struct ::name)
#else	/* !__cplusplus */
#define	__PFC_IPC_STRUCT_SIZE(name)		sizeof(struct name)
#endif	/* __cplusplus */

/*
 * IPC structure definition, which determines layout of structure.
 * This structure must be initialized by PFC_IPC_STDEF_INIT() or
 * PFC_IPC_STDEF_INITIALIZER.
 */
typedef struct {
	const char	*ist_name;		/* structure name */
	const char	*ist_signature;		/* layout signature */
	uint32_t	ist_size;		/* structure size */
} pfc_ipcstdef_t;

/*
 * Initialize IPC structure definition.
 */
#define	PFC_IPC_STDEF_INIT(defp, stname)				\
	do {								\
		(defp)->ist_name = __PFC_IPC_STRUCT_STRINGIFY(stname);	\
		(defp)->ist_signature = __PFC_IPC_STRUCT_SIGNATURE(stname); \
		(defp)->ist_size = __PFC_IPC_STRUCT_SIZE(stname);	\
	} while (0)

/*
 * Static initializer for IPC structure definition.
 */
#define	PFC_IPC_STDEF_INITIALIZER(stname)				\
	{								\
		__PFC_IPC_STRUCT_STRINGIFY(stname),	/* ist_name */	\
		__PFC_IPC_STRUCT_SIGNATURE(stname),	/* ist_signature */ \
		__PFC_IPC_STRUCT_SIZE(stname),		/* ist_size */	\
	}

#endif	/* !_PFC_IN_CFDEFC */

PFC_C_END_DECL

#endif	/* !_PFC_IPC_H */
