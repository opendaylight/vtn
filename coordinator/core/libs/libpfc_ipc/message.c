/*
 * Copyright (c) 2011-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * message.c - IPC message instance and accessor.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pfc/util.h>
#include <iostream_impl.h>
#include "ipc_impl.h"
#include "ipc_struct_impl.h"

/*
 * Empty data.
 * This is used to represent empty binary image.
 */
static const uint8_t	ipc_empty_data[0];

/*
 * Internal prototypes.
 */
static void	ipc_msg_free(ipc_msg_t *msg);
static int	ipc_msg_getpdu(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
			       pfc_ipctype_t type,
			       ipc_pduidx_t **PFC_RESTRICT pdupp);
static int	ipc_msg_recv_stream(pfc_iostream_t PFC_RESTRICT stream,
				    ipc_msg_t *PFC_RESTRICT msg,
				    ctimespec_t *PFC_RESTRICT abstime);
static int	ipc_msg_pdutype_check(uint8_t type);
static int	ipc_msg_fetch_struct(ipc_msg_t *PFC_RESTRICT msg,
				     ipc_pduidx_t *PFC_RESTRICT pdu,
				     ipc_strpdu_t *PFC_RESTRICT strpdu);

/*
 * int	pfc_ipcmsg_get_int8(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			    int8_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_uint8(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			     uint8_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_int16(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			     int16_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_uint16(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			      uint16_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_int32(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			     int32_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_uint32(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			      uint32_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_int64(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			     int64_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_uint64(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			      uint64_t *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_float(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			     float *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_double(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			      double *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_ipv4(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			    struct in_addr *PFC_RESTRICT datap)
 * int	pfc_ipcmsg_get_ipv6(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			    struct in6_addr *PFC_RESTRICT datap)
 *	Fetch data of the PDU at the index specified by `index'.
 *
 * Calling/Exit State:
 *	Upon successful completion, data is stored to `*datap', and
 *	zero is returned.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if PDU type does not match.
 *	EPROTO is returned if the given IPC message is broken.
 */
#define	IPC_MSG_GETADDR(msg, index, addr, argtype, pdutype)		\
	do {								\
		ipc_pduidx_t	*__pdu;					\
		ipc_pdutag_t	*__tag;					\
		int		__err;					\
									\
		__err = ipc_msg_getpdu(msg, index,			\
				       PFC_IPCTYPE_##pdutype, &__pdu);	\
		if (PFC_EXPECT_FALSE(__err != 0)) {			\
			return __err;					\
		}							\
									\
		PFC_ASSERT((msg)->im_data != NULL);			\
		__tag = &__pdu->ipi_tag;				\
									\
		if (PFC_EXPECT_FALSE(__tag->ipt_size != sizeof(argtype))) { \
			IPC_LOG_ERROR("Bad size for %s: %u", #pdutype,	\
				      __tag->ipt_size);			\
									\
			return EPROTO;					\
		}							\
									\
		(addr) = (const argtype *)((msg)->im_data +		\
					   __tag->ipt_off);		\
		PFC_ASSERT((const uint8_t *)((addr) + 1) <=		\
			   IPC_MSG_DATA_LIMIT(msg));			\
	} while (0)

#define	IPC_MSG_GET_DECL(pdutype, argtype, suffix)			\
	int								\
	pfc_ipcmsg_get_##suffix(ipc_msg_t *PFC_RESTRICT msg,		\
				uint32_t index,				\
				argtype *PFC_RESTRICT datap)		\
	{								\
		const argtype	*addr;					\
									\
		IPC_MSG_GETADDR(msg, index, addr, argtype, pdutype);	\
									\
		/* Fetch value. */					\
		IPC_FETCH_INT(*datap, addr);				\
									\
		if (IPC_NEED_BSWAP(msg->im_bflags)) {			\
			/* Byte swapping is needed. */			\
			IPC_BSWAP(*datap);				\
		}							\
									\
		return 0;						\
	}

#define	IPC_MSG_GET_FLOAT_DECL(pdutype, argtype, stubtype, suffix)	\
	int								\
	pfc_ipcmsg_get_##suffix(ipc_msg_t *PFC_RESTRICT msg,		\
				uint32_t index,				\
				argtype *PFC_RESTRICT datap)		\
	{								\
		const argtype	*addr;					\
									\
		PFC_ASSERT(sizeof(argtype) == sizeof(stubtype));	\
									\
		IPC_MSG_GETADDR(msg, index, addr, argtype, pdutype);	\
									\
		/* Fetch value. */					\
		IPC_FETCH_INT(*datap, addr);				\
									\
		if (IPC_NEED_BSWAP_FLOAT(msg->im_bflags)) {		\
			union {						\
				argtype		data;			\
				stubtype	stub;			\
			} value;					\
									\
			/* Byte swapping is needed. */			\
			PFC_ASSERT((void *)&value.data ==		\
				   (void *)&value.stub);		\
			value.data = *datap;				\
			IPC_BSWAP(value.stub);				\
			*datap = value.data;				\
		}							\
									\
		return 0;						\
	}

#define	IPC_MSG_GET_IP_DECL(pdutype, argtype, suffix)			\
	int								\
	pfc_ipcmsg_get_##suffix(ipc_msg_t *PFC_RESTRICT msg,		\
				uint32_t index,				\
				argtype *PFC_RESTRICT datap)		\
	{								\
		const argtype	*addr;					\
									\
		IPC_MSG_GETADDR(msg, index, addr, argtype, pdutype);	\
									\
		/* Fetch value. */					\
		memcpy(datap, addr, sizeof(*datap));			\
									\
		return 0;						\
	}

IPC_MSG_GET_DECL(INT8, int8_t, int8);
IPC_MSG_GET_DECL(UINT8, uint8_t, uint8);
IPC_MSG_GET_DECL(INT16, int16_t, int16);
IPC_MSG_GET_DECL(UINT16, uint16_t, uint16);
IPC_MSG_GET_DECL(INT32, int32_t, int32);
IPC_MSG_GET_DECL(UINT32, uint32_t, uint32);
IPC_MSG_GET_DECL(INT64, int64_t, int64);
IPC_MSG_GET_DECL(UINT64, uint64_t, uint64);

IPC_MSG_GET_FLOAT_DECL(FLOAT, float, uint32_t, float);
IPC_MSG_GET_FLOAT_DECL(DOUBLE, double, uint64_t, double);

IPC_MSG_GET_IP_DECL(IPV4, struct in_addr, ipv4);
IPC_MSG_GET_IP_DECL(IPV6, struct in6_addr, ipv6);

/*
 * int
 * pfc_ipcmsg_get_string(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			 const char **PFC_RESTRICT datapp)
 *	Fetch STRING data in the IPC message at the index specified by `index'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to string is stored to `*datapp',
 *	and zero is returned.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if the type of the specified PDU is not STRING.
 *	EPROTO is returned if the given IPC message is broken.
 *
 * Remarks:
 *	- STRING type can handle NULL. So NULL may be set to `*datapp'.
 *
 *	- The buffer pointed by `*datapp' is read-only. Write access to the
 *	  buffer pointed by `*datapp' results in undefined behavior.
 */
int
pfc_ipcmsg_get_string(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
		      const char **PFC_RESTRICT datapp)
{
	ipc_pduidx_t	*pdu;
	ipc_pdutag_t	*tag;
	int		err;

	err = ipc_msg_getpdu(msg, index, PFC_IPCTYPE_STRING, &pdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	tag = &pdu->ipi_tag;
	if (tag->ipt_flags & IPC_PDUF_NULL) {
		/* NULL pointer. */
		PFC_ASSERT(tag->ipt_size == 0);
		*datapp = NULL;
	}
	else {
		const char	*addr;
		uint32_t	size = tag->ipt_size;

		if (PFC_EXPECT_FALSE(size == 0)) {
			IPC_LOG_ERROR("Empty STRING in IPC message.");

			return EPROTO;
		}

		PFC_ASSERT(msg->im_data != NULL);
		addr = (const char *)(msg->im_data + tag->ipt_off);
		PFC_ASSERT((const uint8_t *)(addr + size) <=
			   IPC_MSG_DATA_LIMIT(msg));

		/* Ensure that the string is terminated by '\0'. */
		if (PFC_EXPECT_FALSE(*(addr + (size - 1)) != '\0')) {
			IPC_LOG_ERROR("Broken STRING in IPC message.");

			return EPROTO;
		}

		/* Set string pointer to *datapp. */
		*datapp = addr;
	}

	return 0;
}

/*
 * int
 * pfc_ipcmsg_get_binary(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			 const uint8_t **PFC_RESTRICT datapp,
 *			 uint32_t *PFC_RESTRICT lengthp)
 *	Fetch BINARY data in the IPC message at the index specified by `index'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to binary image is stored to
 *	`*datapp', and its length in bytes is set to `*lengthp', and then
 *	zero is returned.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if the type of the specified PDU is not BINARY.
 *
 * Remarks:
 *	- BINARY type can handle NULL. If data is NULL, In this case null is
 *	  set to `*datapp',  and zero is set to `*lengthp'.
 *	  BINARY type can handle also empty data. In this case a non-NULL
 *	  pointer is stored to `*datapp', and zero is set to `*lengthp'.
 *	  So the caller must not access the buffer pointed by `*datapp'
 *	  if `*lengthp' is zero.
 *
 *	- The buffer pointed by `*datapp' is read-only. Write access to the
 *	  buffer pointed by `*datapp' results in undefined behavior.
 */
int
pfc_ipcmsg_get_binary(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
		      const uint8_t **PFC_RESTRICT datapp,
		      uint32_t *PFC_RESTRICT lengthp)
{
	ipc_pduidx_t	*pdu;
	ipc_pdutag_t	*tag;
	int		err;

	err = ipc_msg_getpdu(msg, index, PFC_IPCTYPE_BINARY, &pdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Set the size of data. */
	tag = &pdu->ipi_tag;
	*lengthp = tag->ipt_size;

	if (tag->ipt_flags & IPC_PDUF_NULL) {
		/* NULL pointer. */
		PFC_ASSERT(tag->ipt_size == 0);
		*datapp = NULL;
	}
	else if (tag->ipt_size == 0) {
		/* Empty image. */
		*datapp = ipc_empty_data;
	}
	else {
		const uint8_t	*addr;

		PFC_ASSERT(msg->im_data != NULL);
		addr = msg->im_data + tag->ipt_off;
		PFC_ASSERT(addr + tag->ipt_size <= IPC_MSG_DATA_LIMIT(msg));

		/* Set data pointer to *datapp. */
		*datapp = addr;
	}

	return 0;
}

/*
 * int
 * pfc_ipcmsg_get_struct(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			 uint8_t *PFC_RESTRICT datap, uint32_t length,
 *			 const char *PFC_RESTRICT stname,
 *			 const char *PFC_RESTRICT sig)
 *	Fetch STRUCT data in the IPC message at the index specified by `index'.
 *
 *	Data is copied to the buffer pointed by `datap'. It must be `length'
 *	long, and `length' must equal to the size of struct.
 *
 *	`stname' must be a name of struct defined by IPC struct template file.
 *	`sig' must be the layout signature required by the struct.
 *
 * Calling/Exit State:
 *	Upon successful completion, struct data is copied to the buffer pointed
 *	by `datap', and zero is returned.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if data type specified by `stname' does not match
 *	the type of actual PDU type.
 *	ENODEV is returned if IPC struct information associated with the
 *	given struct name is not found or invalid.
 *	EFAULT is returned if `datap' is not aligned on required boundary.
 *	EPROTO is returned if the PDU is broken.
 *	EBADMSG is returned if struct layout in the PDU does not match.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcmsg_get_struct(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
		      uint8_t *PFC_RESTRICT datap, uint32_t length,
		      const char *PFC_RESTRICT stname,
		      const char *PFC_RESTRICT sig)
{
	ipc_pduidx_t	*pdu;
	ipc_cstrinfo_t	*sip;
	ipc_cpduops_t	*ops;
	ipc_strpdu_t	strpdu;
	int		err, diff;

	PFC_ASSERT(msg != NULL && datap != NULL && stname != NULL &&
		   sig != NULL);

	err = ipc_msg_getpdu(msg, index, PFC_IPCTYPE_STRUCT, &pdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Ensure that valid IPC struct name is specified. */
	err = pfc_ipc_struct_get(stname, sig, datap, length, &sip);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Fetch struct data in PDU. */
	err = ipc_msg_fetch_struct(msg, pdu, &strpdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Verify struct name. */
	diff = strcmp(stname, strpdu.isp_name);
	if (PFC_EXPECT_FALSE(diff != 0)) {
		/* Don't log this error because this may often happen. */
		err = EPERM;
		goto out;
	}

	/* Verify struct layout signature. */
	diff = memcmp(sig, strpdu.isp_sig, IPC_STRUCT_SIG_SIZE);
	if (PFC_EXPECT_FALSE(diff != 0)) {
		IPC_LOG_ERROR("Bad layout signature: struct %s", stname);
		err = EBADMSG;
		goto out;
	}

	/* Verify struct size. */
	ops = &sip->sti_pduops;
	if (PFC_EXPECT_FALSE(strpdu.isp_size != ops->ipops_size)) {
		IPC_LOG_ERROR("Invalid data size for struct %s: %u: "
			      "required=%u", stname, strpdu.isp_size,
			      ops->ipops_size);
		err = EBADMSG;
		goto out;
	}

	if (IPC_NEED_BSWAP_ANY(msg->im_bflags)) {
		ipc_bswap_t	bswap;

		/* Copy struct data with byte swapping. */
		bswap.ibs_src = strpdu.isp_data;
		bswap.ibs_dst = datap;
		bswap.ibs_flags = msg->im_bflags;
		pfc_ipc_struct_bswap(ops, &bswap);
	}
	else {
		/* Copy whole struct data. */
		PFC_ASSERT(length == ops->ipops_size);
		memcpy(datap, strpdu.isp_data, length);
	}

out:
	IPC_STRINFO_RELEASE(sip);

	return err;
}

/*
 * int
 * pfc_ipcmsg_fetch_struct(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			   ipc_cstrinfo_t **PFC_RESTRICT sipp,
 *			   ipc_strbuf_t **PFC_RESTRICT sbpp,
 *			   pfc_bool_t need_fields)
 *	Fetch STRUCT data in the IPC message at the index specified by `index'.
 *
 *	Unlike pfc_ipcmsg_get_struct(), this function can fetch any STRUCT
 *	data as long as it is valid IPC structure.
 *
 *	This function loads IPC structure information file if not yet loaded.
 *	If `need_fields' is PFC_TRUE, information about structure fields are
 *	also loaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	- A non-NULL pointer to IPC structure information is set to the
 *	  buffer pointed by `sipp'. It must be released by
 *	  IPC_STRINFO_RELEASE().
 *	- A non-NULL pointer to ipc_strbuf_t, which contains received data,
 *	  is set to the buffer pointed by `sbpp'. It must be released by
 *	  IPC_STRBUF_RELEASE().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	- EINVAL is returned if an invalid index is specified.
 *	- ENODEV is returned if IPC structure information associated with the
 *	  received struct name is not found or invalid.
 *	- EPROTO is returned if the PDU is broken.
 *	- EBADMSG is returned if struct layout in the PDU does not match the
 *	  locally-defined layout.
 */
int
pfc_ipcmsg_fetch_struct(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
			ipc_cstrinfo_t **PFC_RESTRICT sipp,
			ipc_strbuf_t **PFC_RESTRICT sbpp,
			pfc_bool_t need_fields)
{
	ipc_pduidx_t	*pdu;
	ipc_cstrinfo_t	*sip;
	ipc_strpdu_t	strpdu;
	ipc_strbuf_t	*sbp;
	uint32_t	size;
	int		err;

	PFC_ASSERT(msg != NULL && sipp != NULL && sbpp != NULL);

	err = ipc_msg_getpdu(msg, index, PFC_IPCTYPE_STRUCT, &pdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/*
	 * Get IPC structure information associated with the received
	 * struct data.
	 */
	err = pfc_ipcmsg_get_strinfo(msg, pdu, &strpdu, &sip, need_fields);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Allocate buffer for receiving data. */
	size = strpdu.isp_size;
	sbp = pfc_ipc_strbuf_alloc(size);
	if (PFC_EXPECT_FALSE(sbp == NULL)) {
		err = ENOMEM;
		goto error;
	}

	if (IPC_NEED_BSWAP_ANY(msg->im_bflags)) {
		ipc_bswap_t	bswap;

		/* Copy struct data with byte swapping. */
		bswap.ibs_src = strpdu.isp_data;
		bswap.ibs_dst = sbp->isb_data;
		bswap.ibs_flags = msg->im_bflags;
		pfc_ipc_struct_bswap(&sip->sti_pduops, &bswap);
	}
	else {
		/* Copy whole struct data. */
		memcpy(sbp->isb_data, strpdu.isp_data, size);
	}

	*sipp = sip;
	*sbpp = sbp;

	return 0;

error:
	IPC_STRINFO_RELEASE(sip);

	return err;
}

/*
 * int
 * pfc_ipcmsg_get_struct_name(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *			      const char **PFC_RESTRICT namepp)
 *	Return name of the struct name of the STRUCT PDU at the given index.
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to struct name is
 *	set to `*namepp', and zero is returned.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if the type of the specified PDU is not STRUCT.
 *	EPROTO is returned if the PDU is broken.
 */
int
pfc_ipcmsg_get_struct_name(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
			   const char **PFC_RESTRICT namepp)
{
	ipc_pduidx_t	*pdu;
	ipc_strpdu_t	strpdu;
	int		err;

	PFC_ASSERT(namepp != NULL);

	err = ipc_msg_getpdu(msg, index, PFC_IPCTYPE_STRUCT, &pdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = ipc_msg_fetch_struct(msg, pdu, &strpdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	*namepp = strpdu.isp_name;

	return 0;
}

/*
 * void
 * pfc_ipcmsg_destroy(ipc_msg_t *msg)
 *	Release resources held by the given IPC message.
 *	Note that this function never frees the IPC message instance itself.
 */
void
pfc_ipcmsg_destroy(ipc_msg_t *msg)
{
	ipc_msg_free(msg);
}

/*
 * void
 * pfc_ipcmsg_reset(ipc_msg_t *msg)
 *	Reset the IPC message instance to the initial state.
 *	This function destroys any received data in the IPC message instance.
 */
void
pfc_ipcmsg_reset(ipc_msg_t *msg)
{
	ipc_msg_free(msg);

	/* Reset the message. */
	msg->im_size = 0;
	msg->im_count = 0;
	msg->im_bflags = 0;
}

/*
 * int
 * pfc_ipcmsg_recv(ipc_sess_t *PFC_RESTRICT sess, ipc_msg_t *PFC_RESTRICT msg,
 *		   ctimespec_t *PFC_RESTRICT abstime)
 *	Receive IPC message from the given IPC session.
 *
 *	If `abstime' is not NULL, ETIMEDOUT is returned if the absolute time
 *	specified by `abstime' passes before completion of sending data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcmsg_recv(ipc_sess_t *PFC_RESTRICT sess, ipc_msg_t *PFC_RESTRICT msg,
		ctimespec_t *PFC_RESTRICT abstime)
{
	pfc_iostream_t	stream = sess->iss_stream;
	ipc_pduidx_t	*pdarray, *pdu;
	ipc_msgmeta_t	meta;
	uint8_t		bflags = sess->iss_flags, mode;
	uint32_t	off;
	size_t		pdsz;
	int		err;

	/* Receive PDU meta data. */
	err = pfc_ipc_read(stream, &meta, sizeof(meta), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("Failed to receive PDU meta data.");

		return err;
	}

	msg->im_count = meta.imm_count;
	if (msg->im_count == 0) {
		IPC_LOG_VERBOSE("Received empty IPC message.");

		return 0;
	}

	msg->im_size = meta.imm_size;
#ifndef	PFC_LP64
	if (PFC_EXPECT_FALSE(msg->im_size > IPC_OUTSIZE_MAX)) {
		IPC_LOG_ERROR("Too large PDU data: %u", msg->im_size);

		return E2BIG;
	}
#endif	/* !PFC_LP64 */

	IPC_LOG_VERBOSE("Receiving IPC message: count=%u, size=%u",
			msg->im_count, msg->im_size);

	/* Allocate buffer for PDU index. */
	pdsz = sizeof(*pdarray) * msg->im_count;
	pdarray = (ipc_pduidx_t *)malloc(pdsz);
	if (PFC_EXPECT_FALSE(pdarray == NULL)) {
		IPC_LOG_ERROR("Failed to allocate PDU index: %u",
			      msg->im_count);

		return ENOMEM;
	}

	/*
	 * Read PDU index.
	 * Note that below code assumes that ipc_pduidx_t contains
	 * ipc_pdutag_t only.
	 */
	PFC_ASSERT(sizeof(*pdu) == sizeof(pdu->ipi_tag));
	err = pfc_ipc_read(stream, pdarray, pdsz, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("Failed to receive PDU tag.");
		goto error;
	}

	for (off = 0, pdu = pdarray; pdu < pdarray + msg->im_count; pdu++) {
		ipc_pdutag_t	*tag = &pdu->ipi_tag;

		/* Determine PDU type. */
		err = ipc_msg_pdutype_check(tag->ipt_type);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		if (IPC_NEED_BSWAP(bflags)) {
			IPC_BSWAP(tag->ipt_size);
			IPC_BSWAP(tag->ipt_off);
		}

		/*
		 * PDU data in the IPC message must be packed without any
		 * padding.
		 */
		if (PFC_EXPECT_FALSE(off != tag->ipt_off)) {
			IPC_LOG_ERROR("Invalid offset in IPC message: "
				      "off=0x%x, expected=0x%x",
				      tag->ipt_off, off);
			err = EPROTO;
			goto error;
		}

		off = tag->ipt_off + tag->ipt_size;
		if (PFC_EXPECT_FALSE(off > msg->im_size)) {
			IPC_LOG_ERROR("Invalid PDU: off=%u, size=%u, total=%u",
				      tag->ipt_off, tag->ipt_size,
				      msg->im_size);
			err = EPROTO;
			goto error;
		}
	}

	mode = meta.imm_xfermode;
	if (PFC_EXPECT_TRUE(mode == IPC_XFERMODE_STREAM)) {
		/* STREAM mode. */
		if (msg->im_size == 0) {
			/* Nothing to do. */
			err = 0;
		}
		else {
			err = ipc_msg_recv_stream(stream, msg, abstime);
		}
	}
	else {
		IPC_LOG_ERROR("Unknown XFER mode: %u", mode);
		err = EPROTO;
		goto error;
	}

	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	msg->im_pdus = pdarray;

	return 0;

error:
	free(pdarray);

	return err;
}

/*
 * int
 * pfc_ipcmsg_init_uint32(ipc_msg_t *PFC_RESTRICT msg, uint32_t data)
 *	Initialize the IPC message instance with one UINT32 data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Byte swapping flags in the IPC message instance is always cleared.
 */
int
pfc_ipcmsg_init_uint32(ipc_msg_t *PFC_RESTRICT msg, uint32_t data)
{
	ipc_pduidx_t	*pdu;
	ipc_pdutag_t	*tag;
	uint32_t	*datap;

	/* Allocate memory required by one UINT32 data. */
	pdu = (ipc_pduidx_t *)malloc(sizeof(*pdu));
	if (PFC_EXPECT_FALSE(pdu == NULL)) {
		return ENOMEM;
	}

	datap = (uint32_t *)malloc(sizeof(*datap));
	if (PFC_EXPECT_FALSE(datap == NULL)) {
		free(pdu);

		return ENOMEM;
	}

	/* Initialize PDU index. */
	tag = &pdu->ipi_tag;
	tag->ipt_type = PFC_IPCTYPE_UINT32;
	tag->ipt_flags = 0;
	tag->ipt_pad = 0;
	tag->ipt_size = sizeof(data);
	tag->ipt_off = 0;

	/* Initialize the data buffer. */
	*datap = data;

	/* Initialize IPC message instance. */
	msg->im_size = sizeof(data);
	msg->im_count = 1;
	msg->im_bflags = 0;
	msg->im_pdus = pdu;
	msg->im_data = (const uint8_t *)datap;

	return 0;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcmsg_get_strinfo(ipc_msg_t *PFC_RESTRICT msg,
 *			  ipc_pduidx_t *PFC_RESTRICT pdu,
 *			  ipc_strpdu_t *PFC_RESTRICT strpdu,
 *			  ipc_cstrinfo_t **PFC_RESTRICT sipp,
 *			  pfc_bool_t need_fields)
 *	Get IPC structure information associated with the IPC struct specified
 *	by the IPC message `msg' and the PDU index `pdu'.
 *
 *	This function loads IPC structure information file if not yet loaded.
 *	If `need_fields' is PFC_TRUE, information about structure fields are
 *	also loaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned. A non-NULL pointer to
 *	IPC structure information is set to the buffer pointed by `sipp', and
 *	Fetched data is stored into the buffer pointed by `strpdu'.
 *	Note that the caller must release the IPC structure information by
 *	IPC_STRBUF_RELEASE().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
pfc_ipcmsg_get_strinfo(ipc_msg_t *PFC_RESTRICT msg,
		       ipc_pduidx_t *PFC_RESTRICT pdu,
		       ipc_strpdu_t *PFC_RESTRICT strpdu,
		       ipc_cstrinfo_t **PFC_RESTRICT sipp,
		       pfc_bool_t need_fields)
{
	ipc_cstrinfo_t	*sip;
	ipc_cpduops_t	*ops;
	uint32_t	size;
	int		err, diff;

	PFC_ASSERT(msg != NULL && pdu != NULL && sipp != NULL);
	PFC_ASSERT(pdu->ipi_tag.ipt_type == PFC_IPCTYPE_STRUCT);

	/* Fetch struct data in PDU. */
	err = ipc_msg_fetch_struct(msg, pdu, strpdu);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/*
	 * Get IPC structure information associated with the received
	 * struct data.
	 */
	err = pfc_ipc_strinfo_get(strpdu->isp_name, &sip, need_fields);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("Unknown IPC structure: %s", strpdu->isp_name);

		return err;
	}

	/* Verify struct layout signature. */
	diff = memcmp(sip->sti_sig, strpdu->isp_sig, IPC_STRUCT_SIG_SIZE);
	if (PFC_EXPECT_FALSE(diff != 0)) {
		IPC_LOG_ERROR("Bad layout signature: struct %s",
			      strpdu->isp_name);
		err = EBADMSG;
		goto error;
	}

	/* Verify struct size. */
	ops = &sip->sti_pduops;
	size = strpdu->isp_size;
	if (PFC_EXPECT_FALSE(size != ops->ipops_size)) {
		IPC_LOG_ERROR("Invalid data size for struct %s: %u: "
			      "required=%u", strpdu->isp_name, size,
			      ops->ipops_size);
		err = EBADMSG;
		goto error;
	}

	*sipp = sip;

	return 0;

error:
	IPC_STRINFO_RELEASE(sip);

	return err;
}

/*
 * static void
 * ipc_msg_free(ipc_msg_t *msg)
 *	Free any data related to received data in the IPC message instance.
 */
static void
ipc_msg_free(ipc_msg_t *msg)
{
	if (msg->im_data != NULL) {
		free((void *)msg->im_data);
		msg->im_data = NULL;
	}

	if (msg->im_pdus != NULL) {
		free(msg->im_pdus);
		msg->im_pdus = NULL;
	}
}

/*
 * static int
 * ipc_msg_getpdu(ipc_msg_t *PFC_RESTRICT msg, uint32_t index,
 *		  pfc_ipctype_t type, ipc_pduidx_t **PFC_RESTRICT pdupp)
 *	Get PDU index of the PDU in the IPC message at the specified index.
 *
 *	PDU type you request must be specified to `type'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU index is set
 *	to `*pdupp', and zero is returned.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if the type of the specified PDU does not equal
 *	to `type'.
 */
static int
ipc_msg_getpdu(ipc_msg_t *PFC_RESTRICT msg, uint32_t index, pfc_ipctype_t type,
	       ipc_pduidx_t **PFC_RESTRICT pdupp)
{
	ipc_pduidx_t	*pdu = pfc_ipcmsg_getpdu(msg, index);
	ipc_pdutag_t	*tag;

	if (PFC_EXPECT_FALSE(pdu == NULL)) {
		/* Invalid PDU index is specified. */
		return EINVAL;
	}

	tag = &pdu->ipi_tag;
	if (PFC_EXPECT_FALSE(tag->ipt_type != type)) {
		/* Invalid data type. */
		return EPERM;
	}

	*pdupp = pdu;

	return 0;
}

/*
 * static int
 * ipc_msg_recv_stream(pfc_iostream_t PFC_RESTRICT stream,
 *		       ipc_msg_t *PFC_RESTRICT msg,
 *		       ctimespec_t *PFC_RESTRICT abstime)
 *	Receive PDU data using STREAM mode.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_msg_recv_stream(pfc_iostream_t PFC_RESTRICT stream,
		    ipc_msg_t *PFC_RESTRICT msg,
		    ctimespec_t *PFC_RESTRICT abstime)
{
	uint8_t	*buffer;
	int	err;

	/* Allocate buffer for PDU data. */
	buffer = (uint8_t *)malloc(msg->im_size);
	if (PFC_EXPECT_FALSE(buffer == NULL)) {
		IPC_LOG_ERROR("Failed to allocate PDU data buffer.");

		return ENOMEM;
	}

	/* Read PDU data. */
	err = pfc_ipc_read(stream, buffer, msg->im_size, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(buffer);
		IPC_LOG_ERROR("Failed to receive PDU data.");
	}
	else {
		msg->im_data = buffer;
	}

	return err;
}

/*
 * static int
 * ipc_msg_pdutype_check(uint8_t type)
 *	Check whether the give PDU type is valid or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the PDU type specified by `type' is valid.
 *	Otherwise EPROTO is returned.
 */
static int
ipc_msg_pdutype_check(uint8_t type)
{
	if (type != PFC_IPCTYPE_STRUCT) {
		ipc_cpduops_t	*ops = pfc_ipc_pdu_getops(type);
		if (PFC_EXPECT_FALSE(ops == NULL)) {
			IPC_LOG_ERROR("Invalid PDU type: %u", type);

			return EPROTO;
		}
	}

	return 0;
}

/*
 * static int
 * ipc_msg_fetch_struct(ipc_msg_t *PFC_RESTRICT msg,
 *			ipc_pduidx_t *PFC_RESTRICT pdu,
 *			ipc_strpdu_t *PFC_RESTRICT strpdu)
 *	Fetch struct data of the given PDU.
 *
 * Calling/Exit State:
 *	Upon successful completion, STRUCT PDU contents are set to `*strpdu',
 *	and then zero is returned.
 *
 *	EPROTO is returned if struct data is broken.
 */
static int
ipc_msg_fetch_struct(ipc_msg_t *PFC_RESTRICT msg,
		     ipc_pduidx_t *PFC_RESTRICT pdu,
		     ipc_strpdu_t *PFC_RESTRICT strpdu)
{
	ipc_pdutag_t	*tag = &pdu->ipi_tag;
	const uint8_t	*addr = msg->im_data + tag->ipt_off;
	uint32_t	size = tag->ipt_size;
	uint8_t		namelen;

	PFC_ASSERT(addr + size <= IPC_MSG_DATA_LIMIT(msg));

	if (PFC_EXPECT_FALSE(size == 0)) {
		IPC_LOG_ERROR("Empty struct data in IPC message.");

		return EPROTO;
	}

	/* The first octet must be the length of struct name. */
	namelen = *addr;
	size--;
	addr++;
	if (PFC_EXPECT_FALSE(namelen > IPC_STRTYPE_MAX_NAMELEN ||
			     namelen > size ||
			     *(addr + namelen - 1) != '\0')) {
		IPC_LOG_ERROR("Invalid struct name length: %u", namelen);

		return EPROTO;
	}

	strpdu->isp_name = (const char *)addr;
	addr += namelen;
	size -= namelen;
	if (PFC_EXPECT_FALSE(size <= IPC_STRUCT_SIG_SIZE)) {
		IPC_LOG_ERROR("Empty data size for struct %s.",
			      strpdu->isp_name);

		return EPROTO;
	}

	strpdu->isp_sig = (const char *)addr;
	strpdu->isp_data = addr + IPC_STRUCT_SIG_SIZE;
	strpdu->isp_size = size - IPC_STRUCT_SIG_SIZE;

	return 0;
}
