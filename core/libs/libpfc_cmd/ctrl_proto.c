/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ctrl_proto.c - Utilities for PFC daemon control protocol.
 */

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pfc/conf.h>
#include "cmd_impl.h"
#include "ctrl_proto.h"

/*
 * Internal limit of text length.
 */
#define	CPROTO_TEXT_MAXLEN		0x10000U

/*
 * Magic number of PFC daemon control protocol, which is embedded into the
 * handshake message.
 */
static const uint8_t	cproto_magic[CTRL_PROTO_MAGIC_SIZE] = {
	0xf7, 'C', 't', 'L',
};

/*
 * Default timeout for control protocol session.
 */
#define	CPROTO_CTRL_TIMEOUT_DEFAULT	5U

/*
 * Internal prototypes.
 */
static int	cproto_read_uint16(cproto_sess_t *PFC_RESTRICT sess,
				   uint16_t *PFC_RESTRICT valuep,
				   const char *PFC_RESTRICT label);
static int	cproto_read_uint32(cproto_sess_t *PFC_RESTRICT sess,
				   uint32_t *PFC_RESTRICT valuep,
				   const char *PFC_RESTRICT label);
static int	cproto_write_uint16(cproto_sess_t *PFC_RESTRICT sess,
				    uint16_t value,
				    const char *PFC_RESTRICT label);
static int	cproto_write_uint32(cproto_sess_t *PFC_RESTRICT sess,
				    uint32_t value,
				    const char *PFC_RESTRICT label);
static int	cproto_parse_int32(cproto_sess_t *PFC_RESTRICT sess,
				   cproto_data_t *PFC_RESTRICT datap);
static int	cproto_parse_text(cproto_sess_t *PFC_RESTRICT sess,
				  cproto_data_t *PFC_RESTRICT datap);
static void	cproto_data_free_value(cproto_data_t *datap);
static void	cproto_error(cproto_sess_t *PFC_RESTRICT sess, int err,
			     const char *PFC_RESTRICT fmt, ...);

#define	cproto_free_text	cproto_data_free_value

/*
 * Operations for protocol data.
 * Data type is used as array index.
 */
typedef struct {
	/*
	 * Parse received protocol data.
	 * Parsed object is set to datap->cpd_value.
	 */
	int	(*cop_parse)(cproto_sess_t *PFC_RESTRICT sess,
			     cproto_data_t *PFC_RESTRICT datap);

	/*
	 * Release all resources grabbed by protocol data.
	 */
	void	(*cop_free)(cproto_data_t *datap);
} cproto_pdops_t;

#define	CPROTO_OPS_DECL(name)				\
	{						\
		.cop_parse	= cproto_parse_##name,	\
		.cop_free	= cproto_free_##name,	\
	}

#define	CPROTO_OPS_PARSE_DECL(name)			\
	{						\
		.cop_parse	= cproto_parse_##name,	\
	}

static const cproto_pdops_t	cproto_data_ops[] = {
	{ NULL, NULL },				/* CTRL_PDTYPE_NULL */
	CPROTO_OPS_PARSE_DECL(int32),		/* CTRL_PDTYPE_INT32 */
	CPROTO_OPS_DECL(text),			/* CTRL_PDTYPE_TEXT */
};

/*
 * int
 * cproto_data_read(cproto_sess_t *PFC_RESTRICT sess,
 *		    cproto_data_t *PFC_RESTRICT datap)
 *	Read a protocol data from the control protocol session stream.
 *
 *	Protocol data is stored to datap.
 *	- cpd_size keeps size of data.
 *	- cpd_value keeps protocol data.
 *	  cproto_data_xxx() functions may help you to access protocol data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Returned data may grab some resources.
 *	It is up to the caller to free resources by calling cproto_data_free().
 */
int
cproto_data_read(cproto_sess_t *PFC_RESTRICT sess,
		 cproto_data_t *PFC_RESTRICT datap)
{
	const cproto_pdops_t	*ops;
	ctrl_pdtype_t	type;
	int	err;

	/* Read data type. */
	err = cproto_read_uint16(sess, &datap->cpd_type, "data type");
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if ((type = datap->cpd_type) == CTRL_PDTYPE_NULL) {
		/* NULL has no length and data. */
		datap->cpd_size = 0;
		datap->cpd_value = NULL;

		return 0;
	}

	if (PFC_EXPECT_FALSE(type >= PFC_ARRAY_CAPACITY(cproto_data_ops))) {
		/* Unexpected data type. */
		cproto_error(sess, EPROTO, "cproto: Unexpected protocol "
			     "data type: %u", type);

		return EPROTO;
	}

	ops = &cproto_data_ops[type];

	/* Read data size */
	err = cproto_read_uint32(sess, &datap->cpd_size, "data size");
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Parse protocol data. */
	return ops->cop_parse(sess, datap);
}

/*
 * void
 * cproto_data_free(cproto_data_t *datap)
 *	Free resources grabbed by the specified protocol data.
 *	The caller must specify protocol data constructed by the call of
 *	cproto_data_read().
 */
void
cproto_data_free(cproto_data_t *datap)
{
	ctrl_pdtype_t	type = datap->cpd_type;

	if (type != CTRL_PDTYPE_NULL) {
		const cproto_pdops_t	*ops;

		PFC_ASSERT(type < PFC_ARRAY_CAPACITY(cproto_data_ops));
		ops = &cproto_data_ops[type];
		if (ops->cop_free != NULL) {
			ops->cop_free(datap);
		}
	}
}

/*
 * int
 * cproto_data_write_null(cproto_sess_t *sess)
 *	Write NULL data to the session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
int
cproto_data_write_null(cproto_sess_t *sess)
{
	return cproto_write_uint16(sess, CTRL_PDTYPE_NULL, "NULL type");
}

/*
 * int
 * cproto_data_write_int32(cproto_sess_t *sess, int32_t value)
 *	Write 32-bit integer to the session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
int
cproto_data_write_int32(cproto_sess_t *sess, int32_t value)
{
	int	err;

	err = cproto_write_uint16(sess, CTRL_PDTYPE_INT32, "INT32 type");
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = cproto_write_uint32(sess, sizeof(int32_t), "INT32 size");
		if (PFC_EXPECT_TRUE(err == 0)) {
			err = cproto_write_uint32(sess, value, "INT32");
		}
	}

	return err;
}

/*
 * int
 * cproto_data_write_text_len(cproto_sess_t *PFC_RESTRICT sess,
 *			      const char *PFC_RESTRICT value, size_t len)
 *	Write text to the session stream.
 *
 *	The caller must specify length of a string specified by `value'
 *	to `len'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
int
cproto_data_write_text_len(cproto_sess_t *PFC_RESTRICT sess,
			   const char *PFC_RESTRICT value, size_t len)
{
	int	err;
	size_t	size;

	if (PFC_EXPECT_FALSE(len > CPROTO_TEXT_MAXLEN)) {
		return E2BIG;
	}

	err = cproto_write_uint16(sess, CTRL_PDTYPE_TEXT, "TEXT type");
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = cproto_write_uint32(sess, (uint32_t)len, "TEXT size");
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	size = len;
	err = pfc_iostream_write(sess->cps_stream, value, &size,
				 &sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		PFC_ASSERT(size == len);
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session write error: text: %s",
			     strerror(err));
	}

	return err;
}

/*
 * int
 * cproto_handshake_read(cproto_sess_t *sess)
 *	Read handshake message from the specified control protocol session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
cproto_handshake_read(cproto_sess_t *sess)
{
	pfc_iostream_t	stream = sess->cps_stream;
	cproto_hshake_t	msg;
	uint32_t	i;
	size_t		size = sizeof(msg);
	int		err;

	/* Read handshake message. */
	err = pfc_iostream_read(stream, &msg, &size, &sess->cps_timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		cproto_error(sess, err,
			     "cproto: Failed to read handshake: %s",
			     strerror(err));

		return err;
	}

	if (PFC_EXPECT_FALSE(size != sizeof(msg))) {
		/* This should be an unexpected EOF. */
		cproto_error(sess, EPROTO,
			     "cproto: Unexpected handshake size: %"
			     PFC_PFMT_SIZE_T, size);

		return EPROTO;
	}

	/* Check magic number. */
	for (i = 0; i < CTRL_PROTO_MAGIC_SIZE; i++) {
		if (PFC_EXPECT_FALSE(msg.cph_magic[i] != cproto_magic[i])) {
			cproto_error(sess, EPROTO,
				     "cproto: Unexpected handshake magic.");

			return EPROTO;
		}
	}

	/* Check protocol version. */
	if (PFC_EXPECT_FALSE(msg.cph_version != CTRL_PROTO_VERSION)) {
		cproto_error(sess, EPROTO,
			     "cproto: Unexpected protocol version: %u",
			     msg.cph_version);

		return EPROTO;
	}

	/*
	 * Check byte order, and determine whether we need byte swap
	 * or not.
	 */
	if (msg.cph_order == CTRL_ORDER_NATIVE) {
		sess->cps_bswap = PFC_FALSE;
	}
	else if (PFC_EXPECT_FALSE(msg.cph_order != CTRL_ORDER_LITTLE &&
				  msg.cph_order != CTRL_ORDER_BIG)) {
		cproto_error(sess, EPROTO,
			     "cproto: Unexpected byte order: %u",
			     msg.cph_order);

		return EPROTO;
	}
	else {
		sess->cps_bswap = PFC_TRUE;
	}

	return 0;
}

/*
 * int
 * cproto_handshake_write(cproto_sess_t *sess)
 *	Write a handshake message to the specified control protocol session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
cproto_handshake_write(cproto_sess_t *sess)
{
	pfc_iostream_t	stream = sess->cps_stream;
	pfc_timespec_t	*timeout = &sess->cps_timeout;
	cproto_hshake_t	msg;
	uint32_t	i;
	size_t		size = sizeof(msg);
	int		err;

	/* Construct handshake message. */
	for (i = 0; i < CTRL_PROTO_MAGIC_SIZE; i++) {
		msg.cph_magic[i] = cproto_magic[i];
	}
	msg.cph_version = CTRL_PROTO_VERSION;
	msg.cph_order = CTRL_ORDER_NATIVE;
	for (i = 0; i < PFC_ARRAY_CAPACITY(msg.cph_pad); i++) {
		msg.cph_pad[i] = 0;
	}

	/* Send message. */
	err = pfc_iostream_write(stream, &msg, &size, timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		PFC_ASSERT(size == sizeof(msg));
		err = pfc_iostream_flush(stream, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			cproto_error(sess, err,
				     "cproto: Failed to flush handshake: %s",
				     strerror(err));
		}
	}
	else {
		cproto_error(sess, err,
			     "cproto: Failed to write handshake: %s",
			     strerror(err));
	}

	return err;
}

/*
 * int
 * cproto_cmd_read(cproto_sess_t *PFC_RESTRICT sess,
 *		   ctrl_cmdtype_t *PFC_RESTRICT cmdp)
 *	Read command type from the specified session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, command type read from the session is set
 *	to `*cmdp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
cproto_cmd_read(cproto_sess_t *PFC_RESTRICT sess,
		ctrl_cmdtype_t *PFC_RESTRICT cmdp)
{
	int	err;
	size_t	size = sizeof(ctrl_cmdtype_t);

	err = pfc_iostream_read(sess->cps_stream, cmdp, &size,
				&sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (size == 0) {
			/* Disconnected by peer. */
			*cmdp = CTRL_CMDTYPE_EOF;

			return 0;
		}
		if (PFC_EXPECT_FALSE(size != sizeof(ctrl_cmdtype_t))) {
			cproto_error(sess, EPROTO,
				     "cproto: Unexpected size of command: %"
				     PFC_PFMT_SIZE_T, size);

			return EPROTO;
		}
		CPROTO_BSWAP_16(sess, *cmdp);
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session read error: command: %s",
			     strerror(err));
	}

	return err;
}

/*
 * int
 * cproto_cmd_write(cproto_sess_t sess, ctrl_cmdtype_t cmd)
 *	Write command type to the specified session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
int
cproto_cmd_write(cproto_sess_t *sess, ctrl_cmdtype_t cmd)
{
	return cproto_write_uint16(sess, cmd, "command");
}

/*
 * int
 * cproto_resp_read(cproto_sess_t *PFC_RESTRICT sess,
 *		    ctrl_cmdresp_t *PFC_RESTRICT respp)
 *	Read command response code from the specified session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, command response code read from the session
 *	is set to `*respp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
cproto_resp_read(cproto_sess_t *PFC_RESTRICT sess,
		 ctrl_cmdresp_t *PFC_RESTRICT respp)
{
	int	err;
	size_t	size = sizeof(ctrl_cmdresp_t);

	err = pfc_iostream_read(sess->cps_stream, respp, &size,
				&sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (PFC_EXPECT_FALSE(size != sizeof(ctrl_cmdresp_t))) {
			cproto_error(sess, EPROTO,
				     "cproto: Unexpected size of command "
				     "response: %" PFC_PFMT_SIZE_T, size);

			return EPROTO;
		}
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session read error: response: %s",
			     strerror(err));
	}

	return err;
}

/*
 * int
 * cproto_resp_write(cproto_sess_t *sess, ctrl_cmdresp_t resp)
 *	Write command response code to the specified session stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
int
cproto_resp_write(cproto_sess_t *sess, ctrl_cmdresp_t resp)
{
	int	err;
	size_t	size = sizeof(ctrl_cmdresp_t);

	err = pfc_iostream_write(sess->cps_stream, &resp, &size,
				 &sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		PFC_ASSERT(size == sizeof(ctrl_cmdresp_t));
	}
	else {
		cproto_error(sess, err,
			     "cproto: Failed to write command response: %u: %s",
			     resp, strerror(err));
	}

	return err;
}

/*
 * uint32_t
 * cproto_timeout_value(void)
 *	Return timeout value, in seconds, for I/O completion.
 */
uint32_t
cproto_timeout_value(void)
{
	pfc_cfblk_t	options = pfccmd_conf_options();

	return pfc_conf_get_uint32(options, conf_ctrl_timeout,
				   CPROTO_CTRL_TIMEOUT_DEFAULT);
}

/*
 * static int
 * cproto_read_uint16(cproto_sess_t *PFC_RESTRICT sess,
 *		      uint16_t *PFC_RESTRICT valuep,
 *		      const char *PFC_RESTRICT label)
 *	Read unsigned 16-bit integer from the session stream.
 *	`label' is used as label in error message.
 */
static int
cproto_read_uint16(cproto_sess_t *PFC_RESTRICT sess,
		   uint16_t *PFC_RESTRICT valuep,
		   const char *PFC_RESTRICT label)
{
	int	err;
	size_t	size = sizeof(uint16_t);

	err = pfc_iostream_read(sess->cps_stream, valuep, &size,
				&sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (PFC_EXPECT_FALSE(size != sizeof(uint16_t))) {
			cproto_error(sess, EPROTO,
				     "cproto: Unexpected size of %s: %"
				     PFC_PFMT_SIZE_T, label, size);

			return EPROTO;
		}
		CPROTO_BSWAP_16(sess, *valuep);
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session read error: %s: %s",
			     label, strerror(err));
	}

	return err;
}

/*
 * static int
 * cproto_read_uint32(cproto_sess_t *PFC_RESTRICT sess,
 *		      uint32_t *PFC_RESTRICT valuep,
 *		      const char *PFC_RESTRICT label)
 *	Read unsigned 32-bit integer from the session stream.
 *	`label' is used as label in error message.
 */
static int
cproto_read_uint32(cproto_sess_t *PFC_RESTRICT sess,
		   uint32_t *PFC_RESTRICT valuep,
		   const char *PFC_RESTRICT label)
{
	int	err;
	size_t	size = sizeof(uint32_t);

	err = pfc_iostream_read(sess->cps_stream, valuep, &size,
				&sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (PFC_EXPECT_FALSE(size != sizeof(uint32_t))) {
			cproto_error(sess, EPROTO,
				     "cproto: Unexpected size of %s: %"
				     PFC_PFMT_SIZE_T, label, size);

			return EPROTO;
		}
		CPROTO_BSWAP_32(sess, *valuep);
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session read error: %s: %s",
			     label, strerror(err));
	}

	return err;
}

/*
 * static int
 * cproto_write_uint16(cproto_sess_t *PFC_RESTRICT sess, uint16_t value,
 *		       const char *PFC_RESTRICT label)
 *	Write unsigned 16-bit integer to the session stream.
 *	`label' is used as label in error message.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
static int
cproto_write_uint16(cproto_sess_t *PFC_RESTRICT sess, uint16_t value,
		    const char *PFC_RESTRICT label)
{
	int	err;
	size_t	size = sizeof(uint16_t);

	err = pfc_iostream_write(sess->cps_stream, &value, &size,
				 &sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		PFC_ASSERT(size == sizeof(uint16_t));
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session write error: %s: %s",
			     label, strerror(err));
	}

	return err;
}

/*
 * static int
 * cproto_write_uint32(cproto_sess_t *sess, uint32_t value)
 *	Write unsigned 32-bit integer to the session stream.
 *	`label' is used as label in error message.
 *
 * Remarks:
 *	Note that this function never flushes the session stream.
 */
static int
cproto_write_uint32(cproto_sess_t *PFC_RESTRICT sess, uint32_t value,
		    const char *PFC_RESTRICT label)
{
	int	err;
	size_t	size = sizeof(uint32_t);

	err = pfc_iostream_write(sess->cps_stream, &value, &size,
				 &sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		PFC_ASSERT(size == sizeof(uint32_t));
	}
	else {
		cproto_error(sess, err,
			     "cproto: Control session write error: %s %s",
			     label, strerror(err));
	}

	return err;
}

/*
 * static int
 * cproto_parse_int32(cproto_sess_t *PFC_RESTRICT sess,
 *		      cproto_data_t *PFC_RESTRICT datap)
 *	Parser function for CTRL_PDTYPE_INT32.
 */
static int
cproto_parse_int32(cproto_sess_t *PFC_RESTRICT sess,
		   cproto_data_t *PFC_RESTRICT datap)
{
	uint32_t	value;
	int		err;

	if (PFC_EXPECT_FALSE(datap->cpd_size != sizeof(int32_t))) {
		cproto_error(sess, EPROTO,
			     "cproto: Unexpected size of INT32: %u",
			     datap->cpd_size);

		return EPROTO;
	}

	err = cproto_read_uint32(sess, &value, "INT32");
	if (PFC_EXPECT_TRUE(err == 0)) {
		datap->cpd_value = (pfc_ptr_t)(uintptr_t)value;
	}

	return err;
}

/*
 * static int
 * cproto_parse_text(cproto_sess_t *PFC_RESTRICT sess,
 *		     cproto_data_t *PFC_RESTRICT datap)
 *	Parser function for CTRL_PDTYPE_TEXT.
 */
static int
cproto_parse_text(cproto_sess_t *PFC_RESTRICT sess,
		  cproto_data_t *PFC_RESTRICT datap)
{
	size_t	size = datap->cpd_size;
	char	*buf;
	int	err;

	buf = (char *)malloc(size + 1);
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		cproto_error(sess, EPROTO,
			     "cproto: No memory for text: %" PFC_PFMT_SIZE_T,
			     size + 1);

		return ENOMEM;
	}

	/* Read string. */
	err = pfc_iostream_read(sess->cps_stream, buf, &size,
				&sess->cps_timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (size != (size_t)datap->cpd_size) {
			free(buf);
			cproto_error(sess, EPROTO,
				     "cproto: Unexpected TEXT size: %"
				     PFC_PFMT_SIZE_T ", %u",
				     size, datap->cpd_size);

			return EPROTO;
		}
		*(buf + size) = '\0';
		datap->cpd_value = buf;
	}

	return err;
}

/*
 * static void
 * cproto_data_free_value(cproto_data_t *datap)
 *	Release a buffer pointed by datap->cpd_value.
 *	This function simply passes datap->cpd_value to free(3).
 */
static void
cproto_data_free_value(cproto_data_t *datap)
{
	free(datap->cpd_value);
}

/*
 * static void
 * cproto_error(cproto_sess_t *PFC_RESTRICT sess, int err,
 *	        const char *PFC_RESTRICT fmt, ...)
 *	Record logs about error on the control session.
 */
static void
cproto_error(cproto_sess_t *PFC_RESTRICT sess, int err,
	     const char *PFC_RESTRICT fmt, ...)
{
	va_list	ap;

	if (sess->cps_error == NULL) {
		return;
	}

	va_start(ap, fmt);
	sess->cps_error(err, fmt, ap);
	va_end(ap);
}
