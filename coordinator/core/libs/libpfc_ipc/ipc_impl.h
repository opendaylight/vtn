/*
 * Copyright (c) 2011-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_IPC_IPC_IMPL_H
#define	_PFC_LIBPFC_IPC_IPC_IMPL_H

/*
 * Internal definitions for the PFC IPC framework.
 */

#include <string.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pfc/ipc.h>
#include <pfc/log.h>
#include <pfc/synch.h>
#include <pfc/refptr.h>
#include <pfc/iostream.h>
#include <pfc/modconst.h>
#include <pfc/debug.h>
#include <pfc/rbtree.h>
#include <pfc/hostaddr.h>
#include <pfc/epoll.h>
#include <pfc/atomic.h>
#include "ipc_proto.h"

PFC_C_BEGIN_DECL

 /*
  * Shorthand for socket address.
  */
typedef struct sockaddr		sockaddr_t;
typedef const sockaddr_t	csockaddr_t;
typedef struct sockaddr_un	sockaddr_un_t;
typedef const sockaddr_un_t	csockaddr_un_t;

/*
 * Shorthands for data types.
 */
typedef const pfc_timespec_t	ctimespec_t;
typedef struct epoll_event	epoll_event_t;

/*
 * Forward declarations.
 */
#ifndef	__PFC_IPC_STRINFO_DEFINED
#define	__PFC_IPC_STRINFO_DEFINED
struct ipc_strinfo;
typedef struct ipc_strinfo	ipc_strinfo_t;
typedef const ipc_strinfo_t	ipc_cstrinfo_t;
#endif	/* !__PFC_IPC_STRINFO_DEFINED */

/*
 * Maximum data size in output stream.
 */
#ifdef	PFC_LP64
#define	IPC_OUTSIZE_MAX			UINT32_MAX		/* 4G - 1 */
#else	/* !PFC_LP64 */
#define	IPC_OUTSIZE_MAX			PFC_CONST_U(0x80000000)	/* 2G */
#endif	/* PFC_LP64 */

/*
 * Maximum length of IPC channel name.
 */
#define	IPC_CHANNEL_NAMELEN_MAX		PFC_IPC_CHANNEL_NAMELEN_MAX

/*
 * Maximum length of IPC service name.
 */
#define	IPC_SERVICE_NAMELEN_MAX		PFC_IPC_SERVICE_NAMELEN_MAX

#if	IPC_SERVICE_NAMELEN_MAX != PFC_MODULE_NAME_MAX
#error	IPC_SERVICE_NAMELEN_MAX must be the same as PFC_MODULE_NAME_MAX.
#endif	/* IPC_SERVICE_NAMELEN_MAX != PFC_MODULE_NAME_MAX */

/*
 * Maximum length of IPC host set name.
 */
#define	IPC_HOSTSET_NAMELEN_MAX		PFC_IPC_HOSTSET_NAMELEN_MAX

/*
 * Determine whether the length of the IPC service name is valid or not.
 */
#define	IPC_SERVICE_NAMELEN_IS_VALID(namelen)			\
	((namelen) > 0 && (namelen) <= IPC_SERVICE_NAMELEN_MAX)

/*
 * Name of IPC channel socket directory.
 */
#define	IPC_CHANNEL_DIRNAME		".channel"

/*
 * Construct path to directory under IPC working directory.
 * `name' must be a static string literal.
 */
#define	IPC_WORKDIR_PATH(name)		PFC_IPCWORKDIR "/" name

/*
 * Parameters for IPC channel.
 */
#define	IPC_CHANNEL_MAX_CLIENTS		PFC_CONST_U(64)
#define	IPC_CHANNEL_MAX_SESSIONS	PFC_CONST_U(128)
#define	IPC_CHANNEL_TIMEOUT		PFC_CONST_U(30)
#define	IPC_CHANNEL_PERMISSION		(S_IRWXU | S_IRWXG | S_IRWXO)

/*
 * Global options.
 */
#define	IPC_OPT_SNDBUF_SIZE		PFC_CONST_U(0)
#define	IPC_OPT_RCVBUF_SIZE		PFC_CONST_U(0)

/*
 * Initial value for serial ID of IPC event.
 */
#define	IPC_EVENT_SERIAL_INITIAL		(PFC_IPC_EVSERIAL_INVALID + 1)

#ifdef	PFC_HAVE_POLLRDHUP

/*
 * poll(2) event bits to detect connection reset.
 */
#define	IPC_POLL_RESET			POLLRDHUP

/*
 * poll(2) event bits to test whether connection was reset.
 */
#define	IPC_POLL_TEST_RESET		(POLLRDHUP | POLLHUP | POLLERR)

#else	/* !PFC_HAVE_POLLRDHUP */

/* Use POLLHUP instead of POLLRDHUP. */
#define	IPC_POLL_RESET			POLLHUP
#define	IPC_POLL_TEST_RESET		(POLLHUP | POLLERR)

#endif	/* PFC_HAVE_POLLRDHUP */

#ifdef	PFC_HAVE_EPOLLRDHUP

/*
 * Event poll event bits to detect connection reset.
 */
#define	IPC_EPOLLEV_RESET		EPOLLRDHUP

/*
 * Event poll event bits to test whether connection was reset.
 */
#define	IPC_EPOLLEV_TEST_RESET		(EPOLLRDHUP | EPOLLHUP | EPOLLERR)

#else	/* !PFC_HAVE_EPOLLRDHUP */

/* Use EPOLLHUP instead of EPOLLRDHUP. */
#define	IPC_EPOLLEV_RESET		EPOLLHUP
#define	IPC_EPOLLEV_TEST_RESET		(EPOLLHUP | EPOLLERR)

#endif	/* PFC_HAVE_EPOLLRDHUP */

/*
 * Event poll event bits which indicates one-shot event.
 */
#ifdef	PFC_HAVE_EPOLLONESHOT
#define	IPC_EPOLLEV_ONESHOT		EPOLLONESHOT
#else	/* !PFC_HAVE_EPOLLONESHOT */
#define	IPC_EPOLLEV_ONESHOT		(0)
#endif	/* PFC_HAVE_EPOLLONESHOT */

/*
 * I/O buffer size for session stream.
 */
#define	IPC_SESS_BUFSIZE_IN		PFC_CONST_U(256)
#define	IPC_SESS_BUFSIZE_OUT		PFC_CONST_U(256)

/*
 * Global options.
 */
typedef struct {
	uint32_t	iopt_sndbuf_size;	/* send buffer size */
	uint32_t	iopt_rcvbuf_size;	/* receive buffer size */
} ipc_option_t;

typedef const ipc_option_t	ipc_coption_t;

/*
 * Static initializer of global options.
 */
#define	IPC_OPTION_INITIALIZER					\
	{							\
		IPC_OPT_SNDBUF_SIZE,	/* iopt_sndbuf_size */	\
		IPC_OPT_RCVBUF_SIZE,	/* iopt_rcvbuf_size */	\
	}

/*
 * Hook invoked when the global configuration file is opened.
 * Zero must be returned on successful return, and error number defined in
 * errno.h on error return.
 */
typedef int	(*ipc_cfhook_t)(pfc_conf_t conf, pfc_ptr_t arg);

/*
 * Tag for protocol data unit.
 */
typedef struct {
	uint8_t		ipt_type;		/* data type */
	uint8_t		ipt_flags;		/* flag bits */
	uint16_t	ipt_pad;
	uint32_t	ipt_size;		/* size of data */
	uint32_t	ipt_off;		/* offset to data */
} ipc_pdutag_t;

/*
 * Flags for ipt_flags.
 */
#define	IPC_PDUF_NULL		PFC_CONST_U(0x01)	/* NULL pointer */

struct ipc_pdu;
typedef struct ipc_pdu		ipc_pdu_t;

struct ipc_stream;
typedef struct ipc_stream	ipc_stream_t;

struct ipc_msg;
typedef struct ipc_msg		ipc_msg_t;

struct ipc_pduops;
typedef struct ipc_pduops	ipc_pduops_t;
typedef const ipc_pduops_t	ipc_cpduops_t;

/*
 * Context used to swap bytes in PDU data.
 */
typedef struct {
	const uint8_t	*ibs_src;	/* source address */
	uint8_t		*ibs_dst;	/* destination address */
	uint8_t		ibs_flags;	/* bswap flags (IPC_SSF_) */
} ipc_bswap_t;

/*
 * PDU type characteristics and specific operations.
 */
struct ipc_pduops {
	/*
	 * Size of PDU.
	 * This field is used to determine size of IPC struct field.
	 */
	uint32_t	ipops_size;

	/*
	 * Address alignment boundary required by this PDU type.
	 * This field is used to adjust struct field alignment.
	 * So its value may differ from actual address boundary required
	 * by this type. Note that address alignment must be power of 2.
	 *
	 * Zero means that this type can't be used as struct field type.
	 */
	uint32_t	ipops_align;

	/*
	 * int
	 * ipops_write(ipc_stream_t *PFC_RESTRICT stp,
	 *	       pfc_iostream_t PFC_RESTRICT stream,
	 *	       ipc_pdu_t *PFC_RESTRICT pdu,
	 *	       ctimespec_t *PFC_RESTRICT abstime)
	 *	Write encoded PDU data to the stream specified by `stream'.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*ipops_write)(ipc_stream_t *PFC_RESTRICT stp,
			       pfc_iostream_t PFC_RESTRICT stream,
			       ipc_pdu_t *PFC_RESTRICT pdu,
			       ctimespec_t *PFC_RESTRICT abstime);

	/*
	 * void
	 * ipops_bswap(ipc_cpduops_t *PFC_RESTRICT ops,
	 *	       ipc_bswap_t *PFC_RESTRICT bsp, uint32_t nelems)
	 *	Decode serialized PDU data with byte swapping.
	 *
	 *	Source and destination address must be specified by a pointer
	 *	to ipc_deocde_t. bsp->ibs_src is a pointer to serialized PDU
	 *	data, and bsp->ibs_dst is a pointer to buffer to store the
	 *	resulting data. bsp->ibs_src and bsp->ibs_dst must be updated
	 *	to the end boundary of decoded data.
	 *
	 *	`nelems' is the number of elements to be decoded. It must be
	 *	greater than zero.
	 *
	 * Remarks:
	 *	- The caller must ensure that the buffer pointed by
	 *	  bsp->ibs_dst has enough bytes to store the PDU of this type.
	 *
	 *	- The caller must ensure that bsp->ibs_dst is properly
	 *	  aligned to the boundary required by this type, which is
	 *	  set in ops->ipops_align.
	 *
	 *	- NULL is set in ipops_bswap if the type never requires
	 *	  byte swapping.
	 */
	void	(*ipops_bswap)(ipc_cpduops_t *PFC_RESTRICT ops,
			       ipc_bswap_t *PFC_RESTRICT bsp, uint32_t nelems);

	/*
	 * int
	 * ipops_copy(ipc_pdu_t *PFC_RESTRICT pdu,
	 *	      pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags)
	 *	CopyPDU data specified by `addr' into the PDU data specified
	 *	by `pdu'.
	 *
	 *	`bflags' must be byte swapping flags.
	 *	  - IPC_SSF_BSWAP bit in `bflags' indicates an integer value
	 *	    must be byte-swapped.
	 *	  - IPC_SSF_BSWAP_FLOAT bit in `bflags' indicates a floating
	 *	    point value must be byte-swapped.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*ipops_copy)(ipc_pdu_t *PFC_RESTRICT pdu,
			      pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags);

	/*
	 * void
	 * ipops_dtor(ipc_pdu_t *pdu)
	 *	Destructor of PDU.
	 *	This method must release any resource to hold PDU data.
	 */
	void	(*ipops_dtor)(ipc_pdu_t *pdu);
};

/*
 * Index of protocol data unit.
 */
typedef struct {
	ipc_pdutag_t	ipi_tag;		/* PDU tag */
} ipc_pduidx_t;

/*
 * Protocol data unit.
 */
struct ipc_pdu {
	ipc_pdutag_t	ip_tag;			/* PDU tag */
	ipc_pdu_t	*ip_next;		/* next entry */
	ipc_cpduops_t	*ip_ops;		/* type operations */

	/* PDU data */
	union {
		int8_t		i8;		/* INT8 */
		uint8_t		u8;		/* UINT8 */
		int16_t		i16;		/* INT16 */
		uint16_t	u16;		/* UINT16 */
		int32_t		i32;		/* INT32 */
		uint32_t	u32;		/* UINT32 */
		int64_t		i64;		/* INT64 */
		uint64_t	u64;		/* UINT64 */
		float		flt;		/* FLOAT */
		double		dbl;		/* DOUBLE */
		struct in_addr	ipv4;		/* IPV4 */
		struct in6_addr	ipv6;		/* IPV6 */
		pfc_cptr_t	ptr;		/* STRING, BINARY, STRUCT */
	} ip_data;
};

#define	ip_data_INT8		ip_data.i8
#define	ip_data_UINT8		ip_data.u8
#define	ip_data_INT16		ip_data.i16
#define	ip_data_UINT16		ip_data.u16
#define	ip_data_INT32		ip_data.i32
#define	ip_data_UINT32		ip_data.u32
#define	ip_data_INT64		ip_data.i64
#define	ip_data_UINT64		ip_data.u64
#define	ip_data_FLOAT		ip_data.flt
#define	ip_data_DOUBLE		ip_data.dbl
#define	ip_data_IPV4		ip_data.ipv4
#define	ip_data_IPV6		ip_data.ipv6
#define	ip_data_POINTER		ip_data.ptr

/*
 * Output stream used to generate IPC message.
 */
struct ipc_stream {
	uint32_t	is_size;	/* total size of data */
	uint32_t	is_count;	/* number of PDUs */
	uint32_t	is_flags;	/* flags */
	ipc_pdu_t	*is_pdus;	/* list of PDU tags */
	ipc_pdu_t	**is_pdunext;	/* last entry of PDU list */
};

/*
 * Flags for is_flags.
 */
#define	IPC_STRF_BROKEN		PFC_CONST_U(0x1)	/* broken stream */
#define	IPC_STRF_FIN		PFC_CONST_U(0x2)	/* finalized */
#define	IPC_STRF_EVENT		PFC_CONST_U(0x4)	/* IPC event object */

/*
 * IPC message, which keeps received data.
 * Note that IPC message is always immutable.
 */
struct ipc_msg {
	uint32_t	im_size;	/* total size of data */
	uint32_t	im_count;	/* number of PDUs */
	uint8_t		im_bflags;	/* bswap flags (IPC_SSF_) */
	ipc_pduidx_t	*im_pdus;	/* PDU index */
	const uint8_t	*im_data;	/* received data */
};

/*
 * End boundary of PDU data.
 */
#define	IPC_MSG_DATA_LIMIT(msg)		((msg)->im_data + (msg)->im_size)

/*
 * IPC channel configuration.
 */
typedef struct {
	uint32_t	icc_max_clients;	/* max number of clients */
	uint32_t	icc_max_sessions;	/* max number of sessions */
	uint32_t	icc_timeout;		/* session timeout (sec) */
	uint32_t	icc_permission;		/* permission for socket */
} ipc_chconf_t;

/*
 * fork(2) handlers.
 */
typedef struct {
	/*
	 * void
	 * if_prepare(void)
	 * 	Called on parent process just before fork(2).
	 */
	void	(*if_prepare)(void);

	/*
	 * void
	 * if_parent(void)
	 *	Called on parent process just after fork(2).
	 */
	void	(*if_parent)(void);

	/*
	 * void
	 * if_child(void)
	 *	Called on child process just after fork(2).
	 */
	void	(*if_child)(void);
} ipc_fork_t;

/*
 * Argument for pfc_ipc_checkname().
 */
typedef enum {
	IPC_NAME_CHANNEL	= 0,		/* channel name */
	IPC_NAME_SERVICE,			/* service name */
	IPC_NAME_HOSTSET,			/* host set name */
} ipc_nmtype_t;

/*
 * Set of IPC event types on the same IPC service.
 */
typedef struct {
	pfc_refptr_t	*iem_name;		/* service name */
	pfc_ipcevmask_t	iem_mask;		/* target event mask */
	pfc_rbnode_t	iem_node;		/* Red-Black tree node */
} ipc_evmask_t;

#define	IPC_EVMASK_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_evmask_t, iem_node)

/*
 * Set of pairs of IPC event service name and event mask bits.
 */
typedef struct {
	pfc_rbtree_t	est_tree;		/* set of ipc_evmask_t */
} ipc_evset_t;

#define	IPC_EVSET_HOLD(eset)	pfc_atomic_inc_uint32(&(eset)->est_refcnt)
#define	IPC_EVSET_RELEASE(eset)						\
	do {								\
		uint32_t	__ref =					\
			pfc_atomic_dec_uint32_old(&(eset)->est_refcnt);	\
									\
		if (__ref == 1) {					\
			pfc_ipc_evset_destroy(eset);			\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

/*
 * Internal version of IPC event mask APIs.
 */
#define	IPC_EVMASK_BIT(type)		((pfc_ipcevmask_t)(1ULL << (type)))
#define	IPC_EVMASK_TEST(mask, bits)	((mask) & (bits))
#define	IPC_EVMASK_TEST_TYPE(mask, type)		\
	IPC_EVMASK_TEST((mask), IPC_EVMASK_BIT(type))

/*
 * Pseudo event type.
 */
#define	IPC_EVTYPE_NONE			((pfc_ipcevtype_t)0xff)

/*
 * printf(3) format to print out IPC event mask.
 */
#define	IPC_PFMT_EVMASK		PFC_PFMT_x64

/*
 * Buffer for IPC structure data.
 */
typedef struct {
	uint32_t	isb_refcnt;	/* reference counter */
	uint32_t	isb_size;	/* size of js_data */
	uint8_t		isb_data[0];	/* body of IPC structure */
} ipc_strbuf_t;

PFC_TYPE_SIZE_ASSERT(ipc_strbuf_t, 8);

#define	IPC_STRBUF_SIZE(size)	(sizeof(ipc_strbuf_t) + (size))

/*
 * Hold or release reference to IPC structure buffer.
 */
#define	IPC_STRBUF_HOLD(sbp)	pfc_atomic_inc_uint32(&(sbp)->isb_refcnt)
#define	IPC_STRBUF_RELEASE(sbp)						\
	do {								\
		uint32_t	__ref =					\
			pfc_atomic_dec_uint32_old(&(sbp)->isb_refcnt);	\
									\
		if (__ref == 1) {					\
			free(sbp);					\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

/*
 * Fetch an integer value from the specified address.
 */
#ifdef	PFC_UNALIGNED_ACCESS
#define	IPC_FETCH_INT(dst, srcp)		\
	do {					\
		(dst) = *(srcp);		\
	} while (0)
#else	/* !PFC_UNALIGNED_ACCESS */
#define	IPC_FETCH_INT(dst, srcp)			\
	do {						\
		memcpy(&(dst), srcp, sizeof(dst));	\
	} while (0)
#endif	/* PFC_UNALIGNED_ACCESS */

/*
 * Store an integer value to the specified address.
 */
#ifdef	PFC_UNALIGNED_ACCESS
#define	IPC_STORE_INT(dstp, src)		\
	do {					\
		*(dstp) = (src);		\
	} while (0)
#else	/* !PFC_UNALIGNED_ACCESS */
#define	IPC_STORE_INT(dstp, src)			\
	do {						\
		memcpy(dstp, &(src), sizeof(*(dstp)));	\
	} while (0)
#endif	/* PFC_UNALIGNED_ACCESS */

/*
 * Maximum length of a hex string representation of event mask.
 */
#define	IPC_EVMASK_HEXLEN	((sizeof(pfc_ipcevmask_t) << 1) + 2)

#ifdef	_PFC_LIBPFC_IPC_BUILD

/*
 * Internal logging functions.
 */

extern volatile uint32_t	ipc_log_enabled;

#ifdef	__PFC_LOG_GNUC

#define	IPC_LOG_ERROR(format, ...)				\
	if (ipc_log_enabled) {					\
		pfc_log_error((format), ##__VA_ARGS__);		\
	}

#define	IPC_LOG_WARN(format, ...)				\
	if (ipc_log_enabled) {					\
		pfc_log_warn((format), ##__VA_ARGS__);		\
	}

#define	IPC_LOG_INFO(format, ...)				\
	if (ipc_log_enabled) {					\
		pfc_log_info((format), ##__VA_ARGS__);		\
	}

#ifdef	PFC_VERBOSE_DEBUG
#define	IPC_LOG_VERBOSE(format, ...)				\
	if (ipc_log_enabled) {					\
		pfc_log_verbose((format), ##__VA_ARGS__);	\
	}
#else	/* !PFC_VERBOSE_DEBUG */
#define	IPC_LOG_VERBOSE(format, ...)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#else	/* !__PFC_LOG_GNUC */

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPC_LOG_ERROR(const char *format, ...)
{
	if (ipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPC_LOG_WARN(const char *format, ...)
{
	if (ipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_warn_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPC_LOG_INFO(const char *format, ...)
{
	if (ipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_info_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPC_LOG_VERBOSE(const char *format, ...)
{
#ifdef	PFC_VERBOSE_DEBUG
	if (ipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_verbose_v(format, ap);
		va_end(ap);
	}
#endif	/* PFC_VERBOSE_DEBUG */
}

#endif	/* __PFC_LOG_GNUC */

#define	IPC_LOG_ERROR_V(format, ap)					\
	if (ipc_log_enabled) {						\
		pfc_log_error_v((format), (ap));			\
	}

#endif	/* _PFC_LIBPFC_IPC_BUILD */

/*
 * Prototypes.
 */
extern void	pfc_ipcstream_destroy(ipc_stream_t *stp);
extern void	pfc_ipcstream_reset(ipc_stream_t *stp, uint32_t flags);

extern int	pfc_ipcstream_add_int8(ipc_stream_t *stp, int8_t data);
extern int	pfc_ipcstream_add_uint8(ipc_stream_t *stp, uint8_t data);
extern int	pfc_ipcstream_add_int16(ipc_stream_t *stp, int16_t data);
extern int	pfc_ipcstream_add_uint16(ipc_stream_t *stp, uint16_t data);
extern int	pfc_ipcstream_add_int32(ipc_stream_t *stp, int32_t data);
extern int	pfc_ipcstream_add_uint32(ipc_stream_t *stp, uint32_t data);
extern int	pfc_ipcstream_add_int64(ipc_stream_t *stp, int64_t data);
extern int	pfc_ipcstream_add_uint64(ipc_stream_t *stp, uint64_t data);
extern int	pfc_ipcstream_add_float(ipc_stream_t *stp, float data);
extern int	pfc_ipcstream_add_double(ipc_stream_t *stp, double data);
extern int	pfc_ipcstream_add_ipv4(ipc_stream_t *PFC_RESTRICT stp,
				       struct in_addr *PFC_RESTRICT data);
extern int	pfc_ipcstream_add_ipv6(ipc_stream_t *PFC_RESTRICT stp,
				       struct in6_addr *PFC_RESTRICT data);
extern int	pfc_ipcstream_add_string(ipc_stream_t *PFC_RESTRICT stp,
					 const char *PFC_RESTRICT data);
extern int	pfc_ipcstream_add_binary(ipc_stream_t *PFC_RESTRICT stp,
					 const uint8_t *PFC_RESTRICT data,
					 uint32_t length);
extern int	pfc_ipcstream_add_null(ipc_stream_t *stp);
extern int	pfc_ipcstream_add_struct(ipc_stream_t *PFC_RESTRICT stp,
					 const uint8_t *PFC_RESTRICT data,
					 uint32_t length,
					 const char *PFC_RESTRICT stname,
					 const char *PFC_RESTRICT sig);
extern int	pfc_ipcstream_add_known_struct(ipc_stream_t *PFC_RESTRICT stp,
					       const uint8_t *PFC_RESTRICT data,
					       ipc_cstrinfo_t *PFC_RESTRICT
					       sip);
extern int	pfc_ipcstream_copymsg(ipc_stream_t *PFC_RESTRICT stp,
				      ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t begin, uint32_t end);

extern int	pfc_ipcstream_send(ipc_sess_t *PFC_RESTRICT sess,
				   ipc_stream_t *PFC_RESTRICT stp,
				   ctimespec_t *PFC_RESTRICT abstime);

extern int	pfc_ipcmsg_get_int8(ipc_msg_t *PFC_RESTRICT msg,
				    uint32_t index,
				    int8_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_uint8(ipc_msg_t *PFC_RESTRICT msg,
				     uint32_t index,
				     uint8_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_int16(ipc_msg_t *PFC_RESTRICT msg,
				     uint32_t index,
				     int16_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_uint16(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      uint16_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_int32(ipc_msg_t *PFC_RESTRICT msg,
				     uint32_t index,
				     int32_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_uint32(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      uint32_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_int64(ipc_msg_t *PFC_RESTRICT msg,
				     uint32_t index,
				     int64_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_uint64(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      uint64_t *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_float(ipc_msg_t *PFC_RESTRICT msg,
				     uint32_t index,
				     float *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_double(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      double *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_ipv4(ipc_msg_t *PFC_RESTRICT msg,
				    uint32_t index,
				    struct in_addr *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_ipv6(ipc_msg_t *PFC_RESTRICT msg,
				    uint32_t index,
				    struct in6_addr *PFC_RESTRICT datap);
extern int	pfc_ipcmsg_get_string(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      const char **PFC_RESTRICT datapp);
extern int	pfc_ipcmsg_get_binary(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      const uint8_t **PFC_RESTRICT datapp,
				      uint32_t *PFC_RESTRICT sizep);
extern int	pfc_ipcmsg_get_struct(ipc_msg_t *PFC_RESTRICT msg,
				      uint32_t index,
				      uint8_t *PFC_RESTRICT datap,
				      uint32_t length,
				      const char *PFC_RESTRICT stname,
				      const char *PFC_RESTRICT sig);
extern int	pfc_ipcmsg_fetch_struct(ipc_msg_t *PFC_RESTRICT msg,
					uint32_t index,
					ipc_cstrinfo_t **PFC_RESTRICT sipp,
					ipc_strbuf_t **PFC_RESTRICT sbpp,
					pfc_bool_t need_fields);

extern int	pfc_ipcmsg_get_struct_name(ipc_msg_t *PFC_RESTRICT msg,
					   uint32_t index,
					   const char **PFC_RESTRICT namepp);
extern void	pfc_ipcmsg_destroy(ipc_msg_t *msg);
extern void	pfc_ipcmsg_reset(ipc_msg_t *msg);
extern int	pfc_ipcmsg_recv(ipc_sess_t *PFC_RESTRICT sess,
				ipc_msg_t *PFC_RESTRICT msg,
				ctimespec_t *PFC_RESTRICT abstime);
extern int	pfc_ipcmsg_init_uint32(ipc_msg_t *PFC_RESTRICT msg,
				       uint32_t data);

extern void	pfc_ipc_client_init(const ipc_fork_t *handler);
extern void	pfc_ipc_enable_log(pfc_bool_t enable);
extern int	pfc_ipc_sess_init(ipc_sess_t *PFC_RESTRICT sess,
				  const pfc_hostaddr_t *PFC_RESTRICT haddr);
extern int	pfc_ipc_iostream_create(ipc_sess_t *PFC_RESTRICT sess,
					int sock, int canceller,
					ipc_coption_t *PFC_RESTRICT opts);
extern int	pfc_ipc_read(pfc_iostream_t PFC_RESTRICT stream,
			     pfc_ptr_t PFC_RESTRICT buf, uint32_t size,
			     ctimespec_t *PFC_RESTRICT abstime);
extern int	pfc_ipc_write(pfc_iostream_t PFC_RESTRICT stream,
			      pfc_cptr_t PFC_RESTRICT buf, uint32_t size,
			      pfc_bool_t do_flush,
			      ctimespec_t *PFC_RESTRICT abstime);
extern int	pfc_ipc_checkname(ipc_nmtype_t type, const char *name);
extern int	pfc_ipc_checkshutfd(int fd);
extern int	pfc_ipc_thread_create(void *(*func)(void *), void *arg);

extern int	pfc_ipc_conf_dirinit(void);
extern int	pfc_ipc_conf_getpath(const char *PFC_RESTRICT channel,
				     pfc_refptr_t **PFC_RESTRICT pathpp);
extern int	pfc_ipc_conf_getsrvconf(const char *PFC_RESTRICT channel,
					ipc_chconf_t *PFC_RESTRICT chp);
extern int	pfc_ipc_conf_getcliconf(const char *PFC_RESTRICT channel,
					ipc_chconf_t *PFC_RESTRICT chp);

extern int	pfc_ipc_struct_load(void);

extern void	pfc_ipc_evset_init(ipc_evset_t *eset);
extern void	pfc_ipc_evset_destroy(ipc_evset_t *eset);
extern int	pfc_ipc_evset_copy(ipc_evset_t *PFC_RESTRICT dst,
				   ipc_evset_t *PFC_RESTRICT src);
extern int	pfc_ipc_evset_merge(ipc_evset_t *PFC_RESTRICT dst,
				    ipc_evset_t *PFC_RESTRICT src);
extern int	pfc_ipc_evset_mergenew(ipc_evset_t *PFC_RESTRICT dst,
				       ipc_evset_t *PFC_RESTRICT eset,
				       ipc_evset_t *PFC_RESTRICT src);
extern int	pfc_ipc_evset_add(ipc_evset_t *PFC_RESTRICT eset,
				  const char *PFC_RESTRICT name,
				  const pfc_ipcevmask_t *PFC_RESTRICT mask);
extern int	pfc_ipc_evset_refadd(ipc_evset_t *PFC_RESTRICT eset,
					pfc_refptr_t *PFC_RESTRICT rname,
					pfc_ipcevmask_t *PFC_RESTRICT mask);
extern void	pfc_ipc_evset_remove(ipc_evset_t *PFC_RESTRICT eset,
				     const char *PFC_RESTRICT name,
				     pfc_ipcevmask_t *PFC_RESTRICT mask);

extern ipc_evmask_t	*pfc_ipc_evset_lookup(ipc_evset_t *PFC_RESTRICT eset,
					      const char *PFC_RESTRICT name,
					      pfc_ipcevtype_t type);

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_ipc_evset_isempty(ipc_evset_t *eset)
 *	Return PFC_TRUE if no event is contained in the given event set.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_ipc_evset_isempty(ipc_evset_t *eset)
{
	return pfc_rbtree_isempty(&eset->est_tree);
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_ipc_evset_contains(ipc_evset_t *PFC_RESTRICT eset,
 *			  const char *PFC_RESTRICT name, pfc_ipcevtype_t type)
 *	Return PFC_TRUE if the event set `eset' contains a pair of IPC service
 *	name `name' and IPC event type `type'.
 *
 *	Note that an empty event set is treated as wildcard.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_ipc_evset_contains(ipc_evset_t *PFC_RESTRICT eset,
		       const char *PFC_RESTRICT name, pfc_ipcevtype_t type)
{
	if (pfc_ipc_evset_isempty(eset)) {
		return PFC_TRUE;
	}

	if (pfc_ipc_evset_lookup(eset, name, type) != NULL) {
		return PFC_TRUE;
	}

	return PFC_FALSE;
}

#ifdef	_PFC_LIBPFC_IPC_BUILD

/*
 * Context to parse sections in STRUCT PDU.
 */
typedef struct {
	const char	*isp_name;	/* pointer to struct name */
	const char	*isp_sig;	/* pointer to layout signature */
	const uint8_t	*isp_data;	/* base address of serialized data */
	uint32_t	isp_size;	/* size of struct data */
} ipc_strpdu_t;

/*
 * Internal prototypes.
 */
extern int	pfc_ipcmsg_get_strinfo(ipc_msg_t *PFC_RESTRICT msg,
				       ipc_pduidx_t *PFC_RESTRICT pdu,
				       ipc_strpdu_t *PFC_RESTRICT strpdu,
				       ipc_cstrinfo_t **PFC_RESTRICT sipp,
				       pfc_bool_t need_fields);

extern ipc_pdu_t	*pfc_ipc_pdu_alloc(pfc_ipctype_t type);
extern ipc_cpduops_t	*pfc_ipc_pdu_getops(pfc_ipctype_t type);
extern void		pfc_ipc_pdu_setstruct(ipc_pduops_t *ops);

/*
 * Close the given file descriptor.
 */
#define	PFC_IPC_CLOSE(fd)						\
	do {								\
		if (PFC_EXPECT_FALSE(close(fd) != 0)) {			\
			IPC_LOG_ERROR("%s:%u: "				\
				      "Failed to close FD(%d): %s",	\
				      __FILE__, __LINE__, fd,		\
				      strerror(errno));			\
		}							\
	} while (0)

#endif	/* _PFC_LIBPFC_IPC_BUILD */

/*
 * Inline functions.
 */

/*
 * static inline void
 * pfc_ipcstream_init(ipc_stream_t *stp, uint32_t flags)
 *	Initialize IPC stream instance, which is used to generate IPC message
 *	to be sent.
 *
 *	`flags' must be an initial value for is_flags.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcstream_init(ipc_stream_t *stp, uint32_t flags)
{
	stp->is_size = 0;
	stp->is_count = 0;
	stp->is_flags = flags;
	stp->is_pdus = NULL;
	stp->is_pdunext = &stp->is_pdus;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcstream_assert(ipc_stream_t *stp, uint32_t flags)
 *	Ensure that the IPC stream is initialized correctly.
 *	This function is used only for debugging.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcstream_assert(ipc_stream_t *stp, uint32_t flags)
{
	PFC_ASSERT(stp->is_size == 0);
	PFC_ASSERT(stp->is_count == 0);
	PFC_ASSERT(stp->is_flags == 0);
	PFC_ASSERT(stp->is_pdus == NULL);
	PFC_ASSERT(stp->is_pdunext == &stp->is_pdus);
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcstream_is_broken(ipc_stream_t *stp)
 *	Return PFC_TRUE if the given IPC stream is broken.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_ipcstream_is_broken(ipc_stream_t *stp)
{
	return (stp->is_flags & IPC_STRF_BROKEN) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcmsg_setbflags(ipc_msg_t *msg, uint8_t bflags)
 *	Set byte-swapping flags to the given IPC message instance.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcmsg_setbflags(ipc_msg_t *msg, uint8_t bflags)
{
	msg->im_bflags = bflags;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcmsg_init(ipc_msg_t *msg, uint8_t bflags)
 *	Initialize IPC message instance.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcmsg_init(ipc_msg_t *msg, uint8_t bflags)
{
	msg->im_size = 0;
	msg->im_count = 0;
	msg->im_pdus = NULL;
	msg->im_data = NULL;
	pfc_ipcmsg_setbflags(msg, bflags);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcmsg_assert(ipc_msg_t *msg, uint8_t bflags)
 *	Ensure that the IPC message instance is initialized correctly.
 *	This function is used only for debugging.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcmsg_assert(ipc_msg_t *msg, uint8_t bflags)
{
	PFC_ASSERT(msg->im_size == 0);
	PFC_ASSERT(msg->im_count == 0);
	PFC_ASSERT(msg->im_bflags == bflags);
	PFC_ASSERT(msg->im_pdus == NULL);
	PFC_ASSERT(msg->im_data == NULL);
}

/*
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcmsg_getcount(ipc_msg_t *msg)
 *	Return the number of PDUs in the given IPC message.
 */
static inline uint32_t PFC_FATTR_ALWAYS_INLINE
pfc_ipcmsg_getcount(ipc_msg_t *msg)
{
	return msg->im_count;
}

/*
 * static inline ipc_pduidx_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_ipcmsg_getpdu(ipc_msg_t *msg, uint32_t index)
 *	Get PDU index of the PDU in the IPC message at the specified index.
 *
 * Calling/Exit State:
 *	NULL is returned if the specified index is invalid.
 *	Otherwise a non-NULL pointer to ipc_pduidx_t associated with the
 *	PDU specified by `index' is returned.
 */
static inline ipc_pduidx_t PFC_FATTR_ALWAYS_INLINE *
pfc_ipcmsg_getpdu(ipc_msg_t *msg, uint32_t index)
{
	if (PFC_EXPECT_FALSE(index >= msg->im_count)) {
		return NULL;
	}

	return msg->im_pdus + index;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcmsg_gettype(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *		      pfc_ipctype_t *PFC_RESTRICT typep)
 *	Get data type of the PDU in the IPC message at the specified index.
 *
 * Calling/Exit State:
 *	Upon successful completion, PDU data type is stored to `*typep',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcmsg_gettype(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
		   pfc_ipctype_t *PFC_RESTRICT typep)
{
	ipc_pduidx_t	*pdu = pfc_ipcmsg_getpdu(msg, index);

	if (PFC_EXPECT_FALSE(pdu == NULL)) {
		return EINVAL;
	}

	*typep = (pfc_ipctype_t)pdu->ipi_tag.ipt_type;

	return 0;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipc_bswap_align(ipc_bswap_t *bsp, uint32_t align)
 *	Round up struct source and destination addresses in `bsp' to the
 *	boundary specified by `align'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipc_bswap_align(ipc_bswap_t *bsp, uint32_t align)
{
	uint8_t		*dst;

	dst = (uint8_t *)PFC_POW2_ROUNDUP((uintptr_t)bsp->ibs_dst, align);
	if (dst != bsp->ibs_dst) {
		pfc_ptrdiff_t	diff = dst - bsp->ibs_dst;

		PFC_ASSERT(diff > 0);
		bsp->ibs_dst = dst;
		bsp->ibs_src += diff;
	}
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipc_check_service_name(const char *name)
 *	Ensure that the IPC service name is valid.
 *
 * Calling/Exit State:
 *	Zero is returned if the given service name is valid.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipc_check_service_name(const char *name)
{
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	return pfc_ipc_checkname(IPC_NAME_SERVICE, name);
}

/*
 * static inline ipc_strbuf_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_ipc_strbuf_alloc(uint32_t size)
 *	Allocate a buffer for IPC structure data.
 *	`size' must be the size of the target IPC structure.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_strbuf_t is
 *	returned. It must be released by IPC_STRBUF_RELEASE().
 *
 *	Otherwise NULL is returned.
 */
static inline ipc_strbuf_t PFC_FATTR_ALWAYS_INLINE *
pfc_ipc_strbuf_alloc(uint32_t size)
{
	ipc_strbuf_t	*sbp;

	sbp = (ipc_strbuf_t *)malloc(IPC_STRBUF_SIZE(size));
	if (PFC_EXPECT_TRUE(sbp != NULL)) {
		sbp->isb_refcnt = 1;
		sbp->isb_size = size;
		memset(sbp->isb_data, 0, size);
	}

	return sbp;
}

/*
 * static inline ipc_strbuf_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_ipc_strbuf_copy(const uint8_t *addr, uint32_t size)
 *	Allocate a buffer for IPC structure data, and copy the specified
 *	buffer into a new buffer.
 *
 *	`addr' must points a buffer to be copied, and `size' must be the
 *	size of a buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_strbuf_t is
 *	returned. It must be released by IPC_STRBUF_RELEASE().
 *
 *	Otherwise NULL is returned.
 */
static inline ipc_strbuf_t PFC_FATTR_ALWAYS_INLINE *
pfc_ipc_strbuf_copy(const uint8_t *addr, uint32_t size)
{
	ipc_strbuf_t	*sbp;

	sbp = (ipc_strbuf_t *)malloc(IPC_STRBUF_SIZE(size));
	if (PFC_EXPECT_TRUE(sbp != NULL)) {
		sbp->isb_refcnt = 1;
		sbp->isb_size = size;
		memcpy(sbp->isb_data, addr, size);
	}

	return sbp;
}

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_IPC_IPC_IMPL_H */
