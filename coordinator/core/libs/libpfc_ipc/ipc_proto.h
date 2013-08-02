/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_IPC_IPC_PROTO_H
#define	_PFC_LIBPFC_IPC_IPC_PROTO_H

/*
 * Common definitions for PFC IPC protocol.
 */

#include <pfc/iostream.h>
#include <pfc/hostaddr.h>
#include <pfc/debug.h>

PFC_C_BEGIN_DECL

/*
 * Magic number.
 */
#define	IPC_PROTO_MAGIC			PFC_CONST_U(0xf1)

/*
 * Magic number of the handshake message which indicates the connection was
 * refused because of limitation of the number of clients.
 */
#define	IPC_PROTO_MAGIC_TOOMANY		PFC_CONST_U(0xea)

/*
 * Protocol version.
 */
#define	IPC_PROTO_VERSION		PFC_CONST_U(0)

/*
 * Byte order.
 */
#define	IPC_ORDER_LITTLE		PFC_CONST_U(1)	/* little endian */
#define	IPC_ORDER_BIG			PFC_CONST_U(2)	/* big endian */

#ifdef	PFC_LITTLE_ENDIAN
#define	IPC_ORDER_NATIVE		IPC_ORDER_LITTLE
#else	/* !PFC_LITTLE_ENDIAN */
#define	IPC_ORDER_NATIVE		IPC_ORDER_BIG
#endif	/* PFC_LITTLE_ENDIAN */

#define	IPC_ORDER_IS_VALID(order)					\
	((order) == IPC_ORDER_LITTLE || (order) == IPC_ORDER_BIG)

/*
 * Handshake message, which is sent at the beginning of IPC session.
 */
typedef struct {
	uint8_t		ih_magic;	/* magic number */
	uint8_t		ih_version;	/* protocol version */
	uint8_t		ih_order;	/* byte order */
	uint8_t		ih_forder;	/* byte order for floating point */
} ipc_hshake_t;

/*
 * Initialize handshake message with the specified magic number.
 */
#define	IPC_HSHAKE_MAGIC_INIT(hshake, magic)			\
	do {							\
		(hshake)->ih_magic = (magic);			\
		(hshake)->ih_version = IPC_PROTO_VERSION;	\
		(hshake)->ih_order = IPC_ORDER_NATIVE;		\
		(hshake)->ih_forder = IPC_ORDER_NATIVE;		\
	} while (0)

/*
 * Initialize handshake message.
 */
#define	IPC_HSHAKE_INIT(hshake)				\
	IPC_HSHAKE_MAGIC_INIT(hshake, IPC_PROTO_MAGIC)

/*
 * IPC session instance.
 */
typedef struct {
	pfc_iostream_t	iss_stream;		/* session stream */
	pfc_hostaddr_t	iss_addr;		/* host address of peer */
	uint8_t		iss_version;		/* protocol version of peer */
	uint8_t		iss_flags;		/* flags */
	uint16_t	iss_resv1;		/* used by upper layer */
	uint8_t		iss_resv2[4];		/* used by upper layer */
} ipc_sess_t;

/*
 * Flags for iss_flags.
 */
#define	IPC_SSF_BSWAP		PFC_CONST_U(0x01)	/* swap bytes */
#define	IPC_SSF_BSWAP_FLOAT	PFC_CONST_U(0x02)	/* swap float bytes */

#define	IPC_NEED_BSWAP(flags)				\
	(PFC_EXPECT_FALSE(flags & IPC_SSF_BSWAP))
#define	IPC_NEED_BSWAP_FLOAT(flags)			\
	(PFC_EXPECT_FALSE(flags & IPC_SSF_BSWAP_FLOAT))
#define	IPC_NEED_BSWAP_ANY(flags)					\
	(PFC_EXPECT_FALSE(flags & (IPC_SSF_BSWAP | IPC_SSF_BSWAP_FLOAT)))

/*
 * Reset session flags.
 */
#define	IPC_SESS_RESET_FLAGS(sess)				\
	do {							\
		(sess)->iss_flags = 0;				\
	} while (0)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_sess_flags_assert(ipc_sess_t *sess)
 *	Ensure that the session flag is initialized properly.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_sess_flags_assert(ipc_sess_t *sess)
{
#ifdef	PFC_VERBOSE_DEBUG
	int	family = pfc_hostaddr_gettype(&sess->iss_addr);

	PFC_ASSERT(family == AF_UNIX);
	PFC_ASSERT(sess->iss_flags == 0);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * Initialize the IPC session by the handshake message sent from the peer.
 *
 * Remarks:
 *	- The session must be initialized by pfc_ipc_sess_init() in advance.
 *
 *	- Don't check protocol version so that different version of PFC
 *	  software can access the IPC channel.
 *
 *	- If you reuse ipc_sess_t, you need to reset session flags by
 *	  IPC_SESS_RESET_FLAGS().
 */
#define	IPC_SESS_PROTO_INIT(sess, hshake)				\
	do {								\
		ipc_sess_flags_assert(sess);				\
		(sess)->iss_version = (hshake)->ih_version;		\
		if (PFC_EXPECT_FALSE((hshake)->ih_order !=		\
				     IPC_ORDER_NATIVE)) {		\
			(sess)->iss_flags |= IPC_SSF_BSWAP;		\
		}							\
		if (PFC_EXPECT_FALSE((hshake)->ih_forder !=		\
				     IPC_ORDER_NATIVE)) {		\
			(sess)->iss_flags |= IPC_SSF_BSWAP_FLOAT;	\
		}							\
	} while (0)

/*
 * Swap bytes in numerical value.
 */
#define	IPC_BSWAP(value)						\
	do {								\
		if (sizeof(value) == sizeof(uint64_t)) {		\
			(value) = PFC_BSWAP_64(value);			\
		}							\
		else if (sizeof(value) == sizeof(uint32_t)) {		\
			(value) = PFC_BSWAP_32(value);			\
		}							\
		else if (sizeof(value) == sizeof(uint16_t)) {		\
			(value) = PFC_BSWAP_16(value);			\
		}							\
		else {							\
			PFC_ASSERT(sizeof(value) == sizeof(uint8_t));	\
		}							\
	} while (0)

/*
 * Swap bytes in integer value if needed.
 */
#define	IPC_SESS_BSWAP_INT(sess, value)					\
	do {								\
		if (PFC_EXPECT_FALSE(IPC_NEED_BSWAP((sess)->iss_flags))) { \
			IPC_BSWAP(value);				\
		}							\
	} while (0)

/*
 * Meta data of an IPC message.
 */
typedef struct {
	uint32_t	imm_count;	/* number of PDUs */
	uint32_t	imm_size;	/* size of PDU data */
	uint8_t		imm_xfermode;	/* transfer mode (IPC_XFERMODE_XXX) */
	uint8_t		imm_resv1;	/* reserved (must be zero) */
	uint16_t	imm_resv2;	/* reserved (must be zero) */
} ipc_msgmeta_t;

/*
 * Header of IPC service request.
 * An IPC service name and zero or one IPC message follow.
 */
typedef struct {
	uint32_t	isrq_namelen;	/* length of service name */
	uint32_t	isrq_service;	/* service ID */
} ipc_svreq_t;

/*
 * Response code of IPC service.
 */
typedef struct {
	int32_t		isrs_response;	/* response code */
	pfc_ipcresp_t	isrs_result;	/* result of IPC service */
} ipc_svresp_t;

/*
 * Protocol data which represents the command from the client.
 * This value is sent as uint8_t.
 */
#define	IPC_COMMAND_PING	PFC_CONST_U(0x00)	/* ping request */
#define	IPC_COMMAND_INVOKE	PFC_CONST_U(0x01)	/* invoke service */
#define	IPC_COMMAND_EVENT	PFC_CONST_U(0x02)	/* event listener */

/*
 * Protocol data which indicates the PDU data mode.
 * This value is sent as uint8_t.
 */
#define	IPC_XFERMODE_STREAM	PFC_CONST_U(0x00)	/* stream */

/*
 * Response in IPC session protocol.
 * This value is sent as uint8_t.
 */
#define	IPC_PROTO_FALSE		PFC_CONST_U(0x00)
#define	IPC_PROTO_TRUE		PFC_CONST_U(0x01)

/*
 * Protocol data which indicates the command of EVENT session.
 * This value is sent as uint8_t.
 */
#define	IPC_EVENTCMD_MASK_ADD	PFC_CONST_U(0x00)	/* add mask */
#define	IPC_EVENTCMD_MASK_DEL	PFC_CONST_U(0x01)	/* delete mask */
#define	IPC_EVENTCMD_MASK_RESET	PFC_CONST_U(0x02)	/* reset mask */

/*
 * Protocol data which indicates the data type on EVENT session.
 * This value is sent as uint8_t.
 */
#define	IPC_EVENTTYPE_EVENT	PFC_CONST_U(0x01)	/* event object */

/*
 * IPC event data.
 */
typedef struct {
	uint8_t		iev_type;	/* event type */
	uint8_t		iev_namelen;	/* length of IPC service name */
	uint16_t	iev_resv;	/* used by upper layer */
	uint32_t	iev_serial;	/* serial ID */
	uint64_t	iev_time_sec;	/* creation time of event (sec)*/
	uint64_t	iev_time_nsec;	/* creation time of event (nanosec)*/
} ipc_evproto_t;

PFC_TYPE_SIZE_ASSERT(ipc_evproto_t, 24);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_IPC_IPC_PROTO_H */
