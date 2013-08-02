/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ut_struct.c - Utilities to handle IpcStruct JUnit tests.
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pfc/jni.h>
#include <pfc/debug.h>
#include <pfc/rbtree.h>
#include <pfc/util.h>
#include <ipc_struct_impl.h>
#include <org_opendaylight_vtn_core_ipc_IpcStructTest.h>
#include <org_opendaylight_vtn_core_ipc_TestStruct.h>
#include <org_opendaylight_vtn_core_ipc_TestStruct1.h>
#include <ut_struct.h>

/* 
 * Cast IpcStruct information.
 */
#define	JIPC_STRBUF_PTR(buffer)		((ipc_strbuf_t *)(uintptr_t)(buffer))
#define	JIPC_STRINFO_PTR(info)		((ipc_strinfo_t *)(uintptr_t)(info))

/*
 * Maximum length of fully qualified field name.
 */
#define	UT_FLDNAME_MAX		PFC_CONST_U(64)

/*
 * Cast buffer handle.
 */
#define	UT_BUFFER_HANDLE(bufp)		((jlong)(uint64_t)(uintptr_t)(bufp))
#define	UT_BUFFER_PTR(buffer)		((uint8_t *)(uintptr_t)(buffer))
#define	UT_BUFFER_CAST(type, bufp, off, index)		\
	(((type *)(((uint8_t *)(bufp)) + (off))) +	\
	 (((index) > 0) ? (index) : 0))
#define	UT_BUFFER_CAST_DECL(type, name, bufp, off, index)	\
	type	*name = UT_BUFFER_CAST(type, bufp, off, index)

/*
 * Fetch a value in a buffer.
 */
#ifdef	PFC_UNALIGNED_ACCESS
#define	UT_FETCH(dst, srcp, size)		\
	do {					\
		(dst) = *(srcp);		\
	} while (0)
#define	UT_STORE(dstp, src, size)		\
	do {					\
		*(dstp) = (src);		\
	} while (0)
#else	/* !PFC_UNALIGNED_ACCESS */
#define	UT_FETCH(dst, srcp, size)	memcpy(&(dst), (srcp), (size))
#define	UT_STORE(dstp, src, size)	memcpy((dstp), &(src), (size))
#endif	/* PFC_UNALIGNED_ACCESS */

#define	UT_FETCH_BYTE(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(const jbyte, __src, bufp, off, index); \
		UT_FETCH(value, __src, sizeof(jbyte));			\
	} while (0)

#define	UT_FETCH_SHORT(bufp, off, index, value)					\
	do {								\
		UT_BUFFER_CAST_DECL(const jshort, __src, bufp, off, index); \
		UT_FETCH(value, __src, sizeof(jshort));			\
	} while (0)

#define	UT_FETCH_INT(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(const jint, __src, bufp, off, index); \
		UT_FETCH(value, __src, sizeof(jint));			\
	} while (0)

#define	UT_FETCH_LONG(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(const jlong, __src, bufp, off, index); \
		UT_FETCH(value, __src, sizeof(jlong));			\
	} while (0)
		  
#define	UT_FETCH_FLOAT(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(const jfloat, __src, bufp, off, index); \
		UT_FETCH(value, __src, sizeof(jfloat));			\
	} while (0)
			    
#define	UT_FETCH_DOUBLE(bufp, off, index, value)			\
	do {								\
		UT_BUFFER_CAST_DECL(const jdouble, __src, bufp, off, index); \
		UT_FETCH(value, __src, sizeof(jdouble));		\
	} while (0)

#define	UT_STORE_BYTE(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(jbyte, __dst, bufp, off, index);	\
		UT_STORE(__dst, value, sizeof(jbyte));			\
	} while (0)

#define	UT_STORE_SHORT(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(jshort, __dst, bufp, off, index);	\
		UT_STORE(__dst, value, sizeof(jshort));			\
	} while (0)

#define	UT_STORE_INT(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(jint, __dst, bufp, off, index);	\
		UT_STORE(__dst, value, sizeof(jint));			\
	} while (0)

#define	UT_STORE_LONG(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(jlong, __dst, bufp, off, index);	\
		UT_STORE(__dst, value, sizeof(jlong));			\
	} while (0)

#define	UT_STORE_FLOAT(bufp, off, index, value)				\
	do {								\
		UT_BUFFER_CAST_DECL(jfloat, __dst, bufp, off, index);	\
		UT_STORE(__dst, value, sizeof(jfloat));			\
	} while (0)

#define	UT_STORE_DOUBLE(bufp, off, index, value)			\
	do {								\
		UT_BUFFER_CAST_DECL(jdouble, __dst, bufp, off, index);	\
		UT_STORE(__dst, value, sizeof(jdouble));		\
	} while (0)

/*
 * Field information.
 */
typedef struct {
	const char	*uf_name;	/* fully qualified field name */
	uint32_t	uf_offset;	/* field offset */
	uint32_t	uf_size;	/* size of one field element */
	pfc_rbnode_t	uf_node;	/* Red-Black tree node */
} ut_fldinfo_t;

#define	UT_FLDINFO_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ut_fldinfo_t, uf_node)

/*
 * Declare test structure field information.
 */
#define	UT_FLDINFO_DECL(stname, field)					\
	{								\
		.uf_name	= #field,				\
		.uf_offset	= offsetof(struct stname, field),	\
		.uf_size	= sizeof(((struct stname *)0)->field),	\
	}

#define	UT_FLDINFO_ARRAY_DECL(stname, field)				\
	{								\
		.uf_name	= #field,				\
		.uf_offset	= offsetof(struct stname, field),	\
		.uf_size	= sizeof(((struct stname *)0)->field[0]), \
	}

/*
 * Test structure information.
 */
typedef struct {
	const char	*us_name;	/* the name of this structure */
	const char	*us_signature;	/* signature of this structure */
	uint32_t	us_size;	/* size of this structure */
	pfc_rbtree_t	us_field;	/* field information */
} ut_strinfo_t;

#define	UT_STRINFO_FIELD_INIT()						\
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, ut_fldinfo_getkey)

#define	UT_STRINFO_HANDLE(usp)		((jlong)(uint64_t)(uintptr_t)(usp))
#define	UT_STRINFO_PTR(info)		((ut_strinfo_t *)(uintptr_t)(info))

/*
 * Declare test structure information.
 */
#define	UT_STRINFO_DECL(name)					\
	{							\
		.us_name	= #name,			\
		.us_signature	= __PFC_IPCTMPL_SIG_##name,	\
		.us_size	= sizeof(struct name),		\
		.us_field	= UT_STRINFO_FIELD_INIT(),	\
	}

/*
 * Internal prototypes.
 */
static ut_fldinfo_t	*ut_fldinfo_lookup(JNIEnv *PFC_RESTRICT env,
					   ut_strinfo_t *PFC_RESTRICT usp,
					   jstring jname);
static pfc_cptr_t	ut_fldinfo_getkey(pfc_rbnode_t *node);

/*
 * Test structure information.
 */
static ut_strinfo_t	info_ut_struct_1 = UT_STRINFO_DECL(ut_struct_1);
static ut_strinfo_t	info_ut_struct_2 = UT_STRINFO_DECL(ut_struct_2);
static ut_strinfo_t	info_ut_struct_3 = UT_STRINFO_DECL(ut_struct_3);
static ut_strinfo_t	info_ut_struct_4 = UT_STRINFO_DECL(ut_struct_4);

/*
 * Field information for ut_struct_1.
 */
static ut_fldinfo_t	fields_ut_struct_1[] = {
	UT_FLDINFO_DECL(ut_struct_1, ut1_int8),
	UT_FLDINFO_DECL(ut_struct_1, ut1_uint8),
	UT_FLDINFO_DECL(ut_struct_1, ut1_int16),
	UT_FLDINFO_DECL(ut_struct_1, ut1_uint16),
	UT_FLDINFO_DECL(ut_struct_1, ut1_int32),
	UT_FLDINFO_DECL(ut_struct_1, ut1_uint32),
	UT_FLDINFO_DECL(ut_struct_1, ut1_int64),
	UT_FLDINFO_DECL(ut_struct_1, ut1_uint64),
	UT_FLDINFO_DECL(ut_struct_1, ut1_float),
	UT_FLDINFO_DECL(ut_struct_1, ut1_double),
	UT_FLDINFO_DECL(ut_struct_1, ut1_ipv4),
	UT_FLDINFO_DECL(ut_struct_1, ut1_ipv6),
};

/*
 * Field information for ut_struct_2.
 */
static ut_fldinfo_t	fields_ut_struct_2[] = {
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_int8),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_uint8),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_int16),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_uint16),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_int32),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_uint32),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_int64),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_uint64),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_float),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_double),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_ipv4),
	UT_FLDINFO_ARRAY_DECL(ut_struct_2, ut2_ipv6),
};

#define	UT_FLDINFO_STRUCT1_DECL(stname, parent)		\
	UT_FLDINFO_DECL(stname, parent.ut1_int8),	\
	UT_FLDINFO_DECL(stname, parent.ut1_uint8),	\
	UT_FLDINFO_DECL(stname, parent.ut1_int16),	\
	UT_FLDINFO_DECL(stname, parent.ut1_uint16),	\
	UT_FLDINFO_DECL(stname, parent.ut1_int32),	\
	UT_FLDINFO_DECL(stname, parent.ut1_uint32),	\
	UT_FLDINFO_DECL(stname, parent.ut1_int64),	\
	UT_FLDINFO_DECL(stname, parent.ut1_uint64),	\
	UT_FLDINFO_DECL(stname, parent.ut1_float),	\
	UT_FLDINFO_DECL(stname, parent.ut1_double),	\
	UT_FLDINFO_DECL(stname, parent.ut1_ipv4),	\
	UT_FLDINFO_DECL(stname, parent.ut1_ipv6)

#define	UT_FLDINFO_STRUCT2_DECL(stname, parent)			\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_int8),		\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_uint8),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_int16),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_uint16),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_int32),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_uint32),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_int64),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_uint64),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_float),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_double),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_ipv4),		\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut2_ipv6)

/*
 * Field information for ut_struct_3.
 */
static ut_fldinfo_t	fields_ut_struct_3[] = {
	/* ut3_struct1 */
	UT_FLDINFO_DECL(ut_struct_3, ut3_struct1),
	UT_FLDINFO_STRUCT1_DECL(ut_struct_3, ut3_struct1),

	/* ut3_struct2 */
	UT_FLDINFO_ARRAY_DECL(ut_struct_3, ut3_struct2),
	UT_FLDINFO_STRUCT2_DECL(ut_struct_3, ut3_struct2[0]),
	UT_FLDINFO_STRUCT2_DECL(ut_struct_3, ut3_struct2[1]),
	UT_FLDINFO_STRUCT2_DECL(ut_struct_3, ut3_struct2[2]),
	UT_FLDINFO_STRUCT2_DECL(ut_struct_3, ut3_struct2[3]),
};

#define	UT_FLDINFO_STRUCT3_DECL(stname, parent)			\
	UT_FLDINFO_DECL(stname, parent.ut3_struct1),		\
	UT_FLDINFO_STRUCT1_DECL(stname, parent.ut3_struct1),	\
	UT_FLDINFO_ARRAY_DECL(stname, parent.ut3_struct2),	\
	UT_FLDINFO_STRUCT2_DECL(stname, parent.ut3_struct2[0]),	\
	UT_FLDINFO_STRUCT2_DECL(stname, parent.ut3_struct2[1]),	\
	UT_FLDINFO_STRUCT2_DECL(stname, parent.ut3_struct2[2]),	\
	UT_FLDINFO_STRUCT2_DECL(stname, parent.ut3_struct2[3])

/*
 * Field information for ut_struct_4.
 */
static ut_fldinfo_t	fields_ut_struct_4[] = {
	UT_FLDINFO_DECL(ut_struct_4, ut4_int8),
	UT_FLDINFO_DECL(ut_struct_4, ut4_int64),

	/* ut4_struct3 */
	UT_FLDINFO_ARRAY_DECL(ut_struct_4, ut4_struct3),
	UT_FLDINFO_STRUCT3_DECL(ut_struct_4, ut4_struct3[0]),
	UT_FLDINFO_STRUCT3_DECL(ut_struct_4, ut4_struct3[1]),
	UT_FLDINFO_STRUCT3_DECL(ut_struct_4, ut4_struct3[2]),
};

#define	UT_STRUCT_INFO_INIT(info, fields)				\
	do {								\
		ut_fldinfo_t	*__ufp;					\
		pfc_rbtree_t	*__tree = &(info).us_field;		\
									\
		for (__ufp = (fields); __ufp < PFC_ARRAY_LIMIT(fields);	\
		     __ufp++) {						\
			PFC_ASSERT_INT(pfc_rbtree_put(__tree,		\
						      &__ufp->uf_node), 0); \
		}							\
	} while (0)

/*
 * static void PFC_FATTR_INIT
 * ut_struct_init(void)
 *	Initialize structure information.
 */
static void PFC_FATTR_INIT
ut_struct_init(void)
{
	UT_STRUCT_INFO_INIT(info_ut_struct_1, fields_ut_struct_1);
	UT_STRUCT_INFO_INIT(info_ut_struct_2, fields_ut_struct_2);
	UT_STRUCT_INFO_INIT(info_ut_struct_3, fields_ut_struct_3);
	UT_STRUCT_INFO_INIT(info_ut_struct_4, fields_ut_struct_4);
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStructTest_load(
 *	JNIEnv *env, jclass cls)
 *	Load IPC structure information file for JUnit tests.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStructTest_load(
	JNIEnv *env, jclass cls)
{
	char	*path;
	int	err;

	/*
	 * Determine whether the IPC structure information file is safe
	 * or not.
	 */
	path = strdup(IPC_STRUCT_BIN);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to copy IPC structure file path.");

		return JNI_FALSE;
	}

	err = pfc_is_safepath(path);
	free(path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return JNI_FALSE;
	}

	err = pfc_ipc_struct_loaddefault(IPC_STRUCT_BIN, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "IPC struct information does not exist.");
		}
		else {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "Failed to load IPC structure information: "
				   "err=%d", err);
		}

		return JNI_FALSE;
	}

	path = strdup(UT_STRUCT_BIN);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to copy IPC structure file path for UT.");
	}

	err = pfc_is_safepath(path);
	free(path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return JNI_FALSE;
	}

	err = pfc_ipc_struct_loadfile(UT_STRUCT_BIN, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to load %s: err=%d", UT_STRUCT_BIN, err);
	}

	return JNI_TRUE;
}

/*
 * Declare accessor for primitive types.
 */
#define	UT_ACCESSOR_DECL(fpref, mpref, jtype)				\
	JNIEXPORT jtype JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_TestStruct_get##fpref##At(	\
		JNIEnv *env, jobject this, jstring jname, jint index,	\
		jlong buffer, jlong info)				\
	{								\
		uint8_t		*bufp = UT_BUFFER_PTR(buffer);		\
		ut_strinfo_t	*usp = UT_STRINFO_PTR(info);		\
		ut_fldinfo_t	*ufp;					\
		jtype		value;					\
									\
		PFC_ASSERT(usp != NULL);				\
									\
		ufp = ut_fldinfo_lookup(env, usp, jname);		\
		if (PFC_EXPECT_FALSE(ufp == NULL)) {			\
			return (jtype)0;				\
		}							\
									\
		UT_FETCH_##mpref(bufp, ufp->uf_offset, index, value);	\
									\
		return value;						\
	}								\
									\
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_TestStruct_set##fpref##At(	\
		JNIEnv *env, jobject this, jstring jname, jint index,	\
		jtype value, jlong buffer, jlong info)			\
	{								\
		uint8_t		*bufp = UT_BUFFER_PTR(buffer);		\
		ut_strinfo_t	*usp = UT_STRINFO_PTR(info);		\
		ut_fldinfo_t	*ufp;					\
									\
		PFC_ASSERT(usp != NULL);				\
									\
		ufp = ut_fldinfo_lookup(env, usp, jname);		\
		if (PFC_EXPECT_TRUE(ufp != NULL)) {			\
			UT_STORE_##mpref(bufp, ufp->uf_offset, index,	\
					 value);			\
		}							\
	}

UT_ACCESSOR_DECL(Byte, BYTE, jbyte);
UT_ACCESSOR_DECL(Short, SHORT, jshort);
UT_ACCESSOR_DECL(Int, INT, jint);
UT_ACCESSOR_DECL(Long, LONG, jlong);
UT_ACCESSOR_DECL(Float, FLOAT, jfloat);
UT_ACCESSOR_DECL(Double, DOUBLE, jdouble);

/*
 * JNIEXPORT jbyteArray JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_getInetAddressAt(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
 *	jlong info)
 *
 *	Return a raw IP address at the specified field.
 */
JNIEXPORT jbyteArray JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_getInetAddressAt(
	JNIEnv *env, jobject this, jstring jname, jint index, jlong buffer,
	jlong info)
{
	uint8_t		*bufp = UT_BUFFER_PTR(buffer);
	ut_strinfo_t	*usp = UT_STRINFO_PTR(info);
	ut_fldinfo_t	*ufp;
	const jbyte	*ptr;

	PFC_ASSERT(usp != NULL);

	ufp = ut_fldinfo_lookup(env, usp, jname);
	if (PFC_EXPECT_FALSE(ufp == NULL)) {
		return NULL;
	}

	ptr = UT_BUFFER_CAST(const jbyte, bufp, ufp->uf_offset, -1);
	if (index > 0) {
		ptr += ufp->uf_size * index;
	}

	return pjni_bytearray_new(env, ptr, ufp->uf_size);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_setInetAddressAt(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jbyteArray value,
 *	jlong buffer, jlong info)
 *
 *	Set a raw IP address into the specified field.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_setInetAddressAt(
	JNIEnv *env, jobject this, jstring jname, jint index, jbyteArray value,
	jlong buffer, jlong info)
{
	uint8_t		*bufp = UT_BUFFER_PTR(buffer);
	ut_strinfo_t	*usp = UT_STRINFO_PTR(info);
	ut_fldinfo_t	*ufp;

	PFC_ASSERT(usp != NULL);

	if (PFC_EXPECT_FALSE(value == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "value is null.");

		return;
	}

	ufp = ut_fldinfo_lookup(env, usp, jname);
	if (PFC_EXPECT_TRUE(ufp != NULL)) {
		uint32_t	alen = (*env)->GetArrayLength(env, value);
		UT_BUFFER_CAST_DECL(jbyte, ptr, bufp, ufp->uf_offset, -1);

		if (index > 0) {
			ptr += ufp->uf_size * index;
		}
		if (PFC_EXPECT_FALSE(ufp->uf_size != alen)) {
			pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
				   "Unexpected array size: %u", alen);

			return;
		}

		(*env)->GetByteArrayRegion(env, value, 0, alen, ptr);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_randomize(
 *	JNIEnv *env, jclass cls, jlong buffer, jint base, jlong info)
 *
 *	Fill the IPC structure with random bytes.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_randomize(
	JNIEnv *env, jclass cls, jlong buffer, jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	uint8_t		*buf;
	size_t		size;
	int		fd;

	fd = open(PFC_URANDOM_DEVICE_PATH, O_RDONLY);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to open random device(%s): err=%d",
			   PFC_URANDOM_DEVICE_PATH, errno);

		return;
	}

	buf = (uint8_t *)(sbp->isb_data + base);
	size = IPC_STRINFO_SIZE(sip);

	do {
		ssize_t	nbytes = read(fd, buf, size);

		if (PFC_EXPECT_FALSE(nbytes == (ssize_t)-1)) {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "read(%s) failed: err=%d",
				   PFC_URANDOM_DEVICE_PATH, errno);
			break;
		}

		buf += nbytes;
		size -= nbytes;
	} while (size != 0);

	(void)close(fd);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_fill(
 *	JNIEnv *env, jclass cls, jbyte b, jlong buffer, jint base, jlong info)
 *
 *	Fill the IPC structure with the specified byte.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_fill(
	JNIEnv *env, jclass cls, jbyte b, jlong buffer, jint base, jlong info)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	uint8_t		*buf;

	buf = (uint8_t *)(sbp->isb_data + base);
	memset(buf, b, IPC_STRINFO_SIZE(sip));
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_checkObject(
 *	JNIEnv *env, jobject this, jlong buffer, jint base, jlong info,
 *	jlong tinfi)
 *
 *	Ensure that IPC structure is initialized properly.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_checkObject(
	JNIEnv *env, jobject this, jlong buffer, jint base, jlong info,
	jlong tinfo)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ut_strinfo_t	*usp = UT_STRINFO_PTR(tinfo);
	const char	*stname = IPC_STRINFO_NAME(sip);
	uint32_t	limit;

	if (PFC_EXPECT_FALSE(strcmp(stname, usp->us_name) != 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Illegal structure name: \"%s\", expected \"%s\"",
			   stname, usp->us_name);

		return;
	}

	if (PFC_EXPECT_FALSE(memcmp(sip->sti_sig, usp->us_signature,
				    IPC_STRUCT_SIG_SIZE) != 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Illegal signature: \"%s\", expected \"%s\"",
			   sip->sti_sig, usp->us_signature);

		return;
	}

	if (PFC_EXPECT_FALSE(IPC_STRINFO_SIZE(sip) != usp->us_size)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Illegal size: %u, expected %u",
			   IPC_STRINFO_SIZE(sip), usp->us_size);

		return;
	}

	if (PFC_EXPECT_FALSE((uint32_t)base >= sbp->isb_size)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Base offset(%u) exceeds the buffer size(%u).",
			   (uint32_t)base, sbp->isb_size);

		return;
	}

	limit = (uint32_t)base + usp->us_size;
	if (PFC_EXPECT_FALSE(limit > sbp->isb_size)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "End offset(%u) exceeds the buffer size(%u): "
			   "base=%u", limit, sbp->isb_size, (uint32_t)base);

		return;
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_sync(
 *	JNIEnv *env, jobject this, jlong buffer, jint base, jlong tbuffer,
 *	jlong tinfo)
 *
 *	Copy contents of the IPC structure into this structure.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_sync(
	JNIEnv *env, jobject this, jlong buffer, jint base, jlong tbuffer,
	jlong tinfo)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	uint8_t		*bufp = UT_BUFFER_PTR(tbuffer);
	ut_strinfo_t	*usp = UT_STRINFO_PTR(tinfo);
	uint8_t		*ptr;

	ptr = (uint8_t *)(sbp->isb_data + base);

	memcpy(bufp, ptr, usp->us_size);
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_equals(
 *	JNIEnv *env, jobject this, jlong buffer, jint base, jlong tbuffer,
 *	jlong tinfo)
 *
 *	Determine the specified IpcStruct equals this test structure.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_equals(
	JNIEnv *env, jobject this, jlong buffer, jint base, jlong tbuffer,
	jlong tinfo)
{
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	uint8_t		*bufp = UT_BUFFER_PTR(tbuffer);
	ut_strinfo_t	*usp = UT_STRINFO_PTR(tinfo);
	const uint8_t	*ptr;

	ptr = (const uint8_t *)(sbp->isb_data + base);

	return (memcmp(ptr, bufp, usp->us_size) == 0) ? JNI_TRUE : JNI_FALSE;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_setStructAt(
 *	JNIEnv *env, jobject this, jstring jname, jint index, jlong stbuf,
 *	jint stbase, jlong stinfo, jlong buffer, jlong info)
 *
 *	Copy IpcStruct to this test structure field.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_setStructAt(
	JNIEnv *env, jobject this, jstring jname, jint index, jlong stbuf,
	jint stbase, jlong stinfo, jlong buffer, jlong info)
{
	uint8_t		*bufp = UT_BUFFER_PTR(buffer);
	ut_strinfo_t	*usp = UT_STRINFO_PTR(info);
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(stbuf);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(stinfo);
	ut_fldinfo_t	*ufp;

	PFC_ASSERT(usp != NULL);
	PFC_ASSERT(bufp != NULL);
	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	ufp = ut_fldinfo_lookup(env, usp, jname);
	if (PFC_EXPECT_TRUE(ufp != NULL)) {
		uint32_t	size = IPC_STRINFO_SIZE(sip);
		UT_BUFFER_CAST_DECL(uint8_t, ptr, bufp, ufp->uf_offset, -1);

		if (index > 0) {
			ptr += ufp->uf_size * index;
		}

		if (PFC_EXPECT_FALSE(ptr + size > bufp + usp->us_size)) {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "Buffer overflow: uf_offset=%u, index = %d",
				   ufp->uf_offset, index);
		}

		memcpy(ptr, sbp->isb_data + stbase, size);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestStruct_finalize(
 *	JNIEnv *env, jobject this, jlong buffer)
 *
 *	Finalize the test structure object.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestStruct_finalize(
	JNIEnv *env, jobject this, jlong buffer)
{
	uint8_t		*bufp = UT_BUFFER_PTR(buffer);

	if (PFC_EXPECT_TRUE(bufp != NULL)) {
		free(bufp);
	}
}

/*
 * Declare test structure methods.
 */
#define	UT_STRUCT_METHOD_DECL(clname, stname)				\
	JNIEXPORT jlong JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_##clname##_createBuffer(	\
		JNIEnv *env, jobject this)				\
	{								\
		void	*bufp = calloc(1, sizeof(stname##_t));		\
									\
		if (PFC_EXPECT_FALSE(bufp == NULL)) {			\
			pjni_throw(env, PJNI_CLASS(OutOfMemoryError),	\
				   "Unable to allocate %s.", #stname);	\
		}							\
									\
		return UT_BUFFER_HANDLE(bufp);				\
	}								\
									\
	JNIEXPORT jlong JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_##clname##_getInfo(		\
		JNIEnv *env, jobject this)				\
	{								\
		return UT_STRINFO_HANDLE(&info_##stname);		\
	}

UT_STRUCT_METHOD_DECL(TestStruct1, ut_struct_1);
UT_STRUCT_METHOD_DECL(TestStruct2, ut_struct_2);
UT_STRUCT_METHOD_DECL(TestStruct3, ut_struct_3);
UT_STRUCT_METHOD_DECL(TestStruct4, ut_struct_4);

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStructFieldTest_equalsStruct(
 *	JNIEnv *env, jobject this, jlong buffer1, jint base1, jlong buffer2,
 *	jint base2, jlong info)
 *
 *	Compare given two IpcStruct objects.
 *
 * Calling/Exit State:
 *	JNI_TRUE is returned if the specified objects are identical.
 *	Otherwise JNI_FALSE is returned.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_InnerStructFieldTest_equalsStruct(
	JNIEnv *env, jobject this, jlong buffer1, jint base1, jlong buffer2,
	jint base2, jlong info)
{
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_strbuf_t	*sbp1 = JIPC_STRBUF_PTR(buffer1);
	ipc_strbuf_t	*sbp2 = JIPC_STRBUF_PTR(buffer2);
	const uint8_t	*ptr1 = sbp1->isb_data + base1;
	const uint8_t	*ptr2 = sbp2->isb_data + base2;

	return (memcmp(ptr1, ptr2, IPC_STRINFO_SIZE(sip)) == 0)
		? JNI_TRUE : JNI_FALSE;
}

/*
 * static ut_fldinfo_t *
 * ut_fldinfo_lookup(JNIEnv *PFC_RESTRICT env, ut_strinfo_t *PFC_RESTRICT usp,
 *		     jstring jname)
 *	Search for a field information associated with the specified structure
 *	and field name.
 */
static ut_fldinfo_t *
ut_fldinfo_lookup(JNIEnv *PFC_RESTRICT env, ut_strinfo_t *PFC_RESTRICT usp,
		  jstring jname)
{
	ut_fldinfo_t	*ufp;
	const char	*name;

	if (PFC_EXPECT_FALSE(jname == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "name is null.");

		return NULL;
	}

	ufp = NULL;
	name = pjni_string_get(env, jname);
	if (PFC_EXPECT_TRUE(name != NULL)) {
		pfc_rbnode_t	*node;

		node = pfc_rbtree_get(&usp->us_field, name);
		if (PFC_EXPECT_TRUE(node != NULL)) {
			ufp = UT_FLDINFO_NODE2PTR(node);
		}
		else {
			pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
				   "Unknown field name: %s", name);
		}

		pjni_string_release(env, jname, name);
	}

	return ufp;
}

/*
 * static pfc_cptr_t
 * ut_fldinfo_getkey(pfc_rbnode_t *node)
 *	Return the key of the specified node.
 *	`node' must be a pointer to uf_node in ut_fldinfo_t.
 */
static pfc_cptr_t
ut_fldinfo_getkey(pfc_rbnode_t *node)
{
	ut_fldinfo_t	*ufp = UT_FLDINFO_NODE2PTR(node);

	return (pfc_cptr_t)ufp->uf_name;
}
