/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * pdu.c - Implement protocol data unit for PFC IPC framework.
 */

#include "ipc_impl.h"
#include "ipc_struct_impl.h"

/*
 * Assertion to verify address alignment in ipc_pdu_t.
 */
#define	IPC_PDU_ALIGN_ASSERT(type)				\
	PFC_ASSERT(offsetof(ipc_pdu_t, ip_data_UINT8) ==	\
		   offsetof(ipc_pdu_t, ip_data_##type))

/*
 * Internal prototypes.
 */
static int	ipc_pdu_write(ipc_stream_t *PFC_RESTRICT stp,
			      pfc_iostream_t PFC_RESTRICT stream,
			      ipc_pdu_t *PFC_RESTRICT pdu,
			      ctimespec_t *PFC_RESTRICT abstime);
static int	ipc_pdu_write_ptr(ipc_stream_t *PFC_RESTRICT stp,
				  pfc_iostream_t PFC_RESTRICT stream,
				  ipc_pdu_t *PFC_RESTRICT pdu,
				  ctimespec_t *PFC_RESTRICT abstime);
static int	ipc_pdu_copy(ipc_pdu_t *PFC_RESTRICT pdu,
			     pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags);
static int	ipc_pdu_copy_ptr(ipc_pdu_t *PFC_RESTRICT pdu,
				 pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags);
static int	ipc_pdu_copy_struct(ipc_pdu_t *PFC_RESTRICT pdu,
				    pfc_cptr_t PFC_RESTRICT addr,
				    uint8_t bflags);
static void	ipc_pdu_dtor_ptr(ipc_pdu_t *pdu);
static void	ipc_pdu_bswap_struct(ipc_cpduops_t *PFC_RESTRICT ops,
				     ipc_bswap_t *PFC_RESTRICT bsp,
				     uint32_t nelems);
static void	ipc_pdu_bswap_struct_field(ipc_cfldinfo_t *PFC_RESTRICT flp,
					   ipc_bswap_t *PFC_RESTRICT bsp);

/*
 * Declare PDU data decoder with byte swap support.
 */
#define	IPC_PDU_BSWAP(type)		ipc_pdu_bswap_##type
#define	IPC_PDU_NEED_BSWAP(bsp)		IPC_NEED_BSWAP((bsp)->ibs_flags)
#define	IPC_PDU_NEED_BSWAP_FLOAT(bsp)	IPC_NEED_BSWAP_FLOAT((bsp)->ibs_flags)

#define	IPC_PDU_DECODE_DECL_BODY(argtype, need_bswap)			\
	const argtype	*src = (const argtype *)bsp->ibs_src;		\
	argtype		*dst = (argtype *)bsp->ibs_dst;			\
	argtype	*limit = dst + nelems;					\
									\
	PFC_ASSERT(nelems != 0);					\
	PFC_ASSERT(PFC_IS_POW2_ALIGNED(dst, ops->ipops_align));		\
									\
	if (need_bswap) {						\
		do {							\
			argtype	v;					\
									\
			memcpy(&v, src, sizeof(v));			\
			IPC_BSWAP(v);					\
			*dst = v;					\
			dst++;						\
			src++;						\
		} while (dst < limit);					\
	}								\
	else {								\
		do {							\
			memcpy(dst, src, sizeof(*dst));			\
			dst++;						\
			src++;						\
		} while (dst < limit);					\
	}								\
									\
	bsp->ibs_src = (const uint8_t *)src;				\
	bsp->ibs_dst = (uint8_t *)dst

#define	IPC_PDU_BSWAP_INT_DECL(argtype)					\
	static void							\
	IPC_PDU_BSWAP(argtype)(ipc_cpduops_t *PFC_RESTRICT ops,		\
			       ipc_bswap_t *PFC_RESTRICT bsp,		\
			       uint32_t nelems)				\
	{								\
		IPC_PDU_DECODE_DECL_BODY(argtype, IPC_PDU_NEED_BSWAP(bsp)); \
	}

#define	IPC_PDU_BSWAP_FLOAT_DECL(argtype, stubtype)			\
	static void							\
	IPC_PDU_BSWAP(argtype)(ipc_cpduops_t *PFC_RESTRICT ops,		\
			       ipc_bswap_t *PFC_RESTRICT bsp,		\
			       uint32_t nelems)				\
	{								\
		IPC_PDU_DECODE_DECL_BODY(stubtype,			\
					 IPC_PDU_NEED_BSWAP_FLOAT(bsp)); \
	}

IPC_PDU_BSWAP_INT_DECL(uint16_t);
IPC_PDU_BSWAP_INT_DECL(uint32_t);
IPC_PDU_BSWAP_INT_DECL(uint64_t);

IPC_PDU_BSWAP_FLOAT_DECL(float, uint32_t);
IPC_PDU_BSWAP_FLOAT_DECL(double, uint64_t);

/*
 * Declare PDU data copy methods.
 */
#define	IPC_PDU_COPY(type)		ipc_pdu_copy_##type
#define	IPC_PDU_COPY_INT_DECL(type, argtype)				\
	static int							\
	IPC_PDU_COPY(type)(ipc_pdu_t *PFC_RESTRICT pdu,			\
			   pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags) \
	{								\
		const argtype	*src = (const argtype *)addr;		\
									\
		IPC_FETCH_INT(pdu->ip_data_##type, src);		\
		if (IPC_NEED_BSWAP(bflags)) {				\
			IPC_BSWAP(pdu->ip_data_##type);			\
		}							\
									\
		return 0;						\
	}

#define	IPC_PDU_COPY_FLOAT_DECL(type, argtype, stubtype)		\
	static int							\
	IPC_PDU_COPY(type)(ipc_pdu_t *PFC_RESTRICT pdu,			\
			   pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags) \
	{								\
		const argtype	*src = (const argtype *)addr;		\
									\
		IPC_FETCH_INT(pdu->ip_data_##type, src);		\
		if (IPC_NEED_BSWAP_FLOAT(bflags)) {			\
			IPC_BSWAP(pdu->ip_data_##stubtype);		\
		}							\
									\
		return 0;						\
	}

IPC_PDU_COPY_INT_DECL(UINT8, uint8_t);
IPC_PDU_COPY_INT_DECL(UINT16, uint16_t);
IPC_PDU_COPY_INT_DECL(UINT32, uint32_t);
IPC_PDU_COPY_INT_DECL(UINT64, uint64_t);

IPC_PDU_COPY_FLOAT_DECL(FLOAT, float, UINT32);
IPC_PDU_COPY_FLOAT_DECL(DOUBLE, double, UINT64);

/*
 * Declare PDU type operations.
 */
#define	IPC_PDUSPEC_DECL(type)					\
	{							\
		.ipops_size	= IPC_PDU_SIZE_##type,		\
		.ipops_align	= IPC_PDU_ALIGN_##type,		\
		.ipops_write	= ipc_pdu_write_##type,		\
		.ipops_bswap	= ipc_pdu_bswap_##type,		\
		.ipops_copy	= ipc_pdu_copy_##type,		\
		.ipops_dtor	= ipc_pdu_dtor_##type,		\
	}

#define	IPC_PDU_SIZE_INT8		sizeof(int8_t)
#define	IPC_PDU_ALIGN_INT8		sizeof(int8_t)
#define	ipc_pdu_write_INT8		ipc_pdu_write
#define	ipc_pdu_bswap_INT8		NULL
#define	ipc_pdu_copy_INT8		IPC_PDU_COPY(UINT8)
#define	ipc_pdu_dtor_INT8		NULL

#define	IPC_PDU_SIZE_UINT8		sizeof(uint8_t)
#define	IPC_PDU_ALIGN_UINT8		sizeof(uint8_t)
#define	ipc_pdu_write_UINT8		ipc_pdu_write
#define	ipc_pdu_bswap_UINT8		NULL
#define	ipc_pdu_dtor_UINT8		NULL

#define	IPC_PDU_SIZE_INT16		sizeof(int16_t)
#define	IPC_PDU_ALIGN_INT16		sizeof(int16_t)
#define	ipc_pdu_write_INT16		ipc_pdu_write
#define	ipc_pdu_bswap_INT16		IPC_PDU_BSWAP(uint16_t)
#define	ipc_pdu_copy_INT16		IPC_PDU_COPY(UINT16)
#define	ipc_pdu_dtor_INT16		NULL

#define	IPC_PDU_SIZE_UINT16		sizeof(uint16_t)
#define	IPC_PDU_ALIGN_UINT16		sizeof(uint16_t)
#define	ipc_pdu_write_UINT16		ipc_pdu_write
#define	ipc_pdu_bswap_UINT16		IPC_PDU_BSWAP(uint16_t)
#define	ipc_pdu_dtor_UINT16		NULL

#define	IPC_PDU_SIZE_INT32		sizeof(int32_t)
#define	IPC_PDU_ALIGN_INT32		sizeof(int32_t)
#define	ipc_pdu_write_INT32		ipc_pdu_write
#define	ipc_pdu_bswap_INT32		IPC_PDU_BSWAP(uint32_t)
#define	ipc_pdu_copy_INT32		IPC_PDU_COPY(UINT32)
#define	ipc_pdu_dtor_INT32		NULL

#define	IPC_PDU_SIZE_UINT32		sizeof(uint32_t)
#define	IPC_PDU_ALIGN_UINT32		sizeof(uint32_t)
#define	ipc_pdu_write_UINT32		ipc_pdu_write
#define	ipc_pdu_bswap_UINT32		IPC_PDU_BSWAP(uint32_t)
#define	ipc_pdu_dtor_UINT32		NULL

#define	IPC_PDU_SIZE_INT64		sizeof(int64_t)
#define	IPC_PDU_ALIGN_INT64		sizeof(int64_t)
#define	ipc_pdu_write_INT64		ipc_pdu_write
#define	ipc_pdu_bswap_INT64		IPC_PDU_BSWAP(uint64_t)
#define	ipc_pdu_copy_INT64		IPC_PDU_COPY(UINT64)
#define	ipc_pdu_dtor_INT64		NULL

#define	IPC_PDU_SIZE_UINT64		sizeof(uint64_t)
#define	IPC_PDU_ALIGN_UINT64		sizeof(uint64_t)
#define	ipc_pdu_write_UINT64		ipc_pdu_write
#define	ipc_pdu_bswap_UINT64		IPC_PDU_BSWAP(uint64_t)
#define	ipc_pdu_dtor_UINT64		NULL

#define	IPC_PDU_SIZE_FLOAT		sizeof(float)
#define	IPC_PDU_ALIGN_FLOAT		sizeof(float)
#define	ipc_pdu_write_FLOAT		ipc_pdu_write
#define	ipc_pdu_bswap_FLOAT		IPC_PDU_BSWAP(float)
#define	ipc_pdu_dtor_FLOAT		NULL

#define	IPC_PDU_SIZE_DOUBLE		sizeof(double)
#define	IPC_PDU_ALIGN_DOUBLE		sizeof(double)
#define	ipc_pdu_write_DOUBLE		ipc_pdu_write
#define	ipc_pdu_bswap_DOUBLE		IPC_PDU_BSWAP(double)
#define	ipc_pdu_dtor_DOUBLE		NULL

#define	IPC_PDU_SIZE_IPV4		sizeof(struct in_addr)
#define	IPC_PDU_ALIGN_IPV4		PFC_CONST_U(4)
#define	ipc_pdu_write_IPV4		ipc_pdu_write
#define	ipc_pdu_bswap_IPV4		NULL
#define	ipc_pdu_copy_IPV4		ipc_pdu_copy
#define	ipc_pdu_dtor_IPV4		NULL

#define	IPC_PDU_SIZE_IPV6		sizeof(struct in6_addr)
#define	IPC_PDU_ALIGN_IPV6		PFC_CONST_U(8)
#define	ipc_pdu_write_IPV6		ipc_pdu_write
#define	ipc_pdu_bswap_IPV6		NULL
#define	ipc_pdu_copy_IPV6		ipc_pdu_copy
#define	ipc_pdu_dtor_IPV6		NULL

/* IPC struct does not support STRING type. */
#define	IPC_PDU_SIZE_STRING		PFC_CONST_U(0)
#define	IPC_PDU_ALIGN_STRING		PFC_CONST_U(0)
#define	ipc_pdu_write_STRING		ipc_pdu_write_ptr
#define	ipc_pdu_bswap_STRING		NULL
#define	ipc_pdu_copy_STRING		ipc_pdu_copy_ptr
#define	ipc_pdu_dtor_STRING		ipc_pdu_dtor_ptr

/* IPC struct does not support BINARY type. */
#define	IPC_PDU_SIZE_BINARY		PFC_CONST_U(0)
#define	IPC_PDU_ALIGN_BINARY		PFC_CONST_U(0)
#define	ipc_pdu_write_BINARY		ipc_pdu_write_ptr
#define	ipc_pdu_bswap_BINARY		NULL
#define	ipc_pdu_copy_BINARY		ipc_pdu_copy_ptr
#define	ipc_pdu_dtor_BINARY		ipc_pdu_dtor_ptr

/* IPC struct does not support NULL type. */
#define	IPC_PDU_SIZE_NULL		PFC_CONST_U(0)
#define	IPC_PDU_ALIGN_NULL		PFC_CONST_U(0)
#define	ipc_pdu_write_NULL		ipc_pdu_write_ptr
#define	ipc_pdu_bswap_NULL		NULL
#define	ipc_pdu_copy_NULL		ipc_pdu_copy_ptr
#define	ipc_pdu_dtor_NULL		NULL

static ipc_cpduops_t	ipc_pdu_types[] = {
	IPC_PDUSPEC_DECL(INT8),
	IPC_PDUSPEC_DECL(UINT8),
	IPC_PDUSPEC_DECL(INT16),
	IPC_PDUSPEC_DECL(UINT16),
	IPC_PDUSPEC_DECL(INT32),
	IPC_PDUSPEC_DECL(UINT32),
	IPC_PDUSPEC_DECL(INT64),
	IPC_PDUSPEC_DECL(UINT64),
	IPC_PDUSPEC_DECL(FLOAT),
	IPC_PDUSPEC_DECL(DOUBLE),
	IPC_PDUSPEC_DECL(IPV4),
	IPC_PDUSPEC_DECL(IPV6),
	IPC_PDUSPEC_DECL(STRING),
	IPC_PDUSPEC_DECL(BINARY),
	IPC_PDUSPEC_DECL(NULL),
};

/*
 * ipc_pdu_t PFC_ATTR_HIDDEN *
 * pfc_ipc_pdu_alloc(pfc_ipctype_t type)
 *	Allocate a new PDU data.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_pdu_t is
 *	returned.
 *
 * Remarks:
 *	PFC_IPCTYPE_STRUCT can not be specified to `type'.
 */
ipc_pdu_t PFC_ATTR_HIDDEN *
pfc_ipc_pdu_alloc(pfc_ipctype_t type)
{
	ipc_cpduops_t	*ops = &ipc_pdu_types[(uint32_t)type];
	ipc_pdu_t	*pdu;
	ipc_pdutag_t	*tag;

	PFC_ASSERT(ops < PFC_ARRAY_LIMIT(ipc_pdu_types));

	pdu = (ipc_pdu_t *)malloc(sizeof(*pdu));
	if (PFC_EXPECT_FALSE(pdu == NULL)) {
		IPC_LOG_ERROR("No memory for PDU tag.");

		return NULL;
	}

	/* ipt_size and ipt_off will be initialized later. */
	tag = &pdu->ip_tag;
	tag->ipt_type = type;
	tag->ipt_flags = 0;
	tag->ipt_pad = 0;

	pdu->ip_next = NULL;
	pdu->ip_ops = ops;

	return pdu;
}

/*
 * ipc_cpduops_t PFC_ATTR_HIDDEN *
 * pfc_ipc_pdu_getops(pfc_ipctype_t type)
 *	Get PDU type specific operations for the given PDU type.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU operation is
 *	returned.
 *	NULL is returned if the given PDU type is invalid.
 *
 * Remarks:
 *	PFC_IPCTYPE_STRUCT can not be specified to `type'.
 */
ipc_cpduops_t PFC_ATTR_HIDDEN *
pfc_ipc_pdu_getops(pfc_ipctype_t type)
{
	ipc_cpduops_t	*ops = &ipc_pdu_types[(uint32_t)type];

	if (PFC_EXPECT_FALSE(ops >= PFC_ARRAY_LIMIT(ipc_pdu_types))) {
		return NULL;
	}

	return ops;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipc_pdu_setstruct(ipc_pduops_t *ops)
 *	Install PDU operations for STRUCT type to `ops'.
 */
void PFC_ATTR_HIDDEN
pfc_ipc_pdu_setstruct(ipc_pduops_t *ops)
{
	ops->ipops_write = ipc_pdu_write_ptr;
	ops->ipops_dtor = ipc_pdu_dtor_ptr;
	ops->ipops_bswap = ipc_pdu_bswap_struct;
	ops->ipops_copy = ipc_pdu_copy_struct;
}

/*
 * static int
 * ipc_pdu_write(ipc_stream_t *PFC_RESTRICT stp,
 *		 pfc_iostream_t PFC_RESTRICT stream,
 *		 ipc_pdu_t *PFC_RESTRICT pdu,
 *		 ctimespec_t *PFC_RESTRICT abstime)
 *	Write PDU data to the specified output stream.
 *	This function is used to write numeric data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_pdu_write(ipc_stream_t *PFC_RESTRICT stp,
	      pfc_iostream_t PFC_RESTRICT stream, ipc_pdu_t *PFC_RESTRICT pdu,
	      ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_pdutag_t	*tag = &pdu->ip_tag;
	uint32_t	size = tag->ipt_size;

	/* Assertion to verify address alignment. */
	IPC_PDU_ALIGN_ASSERT(INT8);
	IPC_PDU_ALIGN_ASSERT(INT16);
	IPC_PDU_ALIGN_ASSERT(UINT16);
	IPC_PDU_ALIGN_ASSERT(INT32);
	IPC_PDU_ALIGN_ASSERT(UINT32);
	IPC_PDU_ALIGN_ASSERT(INT64);
	IPC_PDU_ALIGN_ASSERT(UINT64);
	IPC_PDU_ALIGN_ASSERT(FLOAT);
	IPC_PDU_ALIGN_ASSERT(DOUBLE);
	IPC_PDU_ALIGN_ASSERT(IPV4);
	IPC_PDU_ALIGN_ASSERT(IPV6);

	/* Ensure that float is IEEE single precision floating point. */
	PFC_ASSERT(sizeof(float) == 4);

	/* Ensure that double is IEEE double precision floating point. */
	PFC_ASSERT(sizeof(double) == 8);

	/* Size must not be zero. */
	PFC_ASSERT(size != 0);

	return pfc_ipc_write(stream, &pdu->ip_data_UINT8, size, PFC_FALSE,
			     abstime);
}

/*
 * static int
 * ipc_pdu_write_ptr(ipc_stream_t *PFC_RESTRICT stp,
 *		     pfc_iostream_t PFC_RESTRICT stream,
 *		     ipc_pdu_t *PFC_RESTRICT pdu,
 *		     ctimespec_t *PFC_RESTRICT abstime)
 *	Write PDU data to the specified output stream.
 *	This function is used to write PDU data which uses ip_data_POINTER
 *	in ipc_pdu_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_pdu_write_ptr(ipc_stream_t *PFC_RESTRICT stp,
		  pfc_iostream_t PFC_RESTRICT stream,
		  ipc_pdu_t *PFC_RESTRICT pdu,
		  ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_pdutag_t	*tag = &pdu->ip_tag;
	uint32_t	size = tag->ipt_size;
	int		err;

	if (size != 0) {
		PFC_ASSERT(pdu->ip_data_POINTER != NULL);
		err = pfc_ipc_write(stream, pdu->ip_data_POINTER, size,
				    PFC_FALSE, abstime);
	}
	else {
		err = 0;
	}

	return err;
}

/*
 * static int
 * ipc_pdu_copy(ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT addr,
 *		uint8_t bflags)
 *	Copy data pointed by `addr' into the PDU data specified by `pdu'.
 *	This function does not implement byte-swapping code.
 *
 * Calling/Exit State:
 *	Zero is always returned.
 */
static int
ipc_pdu_copy(ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT addr,
	     uint8_t bflags)
{
	ipc_pdutag_t	*tag = &pdu->ip_tag;
	uint32_t	size = tag->ipt_size;

	memcpy(&pdu->ip_data_UINT8, addr, size);

	return 0;
}

/*
 * static int
 * ipc_pdu_copy_ptr(ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT addr,
 *		    uint8_t bflags)
 *	Copy data pointed by `addr' into the PDU data specified by `pdu'.
 *	This function is used to copy PDU which uses ip_data_POINTER field
 *	in ipc_pdu_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_pdu_copy_ptr(ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT addr,
		 uint8_t bflags)
{
	ipc_pdutag_t	*tag = &pdu->ip_tag;
	int		err = 0;

	if ((tag->ipt_flags & IPC_PDUF_NULL) || tag->ipt_size == 0) {
		/* NULL pointer, or zero-length binary data. */
		pdu->ip_data_POINTER = NULL;
	}
	else {
		void		*ptr;
		uint32_t	size = tag->ipt_size;

		/* Duplicate PDU data. */
		ptr = malloc(size);
		pdu->ip_data_POINTER = (pfc_cptr_t)ptr;
		if (PFC_EXPECT_FALSE(ptr == NULL)) {
			IPC_LOG_ERROR("No memory for PDU data: size=%u", size);
			err = ENOMEM;
		}
		else {
			memcpy(ptr, addr, size);
		}
	}

	return err;
}

/*
 * static int
 * ipc_pdu_copy_struct(ipc_pdu_t *PFC_RESTRICT pdu,
 *		       pfc_cptr_t PFC_RESTRICT addr, uint8_t bflags)
 *	Copy IPC structure data pointed by `addr' into the PDU data specified
 *	by `pdu'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_pdu_copy_struct(ipc_pdu_t *PFC_RESTRICT pdu, pfc_cptr_t PFC_RESTRICT addr,
		    uint8_t bflags)
{
	ipc_cstrinfo_t	*sip = IPC_STRINFO_PDUOPS2PTR(pdu->ip_ops);
	ipc_pdutag_t	*tag = &pdu->ip_tag;
	uint32_t	size = tag->ipt_size;
	void		*ptr;

	PFC_ASSERT(tag->ipt_type == PFC_IPCTYPE_STRUCT);
	PFC_ASSERT(size == IPC_STRINFO_NAMELEN(sip) + 2 + IPC_STRUCT_SIG_SIZE +
		   IPC_STRINFO_SIZE(sip));
	PFC_ASSERT(*((const uint8_t *)addr) == IPC_STRINFO_NAMELEN(sip) + 1);
	PFC_ASSERT(strcmp((const char *)addr + 1, IPC_STRINFO_NAME(sip)) == 0);
	PFC_ASSERT(memcmp((const char *)addr + IPC_STRINFO_NAMELEN(sip) + 2,
			  sip->sti_sig, IPC_STRUCT_SIG_SIZE) == 0);

	/* Duplicate PDU data. */
	ptr = malloc(size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		IPC_LOG_ERROR("No memory for structure data: name=%s, size=%u",
			      IPC_STRINFO_NAME(sip), size);

		/* Clear ip_data_POINTER field for destructor call. */
		pdu->ip_data_POINTER = NULL;

		return ENOMEM;
	}

	if (IPC_NEED_BSWAP_ANY(bflags)) {
		const uint8_t	*src = (const uint8_t *)addr;
		ipc_bswap_t	bswap;

		/* Copy struct data with byte swapping. */
		bswap.ibs_src = src + IPC_STRINFO_NAMELEN(sip) + 2 +
			IPC_STRUCT_SIG_SIZE;
		bswap.ibs_dst = ptr;
		bswap.ibs_flags = bflags;
		pfc_ipc_struct_bswap(&sip->sti_pduops, &bswap);
	}
	else {
		memcpy(ptr, addr, size);
	}

	pdu->ip_data_POINTER = ptr;

	return 0;
}

/*
 * static void
 * ipc_pdu_dtor_ptr(ipc_pdu_t *pdu)
 *	Destructor of PDU which used ip_data_POINTER in ipc_pdu_t.
 */
static void
ipc_pdu_dtor_ptr(ipc_pdu_t *pdu)
{
	free((void *)pdu->ip_data_POINTER);

	/*
	 * Destructor may be called twice or more.
	 * So we need to reset pointer.
	 */
	pdu->ip_data_POINTER = NULL;
}

/*
 * static void
 * ipc_pdu_bswap_struct(ipc_cpduops_t *PFC_RESTRICT ops,
 *			ipc_bswap_t *PFC_RESTRICT bsp, uint32_t nelems)
 *	Decode serialized STRUCT data with byte swapping.
 *
 *	Source and destination address must be specified by a pointer to
 *	ipc_deocde_t. bsp->ibs_src is a pointer to serialized STRUCT PDU data,
 *	and bsp->ibs_dst is a pointer to buffer to store the resulting data.
 *	bsp->ibs_src and bsp->ibs_dst is updated to the end boundary of
 *	decoded data.
 *
 *	`nelems' is the number of elements to be decoded. It must be greater
 *	than zero.
 *
 * Remarks:
 *	- The caller must ensure that the buffer pointed by bsp->ibs_dst has
 *	  enough bytes to store the PDU of this type.
 *
 *	- The caller must ensure that bsp->ibs_dst is properly aligned to
 *	  the boundary required by this type, which is set in ops->ipops_align.
 */
static void
ipc_pdu_bswap_struct(ipc_cpduops_t *PFC_RESTRICT ops,
		     ipc_bswap_t *PFC_RESTRICT bsp, uint32_t nelems)
{
	ipc_cstrinfo_t	*sip = IPC_STRINFO_PDUOPS2PTR(ops);
	const uint8_t	*sbase;
	uint8_t		*dbase, *dlimit;
	const uint32_t	size = ops->ipops_size;

	PFC_ASSERT(nelems > 0);

	sbase = bsp->ibs_src;
	dbase = bsp->ibs_dst;
	dlimit = dbase + (size * nelems);

	PFC_ASSERT(PFC_IS_POW2_ALIGNED(dbase, ops->ipops_align));

	do {
		ipc_cfldinfo_t	*flp, *flimit;

		flp = sip->sti_field;
		flimit = flp + sip->sti_nfields;
		PFC_ASSERT(flp < flimit);

		do {
			/* Decode struct field with byte swapping. */
			ipc_pdu_bswap_struct_field(flp, bsp);
			flp++;
		} while (flp < flimit);

		/* Adjust buffer addresses for next array element. */
		sbase += size;
		dbase += size;
		bsp->ibs_src = sbase;
		bsp->ibs_dst = dbase;
	} while (dbase < dlimit);
}

/*
 * static void
 * ipc_pdu_bswap_struct_field(ipc_cfldinfo_t *PFC_RESTRICT flp,
 *			      ipc_bswap_t *PFC_RESTRICT bsp)
 *	Decode serialized struct field data with byte swapping.
 *	This is internal function of ipc_pdu_bswap_struct().
 */
static void
ipc_pdu_bswap_struct_field(ipc_cfldinfo_t *PFC_RESTRICT flp,
			   ipc_bswap_t *PFC_RESTRICT bsp)
{
	ipc_cpduops_t	*fops = flp->fli_pduops;
	uint32_t	nelems = flp->fli_array;

	/* Adjust buffer addresses to the boundary required by this field. */
	pfc_ipc_bswap_align(bsp, fops->ipops_align);

	/* Decode struct field with byte swapping. */
	if (nelems == 0) {
		nelems = 1;
	}

	if (fops->ipops_bswap != NULL) {
		/* Decode serialized data with byte swapping. */
		fops->ipops_bswap(fops, bsp, nelems);
	}
	else {
		const uint8_t	*src = bsp->ibs_src;
		uint8_t		*dst = bsp->ibs_dst;
		const uint32_t	size = fops->ipops_size * nelems;

		/* This type does not require byte swapping. */
		memcpy(dst, src, size);

		bsp->ibs_src = src + size;
		bsp->ibs_dst = dst + size;
	}
}
