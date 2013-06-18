/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * struct.c - IPC structure management.
 *
 * Remarks:
 *	This file should contain client-independent code for future expansion.
 */

#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <org_opendaylight_vtn_core_ipc_IpcStruct.h>
#include "ipcclnt_jni.h"
#include "ipc_struct_jni.h"

/*
 * Internal prototypes.
 */
static void	jipc_struct_init(JNIEnv *PFC_RESTRICT env, jobject this,
				 const char *PFC_RESTRICT name);
static void	jipc_struct_unknown(JNIEnv *PFC_RESTRICT env, int err,
				    const char *PFC_RESTRICT name)
	PFC_FATTR_NOINLINE;
static void	jipc_struct_badtype(JNIEnv *PFC_RESTRICT env,
				    const char *PFC_RESTRICT name,
				    pfc_ipctype_t reqtype, pfc_ipctype_t type)
	PFC_FATTR_NOINLINE;
static void	jipc_struct_badtype2(JNIEnv *PFC_RESTRICT env,
				    const char *PFC_RESTRICT name,
				     pfc_ipctype_t reqtype1,
				     pfc_ipctype_t reqtype2,
				     pfc_ipctype_t type) PFC_FATTR_NOINLINE;
static uint32_t	jipc_struct_getscalar(JNIEnv *PFC_RESTRICT env, jstring jname,
				      ipc_strbuf_t *PFC_RESTRICT sbp,
				      jint base,
				      ipc_strinfo_t *PFC_RESTRICT sip,
				      jint index,
				      ipc_fldmeta_t **PFC_RESTRICT fmpp);
static uint32_t	jipc_struct_getarray(JNIEnv *PFC_RESTRICT env, jstring jname,
				     ipc_strbuf_t *PFC_RESTRICT sbp,
				     jint base,
				     ipc_strinfo_t *PFC_RESTRICT sip,
				     ipc_fldmeta_t **PFC_RESTRICT fmpp);
static jobject	jipc_struct_newinner(JNIEnv *PFC_RESTRICT env,
				     ipc_strbuf_t *PFC_RESTRICT sbp,
				     uint32_t offset,
				     ipc_fldmeta_t *PFC_RESTRICT fmp,
				     jboolean deep);
static jobject	jipc_struct_newdeep(JNIEnv *PFC_RESTRICT env,
				    ipc_strbuf_t *PFC_RESTRICT sbp,
				    uint32_t offset, jstring jname,
				    ipc_strinfo_t *PFC_RESTRICT sip);
static jobject	jipc_struct_newshallow(JNIEnv *PFC_RESTRICT env,
				       ipc_strbuf_t *PFC_RESTRICT sbp,
				       uint32_t offset, jstring jname,
				       ipc_strinfo_t *PFC_RESTRICT sip);
static jobject	jipc_struct_newfield(JNIEnv *PFC_RESTRICT env, jstring jstname,
				     ipc_strinfo_t *PFC_RESTRICT sip,
				     jstring jname);
static jobjectArray	jipc_struct_getfieldnames(JNIEnv *PFC_RESTRICT env,
						  ipc_strinfo_t *PFC_RESTRICT
						  sip);

/*
 * static inline ipc_fldmeta_t PFC_FATTR_ALWAYS_INLINE *
 * jipc_struct_field_lookup(JNIEnv *PFC_RESTRICT env,
 *			    ipc_strinfo_t *PFC_RESTRICT sip,
 *			    const char *PFC_RESTRICT name)
 *	Search for a structure field associated with the specified field
 *	name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_fldmeta_t is
 *	returned.
 *	Otherwise NULL is returned with throwing an IllegalArgumentException.
 */
static inline ipc_fldmeta_t PFC_FATTR_ALWAYS_INLINE *
jipc_struct_field_lookup(JNIEnv *PFC_RESTRICT env,
			 ipc_strinfo_t *PFC_RESTRICT sip,
			 const char *PFC_RESTRICT name)
{
	ipc_fldmeta_t	*fmp;

	fmp = pfc_ipc_strinfo_getfield(sip, name);
	if (PFC_EXPECT_FALSE(fmp == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Unknown field name: %s", name);
		/* FALLTHROUGH */
	}

	return fmp;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_struct_checktype(JNIEnv *PFC_RESTRICT env,
 *			 ipc_fldmeta_t *PFC_RESTRICT fmp, pfc_ipctype_t type)
 *	Determine whether data type of the specified field is `type' or not.
 *	Note that `type' must not be PFC_IPCTYPE_STRUCT.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if data type of `fmp' is equal to `type'.
 *	Otherwise PFC_FALSE is returned with throwing an
 *	IllegalArgumentException.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_struct_checktype(JNIEnv *PFC_RESTRICT env,
		      ipc_fldmeta_t *PFC_RESTRICT fmp, pfc_ipctype_t type)
{
	if (PFC_EXPECT_TRUE(!IPC_FLDMETA_IS_STRUCT(fmp) &&
			    IPC_FLDMETA_TYPE(fmp) == type)) {
		return PFC_TRUE;
	}

	/* Invalid data type. */
	jipc_struct_badtype(env, IPC_FLDMETA_NAME(fmp), type,
			    IPC_FLDMETA_GETTYPE(fmp));

	return PFC_FALSE;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_struct_checktype2(JNIEnv *PFC_RESTRICT env,
 *			   ipc_fldmeta_t *PFC_RESTRICT fmp,
 *			   pfc_ipctype_t type1, pfc_ipctype_t type2)
 *	Determine whether data type of the specified field is either `type1'
 *	or `type2'.
 *	Note that both `type1' and `type2' must not be PFC_IPCTYPE_STRUCT.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if data type of `fmp' is equal to either `type1'
 *	or `type2'.
 *	Otherwise PFC_FALSE is returned with throwing an
 *	IllegalArgumentException.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_struct_checktype2(JNIEnv *PFC_RESTRICT env,
		       ipc_fldmeta_t *PFC_RESTRICT fmp,
		       pfc_ipctype_t type1, pfc_ipctype_t type2)
{
	pfc_ipctype_t	t = IPC_FLDMETA_TYPE(fmp);

	if (PFC_EXPECT_TRUE(!IPC_FLDMETA_IS_STRUCT(fmp) &&
			    (t == type1 || t == type2))) {
		return PFC_TRUE;
	}

	/* Invalid data type. */
	jipc_struct_badtype2(env, IPC_FLDMETA_NAME(fmp), type1, type2,
			     IPC_FLDMETA_GETTYPE(fmp));

	return PFC_FALSE;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_struct_bufcheck(JNIEnv *PFC_RESTRICT env,
 *			ipc_strbuf_t *PFC_RESTRICT sbp, uint32_t offset,
 *			uint32_t size)
 *	Ensure that the specified data is contained in the specified IPC
 *	structure buffer.
 *
 *	`offset' must be offset of the data in the buffer.
 *	`size' must be the number of bytes in the data.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the specified data is contained in the buffer.
 *	Otherwise PFC_FALSE is returned with throwing an IllegalStateException.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_struct_bufcheck(JNIEnv *PFC_RESTRICT env, ipc_strbuf_t *PFC_RESTRICT sbp,
		     uint32_t offset, uint32_t size)
{
	uint32_t	remains = sbp->isb_size - offset;

	PFC_ASSERT(offset < sbp->isb_size);

	if (PFC_EXPECT_TRUE(remains >= size)) {
		return PFC_TRUE;
	}

	/* This should never happen. */
	pjni_throw(env, PJNI_CLASS(IllegalStateException),
		   "Buffer overflow: size=%u, remains=%u", size, remains);

	return PFC_FALSE;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_struct_arraycheck(JNIEnv *PFC_RESTRICT env, jarray value,
 *			  ipc_strbuf_t* PFC_RESTRICT sbp, uint32_t offset,
 *			  ipc_fldmeta_t *PFC_RESTRICT fmp, uint32_t alen)
 *	Determine whether a Java array specified by `value' can be set
 *	to an IPC structure field.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the specified array can be set.
 *	Otherwise PFC_FALSE is returned with throwing a runtime exception.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_struct_arraycheck(JNIEnv *PFC_RESTRICT env, jarray value,
		       ipc_strbuf_t* PFC_RESTRICT sbp, uint32_t offset,
		       ipc_fldmeta_t *PFC_RESTRICT fmp, uint32_t alen)
{
	uint32_t	vlen;

	if (PFC_EXPECT_FALSE(value == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "value is null.");
		return PFC_FALSE;
	}

	vlen = (*env)->GetArrayLength(env, value);
	if (PFC_EXPECT_FALSE(alen != vlen)) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Array length(%u) does not match the field %s[%u].",
			   vlen, IPC_FLDMETA_NAME(fmp), alen);

		return PFC_FALSE;
	}

	return jipc_struct_bufcheck(env, sbp, offset, fmp->flm_size * alen);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_load(JNIEnv *env, jclass cls)
 *	Load IPC structure information.
 *
 * Calling/Exit State:
 *	An IllegalStateException is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_load(JNIEnv *env, jclass cls)
{
	int	err = pfc_ipc_struct_load_fields();

	if (PFC_EXPECT_FALSE(err != 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Failed to load IPC structure information: err=%d",
			   err);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_initialize(
 *	JNIEnv *env, jobject this, jstring jname)
 *
 *	Initialize the IpcStruct instance.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_initialize(
	JNIEnv *env, jobject this, jstring jname)
{
	const char	*name;

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_TRUE(name != NULL)) {
		jipc_struct_init(env, this, name);
		pjni_string_release(env, jname, name);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_finalize(
 *	JNIEnv *env, jobject this, jlong buffer, jlong info)
 *
 *	Finalize the specified IpcStruct instance.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_finalize(
	JNIEnv *env, jobject this, jlong buffer, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);

	/*
	 * Note that buffer or structure information may be NULL if an
	 * exception was thrown in constructor.
	 */
	if (PFC_EXPECT_TRUE(sbp != NULL)) {
		IPC_STRBUF_RELEASE(sbp);
	}

	if (PFC_EXPECT_TRUE(sip != NULL)) {
		IPC_STRINFO_RELEASE(sip);
	}
}

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_get(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info, jboolean deep)
 *
 *	Return an IpcDataUnit instance which keeps a value at the specified
 *	structure field.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcDataUnit
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_get(
	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
	jint base, jlong info, jboolean deep)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_fldmeta_t	*fmp;
	uint32_t	offset, size;
	const uint8_t	*data;

	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	offset = jipc_struct_getscalar(env, jname, sbp, base, sip, index,
				       &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return NULL;
	}

	if (IPC_FLDMETA_IS_STRUCT(fmp)) {
		/*
		 * Create a new IpcStruct associated with the specified inner
		 * structure field.
		 */
		return jipc_struct_newinner(env, sbp, offset, fmp, deep);
	}

	data = (const uint8_t *)sbp->isb_data + offset;
	size = sbp->isb_size - offset;

	return jipc_pdu_create(env, IPC_FLDMETA_TYPE(fmp), data, size);
}

/*
 * Declare JNI methods to get a value in a single field.
 */
#define	JIPC_STRUCT_GET_DECL(method, jtype, ipctype)			\
	JNIEXPORT jtype JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname, jint index,	\
		jlong buffer, jint base, jlong info)			\
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	offset;					\
		const jtype	*src;					\
		jtype		data;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the field. */			\
		offset = jipc_struct_getscalar(env, jname, sbp, base,	\
					       sip, index, &fmp);	\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return (jtype)0;				\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype		\
				     (env, fmp, PFC_IPCTYPE_##ipctype))) { \
			return (jtype)0;				\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck		\
				     (env, sbp, offset, sizeof(jtype)))) { \
			return (jtype)0;				\
		}							\
									\
		src = (const jtype *)(sbp->isb_data + offset);		\
		IPC_FETCH_INT(data, src);				\
									\
		return data;						\
	}

#define	JIPC_STRUCT_GET_DECL2(method, jtype, ipctype1, ipctype2)	\
	JNIEXPORT jtype JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname, jint index,	\
		jlong buffer, jint base, jlong info)			\
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	offset;					\
		const jtype	*src;					\
		jtype		data;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the field. */			\
		offset = jipc_struct_getscalar(env, jname, sbp, base,	\
					       sip, index, &fmp);	\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return (jtype)0;				\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype2		\
				     (env, fmp, PFC_IPCTYPE_##ipctype1,	\
				      PFC_IPCTYPE_##ipctype2))) {	\
			return (jtype)0;				\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck		\
				     (env, sbp, offset, sizeof(jtype)))) { \
			return (jtype)0;				\
		}							\
									\
		src = (const jtype *)(sbp->isb_data + offset);		\
		IPC_FETCH_INT(data, src);				\
									\
		return data;						\
	}

/*
 * JNIEXPORT jbyte JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getByte(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 * JNIEXPORT jshort JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getShort(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getInt(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getLong(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 *	Get a value at the specified field as Java primitive integer value.
 *
 * Calling/Exit State:
 *	Upon successful completion, an integer value at the specified structure
 *	field is returned.
 *	Otherwise zero is returned with throwing a runtime exception.
 */
JIPC_STRUCT_GET_DECL2(getByte, jbyte, INT8, UINT8);
JIPC_STRUCT_GET_DECL2(getShort, jshort, INT16, UINT16);
JIPC_STRUCT_GET_DECL2(getInt, jint, INT32, UINT32);
JIPC_STRUCT_GET_DECL2(getLong, jlong, INT64, UINT64);

/*
 * JNIEXPORT jfloat JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFloat(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 * JNIEXPORT jdouble JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getDouble(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 *	Get a value at the specified field as Java primitive floating point.
 *
 * Calling/Exit State:
 *	Upon successful completion, a floating point value at the specified
 *	structure field is returned.
 *	Otherwise zero is returned with throwing a runtime exception.
 */
JIPC_STRUCT_GET_DECL(getFloat, jfloat, FLOAT);
JIPC_STRUCT_GET_DECL(getDouble, jdouble, DOUBLE);

/*
 * JNIEXPORT jbyteArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getInetAddress(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jint base, jlong info)
 *
 *	Get a byte array which contains IPV4 or IPV6 field value.
 *
 * Calling/Exit State:
 *	Upon successful completion, an InetAddress instance which represents
 *	a value at the specified structure field is returned.
 *	Otherwise NULL is returned with throwing a runtime exception.
 */
JNIEXPORT jbyteArray JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_getInetAddress(
	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
	jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_fldmeta_t	*fmp;
	const jbyte	*src;
	uint32_t	offset;

	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	/* Determine offset of the field. */
	offset = jipc_struct_getscalar(env, jname, sbp, base, sip, index,
				       &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return NULL;
	}

	if (PFC_EXPECT_FALSE(!jipc_struct_checktype2
			     (env, fmp, PFC_IPCTYPE_IPV4, PFC_IPCTYPE_IPV6))) {
		return NULL;
	}
	PFC_ASSERT((IPC_FLDMETA_TYPE(fmp) == PFC_IPCTYPE_IPV4 &&
		    fmp->flm_size == sizeof(struct in_addr)) ||
		   (IPC_FLDMETA_TYPE(fmp) == PFC_IPCTYPE_IPV6 &&
		    fmp->flm_size == sizeof(struct in6_addr)));

	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, sbp, offset,
						   fmp->flm_size))) {
		return NULL;
	}

	/* Create a new byte array which represents a raw IP address. */
	src = (const jbyte *)(sbp->isb_data + offset);

	return pjni_bytearray_new(env, src, fmp->flm_size);
}

/*
 * Declare JNI methods to get all elements in the array field.
 */
#define	JIPC_STRUCT_GETARRAY_DECL(method, jtype, ftype, ipctype)	\
	JNIEXPORT jtype##Array JNICALL					\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname, jlong buffer,	\
		jint base, jlong info)					\
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	size, offset, alen;			\
		const jtype	*src;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the array field. */		\
		offset = jipc_struct_getarray(env, jname, sbp, base,	\
					      sip, &fmp);		\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return NULL;					\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype		\
				     (env, fmp, PFC_IPCTYPE_##ipctype))) { \
			return NULL;					\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		alen = IPC_FLDMETA_ARRAY_LEN(fmp);			\
		size = fmp->flm_size * alen;				\
		if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck		\
				     (env, sbp, offset, size))) {	\
			return NULL;					\
		}							\
									\
		src = (const jtype *)(sbp->isb_data + offset);		\
									\
		return pjni_##ftype##_new(env, src, alen);		\
	}

#define	JIPC_STRUCT_GETARRAY_DECL2(method, jtype, ftype, ipctype1, ipctype2) \
	JNIEXPORT jtype##Array JNICALL					\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname, jlong buffer,	\
		jint base, jlong info)					\
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	size, offset, alen;			\
		const jtype	*src;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the array field. */		\
		offset = jipc_struct_getarray(env, jname, sbp, base,	\
					      sip, &fmp);		\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return NULL;					\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype2		\
				     (env, fmp, PFC_IPCTYPE_##ipctype1,	\
				      PFC_IPCTYPE_##ipctype2))) {	\
			return NULL;					\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		alen = IPC_FLDMETA_ARRAY_LEN(fmp);			\
		size = fmp->flm_size * alen;				\
		if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck		\
				     (env, sbp, offset, size))) {	\
			return NULL;					\
		}							\
									\
		src = (const jtype *)(sbp->isb_data + offset);		\
									\
		return pjni_##ftype##_new(env, src, alen);		\
	}

/*
 * JNIEXPORT jbyteArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getByteArray(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 * JNIEXPORT jshortArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getShortArray(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 * JNIEXPORT jintArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getIntArray(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 * JNIEXPORT jlongArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getLongArray(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 *	Get all integer array elements in the specified array field.
 *
 * Calling/Exit State:
 *	Upon successful completion, a Java array object which contains all
 *	elements in the specified structure field is returned.
 *	Otherwise NULL is returned with throwing a runtime exception.
 */
JIPC_STRUCT_GETARRAY_DECL2(getByteArray, jbyte, bytearray, INT8, UINT8);
JIPC_STRUCT_GETARRAY_DECL2(getShortArray, jshort, shortarray, INT16, UINT16);
JIPC_STRUCT_GETARRAY_DECL2(getIntArray, jint, intarray, INT32, UINT32);
JIPC_STRUCT_GETARRAY_DECL2(getLongArray, jlong, longarray, INT64, UINT64);

/*
 * JNIEXPORT jfloatArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFloatArray(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 * JNIEXPORT jdoubleArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getDoubleArray(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 *	Get all floating point array elements in the specified array field.
 *
 * Calling/Exit State:
 *	Upon successful completion, a Java array object which contains all
 *	elements in the specified structure field is returned.
 *	Otherwise NULL is returned with throwing a runtime exception.
 */
JIPC_STRUCT_GETARRAY_DECL(getFloatArray, jfloat, floatarray, FLOAT);
JIPC_STRUCT_GETARRAY_DECL(getDoubleArray, jdouble, doublearray, DOUBLE);

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getString(
 *	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
 *	jlong info)
 *
 *	Construct a String from INT8 or UINT8 array field.
 *
 * Calling/Exit State:
 *	Upon successful completion, a String object constructed from INT8 or
 *	UINT8 array is returned.
 *	Otherwise NULL is returned with throwing a runtime exception.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_getString(
	JNIEnv *env, jobject this, jstring jname, jlong buffer, jint base,
	jlong info)
{
	jstring		jstr;
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_fldmeta_t	*fmp;
	uint32_t	offset, alen;
	const char	*src, *limit, *p;
	char		*buf;

	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	/* Determine offset of the array field. */
	offset = jipc_struct_getarray(env, jname, sbp, base, sip, &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return NULL;
	}

	if (PFC_EXPECT_FALSE(!jipc_struct_checktype2
			     (env, fmp, PFC_IPCTYPE_INT8,
			      PFC_IPCTYPE_UINT8))) {
		return NULL;
	}

	PFC_ASSERT(fmp->flm_size == sizeof(*src) && fmp->flm_size == 1U);
	alen = IPC_FLDMETA_ARRAY_LEN(fmp);
	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, sbp, offset, alen))) {
		/* This should never happen. */
		return NULL;
	}

	/*
	 * Determine a string in the array field is terminated by '\0'
	 * or not.
	 */
	src = (const char *)(sbp->isb_data + offset);
	limit = src + alen;
	for (p = src; p < limit && *p != '\0'; p++) {}
	if (p >= limit) {
		/* A terminator must be appended. */
		buf = (char *)malloc(alen + 1);
		if (PFC_EXPECT_FALSE(buf == NULL)) {
			pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
				   "No memory for INT8/UINT8 array.");

			return NULL;
		}

		memcpy(buf, src, alen);
		*(buf + alen) = '\0';
		src = (const char *)buf;
	}
	else {
		buf = NULL;
	}

	jstr = pjni_newstring(env, src);
	if (buf != NULL) {
		free(buf);
	}

	return jstr;
}

/*
 * Declare JNI methods to set a value into a single field.
 */
#define	JIPC_STRUCT_SET_DECL(method, jtype, ipctype)			\
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname, jint index,	\
		jtype value, jlong buffer, jint base, jlong info)	\
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	offset;					\
		jtype		*dst;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the field. */			\
		offset = jipc_struct_getscalar(env, jname, sbp, base,	\
					       sip, index, &fmp);	\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return;						\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype		\
				     (env, fmp, PFC_IPCTYPE_##ipctype))) { \
			return;						\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck		\
				     (env, sbp, offset, sizeof(jtype)))) { \
			return;						\
		}							\
									\
		dst = (jtype *)(sbp->isb_data + offset);		\
		IPC_STORE_INT(dst, value);				\
	}

#define	JIPC_STRUCT_SET_DECL2(method, jtype, ipctype1, ipctype2)	\
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname, jint index,	\
		jint reqtype, jtype value, jlong buffer, jint base,	\
		jlong info)						\
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	offset;					\
		jtype		*dst;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the field. */			\
		offset = jipc_struct_getscalar(env, jname, sbp, base,	\
					       sip, index, &fmp);	\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return;						\
		}							\
									\
		if (reqtype < 0) {					\
			if (PFC_EXPECT_FALSE(!jipc_struct_checktype2	\
					     (env, fmp,			\
					      PFC_IPCTYPE_##ipctype1,	\
					      PFC_IPCTYPE_##ipctype2))) { \
				return;					\
			}						\
		}							\
		else if (PFC_EXPECT_FALSE(!jipc_struct_checktype	\
					  (env, fmp, reqtype))) {	\
			return;						\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck		\
				     (env, sbp, offset, sizeof(jtype)))) { \
			return;						\
		}							\
									\
		dst = (jtype *)(sbp->isb_data + offset);		\
		IPC_STORE_INT(dst, value);				\
	}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setByte(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jint reqtype,
 *	jbyte value, jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setShort(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jint reqtype,
 *	jshort value, jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setInt(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jint reqtype,
 *	jint value, jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setLong(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jint reqtype,
 *	jlong value, jlong buffer, jint base, jlong info)
 *
 *	Set an integer value to the specified field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JIPC_STRUCT_SET_DECL2(setByte, jbyte, INT8, UINT8);
JIPC_STRUCT_SET_DECL2(setShort, jshort, INT16, UINT16);
JIPC_STRUCT_SET_DECL2(setInt, jint, INT32, UINT32);
JIPC_STRUCT_SET_DECL2(setLong, jlong, INT64, UINT64);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setFloat(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jfloat value,
 *	jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setDouble(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jdouble value,
 *	jlong buffer, jint base, jlong info)
 *
 *	Set a floating point value to the specified field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JIPC_STRUCT_SET_DECL(setFloat, jfloat, FLOAT);
JIPC_STRUCT_SET_DECL(setDouble, jdouble, DOUBLE);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setInetAddress(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jbyteArray value,
 *	jlong buffer, jint base, jlong info)
 *
 *	Set a byte array which represents raw IP address into the specified
 *	structure field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_setInetAddress(
	JNIEnv *env, jobject this, jstring jname, jint index, jbyteArray value,
	jlong buffer, jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_fldmeta_t	*fmp;
	pfc_ipctype_t	vtype;
	jbyte		*array;
	uint32_t	offset, alen;

	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	/* Determine offset of the field. */
	offset = jipc_struct_getscalar(env, jname, sbp, base, sip, index,
				       &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return;
	}

	alen = (uint32_t)(*env)->GetArrayLength(env, value);
	if (alen == sizeof(struct in_addr)) {
		vtype = PFC_IPCTYPE_IPV4;
	}
	else if (PFC_EXPECT_TRUE(alen == sizeof(struct in6_addr))) {
		vtype = PFC_IPCTYPE_IPV6;
	}
	else {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Invalid address length(%u) for the field \"%s\".",
			   alen, IPC_FLDMETA_NAME(fmp));

		return;
	}

	if (PFC_EXPECT_FALSE(!jipc_struct_checktype(env, fmp, vtype))) {
		return;
	}

	PFC_ASSERT(alen == fmp->flm_size);
	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, sbp, offset,
						   fmp->flm_size))) {
		return;
	}

	array = pjni_bytearray_get(env, value);
	if (PFC_EXPECT_TRUE(array != NULL)) {
		uint8_t		*dst = (uint8_t *)(sbp->isb_data + offset);

		/* Set raw IP address to the specified field. */
		memcpy(dst, array, fmp->flm_size);
	}

	pjni_bytearray_release(env, value, array, PFC_TRUE);
}

/*
 * Declare JNI methods to set all elements in the specified array to
 * the array field.
 */
#define	JIPC_STRUCT_SETARRAY_DECL(method, jtype, ftype, ipctype)	\
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname,		\
		jtype##Array value, jlong buffer, jint base, jlong info) \
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	offset, alen;				\
		jtype		*dst;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the array field. */		\
		offset = jipc_struct_getarray(env, jname, sbp, base,	\
					      sip, &fmp);		\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return;						\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype		\
				     (env, fmp, PFC_IPCTYPE_##ipctype))) { \
			return;						\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		alen = IPC_FLDMETA_ARRAY_LEN(fmp);			\
		if (PFC_EXPECT_FALSE(!jipc_struct_arraycheck		\
				     (env, value, sbp, offset, fmp,	\
				      alen))) {				\
			return;						\
		}							\
									\
		dst = (jtype *)(sbp->isb_data + offset);		\
		(*env)->Get##ftype##ArrayRegion(env, value, 0, alen,	\
						dst);			\
	}

#define	JIPC_STRUCT_SETARRAY_DECL2(method, jtype, ftype, ipctype1, ipctype2) \
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_IpcStruct_##method(		\
		JNIEnv *env, jobject this, jstring jname,		\
		jtype##Array value, jlong buffer, jint base, jlong info) \
	{								\
		ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);		\
		ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);		\
		ipc_fldmeta_t	*fmp;					\
		uint32_t	offset, alen;				\
		jtype		*dst;					\
									\
		PFC_ASSERT(sbp != NULL);				\
		PFC_ASSERT(sip != NULL);				\
									\
		/* Determine offset of the array field. */		\
		offset = jipc_struct_getarray(env, jname, sbp, base,	\
					      sip, &fmp);		\
		if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {		\
			return;						\
		}							\
									\
		if (PFC_EXPECT_FALSE(!jipc_struct_checktype2		\
				     (env, fmp, PFC_IPCTYPE_##ipctype1,	\
				      PFC_IPCTYPE_##ipctype2))) {	\
			return;						\
		}							\
									\
		PFC_ASSERT(fmp->flm_size == sizeof(jtype));		\
		alen = IPC_FLDMETA_ARRAY_LEN(fmp);			\
		if (PFC_EXPECT_FALSE(!jipc_struct_arraycheck		\
				     (env, value, sbp, offset, fmp,	\
				      alen))) {				\
			return;						\
		}							\
									\
		dst = (jtype *)(sbp->isb_data + offset);		\
		(*env)->Get##ftype##ArrayRegion(env, value, 0, alen,	\
						dst);			\
	}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setByteArray(
 *	JNIEnv *env, jobject this, jstring jname, jbyteArray value,
 *	jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setShortArray(
 *	JNIEnv *env, jobject this, jstring jname, jshortArray value,
 *	jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setIntArray(
 *	JNIEnv *env, jobject this, jstring jname, jintArray value,
 *	jlong buffer, jint base, jlong info)
 *
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setLongArray(
 *	JNIEnv *env, jobject this, jstring jname, jlongArray value,
 *	jlong buffer, jint base, jlong info)
 *
 *	Set all integer array elements in the specified Java array into
 *	the specified structure field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JIPC_STRUCT_SETARRAY_DECL2(setByteArray, jbyte, Byte, INT8, UINT8);
JIPC_STRUCT_SETARRAY_DECL2(setShortArray, jshort, Short, INT16, UINT16);
JIPC_STRUCT_SETARRAY_DECL2(setIntArray, jint, Int, INT32, UINT32);
JIPC_STRUCT_SETARRAY_DECL2(setLongArray, jlong, Long, INT64, UINT64);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setFloatArray(
 *	JNIEnv *env, jobject this, jstring jname, jfloatArray value,
 *	jlong buffer, jint base, jlong info)
 *
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setDoubleArray(
 *	JNIEnv *env, jobject this, jstring jname, jdoubleArray value,
 *	jlong buffer, jint base, jlong info)
 *
 *	Set all floating point array elements in the specified Java array into
 *	the specified structure field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JIPC_STRUCT_SETARRAY_DECL(setFloatArray, jfloat, Float, FLOAT);
JIPC_STRUCT_SETARRAY_DECL(setDoubleArray, jdouble, Double, DOUBLE);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setInetAddressArray(
 *	JNIEnv *env, jobject this, jstring jname, jbyteArray value,
 *	jlong buffer, jint base, jlong info)
 *
 *	Set all raw IP addresses in `value' to the IPV4 or IPV6 array field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_setInetAddressArray(
	JNIEnv *env, jobject this, jstring jname, jbyteArray value,
	jlong buffer, jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_fldmeta_t	*fmp;
	uint32_t	offset, alen, blen;
	jbyte		*dst;

	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);
	PFC_ASSERT(value != NULL);

	/* Determine offset of the array field. */
	offset = jipc_struct_getarray(env, jname, sbp, base,
				      sip, &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return;
	}

	alen = IPC_FLDMETA_ARRAY_LEN(fmp);
	PFC_ASSERT(IPC_FLDMETA_GETTYPE(fmp) == PFC_IPCTYPE_IPV4 ||
		   IPC_FLDMETA_GETTYPE(fmp) == PFC_IPCTYPE_IPV6);
	blen = alen * fmp->flm_size;
	PFC_ASSERT((uint32_t)(*env)->GetArrayLength(env, value) == blen);

	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, sbp, offset, blen))) {
		return;
	}

	dst = (jbyte *)(sbp->isb_data + offset);
	(*env)->GetByteArrayRegion(env, value, 0, blen, dst);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setString(
 *	JNIEnv *env, jobject this, jstring jname, jstring jvalue,
 *	jlong buffer, jint base, jlong info)
 *
 *	Set a string into INT8 or UINT8 array field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_setString(
	JNIEnv *env, jobject this, jstring jname, jstring jvalue,
	jlong buffer, jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_fldmeta_t	*fmp;
	uint32_t	offset, alen, vlen;
	const char	*value;
	char		*dst;

	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	if (PFC_EXPECT_FALSE(jvalue == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "value is null.");

		return;
	}

	/* Determine offset of the array field. */
	offset = jipc_struct_getarray(env, jname, sbp, base, sip, &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return;
	}

	if (PFC_EXPECT_FALSE(!jipc_struct_checktype2
			     (env, fmp, PFC_IPCTYPE_INT8,
			      PFC_IPCTYPE_UINT8))) {
		return;
	}

	PFC_ASSERT(fmp->flm_size == sizeof(*dst) && fmp->flm_size == 1U);

	value = pjni_string_get(env, jvalue);
	if (PFC_EXPECT_FALSE(value == NULL)) {
		return;
	}
	vlen = strlen(value);
	alen = IPC_FLDMETA_ARRAY_LEN(fmp);
	if (PFC_EXPECT_FALSE(vlen > alen)) {
		pjni_throw(env, PJNI_CLASS(ArrayIndexOutOfBoundsException),
			   "Too long string: nbytes=%u, field=%s[%u]",
			   vlen, IPC_FLDMETA_NAME(fmp), alen);
		goto out;
	}

	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, sbp, offset, alen))) {
		/* This should never happen. */
		goto out;
	}

	/* Copy the string into the array field. */
	dst = (char *)(sbp->isb_data + offset);
	if (vlen > 0) {
		uint32_t	cplen;

		/* Append a string terminator if possible. */
		cplen = (vlen < alen) ? vlen + 1 : vlen;
		memcpy(dst, value, cplen);
	}
	else {
		/* Set a string terminator. */
		PFC_ASSERT(vlen == 0);
		*dst = '\0';
	}

out:
	pjni_string_release(env, jvalue, value);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_setStructField(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong dstbuf,
 *	jint dstbase, jlong dstinfo, jlong srcbuf, jint srcbase, jlong srcinfo)
 *
 *	Copy whole contents of this structure to the specified structure
 *	field.
 *
 * Calling/Exit State:
 *	A runtime error is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_setStructField(
	JNIEnv *env, jobject this, jstring jname, jint index, jlong dstbuf,
	jint dstbase, jlong dstinfo, jlong srcbuf, jint srcbase, jlong srcinfo)
{
	ipc_strbuf_t	*dstbp = JIPC_STRBUF_PTR(dstbuf);
	ipc_strbuf_t	*srcbp = JIPC_STRBUF_PTR(srcbuf);
	ipc_strinfo_t	*dstip = JIPC_STRINFO_PTR(dstinfo);
	ipc_strinfo_t	*srcip = JIPC_STRINFO_PTR(srcinfo);
	ipc_strinfo_t	*fldip;
	ipc_fldmeta_t	*fmp;
	uint32_t	offset, size;
	uint8_t		*src, *dst;

	PFC_ASSERT(dstbp != NULL);
	PFC_ASSERT(srcbp != NULL);
	PFC_ASSERT(dstip != NULL);
	PFC_ASSERT(srcip != NULL);

	/* Determine offset of the field. */
	offset = jipc_struct_getscalar(env, jname, dstbp, dstbase, dstip,
				       index, &fmp);
	if (PFC_EXPECT_FALSE(offset == UINT32_MAX)) {
		return;
	}

	if (PFC_EXPECT_FALSE(!IPC_FLDMETA_IS_STRUCT(fmp))) {
		jipc_struct_badtype(env, IPC_FLDMETA_NAME(fmp),
				    PFC_IPCTYPE_STRUCT,
				    IPC_FLDMETA_GETTYPE(fmp));

		return;
	}

	/* Ensure that the field type is exactly matched to the source. */
	fldip = IPC_FLDMETA_STRINFO(fmp);
	if (PFC_EXPECT_FALSE(fldip != srcip)) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Data type of the field \"%s\" is \"%s\", but "
			   "\"%s\" is specified.", IPC_FLDMETA_NAME(fmp),
			   IPC_STRINFO_NAME(fldip), IPC_STRINFO_NAME(srcip));

		return;
	}

	size = fmp->flm_size;
	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, dstbp, offset, size) ||
			     !jipc_struct_bufcheck(env, srcbp,
						   (uint32_t)srcbase, size))) {
		return;
	}

	/* Copy whole structure data. */
	src = srcbp->isb_data + (uint32_t)srcbase;
	dst = dstbp->isb_data + offset;
	memcpy(dst, src, size);
}

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getField(
 *	JNIEnv *env, jclass cls, jstring jstname, jstring jname)
 *
 *	Construct a new IpcStructField instance which contains information
 *	about the specified IPC structure field.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcStructField
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_getField(
	JNIEnv *env, jclass cls, jstring jstname, jstring jname)
{
	jobject		obj = NULL;
	ipc_cstrinfo_t	*sip;
	const char	*stname;
	int		err;

	stname = jipc_getstring(env, jstname, "stname");
	if (PFC_EXPECT_FALSE(stname == NULL)) {
		return NULL;
	}

	/* Obtain IPC structure information. */
	err = pfc_ipc_strinfo_get(stname, &sip, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_struct_unknown(env, err, stname);
	}
	else {
		obj = jipc_struct_newfield(env, jstname, (ipc_strinfo_t *)sip,
					   jname);
		IPC_STRINFO_RELEASE(sip);
	}

	pjni_string_release(env, jstname, stname);

	return obj;
}

/*
 * JNIEXPORT jobjectArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFieldNames(
 *	JNIEnv *env, jclass cls, jstring jstname)
 *
 *	Return a String array which contains all field names defined in the
 *	specified IPC structure.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to a String array
 *	is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
JNIEXPORT jobjectArray JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFieldNames(
	JNIEnv *env, jclass cls, jstring jstname)
{
	jobjectArray	array;
	ipc_cstrinfo_t	*sip;
	const char	*stname;
	int		err;

	stname = jipc_getstring(env, jstname, "stname");
	if (PFC_EXPECT_FALSE(stname == NULL)) {
		return NULL;
	}

	/* Obtain IPC structure information. */
	array = NULL;
	err = pfc_ipc_strinfo_get(stname, &sip, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_struct_unknown(env, err, stname);
	}
	else {
		array = jipc_struct_getfieldnames(env, (ipc_strinfo_t *)sip);
		IPC_STRINFO_RELEASE(sip);
	}

	pjni_string_release(env, jstname, stname);

	return array;
}

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFieldImpl(
 *	JNIEnv *env, jobject this, jstring jstname, jstring jname, jlong info)
 *
 *	Construct a new IpcStructField instance which contains information
 *	about the specified field in this structure.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcStructField
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFieldImpl(
	JNIEnv *env, jobject this, jstring jstname, jstring jname, jlong info)
{
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);

	PFC_ASSERT(jstname != NULL);
	PFC_ASSERT(sip != NULL);

	return jipc_struct_newfield(env, jstname, sip, jname);
}

/*
 * JNIEXPORT jobjectArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFieldNamesImpl(
 *	JNIEnv *env, jobject this, jlong info)
 *
 *	Return a String array which contains all field names defined in this
 *	structure.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to a String array
 *	is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
JNIEXPORT jobjectArray JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_getFieldNamesImpl(
	JNIEnv *env, jobject this, jlong info)
{
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);

	PFC_ASSERT(sip != NULL);

	return jipc_struct_getfieldnames(env, sip);
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_deepCopy(
 *	JNIEnv *env, jobject this, jlong buffer, jint base, jlong info)
 *
 *	Return a deep copy of the specified IPC structure.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-zero value which is associated with
 *	a deep copy of IPC structure is returned.
 *	Otherwise zero is returned with throwing an exception.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_deepCopy(
	JNIEnv *env, jobject this, jlong buffer, jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_strbuf_t	*newsbp;
	uint32_t	offset = (uint32_t)base;
	uint32_t	size = IPC_STRINFO_SIZE(sip);

	if (PFC_EXPECT_FALSE(offset >= sbp->isb_size ||
			     offset + size > sbp->isb_size)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unexpected buffer offset: size=%u, base=%u",
			   size, offset);

		return 0;
	}

	/* Create a deep copy of the buffer. */
	newsbp = pfc_ipc_strbuf_copy(sbp->isb_data + offset, size);
	if (PFC_EXPECT_FALSE(newsbp == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Failed to copy IPC structure buffer.");

		return 0;
	}

	IPC_STRINFO_HOLD(sip);

	return JIPC_STRBUF_HANDLE(newsbp);
}

/*
 * static void
 * jipc_struct_init(JNIEnv *PFC_RESTRICT env, jobject this,
 *		    const char *PFC_RESTRICT name)
 *	Initialize the IpcStruct instance.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
static void
jipc_struct_init(JNIEnv *PFC_RESTRICT env, jobject this,
		 const char *PFC_RESTRICT name)
{
	jclass		cls;
	jfieldID	bufid, infoid;
	ipc_strbuf_t	*sbp;
	ipc_cstrinfo_t	*sip;
	int		err;

	/* Obtain IPC structure information. */
	err = pfc_ipc_strinfo_get(name, &sip, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_struct_unknown(env, err, name);

		return;
	}

	/* Allocate a new IPC structure buffer. */
	sbp = pfc_ipc_strbuf_alloc(IPC_STRINFO_SIZE(sip));
	if (PFC_EXPECT_FALSE(sbp == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Failed to allocate IPC structure buffer.");
		goto error;
	}

	cls = pjni_getclass(env, this);
	if (PFC_EXPECT_FALSE(cls == NULL)) {
		/* This should never happen. */
		goto error;
	}

	/* Determine instance field IDs. */
	bufid = pjni_getfield(env, cls, "_buffer", PJNI_SIG_long);
	if (PFC_EXPECT_FALSE(bufid == NULL)) {
		goto error;
	}

	infoid = pjni_getfield(env, cls, "_info", PJNI_SIG_long);
	if (PFC_EXPECT_FALSE(infoid == NULL)) {
		goto error;
	}

	(*env)->SetLongField(env, this, bufid, JIPC_STRBUF_HANDLE(sbp));
	(*env)->SetLongField(env, this, infoid, JIPC_STRINFO_HANDLE(sip));

	return;

error:
	IPC_STRINFO_RELEASE(sip);
	if (sbp != NULL) {
		free(sbp);
	}
}

/*
 * static void
 * jipc_struct_unknown(JNIEnv *PFC_RESTRICT env, int err,
 *		       const char *PFC_RESTRICT name)
 *	Throw an exception which indicates the specified IPC structure name
 *	is not found.
 */
static void
jipc_struct_unknown(JNIEnv *PFC_RESTRICT env, int err,
		    const char *PFC_RESTRICT name)
{
	/*
	 * Don't call jipc_throw() here because this function can not
	 * throw IpcException.
	 */
	if (err == ENODEV) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Unknown IPC struct: %s", name);
	}
	else {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Failed to get IPC struct information: name=%s, "
			   "err=%d", name, err);
	}
}

/*
 * static void
 * jipc_struct_badtype(JNIEnv *PFC_RESTRICT env, const char *PFC_RESTRICT name,
 *		       pfc_ipctype_t reqtype, pfc_ipctype_t type)
 *	Throw an IllegalArgumentException which indicates an illegal data type
 *	`type' is specified though `reqtype' is required.
 */
static void
jipc_struct_badtype(JNIEnv *PFC_RESTRICT env, const char *PFC_RESTRICT name,
		    pfc_ipctype_t reqtype, pfc_ipctype_t type)
{
	char	rbuf[JIPC_TYPEBUF_SIZE], buf[JIPC_TYPEBUF_SIZE];

	pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
		   "Data type of the field \"%s\" is not %s: type=%s", name,
		   jipc_pdu_typename(reqtype, rbuf, sizeof(rbuf)),
		   jipc_pdu_typename(type, buf, sizeof(buf)));
}

/*
 * static void
 * jipc_struct_badtype2(JNIEnv *PFC_RESTRICT env,
 *			const char *PFC_RESTRICT name, pfc_ipctype_t reqtype1,
 *			pfc_ipctype_t reqtype2, pfc_ipctype_t type)
 *	Throw an IllegalArgumentException which indicates an illegal data type
 *	`type' is specified though either `reqtype1' or `reqtype2' is required.
 */
static void
jipc_struct_badtype2(JNIEnv *PFC_RESTRICT env, const char *PFC_RESTRICT name,
		     pfc_ipctype_t reqtype1, pfc_ipctype_t reqtype2,
		     pfc_ipctype_t type)
{
	char	r1buf[JIPC_TYPEBUF_SIZE], r2buf[JIPC_TYPEBUF_SIZE];
	char	buf[JIPC_TYPEBUF_SIZE];

	pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
		   "Data type of the field \"%s\" is neither %s nor %s: "
		   "type=%s", name,
		   jipc_pdu_typename(reqtype1, r1buf, sizeof(r1buf)),
		   jipc_pdu_typename(reqtype2, r2buf, sizeof(r2buf)),
		   jipc_pdu_typename(type, buf, sizeof(buf)));
}

/*
 * static uint32_t
 * jipc_struct_getscalar(JNIEnv *PFC_RESTRICT env, jstring jname,
 *			 ipc_strbuf_t *PFC_RESTRICT sbp, jint base,
 *			 ipc_strinfo_t *PFC_RESTRICT sip, jint index,
 *			 ipc_fldmeta_t **PFC_RESTRICT fmpp)
 *	Determine offset of the scalar field, which is a non-array field.
 *
 * Calling/Exit State:
 *	Upon successful completion, offset of the specified field is returned.
 *	A pointer to ipc_fldmeta_t, which keeps meta data of the specified
 *	field, is also set to the buffer pointed by `fmpp'
 *
 *	Otherwise UINT32_MAX is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static uint32_t
jipc_struct_getscalar(JNIEnv *PFC_RESTRICT env, jstring jname,
		      ipc_strbuf_t *PFC_RESTRICT sbp, jint base,
		      ipc_strinfo_t *PFC_RESTRICT sip, jint index,
		      ipc_fldmeta_t **PFC_RESTRICT fmpp)
{
	ipc_fldmeta_t	*fmp;
	const char	*name;
	uint32_t	offset = UINT32_MAX, foff, alen;

	PFC_ASSERT(base >= 0 && (uint32_t)base < sbp->isb_size);

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return UINT32_MAX;
	}

	/* Determine the field. */
	fmp = jipc_struct_field_lookup(env, sip, name);
	if (PFC_EXPECT_FALSE(fmp == NULL)) {
		goto out;
	}

	foff = fmp->flm_offset;
	alen = IPC_FLDMETA_ARRAY_LEN(fmp);
	if (index >= 0) {
		uint32_t	idx = (uint32_t)index;

		/* Attempt to access an array element. */
		if (PFC_EXPECT_FALSE(alen == 0)) {
			pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
				   "\"%s\" is not an array field.", name);
			goto out;
		}
		if (PFC_EXPECT_FALSE(idx >= alen)) {
			pjni_throw(env,
				   PJNI_CLASS(ArrayIndexOutOfBoundsException),
				   "Illegal array index for \"%s\": %u >= %u",
				   name, idx, alen);
			goto out;
		}

		foff += (idx * fmp->flm_size);
	}
	else if (PFC_EXPECT_FALSE(alen != 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Array index for \"%s\" is not specified.", name);
		goto out;
	}

	offset = foff + (uint32_t)base;
	if (PFC_EXPECT_FALSE(offset >= sbp->isb_size)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Unexpected field offset: size=%u, base=%u, off=%u",
			   sbp->isb_size, (uint32_t)base, foff);
		offset = UINT32_MAX;
	}
	else {
		*fmpp = fmp;
	}

out:
	pjni_string_release(env, jname, name);

	return offset;
}

/*
 * static uint32_t
 * jipc_struct_getarray(JNIEnv *PFC_RESTRICT env, jstring jname,
 *			ipc_strbuf_t *PFC_RESTRICT sbp, jint base,
 *			ipc_strinfo_t *PFC_RESTRICT sip,
 *			ipc_fldmeta_t **PFC_RESTRICT fmpp)
 *	Determine offset of the array field.
 *
 * Calling/Exit State:
 *	Upon successful completion, offset of the specified field is returned.
 *	A pointer to ipc_fldmeta_t, which keeps meta data of the specified
 *	field, is also set to the buffer pointed by `fmpp'
 *
 *	Otherwise UINT32_MAX is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static uint32_t
jipc_struct_getarray(JNIEnv *PFC_RESTRICT env, jstring jname,
		     ipc_strbuf_t *PFC_RESTRICT sbp, jint base,
		     ipc_strinfo_t *PFC_RESTRICT sip,
		     ipc_fldmeta_t **PFC_RESTRICT fmpp)
{
	ipc_fldmeta_t	*fmp;
	const char	*name;
	uint32_t	offset = UINT32_MAX, foff, alen;

	PFC_ASSERT(base >= 0 && (uint32_t)base < sbp->isb_size);

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return UINT32_MAX;
	}

	/* Determine the field. */
	fmp = jipc_struct_field_lookup(env, sip, name);
	if (PFC_EXPECT_FALSE(fmp == NULL)) {
		goto out;
	}

	foff = fmp->flm_offset;
	alen = IPC_FLDMETA_ARRAY_LEN(fmp);
	if (PFC_EXPECT_FALSE(alen == 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "\"%s\" is not an array field.", name);
		goto out;
	}

	offset = foff + (uint32_t)base;
	if (PFC_EXPECT_FALSE(offset >= sbp->isb_size)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Unexpected field offset: size=%u, base=%u, off=%u",
			   sbp->isb_size, (uint32_t)base, foff);
		offset = UINT32_MAX;
	}
	else {
		*fmpp = fmp;
	}

out:
	pjni_string_release(env, jname, name);

	return offset;
}

/*
 * static jobject
 * jipc_struct_newinner(JNIEnv *PFC_RESTRICT env,
 *			ipc_strbuf_t *PFC_RESTRICT sbp, uint32_t offset,
 *			ipc_fldmeta_t *PFC_RESTRICT fmp, jboolean deep)
 *	Construct a new IpcStruct instance associated with the specified
 *	inner field.
 *
 *	If `deep' is JNI_TRUE, a deep copy of the specified field is returned.
 *	Otherwise shallow copy of the specified field is returned.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcStruct
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static jobject
jipc_struct_newinner(JNIEnv *PFC_RESTRICT env, ipc_strbuf_t *PFC_RESTRICT sbp,
		     uint32_t offset, ipc_fldmeta_t *PFC_RESTRICT fmp,
		     jboolean deep)
{
	jobject		obj;
	jstring		jname;
	ipc_strinfo_t	*sip = IPC_FLDMETA_STRINFO(fmp);

	PFC_ASSERT(IPC_FLDMETA_IS_STRUCT(fmp));

	if (PFC_EXPECT_FALSE(!jipc_struct_bufcheck(env, sbp, offset,
						   IPC_STRINFO_SIZE(sip)))) {
		return NULL;
	}

	/*
	 * Create a String object which represents the IPC structure name of
	 * the specified inner field.
	 */
	jname = pjni_newstring(env, IPC_STRINFO_NAME(sip));
	if (PFC_EXPECT_FALSE(jname == NULL)) {
		return NULL;
	}

	/* Construct an IpcStruct object. */
	obj = (deep)
		? jipc_struct_newdeep(env, sbp, offset, jname, sip)
		: jipc_struct_newshallow(env, sbp, offset, jname, sip);
	(*env)->DeleteLocalRef(env, jname);

	return obj;
}

/*
 * static jobject
 * jipc_struct_newdeep(JNIEnv *PFC_RESTRICT env, ipc_strbuf_t *PFC_RESTRICT sbp,
 *		       uint32_t offset, jstring jname,
 *		       ipc_strinfo_t *PFC_RESTRICT sip)
 *	Construct a new deep copy of an IpcStruct instance.
 *
 *	The buffer to be copied is specified by `sbp' and `offset'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcStruct
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static jobject
jipc_struct_newdeep(JNIEnv *PFC_RESTRICT env, ipc_strbuf_t *PFC_RESTRICT sbp,
		    uint32_t offset, jstring jname,
		    ipc_strinfo_t *PFC_RESTRICT sip)
{
	jobject		obj;
	ipc_strbuf_t	*newsbp;

	/* Create a deep copy of the buffer. */
	newsbp = pfc_ipc_strbuf_copy(sbp->isb_data + offset,
				     IPC_STRINFO_SIZE(sip));
	if (PFC_EXPECT_FALSE(newsbp == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Failed to copy IPC structure buffer.");

		return NULL;
	}

	/* Construct an IpcStruct object. */
	obj = pjni_newobject(env, PJNI_CLASS(IpcStruct),
			     JIPC_CTORSIG_IpcStruct, jname,
			     JIPC_STRBUF_HANDLE(newsbp), 0,
			     JIPC_STRINFO_HANDLE(sip));
	if (PFC_EXPECT_FALSE(obj == NULL)) {
		IPC_STRBUF_RELEASE(newsbp);
	}
	else {
		IPC_STRINFO_HOLD(sip);
	}

	return obj;
}

/*
 * static jobject
 * jipc_struct_newshallow(JNIEnv *PFC_RESTRICT env,
 *			  ipc_strbuf_t *PFC_RESTRICT sbp, uint32_t offset,
 *			  jstring jname, ipc_strinfo_t *PFC_RESTRICT sip)
 *	Construct a new shallow copy of an IpcStruct instance, which maps
 *	the buffer specified by `sbp' and `offset'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcStruct
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static jobject
jipc_struct_newshallow(JNIEnv *PFC_RESTRICT env,
		       ipc_strbuf_t *PFC_RESTRICT sbp, uint32_t offset,
		       jstring jname, ipc_strinfo_t *PFC_RESTRICT sip)
{
	jobject		obj;

	/* Construct an IpcStruct object. */
	obj = pjni_newobject(env, PJNI_CLASS(IpcStruct),
			     JIPC_CTORSIG_IpcStruct, jname,
			     JIPC_STRBUF_HANDLE(sbp), offset,
			     JIPC_STRINFO_HANDLE(sip));
	if (PFC_EXPECT_TRUE(obj != NULL)) {
		IPC_STRINFO_HOLD(sip);
		IPC_STRBUF_HOLD(sbp);
	}

	return obj;
}

/*
 * static jobject
 * jipc_struct_newfield(JNIEnv *PFC_RESTRICT env, jstring jstname,
 *			ipc_strinfo_t *PFC_RESTRICT sip, jstring jname)
 *	Construct a new IpcStructField instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcStructField
 *	instance is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static jobject
jipc_struct_newfield(JNIEnv *PFC_RESTRICT env, jstring jstname,
		     ipc_strinfo_t *PFC_RESTRICT sip, jstring jname)
{
	jobject		obj = NULL;
	jstring		jnested = NULL;
	jint		alen;
	ipc_fldmeta_t	*fmp;
	pfc_ipctype_t	type;
	const char	*name;

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return NULL;
	}

	/* Determine the field. */
	fmp = jipc_struct_field_lookup(env, sip, name);
	if (PFC_EXPECT_FALSE(fmp == NULL)) {
		goto out;
	}

	if (IPC_FLDMETA_IS_STRUCT(fmp)) {
		ipc_strinfo_t	*nsip = IPC_FLDMETA_STRINFO(fmp);

		/* Nested structure field. */
		type = PFC_IPCTYPE_STRUCT;
		jnested = pjni_newstring(env, IPC_STRINFO_NAME(nsip));
		if (PFC_EXPECT_FALSE(jnested == NULL)) {
			goto out;
		}
	}
	else {
		type = IPC_FLDMETA_TYPE(fmp);
		jnested = NULL;
	}

	alen = IPC_FLDMETA_ARRAY_LEN(fmp);
	obj = pjni_newobject(env, PJNI_CLASS(IpcStructField),
			     JIPC_CTORSIG_IpcStructField, jstname, jname,
			     type, alen, jnested);

out:
	pjni_string_release(env, jname, name);

	if (jnested != NULL) {
		(*env)->DeleteLocalRef(env, jnested);
	}

	return obj;
}

/*
 * static jobjectArray
 * jipc_struct_getfieldnames(JNIEnv *PFC_RESTRICT env,
 *			     ipc_strinfo_t *PFC_RESTRICT sip)
 *	Return a String array which contains all field names defined in the
 *	specified IPC structure.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to a String array
 *	is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	This function throws a runtime exception instead of an IpcException
 *	on error.
 */
static jobjectArray
jipc_struct_getfieldnames(JNIEnv *PFC_RESTRICT env,
			  ipc_strinfo_t *PFC_RESTRICT sip)
{
	jclass		cls;
	jobjectArray	array;
	jsize		index;
	pfc_rbnode_t	*node;

	cls = pjni_findclass(env, PJNI_CLASS(String));
	if (PFC_EXPECT_FALSE(cls == NULL)) {
		return NULL;
	}

	/* Allocate a String array. */
	array = (*env)->NewObjectArray(env, sip->sti_nfields, cls, NULL);
	(*env)->DeleteLocalRef(env, cls);
	if (PFC_EXPECT_FALSE(array == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to construct a String array.");

		return NULL;
	}

	node = NULL;
	index = 0;
	while ((node = pfc_rbtree_next(&sip->sti_fldmeta, node)) != NULL) {
		ipc_fldmeta_t	*fmp = IPC_FLDMETA_NODE2PTR(node);
		jstring		jstr;

		jstr = pjni_newstring(env, IPC_FLDMETA_NAME(fmp));
		if (PFC_EXPECT_FALSE(jstr == NULL)) {
			goto error;
		}

		(*env)->SetObjectArrayElement(env, array, index, jstr);
		(*env)->DeleteLocalRef(env, jstr);
		if (PJNI_EXCEPTION_CHECK(env)) {
			goto error;
		}

		index++;
	}

	return array;

error:
	(*env)->DeleteLocalRef(env, array);

	return NULL;
}
