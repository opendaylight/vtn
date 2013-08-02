/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * stream.c - Generate IPC message via output stream.
 *
 * Remarks:
 *	ipc_stream_t does not have any synchronization mechanism.
 *	The caller must serialize accesses to ipc_stream_t.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pfc/util.h>
#include <pfc/iostream.h>
#include "ipc_impl.h"
#include "ipc_struct_impl.h"

/*
 * Destroy the given PDU list entry.
 */
#define	IPC_PDU_FREE(pdu)						\
	do {								\
		ipc_cpduops_t	*__ops = (pdu)->ip_ops;			\
									\
		if (__ops->ipops_dtor != NULL) {			\
			__ops->ipops_dtor(pdu);				\
		}							\
		if ((pdu)->ip_tag.ipt_type == PFC_IPCTYPE_STRUCT) {	\
			IPC_STRINFO_RELEASE(IPC_STRINFO_PDUOPS2PTR(__ops)); \
		}							\
		free(pdu);						\
	} while (0)

/*
 * Internal prototypes.
 */
static void	ipc_stream_free(ipc_stream_t *stp);
static int	ipc_stream_addpdu(ipc_stream_t *PFC_RESTRICT stp,
				  ipc_pdu_t *PFC_RESTRICT pdu,
				  pfc_cptr_t PFC_RESTRICT data);
static int	ipc_stream_send_data(pfc_iostream_t PFC_RESTRICT stream,
				     ipc_stream_t *PFC_RESTRICT stp,
				     ctimespec_t *PFC_RESTRICT abstime);

/*
 * void
 * pfc_ipcstream_destroy(ipc_stream_t *stp)
 *	Destroy the given IPC output stream.
 *	Note that this function never frees the IPC stream instance itself.
 */
void
pfc_ipcstream_destroy(ipc_stream_t *stp)
{
	ipc_stream_free(stp);
}

/*
 * void
 * pfc_ipcstream_reset(ipc_stream_t *stp, uint32_t flags)
 *	Reset IPC stream to the initial state.
 *	This function destroys any data related to output data in the stream.
 */
void
pfc_ipcstream_reset(ipc_stream_t *stp, uint32_t flags)
{
	ipc_stream_free(stp);

	/* Reset the stream. */
	stp->is_size = 0;
	stp->is_count = 0;
	stp->is_flags = flags;
	stp->is_pdunext = &stp->is_pdus;
}

/*
 * int	pfc_ipcstream_add_int8(ipc_stream_t *stp, int8_t data)
 * int	pfc_ipcstream_add_uint8(ipc_stream_t *stp, uint8_t data)
 * int	pfc_ipcstream_add_int16(ipc_stream_t *stp, int16_t data)
 * int	pfc_ipcstream_add_uint16(ipc_stream_t *stp, uint16_t data)
 * int	pfc_ipcstream_add_int32(ipc_stream_t *stp, int32_t data)
 * int	pfc_ipcstream_add_uint32(ipc_stream_t *stp, uint32_t data)
 * int	pfc_ipcstream_add_int64(ipc_stream_t *stp, int64_t data)
 * int	pfc_ipcstream_add_uint64(ipc_stream_t *stp, uint64_t data)
 * int	pfc_ipcstream_add_float(ipc_stream_t *stp, float data)
 * int	pfc_ipcstream_add_double(ipc_stream_t *stp, double data)
 * int	pfc_ipcstream_add_ipv4(ipc_stream_t *PFC_RESTRICT stp,
 *			       struct in_addr *PFC_RESTRICT data)
 * int	pfc_ipcstream_add_ipv6(ipc_stream_t *PFC_RESTRICT stp,
 *			       struct in6_addr *PFC_RESTRICT data)
 *	Append the specified data to the IPC output stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the specified stream is broken.
 *	EBADF is returned if the specified stream is already finalized.
 *	Otherwise error number which indicates the cause of error is returned.
 */
#define	IPC_STREAM_ADD_DECL(pdutype, argtype, suffix)			\
	int								\
	pfc_ipcstream_add_##suffix(ipc_stream_t *stp, argtype data)	\
	{								\
		ipc_pdu_t	*pdu;					\
									\
		pdu = pfc_ipc_pdu_alloc(PFC_IPCTYPE_##pdutype);		\
		if (PFC_EXPECT_FALSE(pdu == NULL)) {			\
			return ENOMEM;					\
		}							\
									\
		pdu->ip_tag.ipt_size = sizeof(data);			\
		pdu->ip_data_##pdutype = data;				\
									\
		return ipc_stream_addpdu(stp, pdu, NULL);		\
	}

#define	IPC_STREAM_ADD_IP_DECL(pdutype, argtype, suffix)		\
	int								\
	pfc_ipcstream_add_##suffix(ipc_stream_t *PFC_RESTRICT stp,	\
				   argtype *PFC_RESTRICT data)		\
	{								\
		ipc_pdu_t	*pdu;					\
									\
		PFC_ASSERT(data != NULL);				\
									\
		pdu = pfc_ipc_pdu_alloc(PFC_IPCTYPE_##pdutype);		\
		if (PFC_EXPECT_FALSE(pdu == NULL)) {			\
			return ENOMEM;					\
		}							\
									\
		pdu->ip_tag.ipt_size = sizeof(*data);			\
		pdu->ip_data_##pdutype = *data;				\
									\
		return ipc_stream_addpdu(stp, pdu, NULL);		\
	}

IPC_STREAM_ADD_DECL(INT8, int8_t, int8);
IPC_STREAM_ADD_DECL(UINT8, uint8_t, uint8);
IPC_STREAM_ADD_DECL(INT16, int16_t, int16);
IPC_STREAM_ADD_DECL(UINT16, uint16_t, uint16);
IPC_STREAM_ADD_DECL(INT32, int32_t, int32);
IPC_STREAM_ADD_DECL(UINT32, uint32_t, uint32);
IPC_STREAM_ADD_DECL(INT64, int64_t, int64);
IPC_STREAM_ADD_DECL(UINT64, uint64_t, uint64);
IPC_STREAM_ADD_DECL(FLOAT, float, float);
IPC_STREAM_ADD_DECL(DOUBLE, double, double);

IPC_STREAM_ADD_IP_DECL(IPV4, struct in_addr, ipv4);
IPC_STREAM_ADD_IP_DECL(IPV6, struct in6_addr, ipv6);

#define	IPC_STREAM_ADD_POINTER_BODY(stp, data, length, pdutype)	\
	ipc_pdu_t	*pdu;					\
	ipc_pdutag_t	*tag;					\
								\
	pdu = pfc_ipc_pdu_alloc(PFC_IPCTYPE_##pdutype);		\
	if (PFC_EXPECT_FALSE(pdu == NULL)) {			\
		return ENOMEM;					\
	}							\
								\
	tag = &pdu->ip_tag;					\
	if (data == NULL) {					\
		tag->ipt_flags |= IPC_PDUF_NULL;		\
		tag->ipt_size = 0;				\
	}							\
	else {							\
		tag->ipt_size = (length);			\
	}							\
								\
	/* Initialize PDU data pointer with NULL. */		\
	pdu->ip_data_POINTER = NULL;				\
								\
	return ipc_stream_addpdu(stp, pdu, data)

/*
 * int
 * pfc_ipcstream_add_string(ipc_stream_t *PFC_RESTRICT stp,
 *			    const char *PFC_RESTRICT data)
 *	Append a string PDU to the specified IPC output stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the specified stream is broken.
 *	EBADF is returned if the specified stream is already finalized.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcstream_add_string(ipc_stream_t *PFC_RESTRICT stp,
			 const char *PFC_RESTRICT data)
{
	IPC_STREAM_ADD_POINTER_BODY(stp, data, strlen(data) + 1, STRING);
}

/*
 * int
 * pfc_ipcstream_add_binary(ipc_stream_t *PFC_RESTRICT stp,
 *			    const uint8_t *PFC_RESTRICT data, uint32_t length)
 *	Append a binary image PDU to the specified IPC output stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the specified stream is broken.
 *	EBADF is returned if the specified stream is already finalized.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcstream_add_binary(ipc_stream_t *PFC_RESTRICT stp,
			 const uint8_t *PFC_RESTRICT data, uint32_t length)
{
	IPC_STREAM_ADD_POINTER_BODY(stp, data, length, BINARY);
}

/*
 * int
 * pfc_ipcstream_add_null(ipc_stream_t *stp)
 *	Append a NULL data to the specified IPC output stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the specified stream is broken.
 *	EBADF is returned if the specified stream is already finalized.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcstream_add_null(ipc_stream_t *stp)
{
	IPC_STREAM_ADD_POINTER_BODY(stp, NULL, 0, NULL);
}

/*
 * int
 * pfc_ipcstream_add_struct(ipc_stream_t *PFC_RESTRICT stp,
 *			    const uint8_t *PFC_RESTRICT data, uint32_t length,
 *			    const char *PFC_RESTRICT stname,
 *			    const char *PFC_RESTRICT sig)
 *	Append a struct data to the specified IPC output stream.
 *
 *	`data' must be a pointer to struct, and `length' must equal to the size
 *	of struct.
 *
 *	`stname' must be a name of struct defined by IPC struct template file.
 *	`sig' must be the layout signature required by the struct.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the specified stream is broken.
 *	EBADF is returned if the specified stream is already finalized.
 *	ENODEV is returned if IPC struct information associated with the
 *	given struct name is not found or invalid.
 *	EFAULT is returned if `data' is not aligned on required boundary.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcstream_add_struct(ipc_stream_t *PFC_RESTRICT stp,
			 const uint8_t *PFC_RESTRICT data, uint32_t length,
			 const char *PFC_RESTRICT stname,
			 const char *PFC_RESTRICT sig)
{
	ipc_cstrinfo_t	*sip;
	int		err;

	PFC_ASSERT(stp != NULL && data != NULL && stname != NULL &&
		   sig != NULL);

	err = pfc_ipc_struct_get(stname, sig, data, length, &sip);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_ipcstream_add_known_struct(stp, data, sip);
		IPC_STRINFO_RELEASE(sip);
	}

	return err;
}

/*
 * int
 * pfc_ipcstream_add_known_struct(ipc_stream_t *PFC_RESTRICT stp,
 *				  const uint8_t *PFC_RESTRICT data,
 *				  ipc_cstrinfo_t *PFC_RESTRICT sip)
 *	Append a struct data to the specified IPC output stream.
 *
 *	`data' must be a pointer to struct, and `length' must equal to the size
 *	of struct.
 *
 *	`sip' must be a non-NULL pointer to IPC structure information.
 *	The caller must guarantee that `sip' represents the layout of the
 *	IPC structure data specified by `data'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the specified stream is broken.
 *	EBADF is returned if the specified stream is already finalized.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcstream_add_known_struct(ipc_stream_t *PFC_RESTRICT stp,
			       const uint8_t *PFC_RESTRICT data,
			       ipc_cstrinfo_t *PFC_RESTRICT sip)
{
	ipc_pdu_t	*pdu;
	ipc_pdutag_t	*tag;
	uint32_t	psize, namelen, length;
	uint8_t		*pdata, *p;
	const char	*stname;

	PFC_ASSERT(stp != NULL && data != NULL && sip != NULL);
	PFC_ASSERT(PFC_IS_POW2_ALIGNED(data, IPC_STRINFO_ALIGN(sip)));

	/* Determine total size of serialized data. */
	stname = IPC_STRINFO_NAME(sip);
	namelen = IPC_STRINFO_NAMELEN(sip) + 1;
	PFC_ASSERT(namelen <= IPC_STRTYPE_MAX_NAMELEN);
	length = IPC_STRINFO_SIZE(sip);
	psize = sizeof(uint8_t) + namelen + sizeof(sip->sti_sig) + length;

	/* Allocate buffer to serialize struct data. */
	pdata = (uint8_t *)malloc(psize);
	if (PFC_EXPECT_FALSE(pdata == NULL)) {
		IPC_LOG_ERROR("No memory for struct %s: size=%u",
			      stname, psize);

		return ENOMEM;
	}

	/*
	 * Serialize struct data.
	 *   1. Name length (uint8_t), including terminator.
	 *   2. NULL byte terminated struct name.
	 *   3. Layout signature.
	 *   4. Struct data.
	 */
	*pdata = (uint8_t)namelen;
	p = pdata + 1;
	memcpy(p, stname, namelen);
	p += namelen;
	memcpy(p, sip->sti_sig, sizeof(sip->sti_sig));
	p += sizeof(sip->sti_sig);
	memcpy(p, data, length);
	PFC_ASSERT(p + length == pdata + psize);

	/* Allocate a PDU data for struct. */
	pdu = (ipc_pdu_t *)malloc(sizeof(*pdu));
	if (PFC_EXPECT_FALSE(pdu == NULL)) {
		IPC_LOG_ERROR("No memory for struct PDU tag: %s", stname);
		free(pdata);

		return ENOMEM;
	}

	/* ipt_off will be initialized by ipc_stream_addpdu(). */
	tag = &pdu->ip_tag;
	tag->ipt_type = PFC_IPCTYPE_STRUCT;
	tag->ipt_flags = 0;
	tag->ipt_pad = 0;
	tag->ipt_size = psize;

	pdu->ip_next = NULL;
	pdu->ip_ops = &sip->sti_pduops;
	pdu->ip_data_POINTER = pdata;
	IPC_STRINFO_HOLD(sip);

	return ipc_stream_addpdu(stp, pdu, pdata);
}

/*
 * int
 * pfc_ipcstream_copymsg(ipc_stream_t *PFC_RESTRICT stp,
 *			 ipc_msg_t *PFC_RESTRICT msg, uint32_t begin,
 *			 uint32_t end)
 *	Copy PDUs in the IPC message specified by `msg', `begin', and `end'.
 *
 *	`begin' is the inclusive beginning index of PDUs in the IPC message,
 *	and `end' is the exclusive ending index of PDUs. For instance,
 *	if `begin' is 3 and `end' is 10, PDUs in `msg' at the index from 3
 *	to 9 are appended to the IPC stream specified by `stp'.
 *	Needless to say, `end' must be greater than `begin'.
 *
 *	If the value specified to `end' is greater than the number of
 *	PDUs in the IPC message `msg', it is treated as if the number of PDUs
 *	is specified to `end'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- The caller must guarantee that both `stp' and `msg' are never updated
 *	  by another thread.
 *
 *	- This function returns zero if the same value is specified to `begin'
 *	  and `end', even if it is not valid additional data index.
 */
int
pfc_ipcstream_copymsg(ipc_stream_t *PFC_RESTRICT stp,
		      ipc_msg_t *PFC_RESTRICT msg, uint32_t begin,
		      uint32_t end)
{
	ipc_pdu_t	*pdu, *newpdus, **pdunext;
	ipc_pduidx_t	*pidxp, *begidxp, *endidxp;
	ipc_pdutag_t	*tag, *etag;
	uint32_t	npdus, offset;
	uint64_t	pdusize, newsize;
	int		err;

	PFC_ASSERT(stp != NULL && msg != NULL);

	/* At first, check the state of the IPC stream. */
	if (PFC_EXPECT_FALSE(stp->is_flags & IPC_STRF_BROKEN)) {
		/* This output stream must be disposed. */
		return EPERM;
	}

	if (PFC_EXPECT_FALSE(stp->is_flags & IPC_STRF_FIN)) {
		/* Already finalized. */
		return EBADF;
	}

	if (end > msg->im_count) {
		end = msg->im_count;
	}
	if (PFC_EXPECT_FALSE(end < begin)) {
		return EINVAL;
	}

	npdus = end - begin;
	if (PFC_EXPECT_FALSE(npdus == 0)) {
		/* No PDU is specified. */
		return 0;
	}
	if (PFC_EXPECT_FALSE(begin >= msg->im_count)) {
		return EINVAL;
	}

	begidxp = msg->im_pdus + begin;
	endidxp = msg->im_pdus + (end - 1);

	/* Determine total size of PDUs to be added. */
	tag = &begidxp->ipi_tag;
	etag = &endidxp->ipi_tag;
	PFC_ASSERT(etag->ipt_off + etag->ipt_size <= msg->im_size);

	pdusize = ((uint64_t)etag->ipt_off + (uint64_t)etag->ipt_size) -
		(uint64_t)tag->ipt_off;
	newsize = (uint64_t)stp->is_size + pdusize;
	if (PFC_EXPECT_FALSE(newsize > IPC_OUTSIZE_MAX)) {
		/* Too large data. */
		IPC_LOG_ERROR("Output message size exceeds the limit: %u -> %"
			      PFC_PFMT_u64, stp->is_size, newsize);

		return E2BIG;
	}

	/* Create PDU meta data for all PDUs to be added. */
	newpdus = NULL;
	pdunext = &newpdus;
	offset = stp->is_size;
	for (pidxp = begidxp; pidxp <= endidxp; pidxp++) {
		ipc_pdutag_t	*tag, *srctag = &pidxp->ipi_tag;
		ipc_cpduops_t	*ops;
		const uint8_t	*src;
		uint8_t		type;

		pdu = (ipc_pdu_t *)malloc(sizeof(*pdu));
		if (PFC_EXPECT_FALSE(pdu == NULL)) {
			IPC_LOG_ERROR("No memory for PDU tag.");
			err = ENOMEM;
			goto error;
		}

		pdu->ip_next = NULL;

		/* Copy PDU tag except for ipt_off. */
		tag = &pdu->ip_tag;
		type = tag->ipt_type = srctag->ipt_type;
		tag->ipt_flags = srctag->ipt_flags;
		tag->ipt_pad = 0;
		tag->ipt_size = srctag->ipt_size;

		/* Set PDU offset. */
		tag->ipt_off = offset;
		offset += tag->ipt_size;

		if (type == PFC_IPCTYPE_STRUCT) {
			ipc_cstrinfo_t	*sip;
			ipc_strpdu_t	strpdu;

			/*
			 * Get IPC structure information associated with this
			 * PDU.
			 */
			err = pfc_ipcmsg_get_strinfo(msg, pidxp, &strpdu, &sip,
						     PFC_FALSE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				/*
				 * IPC_PDU_FREE() can not be used unless
				 * ip_ops field is initialized.
				 */
				free(pdu);
				goto error;
			}

			ops = &sip->sti_pduops;
		}
		else {
			ops = pfc_ipc_pdu_getops(type);
			if (PFC_EXPECT_FALSE(ops == NULL)) {
				IPC_LOG_ERROR("Unexpected PDU type: %u", type);
				free(pdu);
				err = EPROTO;
				goto error;
			}
		}

		pdu->ip_ops = ops;
		*pdunext = pdu;
		pdunext = &(pdu->ip_next);

		/* PDU data must be copied. */
		src = msg->im_data + srctag->ipt_off;
		err = ops->ipops_copy(pdu, src, msg->im_bflags);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
	}

	PFC_ASSERT((uint64_t)offset == newsize);

	/* Link PDU tags to the tail of the PDU list. */
	*(stp->is_pdunext) = newpdus;
	stp->is_pdunext = pdunext;
	PFC_ASSERT(*pdunext == NULL);

	/* Update statistics. */
	stp->is_size = newsize;
	stp->is_count += npdus;

	return 0;

error:
	while (newpdus != NULL) {
		pdu = newpdus;
		newpdus = newpdus->ip_next;
		IPC_PDU_FREE(pdu);
	}

	return err;
}

/*
 * int
 * pfc_ipcstream_send(ipc_sess_t *PFC_RESTRICT sess,
 *		      ipc_stream_t *PFC_RESTRICT stp,
 *		      ctimespec_t *PFC_RESTRICT abstime)
 *	Send all data in the given IPC stream to the IPC session stream
 *	specified by `sess'.
 *
 *	If `abstime' is not NULL, ETIMEDOUT is returned if the absolute time
 *	specified by `abstime' passes before completion of sending data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EBADF is returned if the specified stream is already finalized.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function always send data in host byte order.
 *
 *	- This function always flushes the output buffer of the given IPC
 *	  session.
 */
int
pfc_ipcstream_send(ipc_sess_t *PFC_RESTRICT sess,
		   ipc_stream_t *PFC_RESTRICT stp,
		   ctimespec_t *PFC_RESTRICT abstime)
{
	pfc_iostream_t	stream = sess->iss_stream;
	pfc_bool_t	do_flush;
	ipc_pdu_t	*pdu;
	ipc_msgmeta_t	meta;
	int		err;

	if (PFC_EXPECT_FALSE((stp->is_flags & (IPC_STRF_FIN | IPC_STRF_EVENT))
			     == IPC_STRF_FIN)) {
		/* Already finalized. */
		return EBADF;
	}

	/* Finalize this output stream. */
	stp->is_flags |= IPC_STRF_FIN;

	IPC_LOG_VERBOSE("Sending IPC message: count=%u, size=%u",
			stp->is_count, stp->is_size);

	/* Construct meta data. */
	meta.imm_count = stp->is_count;
	meta.imm_size = stp->is_size;
	meta.imm_xfermode = IPC_XFERMODE_STREAM;
	meta.imm_resv1 = 0;
	meta.imm_resv2 = 0;

	do_flush = (stp->is_count == 0) ? PFC_TRUE : PFC_FALSE;

	/* Send meta data. */
	err = pfc_ipc_write(stream, &meta, sizeof(meta), do_flush, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("Failed to send PDU meta data.");

		return err;
	}

	if (stp->is_count == 0) {
		/* No data to be sent. */
		return 0;
	}

	/* Send PDU tags. */
	for (pdu = stp->is_pdus; pdu != NULL; pdu = pdu->ip_next) {
		ipc_pdutag_t	*tag = &pdu->ip_tag;

		err = pfc_ipc_write(stream, tag, sizeof(*tag), PFC_FALSE,
				    abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPC_LOG_ERROR("Failed to send PDU tag.");

			return err;
		}
	}

	/* Send PDU data via IPC session stream. */
	return ipc_stream_send_data(stream, stp, abstime);
}

/*
 * static void
 * ipc_stream_free(ipc_stream_t *stp)
 *	Free any resources related to output data in the specified IPC stream.
 */
static void
ipc_stream_free(ipc_stream_t *stp)
{
	ipc_pdu_t	*pdu, *next;

	for (pdu = stp->is_pdus; pdu != NULL; pdu = next) {
		next = pdu->ip_next;
		IPC_PDU_FREE(pdu);
	}
	stp->is_pdus = NULL;
}

/*
 * static int
 * ipc_stream_addpdu(ipc_stream_t *PFC_RESTRICT stp,
 *		     ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT data)
 *	Add the PDU specified by `pdu' to the tail of PDU list in IPC stream.
 *
 *	If PDU data is not set in the PDU specified by `pdu', the caller must
 *	specify a pointer to PDU data to `data'.
 *
 *	If the given PDU is STRUCT PDU, the caller must set PDU data to both
 *	pdu->ip_data_POINTER and `data'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	On error, the PDU specified by `pdu' is destroyed by this function.
 */
static int
ipc_stream_addpdu(ipc_stream_t *PFC_RESTRICT stp,
		  ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT data)
{
	ipc_pdutag_t	*tag = &pdu->ip_tag;
	uint64_t	size;
	int		err;

	if (PFC_EXPECT_FALSE(stp->is_flags & IPC_STRF_BROKEN)) {
		/* This output stream must be disposed. */
		err = EPERM;
		goto error;
	}

	if (PFC_EXPECT_FALSE(stp->is_flags & IPC_STRF_FIN)) {
		/* Already finalized. */
		err = EBADF;
		goto error;
	}

	/* Set data offset for this PDU. */
	tag->ipt_off = stp->is_size;

	size = (uint64_t)stp->is_size + (uint64_t)pdu->ip_tag.ipt_size;
	if (PFC_EXPECT_FALSE(size > IPC_OUTSIZE_MAX)) {
		/* Too large data. */
		IPC_LOG_ERROR("Output message size exceeds the limit: %"
			      PFC_PFMT_u64, size);
		err = E2BIG;
		goto error;
	}

	if (data != NULL && tag->ipt_size != 0 &&
	    pdu->ip_data_POINTER == NULL) {
		void	*ptr;

		/* Need to duplicate PDU data. */
		ptr = malloc(tag->ipt_size);
		if (PFC_EXPECT_FALSE(ptr == NULL)) {
			IPC_LOG_ERROR("No memory for PDU data: size=%u",
				      tag->ipt_size);
			err = ENOMEM;
			goto error;
		}

		memcpy(ptr, data, tag->ipt_size);
		pdu->ip_data_POINTER = ptr;
	}

	/* Link PDU tag to the tail of the PDU list. */
	stp->is_size = (uint32_t)size;
	*(stp->is_pdunext) = pdu;
	stp->is_pdunext = &pdu->ip_next;
	PFC_ASSERT(pdu->ip_next == NULL);
	stp->is_count++;

	return 0;

error:
	IPC_PDU_FREE(pdu);

	return err;
}

/*
 * static int
 * ipc_stream_send_data(pfc_iostream_t PFC_RESTRICT stream,
 *			ipc_stream_t *PFC_RESTRICT stp,
 *			ctimespec_t *PFC_RESTRICT abstime)
 *	Send PDU data to the peer connected to the given I/O stream.
 *
 *	This function send data via IPC session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function always send PDU data in host byte order.
 */
static int
ipc_stream_send_data(pfc_iostream_t PFC_RESTRICT stream,
		     ipc_stream_t *PFC_RESTRICT stp,
		     ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_pdu_t	*pdu;
	int		err;
#ifdef	PFC_VERBOSE_DEBUG
	uint32_t	nbytes = 0;
#endif	/* PFC_VERBOSE_DEBUG */

	/* Notify the start of STREAM transfer mode. */
	IPC_LOG_VERBOSE("Use STREAM mode: size=%u", stp->is_size);

	/* Send PDU data. */
	for (pdu = stp->is_pdus; pdu != NULL; pdu = pdu->ip_next) {
		ipc_cpduops_t	*ops = pdu->ip_ops;

		err = ops->ipops_write(stp, stream, pdu, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPC_LOG_ERROR("Failed to send PDU data.");

			return err;
		}

#ifdef	PFC_VERBOSE_DEBUG
		nbytes += pdu->ip_tag.ipt_size;
#endif	/* PFC_VERBOSE_DEBUG */
	}

	PFC_ASSERT(nbytes == stp->is_size);

	/* Flush output buffer. */
	err = pfc_iostream_flush_abs(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != ECANCELED) {
			IPC_LOG_ERROR("Failed to flush IPC session: %s",
				      strerror(err));
		}
	}

	return err;
}
