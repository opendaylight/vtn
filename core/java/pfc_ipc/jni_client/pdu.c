/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * pdu.c - IPC protocol data unit management.
 *
 * Remarks:
 *	This file should contain client-independent code for future expansion.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <ipc_impl.h>
#include "ipcclnt_jni.h"
#include "ipc_struct_jni.h"
#include <org_opendaylight_vtn_core_ipc_IpcDataUnit.h>

/*
 * String representation of IPC data types.
 * This is used to show data type in error message.
 */
static const char	*jipc_pdu_names[] = {
	"INT8",
	"UINT8",
	"INT16",
	"UINT16",
	"INT32",
	"UINT32",
	"INT64",
	"UINT64",
	"FLOAT",
	"DOUBLE",
	"IPV4",
	"IPV6",
	"STRING",
	"BINARY",
	"NULL",
};

static const char	jipc_pdu_name_struct[] = "STRUCT";

/*
 * Empty binary image.
 */
static const uint8_t	jipc_pdu_empty_binary[0];

/*
 * PDU operations.
 */
typedef struct {
	/*
	 * Expected data size in bytes.
	 * Zero means variable sized data.
	 */
	size_t		jpops_size;

	/*
	 * jobject
	 * jpops_getmsg(JNIEnv *PFC_RESTRICT env, ipc_msg_t *PFC_RESTRICT msg,
	 *		uint32_t index, ipc_pdutag_t *PFC_RESTRICT tag)
	 *	Construct a new object which represents an IPC protocol data
	 *	unit received via IPC connection.
	 *
	 *	`tag' must be a pointer to PDU tag specified by the PDU index
	 *	`index'.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, a non-NULL pointer to PDU object
	 *	is returned.
	 *	Otherwise an exception is thrown and NULL is returned.
	 */
	jobject		(*jpops_getmsg)(JNIEnv *PFC_RESTRICT env,
					ipc_msg_t *PFC_RESTRICT msg,
					uint32_t index,
					ipc_pdutag_t *PFC_RESTRICT tag);

	/*
	 * jobject
	 * jpops_ctor(JNIEnv *PFC_RESTRICT env,
	 *	      const uint8_t *PFC_RESTRICT addr, uint32_t size)
	 *	Construct a new object which represents an IPC protocol data
	 *	unit.
	 *
	 *	`addr' must be a pointer to a buffer which keeps data, and
	 *	`size' must be its length in bytes.
	 *	Note that data in the buffer pointed by `addr' must be in
	 *	host byte order.
	 *
	 *	This method is used to create PDU instance associated with
	 *	structure field. So variable-length type and NULL does not
	 *	implement this method.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, a non-NULL pointer to PDU object
	 *	is returned.
	 *	Otherwise an exception is thrown and NULL is returned.
	 */
	jobject		(*jpops_ctor)(JNIEnv *PFC_RESTRICT env,
				      const uint8_t *PFC_RESTRICT addr,
				      uint32_t size);
} jipc_pduops_t;

typedef const jipc_pduops_t	jipc_cpduops_t;

/*
 * Signatures of PDU class constructors.
 */
#define	JIPC_PDUSIG_IpcInt8	PJNI_SIG_METHOD1(void, byte)
#define	JIPC_PDUSIG_IpcInt16	PJNI_SIG_METHOD1(void, short)
#define	JIPC_PDUSIG_IpcInt32	PJNI_SIG_METHOD1(void, int)
#define	JIPC_PDUSIG_IpcInt64	PJNI_SIG_METHOD1(void, long)
#define	JIPC_PDUSIG_IpcUint8	PJNI_SIG_METHOD1(void, byte)
#define	JIPC_PDUSIG_IpcUint16	PJNI_SIG_METHOD1(void, short)
#define	JIPC_PDUSIG_IpcUint32	PJNI_SIG_METHOD1(void, int)
#define	JIPC_PDUSIG_IpcUint64	PJNI_SIG_METHOD1(void, long)
#define	JIPC_PDUSIG_IpcFloat	PJNI_SIG_METHOD1(void, float)
#define	JIPC_PDUSIG_IpcDouble	PJNI_SIG_METHOD1(void, double)
#define	JIPC_PDUSIG_IpcString	PJNI_SIG_METHOD1(void, String)
#define	JIPC_PDUSIG_IpcBinary	PJNI_SIG_METHOD2(void, jbyteArray, boolean)
#define	JIPC_PDUSIG_IpcNull	PJNI_SIG_METHOD0(void)

#define	JIPC_PDUSIG_IpcString_NULL	PJNI_SIG_METHOD0(void)
#define	JIPC_PDUSIG_IpcBinary_NULL	PJNI_SIG_METHOD0(void)

/*
 * Signature of IpcInetAddress.create(byte[]), which is a factory method
 * of IpcInetAddress.
 */
#define	JIPC_FACSIG_IpcInetAddress			\
	PJNI_SIG_METHOD1(IpcInetAddress, jbyteArray)

/*
 * Obtain address to received data.
 * The address is set to `addr'.
 */
#define	JIPC_PDU_GETADDR(msg, tag, addr, argtype)		\
	do {							\
		PFC_ASSERT((msg)->im_data != NULL);		\
		(addr) = (const argtype *)((msg)->im_data +	\
					   (tag)->ipt_off);	\
		PFC_ASSERT((const uint8_t *)((addr) + 1) <=	\
			   IPC_MSG_DATA_LIMIT(msg));		\
	} while (0)

#define	JIPC_PDU_FUNCNAME(method, clname)	jipc_pdu_##method##_##clname

/*
 * Exception table for receiving IPC structure.
 */
static jipc_cexcept_t	extable_struct[] = {
	JIPC_EXCEPT_DECL(ENODEV, IpcUnknownStructException),
	JIPC_EXCEPT_DECL(EBADMSG, IpcStructLayoutMismatchException),
	JIPC_EXCEPT_DECL(EPROTO, IpcBadProtocolException),
	JIPC_EXCEPT_END(),
};

/*
 * Declare constructor of PDU object.
 */
#define	JIPC_PDU_FUNC_DECL(clname, argtype, pdutype)			\
	static jobject							\
	JIPC_PDU_FUNCNAME(getmsg, clname)(JNIEnv *PFC_RESTRICT env,	\
					  ipc_msg_t *PFC_RESTRICT msg,	\
					  uint32_t index PFC_ATTR_UNUSED, \
					  ipc_pdutag_t *PFC_RESTRICT tag) \
	{								\
		const argtype	*addr;					\
		argtype		data;					\
									\
		PFC_ASSERT(tag->ipt_type == PFC_IPCTYPE_##pdutype);	\
									\
		/* Determine address of the data. */			\
		JIPC_PDU_GETADDR(msg, tag, addr, argtype);		\
									\
		/* Fetch value. */					\
		IPC_FETCH_INT(data, addr);				\
									\
		if (IPC_NEED_BSWAP(msg->im_bflags)) {			\
			/* Byte swapping is needed. */			\
			IPC_BSWAP(data);				\
		}							\
									\
		return pjni_newobject(env, PJNI_CLASS_##clname,		\
				      JIPC_PDUSIG_##clname, data);	\
	}								\
									\
	static jobject							\
	JIPC_PDU_FUNCNAME(ctor, clname)(JNIEnv *PFC_RESTRICT env,	\
					const uint8_t *PFC_RESTRICT addr, \
					uint32_t size)			\
	{								\
		const argtype	*ptr = (const argtype *)addr;		\
		argtype		data;					\
									\
		PFC_ASSERT(size == sizeof(argtype));			\
									\
		/* Fetch value. */					\
		IPC_FETCH_INT(data, ptr);				\
									\
		return pjni_newobject(env, PJNI_CLASS_##clname,		\
				      JIPC_PDUSIG_##clname, data);	\
	}

JIPC_PDU_FUNC_DECL(IpcInt8, int8_t, INT8);
JIPC_PDU_FUNC_DECL(IpcUint8, uint8_t, UINT8);
JIPC_PDU_FUNC_DECL(IpcInt16, int16_t, INT16);
JIPC_PDU_FUNC_DECL(IpcUint16, uint16_t, UINT16);
JIPC_PDU_FUNC_DECL(IpcInt32, int32_t, INT32);
JIPC_PDU_FUNC_DECL(IpcUint32, uint32_t, UINT32);
JIPC_PDU_FUNC_DECL(IpcInt64, int64_t, INT64);
JIPC_PDU_FUNC_DECL(IpcUint64, uint64_t, UINT64);

#define	JIPC_PDU_FUNC_FLOAT_DECL(clname, argtype, pdutype, stubtype)	\
	static jobject							\
	JIPC_PDU_FUNCNAME(getmsg, clname)(JNIEnv *PFC_RESTRICT env,	\
					  ipc_msg_t *PFC_RESTRICT msg,	\
					  uint32_t index PFC_ATTR_UNUSED, \
					  ipc_pdutag_t *PFC_RESTRICT tag) \
	{								\
		const argtype	*addr;					\
		union {							\
			argtype		data;				\
			stubtype	stub;				\
		} value;						\
									\
		PFC_ASSERT((void *)&value.data == (void *)&value.stub);	\
		PFC_ASSERT(tag->ipt_type == PFC_IPCTYPE_##pdutype);	\
									\
		/* Determine address of the data. */			\
		JIPC_PDU_GETADDR(msg, tag, addr, argtype);		\
									\
		/* Fetch value. */					\
		IPC_FETCH_INT(value.data, addr);			\
									\
		if (IPC_NEED_BSWAP_FLOAT(msg->im_bflags)) {		\
			/* Byte swapping is needed. */			\
			IPC_BSWAP(value.stub);				\
		}							\
									\
		return pjni_newobject(env, PJNI_CLASS_##clname,		\
				      JIPC_PDUSIG_##clname, value.data); \
	}								\
									\
	static jobject							\
	JIPC_PDU_FUNCNAME(ctor, clname)(JNIEnv *PFC_RESTRICT env,	\
					const uint8_t *PFC_RESTRICT addr, \
					uint32_t size)			\
	{								\
		const argtype	*ptr = (const argtype *)addr;		\
		argtype		data;					\
									\
		PFC_ASSERT(size == sizeof(argtype));			\
									\
		/* Fetch value. */					\
		IPC_FETCH_INT(data, ptr);				\
									\
		return pjni_newobject(env, PJNI_CLASS_##clname,		\
				      JIPC_PDUSIG_##clname, data);	\
	}

JIPC_PDU_FUNC_FLOAT_DECL(IpcFloat, float, FLOAT, uint32_t);
JIPC_PDU_FUNC_FLOAT_DECL(IpcDouble, double, DOUBLE, uint64_t);

/*
 * static jobject
 * jipc_pdu_ctor_ipaddr(JNIEnv *PFC_RESTRICT env,
 *			const uint8_t *PFC_RESTRICT addr, uint32_t size)
 *	Create a new IpcInetAddress object which represents data in the
 *	specified buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_ctor_ipaddr(JNIEnv *PFC_RESTRICT env,
		     const uint8_t *PFC_RESTRICT addr, uint32_t size)
{
	jobject		obj;
	jbyteArray	jarray;

	PFC_ASSERT(size == sizeof(struct in_addr) ||
		   size == sizeof(struct in6_addr));

	/* Create a new byte array which represents a raw IP address. */
	jarray = pjni_bytearray_new(env, (const jbyte *)addr, size);
	if (PFC_EXPECT_FALSE(jarray == NULL)) {
		return NULL;
	}

	/* Create an IpcInetAddress object. */
	obj = pjni_stcall_object(env, PJNI_CLASS(IpcInetAddress), "create",
				 JIPC_FACSIG_IpcInetAddress,jarray);
	if (PJNI_EXCEPTION_CHECK(env)) {
		obj = NULL;
		/* FALLTHROUGH */
	}
	else {
		PFC_ASSERT(obj != NULL);
	}

	(*env)->DeleteLocalRef(env, jarray);

	return obj;
}

/*
 * static jobject
 * jipc_pdu_getmsg_ipaddr(JNIEnv *PFC_RESTRICT env, ipc_msg_t *PFC_RESTRICT msg,
 *			  uint32_t PFC_ATTR_UNUSED index,
 *			  ipc_pdutag_t *PFC_RESTRICT tag)
 *	Create a new IpcInetAddress object which represents the specified
 *	additional data.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_getmsg_ipaddr(JNIEnv *PFC_RESTRICT env, ipc_msg_t *PFC_RESTRICT msg,
		       uint32_t PFC_ATTR_UNUSED index,
		       ipc_pdutag_t *PFC_RESTRICT tag)
{
	const uint8_t	*addr;
	uint32_t	size = tag->ipt_size;

	PFC_ASSERT((tag->ipt_type == PFC_IPCTYPE_IPV4 &&	\
		    size == sizeof(struct in_addr)) ||		\
		   (tag->ipt_type == PFC_IPCTYPE_IPV6 &&	\
		    size == sizeof(struct in6_addr)));

	/* Determine address of the data. */
	addr = (const uint8_t *)msg->im_data + tag->ipt_off;
	PFC_ASSERT(addr + size <= IPC_MSG_DATA_LIMIT(msg));

	return jipc_pdu_ctor_ipaddr(env, addr, size);
}

#define	jipc_pdu_getmsg_IpcInet4Address	jipc_pdu_getmsg_ipaddr
#define	jipc_pdu_getmsg_IpcInet6Address	jipc_pdu_getmsg_ipaddr
#define	jipc_pdu_ctor_IpcInet4Address	jipc_pdu_ctor_ipaddr
#define	jipc_pdu_ctor_IpcInet6Address	jipc_pdu_ctor_ipaddr
#define	jipc_pdu_ctor_IpcString		NULL
#define	jipc_pdu_ctor_IpcBinary		NULL
#define	jipc_pdu_ctor_IpcNull		NULL

/*
 * static jobject
 * jipc_pdu_ctor_string(JNIEnv *PFC_RESTRICT env,
 *			const uint8_t *PFC_RESTRICT addr, uint32_t size)
 *	Create a new IpcString object which represents data in the
 *	specified buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_ctor_string(JNIEnv *PFC_RESTRICT env,
		     const uint8_t *PFC_RESTRICT addr, uint32_t size)
{
	jobject		obj = NULL;
	jstring		jstr;

	if (addr == NULL) {
		PFC_ASSERT(size == 0);

		/* Construct an IpcString object which represents NULL. */
		return pjni_newobject(env, PJNI_CLASS(IpcString),
				      JIPC_PDUSIG_IpcString_NULL);
	}
	else {
		PFC_ASSERT(size != 0);
		PFC_ASSERT(*(addr + (size - 1)) == '\0');
	}

	/* Create a String object. */
	jstr = pjni_newstring(env, (const char *)addr);
	if (PFC_EXPECT_FALSE(jstr == NULL)) {
		return NULL;
	}

	/* Construct an IpcString object. */
	obj = pjni_newobject(env, PJNI_CLASS(IpcString), JIPC_PDUSIG_IpcString,
			     jstr);
	(*env)->DeleteLocalRef(env, jstr);

	return obj;
}

/*
 * static jobject
 * jipc_pdu_getmsg_IpcString(JNIEnv *PFC_RESTRICT env,
 *			     ipc_msg_t *PFC_RESTRICT msg,
 *			     uint32_t PFC_ATTR_UNUSED index,
 *			     ipc_pdutag_t *PFC_RESTRICT tag)
 *	Create a new IpcString object which represents the specified
 *	additional data.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_getmsg_IpcString(JNIEnv *PFC_RESTRICT env,
			  ipc_msg_t *PFC_RESTRICT msg,
			  uint32_t PFC_ATTR_UNUSED index,
			  ipc_pdutag_t *PFC_RESTRICT tag)
{
	const uint8_t	*addr;
	uint32_t	size = tag->ipt_size;

	PFC_ASSERT(tag->ipt_type == PFC_IPCTYPE_STRING);

	if (tag->ipt_flags & IPC_PDUF_NULL) {
		/* NULL pointer. */
		addr = NULL;
	}
	else {
		/* Determine address of the data. */
		PFC_ASSERT(msg->im_data != NULL);
		addr = (const uint8_t *)msg->im_data + tag->ipt_off;

		/* Ensure that the string is terminated by '\0'. */
		if (PFC_EXPECT_FALSE(size == 0)) {
			pjni_throw(env, PJNI_CLASS(IpcBadProtocolException),
				   "Empty STRING in IPC message.");

			return NULL;
		}

		if (PFC_EXPECT_FALSE(*(addr + (size - 1)) != '\0')) {
			pjni_throw(env, PJNI_CLASS(IpcBadProtocolException),
				   "Broken STRING in IPC message.");

			return NULL;
		}

		PFC_ASSERT(addr + size <= IPC_MSG_DATA_LIMIT(msg));
	}

	return jipc_pdu_ctor_string(env, addr, size);
}

/*
 * static jobject
 * jipc_pdu_ctor_binary(JNIEnv *PFC_RESTRICT env,
 *			const uint8_t *PFC_RESTRICT addr, uint32_t size)
 *	Create a new IpcBinary object which represents data in the
 *	specified buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_ctor_binary(JNIEnv *PFC_RESTRICT env,
		     const uint8_t *PFC_RESTRICT addr, uint32_t size)
{
	jobject		obj;
	jbyteArray	jarray;

	if (addr == NULL) {
		/* Construct an IpcBinary object which represents NULL. */
		return pjni_newobject(env, PJNI_CLASS(IpcBinary),
				      JIPC_PDUSIG_IpcBinary_NULL);
	}

	/* Create a new byte array which represents a binary image. */
	jarray = pjni_bytearray_new(env, (const jbyte *)addr, size);
	if (PFC_EXPECT_FALSE(jarray == NULL)) {
		return NULL;
	}

	/* Construct an IpcBinary object. */
	obj = pjni_newobject(env, PJNI_CLASS(IpcBinary),
			     JIPC_PDUSIG_IpcBinary, jarray, JNI_TRUE);
	(*env)->DeleteLocalRef(env, jarray);

	return obj;
}

/*
 * static jobject
 * jipc_pdu_getmsg_IpcBinary(JNIEnv *PFC_RESTRICT env,
 *			     ipc_msg_t *PFC_RESTRICT msg,
 *			     uint32_t PFC_ATTR_UNUSED index,
 *			     ipc_pdutag_t *PFC_RESTRICT tag)
 *	Create a new IpcBinary object which represents the specified
 *	additional data.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_getmsg_IpcBinary(JNIEnv *PFC_RESTRICT env,
			  ipc_msg_t *PFC_RESTRICT msg,
			  uint32_t PFC_ATTR_UNUSED index,
			  ipc_pdutag_t *PFC_RESTRICT tag)
{
	const uint8_t	*addr;
	uint32_t	size = tag->ipt_size;

	PFC_ASSERT(tag->ipt_type == PFC_IPCTYPE_BINARY);

	if (tag->ipt_flags & IPC_PDUF_NULL) {
		/* NULL pointer. */
		addr = NULL;
	}
	else if (size == 0) {
		/* Empty byte array. */
		addr = jipc_pdu_empty_binary;
	}
	else {
		/* Determine address of the data. */
		PFC_ASSERT(msg->im_data != NULL);
		addr = (const uint8_t *)msg->im_data + tag->ipt_off;
		PFC_ASSERT(addr + size <= IPC_MSG_DATA_LIMIT(msg));
	}

	return jipc_pdu_ctor_binary(env, addr, size);
}

/*
 * static jobject
 * jipc_pdu_getmsg_IpcNull(JNIEnv *PFC_RESTRICT env,
 *			   ipc_msg_t *PFC_RESTRICT msg,
 *			   uint32_t PFC_ATTR_UNUSED index,
 *			   ipc_pdutag_t *PFC_RESTRICT tag)
 *	Create a new IpcNull object which represents the specified
 *	additional data.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_getmsg_IpcNull(JNIEnv *PFC_RESTRICT env,
			ipc_msg_t *PFC_RESTRICT msg,
			uint32_t PFC_ATTR_UNUSED index,
			ipc_pdutag_t *PFC_RESTRICT tag)
{
	PFC_ASSERT(tag->ipt_type == PFC_IPCTYPE_NULL);
	PFC_ASSERT(tag->ipt_flags & IPC_PDUF_NULL);
	PFC_ASSERT(tag->ipt_size == 0);

	return pjni_newobject(env, PJNI_CLASS(IpcNull), JIPC_PDUSIG_IpcNull);
}

/*
 * static jobject
 * jipc_pdu_getmsg_IpcStruct(JNIEnv *PFC_RESTRICT env,
 *			     ipc_msg_t *PFC_RESTRICT msg, uint32_t index)
 *	Create a new IpcStruct object which represents the specified additional
 *	data.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jobject
jipc_pdu_getmsg_IpcStruct(JNIEnv *PFC_RESTRICT env,
			  ipc_msg_t *PFC_RESTRICT msg, uint32_t index)
{
	jobject		obj;
	jstring		jname;
	ipc_strinfo_t	*sip;
	ipc_strbuf_t	*sbp;
	int		err;

	/* Fetch IPC structure. */
	err = pfc_ipcmsg_fetch_struct(msg, index, (ipc_cstrinfo_t **)&sip,
				      &sbp, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_struct,
			   "Failed to fetch IPC structure: err=%d", err);

		return NULL;
	}

	/*
	 * Create a String object which represents the name of received
	 * IPC structure.
	 */
	jname = pjni_newstring(env, IPC_STRINFO_NAME(sip));
	if (PFC_EXPECT_FALSE(jname == NULL)) {
		goto error;
	}

	/* Construct an IpcStruct object. */
	obj = pjni_newobject(env, PJNI_CLASS(IpcStruct),
			     JIPC_CTORSIG_IpcStruct, jname,
			     JIPC_STRBUF_HANDLE(sbp), 0,
			     JIPC_STRINFO_HANDLE(sip));
	(*env)->DeleteLocalRef(env, jname);

	return obj;

error:
	IPC_STRINFO_RELEASE(sip);
	IPC_STRBUF_RELEASE(sbp);

	return NULL;
}

/*
 * Declare PDU operations.
 */
#define	JIPC_PDUOPS_DECL_IMPL(clname, size)				\
	{								\
		.jpops_size	= (size),				\
		.jpops_getmsg	= JIPC_PDU_FUNCNAME(getmsg, clname),	\
		.jpops_ctor	= JIPC_PDU_FUNCNAME(ctor, clname),	\
	}

#define	JIPC_PDUOPS_DECL(clname, type)			\
	JIPC_PDUOPS_DECL_IMPL(clname, sizeof(type))

#define	JIPC_PDUOPS_VARSIZE_DECL(clname)	\
	JIPC_PDUOPS_DECL_IMPL(clname, 0)

/*
 * Collection of PDU operations, excluding STRUCT type.
 * Array index must be identical to pfc_ipctype_t.
 */
static jipc_cpduops_t	jipc_pdu_ops[] = {
	JIPC_PDUOPS_DECL(IpcInt8, int8_t),	/* PFC_IPCTYPE_INT8 */
	JIPC_PDUOPS_DECL(IpcUint8, uint8_t),	/* PFC_IPCTYPE_UINT8 */
	JIPC_PDUOPS_DECL(IpcInt16, int16_t),	/* PFC_IPCTYPE_INT16 */
	JIPC_PDUOPS_DECL(IpcUint16, uint16_t),	/* PFC_IPCTYPE_UINT16 */
	JIPC_PDUOPS_DECL(IpcInt32, int32_t),	/* PFC_IPCTYPE_INT32 */
	JIPC_PDUOPS_DECL(IpcUint32, uint32_t),	/* PFC_IPCTYPE_UINT32 */
	JIPC_PDUOPS_DECL(IpcInt64, int64_t),	/* PFC_IPCTYPE_INT64 */
	JIPC_PDUOPS_DECL(IpcUint64, uint64_t),	/* PFC_IPCTYPE_UINT64 */
	JIPC_PDUOPS_DECL(IpcFloat, float),	/* PFC_IPCTYPE_FLOAT */
	JIPC_PDUOPS_DECL(IpcDouble, double),	/* PFC_IPCTYPE_DOUBLE */

	/* PFC_IPCTYPE_IPV4 */
	JIPC_PDUOPS_DECL(IpcInet4Address, struct in_addr),

	/* PFC_IPCTYPE_IPV6 */
	JIPC_PDUOPS_DECL(IpcInet6Address, struct in6_addr),

	JIPC_PDUOPS_VARSIZE_DECL(IpcString),	/* PFC_IPCTYPE_STRING */
	JIPC_PDUOPS_VARSIZE_DECL(IpcBinary),	/* PFC_IPCTYPE_BINARY */
	JIPC_PDUOPS_VARSIZE_DECL(IpcNull),	/* PFC_IPCTYPE_NULL */
};

/*
 * PDU type assertion.
 */
#define	JIPC_PDUTYPE_ASSERT(type)					\
	PFC_ASSERT((uint32_t)org_opendaylight_vtn_core_ipc_IpcDataUnit_##type == \
		   (uint32_t)PFC_IPCTYPE_##type)

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcDataUnit_getTypeName(
 *	JNIEnv *env, jclass cls, jint type)
 *
 *	Return a string representation of the given data type identifier.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcDataUnit_getTypeName(
	JNIEnv *env, jclass cls, jint type)
{
	const char	*name;
	char		buf[JIPC_TYPEBUF_SIZE];

	name = jipc_pdu_typename(type, buf, sizeof(buf));

	return pjni_newstring(env, name);
}

/*
 * jobject PFC_ATTR_HIDDEN
 * jipc_pdu_getmsg(JNIEnv *PFC_RESTRICT env, ipc_msg_t *PFC_RESTRICT msg,
 *		uint32_t index)
 *	Construct a new Java object which represents a received IPC data
 *	specified by `msg' and `index'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
jobject PFC_ATTR_HIDDEN
jipc_pdu_getmsg(JNIEnv *PFC_RESTRICT env, ipc_msg_t *PFC_RESTRICT msg,
		uint32_t index)
{
	ipc_pduidx_t	*pdu;
	ipc_pdutag_t	*tag;
	jipc_cpduops_t	*ops;
	uint32_t	reqsize;

	JIPC_PDUTYPE_ASSERT(INT8);
	JIPC_PDUTYPE_ASSERT(UINT8);
	JIPC_PDUTYPE_ASSERT(INT16);
	JIPC_PDUTYPE_ASSERT(UINT16);
	JIPC_PDUTYPE_ASSERT(INT32);
	JIPC_PDUTYPE_ASSERT(UINT32);
	JIPC_PDUTYPE_ASSERT(INT64);
	JIPC_PDUTYPE_ASSERT(UINT64);
	JIPC_PDUTYPE_ASSERT(FLOAT);
	JIPC_PDUTYPE_ASSERT(DOUBLE);
	JIPC_PDUTYPE_ASSERT(IPV4);
	JIPC_PDUTYPE_ASSERT(IPV6);
	JIPC_PDUTYPE_ASSERT(STRING);
	JIPC_PDUTYPE_ASSERT(BINARY);
	JIPC_PDUTYPE_ASSERT(NULL);
	JIPC_PDUTYPE_ASSERT(STRUCT);

	/* Determine PDU index. */
	PFC_ASSERT(msg != NULL);
	pdu = pfc_ipcmsg_getpdu(msg, index);
	if (PFC_EXPECT_FALSE(pdu == NULL)) {
		pjni_throw(env, PJNI_CLASS(IpcDataIndexOutOfBoundsException),
			   "Invalid data index: %u", index);

		return NULL;
	}

	/* Determine PDU operation. */
	tag = &pdu->ipi_tag;
	if (tag->ipt_type == PFC_IPCTYPE_STRUCT) {
		/* Fetch an IPC structure. */
		return jipc_pdu_getmsg_IpcStruct(env, msg, index);
	}

	ops = &jipc_pdu_ops[tag->ipt_type];
	if (PFC_EXPECT_FALSE(ops >= PFC_ARRAY_LIMIT(jipc_pdu_ops))) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IpcBadProtocolException),
			   "Unexpected data type: %u", tag->ipt_type);

		return NULL;
	}

	/* Ensure that data size is valid. */
	reqsize = ops->jpops_size;
	if (PFC_EXPECT_FALSE(reqsize != 0 && reqsize != tag->ipt_size)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IpcBadProtocolException),
			   "Unexpected data size: %u != %u",
			   tag->ipt_size, reqsize);

		return NULL;
	}

	/* Create a new object which keeps the specified additional data. */
	return ops->jpops_getmsg(env, msg, index, tag);
}

/*
 * jobject PFC_ATTR_HIDDEN
 * jipc_pdu_create(JNIEnv *PFC_RESTRICT env, pfc_ipctype_t type,
 *		   const uint8_t *PFC_RESTRICT addr, uint32_t size)
 *	Construct a new Java object which represents an IPC protocol data unit.
 *
 *	`type' is a PDU type which determines data type.
 *	`addr' must be a pointer to a buffer which keeps data.
 *	`size' must be the number of available bytes in the buffer pointed
 *	by `addr'.
 *
 *	Note that data in the buffer pointed by `addr' must be in host byte
 *	order.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to PDU object is
 *	returned.
 *	Otherwise an exception is thrown and NULL is returned.
 *
 * Remarks:
 *	- This function supports only PDU types whose size is fixed to
 *	  non-zero value. So the following types must not be specified
 *	  to `type'.
 *	  + PFC_IPCTYPE_STRING
 *	  + PFC_IPCTYPE_BINARY
 *	  + PFC_IPCTYPE_NULL
 *	  + PFC_IPCTYPE_STRUCT
 *
 *	- This function throws a runtime exception instead of an IpcException
 *	  on error.
 */
jobject PFC_ATTR_HIDDEN
jipc_pdu_create(JNIEnv *PFC_RESTRICT env, pfc_ipctype_t type,
		const uint8_t *PFC_RESTRICT addr, uint32_t size)
{
	jipc_cpduops_t	*ops;
	uint32_t	reqsize;

	ops = &jipc_pdu_ops[(uint32_t)type];
	if (PFC_EXPECT_FALSE((uint32_t)type >=
			     PFC_ARRAY_CAPACITY(jipc_pdu_ops) ||
			     ops->jpops_ctor == NULL)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unexpected data type: %u", type);

		return NULL;
	}

	reqsize = ops->jpops_size;
	PFC_ASSERT(reqsize != 0);

	/* Buffer overflow check. */
	if (PFC_EXPECT_FALSE(size < reqsize)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Buffer overflow: size=%u, remains=%u",
			   reqsize, size);

		return NULL;
	}

	/* Create a new object which keeps the specified data. */
	return ops->jpops_ctor(env, addr, reqsize);
}

/*
 * const char PFC_ATTR_HIDDEN *
 * jipc_pdu_typename(pfc_ipctype_t type, char *buf, size_t size)
 *	Return a string representation of the specified PDU type.
 *
 *	`buf' is used to store a string if `type' is unknown value.
 *	The buffer pointed by `buf' must be at least `size' bytes long.
 */
const char PFC_ATTR_HIDDEN *
jipc_pdu_typename(pfc_ipctype_t type, char *buf, size_t size)
{
	if (type == PFC_IPCTYPE_STRUCT) {
		return jipc_pdu_name_struct;
	}

	if (PFC_EXPECT_FALSE((uint32_t)type >=
			     PFC_ARRAY_CAPACITY(jipc_pdu_names))) {
		snprintf(buf, size, "unknown:%u", type);

		return buf;
	}

	return jipc_pdu_names[(uint32_t)type];
}
