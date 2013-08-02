/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * session.c - IPC client session management.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <org_opendaylight_vtn_core_ipc_DefaultConnection.h>
#include <org_opendaylight_vtn_core_ipc_ClientSession.h>
#include <org_opendaylight_vtn_core_ipc_IpcInt8.h>
#include <org_opendaylight_vtn_core_ipc_IpcInt16.h>
#include <org_opendaylight_vtn_core_ipc_IpcInt32.h>
#include <org_opendaylight_vtn_core_ipc_IpcInt64.h>
#include <org_opendaylight_vtn_core_ipc_IpcUint8.h>
#include <org_opendaylight_vtn_core_ipc_IpcUint16.h>
#include <org_opendaylight_vtn_core_ipc_IpcUint32.h>
#include <org_opendaylight_vtn_core_ipc_IpcUint64.h>
#include <org_opendaylight_vtn_core_ipc_IpcFloat.h>
#include <org_opendaylight_vtn_core_ipc_IpcDouble.h>
#include <org_opendaylight_vtn_core_ipc_IpcInet4Address.h>
#include <org_opendaylight_vtn_core_ipc_IpcInet6Address.h>
#include <org_opendaylight_vtn_core_ipc_IpcString.h>
#include <org_opendaylight_vtn_core_ipc_IpcBinary.h>
#include <org_opendaylight_vtn_core_ipc_IpcNull.h>
#include <org_opendaylight_vtn_core_ipc_IpcStruct.h>
#include "ipcclnt_jni.h"
#include "ipc_struct_jni.h"

#if	org_opendaylight_vtn_core_ipc_ClientSession_C_CANCELABLE != \
	PFC_IPCSSF_CANCELABLE
#error	Invalid value: ClientSession.C_CANCELABLE
#endif	/* ClientSession.C_CANCELABLE != PFC_IPCSSF_CANCELABLE */

#if	org_opendaylight_vtn_core_ipc_ClientSession_C_NOGLOBCANCEL != \
	PFC_IPCSSF_NOGLOBCANCEL
#error	Invalid value: ClientSession.C_NOGLOBCANCEL
#endif	/* ClientSession.C_NOGLOBCANCEL != PFC_IPCSSF_NOGLOBCANCEL */

#if	org_opendaylight_vtn_core_ipc_ClientSession_RESP_FATAL != \
	PFC_IPCRESP_FATAL
#error	Invalid value: ClientSession.RESP_FATAL
#endif	/* ClientSession.RESP_FATAL != PFC_IPCRESP_FATAL */

#define	SESS_INVALID	org_opendaylight_vtn_core_ipc_ClientSession_SESS_INVALID

/*
 * Determine error number which indicates invalid client session state.
 */
#define	JIPC_CLSESS_STATE_ERROR(state)		\
	(((state) == IPC_SSTATE_BUSY) ? EBUSY		\
	 : ((state) == IPC_SSTATE_DISCARD) ? ESHUTDOWN	\
	 : EBADFD)

/*
 * Exception table for IPC service invocation.
 */
static jipc_cexcept_t	extable_invoke[] = {
	JIPC_EXCEPT_DECL(EBADFD, IpcBadStateException),
	JIPC_EXCEPT_DECL(EPROTO, IpcBadProtocolException),
	JIPC_EXCEPT_DECL(ENOSPC, IpcTooManyClientsException),
	JIPC_EXCEPT_DECL(ENOSYS, IpcUnknownServiceException),
	JIPC_EXCEPT_END(),
};

/*
 * Exception table for adding output to the IPC server.
 */
static jipc_cexcept_t	extable_output[] = {
	JIPC_EXCEPT_DECL(EBADFD, IpcBadStateException),
	JIPC_EXCEPT_DECL(E2BIG, IpcTooBigDataException),
	JIPC_EXCEPT_END(),
};

/*
 * Exception table for getting response of the IPC server.
 */
static jipc_cexcept_t	extable_response[] = {
	JIPC_EXCEPT_DECL(EBADFD, IpcBadStateException),
	JIPC_EXCEPT_DECL(EPROTO, IpcBadProtocolException),
	JIPC_EXCEPT_END(),
};

/*
 * Exception table for forwarding additional data.
 */
static jipc_cexcept_t	extable_forward[] = {
	JIPC_EXCEPT_DECL(EINVAL, IpcDataIndexOutOfBoundsException),
	JIPC_EXCEPT_DECL(EBADFD, IpcBadStateException),
	JIPC_EXCEPT_DECL(E2BIG, IpcTooBigDataException),
	JIPC_EXCEPT_DECL(EBADFD, IpcBadStateException),
	JIPC_EXCEPT_DECL(EPROTO, IpcBadProtocolException),
	JIPC_EXCEPT_DECL(ENODEV, IpcUnknownStructException),
	JIPC_EXCEPT_DECL(EBADMSG, IpcStructLayoutMismatchException),
	JIPC_EXCEPT_END(),
};

/*
 * Internal prototypes.
 */
static void	conn_not_found(JNIEnv *env, pfc_ipcconn_t conn);

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_ipc_DefaultConnection_createSession(
 *	JNIEnv *env, jobject this, jint handle, jstring name, jint service,
 *	jint flags)
 *
 *	Create a new IPC client session on the default connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new client session handle is returned.
 *	Otherwise SESS_INVALID is returned with throwing an exception.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_ipc_DefaultConnection_createSession(
	JNIEnv *env, jobject this, jint handle, jstring name, jint service,
	jint flags)
{
	const char	*svname;
	pfc_ipcsess_t	*sess;
	int		err;
	jlong		session;

	svname = pjni_string_get(env, name);
	if (PFC_EXPECT_FALSE(svname == NULL)) {
		return SESS_INVALID;
	}

	err = pfc_ipcclnt_sess_create4(&sess, svname, service, flags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to create a client session on the default "
			   "connection: err=%d", err);
		session = SESS_INVALID;
	}
	else {
		session = SESS_HANDLE(sess);
	}

	pjni_string_release(env, name, svname);

	return session;
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_ipc_AltConnection_createSession(
 *	JNIEnv *env, jobject this, jint handle, jstring name, jint service,
 *	jint flags)
 *
 *	Create a new IPC client session on the specified alternative
 *	connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new client session handle is returned.
 *	Otherwise SESS_INVALID is returned with throwing an exception.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_ipc_AltConnection_createSession(
	JNIEnv *env, jobject this, jint handle, jstring name, jint service,
	jint flags)
{
	const char	*svname;
	pfc_ipcconn_t	conn = (pfc_ipcconn_t)handle;
	pfc_ipcsess_t	*sess;
	int		err;
	jlong		session;

	svname = pjni_string_get(env, name);
	if (PFC_EXPECT_FALSE(svname == NULL)) {
		return SESS_INVALID;
	}

	err = pfc_ipcclnt_sess_altcreate5(&sess, conn, svname, service, flags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == EBADF) {
			conn_not_found(env, conn);
		}
		else {
			jipc_throw(env, err, NULL,
				   "Failed to create a client session: "
				   "conn=%u, err=%d", conn, err);
		}
		session = SESS_INVALID;
	}
	else {
		session = SESS_HANDLE(sess);
	}

	pjni_string_release(env, name, svname);

	return session;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_reset(
 *	JNIEnv *env, jobject this, jlong session, jstring name, jint service)
 *
 *	Reset the specified IPC client session.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_reset(
	JNIEnv *env, jobject this, jlong session, jstring name, jint service)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	const char	*svname;

	PFC_ASSERT(sess != NULL);
	svname = jipc_getstring(env, name, "IPC service name");
	if (PFC_EXPECT_TRUE(svname != NULL)) {
		int	err;

		err = pfc_ipcclnt_sess_reset(sess, svname, service);
		if (PFC_EXPECT_FALSE(err != 0)) {
			jipc_throw(env, err, NULL,
				   "Failed to reset the client session: "
				   "sess=%p, err=%d", sess, err);
		}

		pjni_string_release(env, name, svname);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_setTimeout(
 *	JNIEnv *env, jobject this, jlong session, jlong sec, jlong nsec)
 *
 *	Set session timeout to the specified IPC client session.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_setTimeout(
	JNIEnv *env, jobject this, jlong session, jlong sec, jlong nsec)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	pfc_timespec_t	*tsp, tsbuf;
	int		err;

	PFC_ASSERT(sess != NULL);

	if (sec < 0) {
		/* No timeout. */
		PFC_ASSERT(nsec < 0);
		tsp = NULL;
	}
	else {
		tsbuf.tv_sec = sec;
		tsbuf.tv_nsec = nsec;
		tsp = &tsbuf;
	}

	err = pfc_ipcclnt_sess_settimeout(sess, tsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to set client session timeout: "
			   "sess=%p, err=%d", sess, err);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_cancel(
 *	JNIEnv *env, jobject this, jlong session, jboolean discard)
 *
 *	Cancel ongoing IPC service invocation on the specified IPC client
 *	session.
 *
 *	If PC_TRUE is passed to `discard', the state of the client session
 *	will be changed to DISCARD. That is, further IPC service request on
 *	the specified session will get ESHUTDOWN error.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_cancel(
	JNIEnv *env, jobject this, jlong session, jboolean discard)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	int		err;

	PFC_ASSERT(sess != NULL);
	err = pfc_ipcclnt_sess_cancel(sess, discard);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to cancel service invocation: "
			   "sess=%p, err=%d", sess, err);
	}
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_invoke(
 *	JNIEnv *env, jobject this, jlong session)
 *
 *	Invoke an IPC service on the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, a response code received from the IPC
 *	server is returned.
 *
 *	Otherwise PFC_IPCRESP_FATAL is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_invoke(
	JNIEnv *env, jobject this, jlong session)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	pfc_ipcresp_t	resp;
	int		err;

	PFC_ASSERT(sess != NULL);
	err = pfc_ipcclnt_sess_invoke(sess, &resp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_invoke,
			   "Failed to invoke IPC service: sess=%p, err=%d",
			   sess, err);
		resp = PFC_IPCRESP_FATAL;
	}

	return (jint)resp;
}

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_getResponse(
 *	JNIEnv *env, jobject this, jlong session, jint index)
 *
 *	Get an additional data received from the IPC server at the specified
 *	additional array index.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IpcDataUnit object
 *	is returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_getResponse(
	JNIEnv *env, jobject this, jlong session, jint index)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	ipc_sstate_t	state;
	jobject		obj;

	/*
	 * Remarks:
	 *	IPC client library returns a pointer to raw data in the session
	 *	if STRING or BINARY data is specified by index. But returned
	 *	pointer may be invalidated by another thread while constructing
	 *	a new IpcDataUnit object. So we construct an IpcDataUnit
	 *	object without use of IPC client library.
	 */

	/* Check session state. */
	IPC_CLSESS_LOCK(sess);
	state = sess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_RESULT)) {
		obj = jipc_pdu_getmsg(env, &sess->icss_msg, (uint32_t)index);
	}
	else {
		int	err = JIPC_CLSESS_STATE_ERROR(state);

		obj = NULL;
		jipc_throw(env, err, extable_response,
			   "Invalid client session state.");
	}
	IPC_CLSESS_UNLOCK(sess);

	return obj;
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_getResponseCount(
 *	JNIEnv *env, jobject this, jlong session)
 *
 *	Return the number of the additional data array elements received
 *	from the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of the additional data array
 *	is returned.
 *
 *	Otherwise zero is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_getResponseCount(
	JNIEnv *env, jobject this, jlong session)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	uint32_t	count;
	int		err;

	PFC_ASSERT(sess != NULL);
	err = pfc_ipcclnt_getrescount2(sess, &count);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_response,
			   "Failed to get the number of additional data: "
			   "sess=%p, err=%d", sess, err);
		count = 0;
	}

	return (jint)count;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_forward(
 *	JNIEnv *env, jobject this, jlong dst, jlong src, jint beginIndex,
 *	jint endIndex)
 *
 *	Copy additional data in `src' to `dst'.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_forward(
	JNIEnv *env, jobject this, jlong dst, jlong src, jint beginIndex,
	jint endIndex)
{
	pfc_ipcsess_t	*dsess = SESS_PTR(dst);
	pfc_ipcsess_t	*ssess = SESS_PTR(src);
	int		err;

	PFC_ASSERT(dsess != NULL);
	PFC_ASSERT(ssess != NULL);
	err = pfc_ipcclnt_forward(dsess, ssess, beginIndex, endIndex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_forward,
			   "Failed to forward additional data: err=%d", err);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientSession_destroy(
 *	JNIEnv *env, jclass cls, jlong session)
 *	Destroy the specified IPC client session.
 *
 * Remarks:
 *	The caller must ensure that the specified session is no longer used by
 *	another thread.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientSession_destroy(
	JNIEnv *env, jclass cls, jlong session)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);

	PFC_ASSERT(sess != NULL);
	PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);
}

/*
 * Declare methods to add a numerical data to the additional data array.
 */
#define	JIPC_CLIENT_OUTPUT_DECL(clname, jtype, itype, suffix)		\
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_##clname##_addClientOutput(	\
		JNIEnv *env, jobject this, jlong session, jtype value)	\
	{								\
		pfc_ipcsess_t	*sess = SESS_PTR(session);		\
		int		err;					\
									\
		PFC_ASSERT(sess != NULL);				\
		err = pfc_ipcclnt_output_##suffix(sess, (itype)value);	\
		if (PFC_EXPECT_TRUE(err == 0)) {			\
			return;						\
		}							\
									\
		jipc_throw(env, err, extable_output,			\
			   "Failed to add %s: sess=%p, err=%d",		\
			   #clname, sess, err);				\
	}

/*
 * void Java_org_opendaylight_vtn_core_ipc_IpcInt8_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jbyte value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcUint8_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jbyte value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcInt16_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jshort value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcUint16_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jshort value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcInt32_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jint value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcUint32_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jint value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcInt64_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jlong value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcUint64_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jlong value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcFloat_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jfloat value)
 * void Java_org_opendaylight_vtn_core_ipc_IpcDouble_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jdouble value)
 *
 *	Add an additional data to be sent to the IPC server.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JIPC_CLIENT_OUTPUT_DECL(IpcInt8, jbyte, int8_t, int8);
JIPC_CLIENT_OUTPUT_DECL(IpcUint8, jbyte, uint8_t, uint8);
JIPC_CLIENT_OUTPUT_DECL(IpcInt16, jshort, int16_t, int16);
JIPC_CLIENT_OUTPUT_DECL(IpcUint16, jshort, uint16_t, uint16);
JIPC_CLIENT_OUTPUT_DECL(IpcInt32, jint, int32_t, int32);
JIPC_CLIENT_OUTPUT_DECL(IpcUint32, jint, uint32_t, uint32);
JIPC_CLIENT_OUTPUT_DECL(IpcInt64, jlong, int64_t, int64);
JIPC_CLIENT_OUTPUT_DECL(IpcUint64, jlong, uint64_t, uint64);
JIPC_CLIENT_OUTPUT_DECL(IpcFloat, jfloat, float, float);
JIPC_CLIENT_OUTPUT_DECL(IpcDouble, jdouble, double, double);

/*
 * Declare methods to add an IP address to the additional data array.
 */
#define	JIPC_CLIENT_OUTPUT_IPADDR_DECL(clname, iaddrtype, suffix)	\
	JNIEXPORT void JNICALL						\
	Java_org_opendaylight_vtn_core_ipc_##clname##_addClientOutput(	\
		JNIEnv *env, jobject this, jlong session, jbyteArray addr) \
	{								\
		pfc_ipcsess_t	*sess = SESS_PTR(session);		\
		jsize		nelems;					\
		jbyte		*ptr;					\
		int		err;					\
									\
		PFC_ASSERT(sess != NULL);				\
		nelems = (*env)->GetArrayLength(env, addr);		\
		if (PFC_EXPECT_FALSE(nelems != sizeof(iaddrtype))) {	\
			/* This should never happen. */			\
			pjni_throw(env,					\
				   PJNI_CLASS(IllegalStateException),	\
				   "Unexpected IP address size: %u",	\
				   nelems);				\
									\
			return;						\
		}							\
									\
		ptr = pjni_bytearray_get(env, addr);			\
		if (PFC_EXPECT_FALSE(ptr == NULL)) {			\
			return;						\
		}							\
									\
		err = pfc_ipcclnt_output_##suffix(sess, (iaddrtype *)ptr); \
		pjni_bytearray_release(env, addr, ptr, PFC_TRUE);	\
		if (PFC_EXPECT_TRUE(err == 0)) {			\
			return;						\
		}							\
									\
		jipc_throw(env, err, extable_output,			\
			   "Failed to add %s: sess=%p, err=%d",		\
			   #clname, sess, err);				\
	}

/*
 * void Java_org_opendaylight_vtn_core_ipc_IpcInet4Address_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jbyteArray addr)
 * void Java_org_opendaylight_vtn_core_ipc_IpcInet6Address_addClientOutput
 *		(JNIEnv *env, jobject this, jlong session, jbyteArray addr)
 *
 *	Append an IP address to the end of the additional data array to be
 *	sent to the IPC server.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JIPC_CLIENT_OUTPUT_IPADDR_DECL(IpcInet4Address, struct in_addr, ipv4);
JIPC_CLIENT_OUTPUT_IPADDR_DECL(IpcInet6Address, struct in6_addr, ipv6);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcString_addClientOutput(
 *	JNIEnv *env, jobject this, jlong session, jstring value)
 *
 *	Append a string to the end of the additional data array to be sent
 *	to the IPC server.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcString_addClientOutput(
	JNIEnv *env, jobject this, jlong session, jstring value)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	const char	*string;
	int		err;

	PFC_ASSERT(sess != NULL);
	if (value == NULL) {
		/* Append a NULL string. */
		string = NULL;
	}
	else {
		string = pjni_string_get(env, value);
		if (PFC_EXPECT_FALSE(string == NULL)) {
			return;
		}
	}

	err = pfc_ipcclnt_output_string(sess, string);
	pjni_string_release(env, value, string);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return;
	}

	jipc_throw(env, err, extable_output,
		   "Failed to add IpcString: sess=%p, err=%d", sess, err);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcBinary_addClientOutput(
 *	JNIEnv *env, jobject this, jlong session, jbyteArray value)
 *
 *	Append a binary data to the end of the additional data array to be
 *	sent to the IPC server.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcBinary_addClientOutput(
	JNIEnv *env, jobject this, jlong session, jbyteArray value)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	jsize		nelems;
	jbyte		*ptr;
	int		err;

	PFC_ASSERT(sess != NULL);

	if (value == NULL) {
		/* Append a NULL data. */
		nelems = 0;
		ptr = NULL;
	}
	else {
		nelems = (*env)->GetArrayLength(env, value);
		ptr = pjni_bytearray_get(env, value);
		if (PFC_EXPECT_FALSE(ptr == NULL)) {
			return;
		}
	}

	err = pfc_ipcclnt_output_binary(sess, (const uint8_t *)ptr, nelems);
	pjni_bytearray_release(env, value, ptr, PFC_TRUE);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return;
	}

	jipc_throw(env, err, extable_output,
		   "Failed to add IpcBinary: sess=%p, err=%d", sess, err);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcNull_addClientOutput(
 *	JNIEnv *env, jobject this, jlong session)
 *
 *	Append a NULL data to the end of the additional data array to be
 *	sent to the IPC server.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcNull_addClientOutput(
	JNIEnv *env, jobject this, jlong session)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	int		err;

	PFC_ASSERT(sess != NULL);

	err = pfc_ipcclnt_output_null(sess);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return;
	}

	jipc_throw(env, err, extable_output,
		   "Failed to add IpcNull: sess=%p, err=%d", sess, err);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcStruct_addClientOutput(
 *	JNIEnv *env, jobject this, jlong session, jlong buffer, jint offset,
 *	jlong info)
 *
 *	Append an IPC structure to the end of the additional data array to be
 *	sent to the IPC server.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcStruct_addClientOutput(
	JNIEnv *env, jobject this, jlong session, jlong buffer, jint offset,
	jlong info)
{
	pfc_ipcsess_t	*sess = SESS_PTR(session);
	ipc_strbuf_t	*sbp = JIPC_STRBUF_PTR(buffer);
	ipc_strinfo_t	*sip = JIPC_STRINFO_PTR(info);
	ipc_sstate_t	state;
	const uint8_t	*data;
	int		err;

	PFC_ASSERT(sess != NULL);
	PFC_ASSERT(sbp != NULL);
	PFC_ASSERT(sip != NULL);

	data = (const uint8_t *)sbp->isb_data + offset;
	PFC_ASSERT(offset >= 0 &&
		   offset + IPC_STRINFO_SIZE(sip) <= sbp->isb_size);

	/*
	 * We can not use __pfc_ipcclnt_output_struct() here because
	 * it requires a structure layout signature to be terminated by
	 * '\0', but sip->sti_sig is not. So we call libpfc_ipc APIs
	 * directly.
	 */
	IPC_CLSESS_LOCK(sess);

	/* Check session state. */
	state = sess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_READY)) {
		err = pfc_ipcstream_add_known_struct(&sess->icss_output,
						     data, sip);
		if (PFC_EXPECT_TRUE(err == 0)) {
			IPC_CLSESS_UNLOCK(sess);

			return;
		}
	}
	else {
		err = JIPC_CLSESS_STATE_ERROR(state);
	}

	IPC_CLSESS_UNLOCK(sess);

	jipc_throw(env, err, extable_output,
		   "Failed to add IpcStruct: sess=%p, err=%d", sess, err);
}

/*
 * static void
 * conn_not_found(JNIEnv *env, pfc_ipcconn_t conn)
 *	Thrown an IpcBadConnectionException which indicates the connection
 *	does not exist.
 */
static void
conn_not_found(JNIEnv *env, pfc_ipcconn_t conn)
{
	pjni_throw(env, PJNI_CLASS(IpcBadConnectionException),
		   "The connection does not exist: conn=%u", conn);
}
