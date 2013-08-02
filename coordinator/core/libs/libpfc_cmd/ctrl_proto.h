/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_CMD_CTRL_PROTO_H
#define	_PFC_LIBPFC_CMD_CTRL_PROTO_H

/*
 * Definitions for PFC daemon control protocol.
 */

#include <stdarg.h>
#include <string.h>
#include <pfc/base.h>
#include <pfc/iostream.h>
#include <pfc/clock.h>
#include <pfc/debug.h>

PFC_C_BEGIN_DECL

/*
 * Protocol version.
 */
#define	CTRL_PROTO_VERSION	1

/*
 * Byte order.
 */
#define	CTRL_ORDER_LITTLE	1	/* little endian */
#define	CTRL_ORDER_BIG		2	/* big endian */

#ifdef	PFC_LITTLE_ENDIAN
#define	CTRL_ORDER_NATIVE	CTRL_ORDER_LITTLE
#else	/* !PFC_LITTLE_ENDIAN */
#define	CTRL_ORDER_NATIVE	CTRL_ORDER_BIG
#endif	/* PFC_LITTLE_ENDIAN */

/*
 * Each protocol data is encoded in TLV format, except for NULL.
 *
 * - Type is 16-bit unsigned integer.
 * - Length is 32-bit unsigned integer.
 */
typedef uint16_t	ctrl_pdtype_t;
typedef uint32_t	ctrl_pdsize_t;

/*
 * Data type in protocol data.
 */
#define	CTRL_PDTYPE_NULL		0x0000U		/* NULL */
#define	CTRL_PDTYPE_INT32		0x0001U		/* 32-bit integer */
#define	CTRL_PDTYPE_TEXT		0x0002U		/* text */

/*
 * Abstract data for protocol data.
 */
typedef struct {
	ctrl_pdtype_t	cpd_type;	/* data type */
	ctrl_pdsize_t	cpd_size;	/* data size */
	pfc_ptr_t	cpd_value;	/* value */
} cproto_data_t;

#define	CTRL_PROTO_MAGIC_SIZE		PFC_CONST_U(4)

/*
 * Handshake message, which is sent at the beginning of control
 * protocol session.
 */
typedef struct {
	/* First 4 octets are magic number. */
	uint8_t		cph_magic[CTRL_PROTO_MAGIC_SIZE];
	uint8_t		cph_version;		/* protocol version */
	uint8_t		cph_order;		/* byte order */
	uint8_t		cph_pad[2];
} PFC_ATTR_PACKED cproto_hshake_t;

/*
 * Control command is a message to control PFC daemon.
 * It starts with a command type which is represented by 2 octets.
 * Argument data sequence may follow the command type. Its format is defined
 * by each command.
 */
typedef uint16_t	ctrl_cmdtype_t;

/*
 * Supported commands.
 */
#define	CTRL_CMDTYPE_NOP		0U	/* do nothing */
#define	CTRL_CMDTYPE_QUIT		1U	/* quit session */
#define	CTRL_CMDTYPE_LOGLEVEL		2U	/* change log level */
#define	CTRL_CMDTYPE_MODLIST		3U	/* get module list */
#define	CTRL_CMDTYPE_EVENT		4U	/* send module-specific event */

/* Pseudo command type which means EOF has been detected. */
#define	CTRL_CMDTYPE_EOF		0xffffU

/*
 * Pseudo logging levels used by LOGLEVEL command.
 */
#define	CTRL_LOGLVL_NONE	(-1)	/* No logging level is specified. */
#define	CTRL_LOGLVL_RESET	(-2)	/* Reset to the initial level */
#define	CTRL_LOGLVL_MODRESET	(-3)	/* Reset all per-module level */
#define	CTRL_LOGLVL_MODLIST	(-4)	/* List per-module level */

/*
 * Response of command start with an octet. 0 means success, and 1 means
 * error.
 *
 * - Succeeded response may contains additional protocol data, which is
 *   determined by each command. If no additional data, no octet is sent.
 * - Only one TEXT protocol data must follow error response octet, which
 *   contains an error message.
 * - NOP and QUIT don't return any response.
 */
typedef uint8_t		ctrl_cmdresp_t;

#define	CTRL_RESP_OK			0x00U		/* succeeded */
#define	CTRL_RESP_FAIL			0x01U		/* failed */

/*
 * Maximum length of error message in FAIL response.
 */
#define	CTRL_RESP_ERRMSG_MAX		127U

/*
 * Prototype of error message handler for control protocol session.
 */
typedef void	(*cproto_err_t)(int err, const char *fmt, va_list ap);

/*
 * Control protocol session data.
 *
 * Remarks:
 *	Control protocol session is not thread safe.
 *	It must be grabbed by only one thread.
 */
typedef struct {
	pfc_iostream_t	cps_stream;	/* session stream */
	pfc_timespec_t	cps_timeout;	/* I/O timeout */
	pfc_bool_t	cps_bswap;	/* true if byte swap is needed */
	cproto_err_t	cps_error;	/* error message handler */
} cproto_sess_t;

/*
 * Byte swap macros.
 */
#define	__CPROTO_BSWAP(sess, x, bits)				\
	do {							\
		if ((sess)->cps_bswap) {			\
			(x) = PFC_BSWAP_##bits(x);		\
		}						\
	} while (0)
#define	CPROTO_BSWAP_16(sess, x)	__CPROTO_BSWAP(sess, x, 16)
#define	CPROTO_BSWAP_32(sess, x)	__CPROTO_BSWAP(sess, x, 32)
#define	CPROTO_BSWAP_64(sess, x)	__CPROTO_BSWAP(sess, x, 64)

/*
 * Prototypes.
 */
extern int	cproto_data_read(cproto_sess_t *PFC_RESTRICT sess,
				 cproto_data_t *PFC_RESTRICT datap);
extern void	cproto_data_free(cproto_data_t *datap);
extern int	cproto_data_write_null(cproto_sess_t *sess);
extern int	cproto_data_write_int32(cproto_sess_t *sess, int32_t value);
extern int	cproto_data_write_text_len(cproto_sess_t *PFC_RESTRICT sess,
					   const char *PFC_RESTRICT value,
					   size_t len);
extern int	cproto_handshake_read(cproto_sess_t *sess);
extern int	cproto_handshake_write(cproto_sess_t *sess);
extern int	cproto_cmd_read(cproto_sess_t *PFC_RESTRICT sess,
				ctrl_cmdtype_t *PFC_RESTRICT cmdp);
extern int	cproto_cmd_write(cproto_sess_t *sess, ctrl_cmdtype_t cmd);
extern int	cproto_resp_read(cproto_sess_t *PFC_RESTRICT sess,
				 ctrl_cmdresp_t *PFC_RESTRICT respp);
extern int	cproto_resp_write(cproto_sess_t *sess, ctrl_cmdresp_t resp);

extern uint32_t	cproto_timeout_value(void);

/*
 * static inline int
 * cproto_data_write_text(cproto_sess_t *PFC_RESTRICT sess,
 *			  const char *PFC_RESTRICT value)
 *	Write text to the session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
static inline int
cproto_data_write_text(cproto_sess_t *PFC_RESTRICT sess,
		       const char *PFC_RESTRICT value)
{
	return cproto_data_write_text_len(sess, value, strlen(value));
}

/*
 * static inline int32_t
 * cproto_data_int32(cproto_data_t *datap)
 *	Read protocol data as signed 32-bit integer.
 */
static inline int32_t
cproto_data_int32(cproto_data_t *datap)
{
	PFC_ASSERT(datap->cpd_type == CTRL_PDTYPE_INT32);

	return (int32_t)(uintptr_t)datap->cpd_value;
}

/*
 * static inline uint32_t
 * cproto_data_uint32(cproto_data_t *datap)
 *	Read protocol data as unsigned 32-bit integer.
 */
static inline uint32_t
cproto_data_uint32(cproto_data_t *datap)
{
	PFC_ASSERT(datap->cpd_type == CTRL_PDTYPE_INT32);

	return (uint32_t)(uintptr_t)datap->cpd_value;
}

/*
 * static inline const char *
 * cproto_data_text(cproto_data_t *datap)
 *	Read protocol data as text.
 */
static inline const char *
cproto_data_text(cproto_data_t *datap)
{
	PFC_ASSERT(datap->cpd_type == CTRL_PDTYPE_TEXT);

	return (const char *)datap->cpd_value;
}

/*
 * static inline void
 * cproto_sess_init(cproto_sess_t *PFC_RESTRICT sess,
 *		    pfc_iostream_t PFC_RESTRICT stream, cproto_err_t errfunc,
 *		    const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Initialize protocol session.
 *	`timeout' is a protocol timeout. If NULL is specified, the default
 *	value is used, which is specified by options.ctrl_timeout in pfcd.conf.
 */
static inline void
cproto_sess_init(cproto_sess_t *PFC_RESTRICT sess,
		 pfc_iostream_t PFC_RESTRICT stream, cproto_err_t errfunc,
		 const pfc_timespec_t *PFC_RESTRICT timeout)
{
	sess->cps_stream = stream;
	if (timeout != NULL) {
		sess->cps_timeout.tv_sec = timeout->tv_sec;
		sess->cps_timeout.tv_nsec = timeout->tv_nsec;
	}
	else {
		sess->cps_timeout.tv_sec = cproto_timeout_value();
		sess->cps_timeout.tv_nsec = 0;
	}
	sess->cps_error = errfunc;

	/*
	 * Set PFC_FALSE to cps_bswap as initial value.
	 * It will be initialized by the call of cproto_handshake_read().
	 */
	sess->cps_bswap = PFC_FALSE;
}

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_CMD_CTRL_PROTO_H */
