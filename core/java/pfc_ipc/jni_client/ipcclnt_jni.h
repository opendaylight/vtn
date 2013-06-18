/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPCCLNT_JNI_H
#define	_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPCCLNT_JNI_H

/*
 * Internal definitions for libpfc_ipcclnt_jni JNI library.
 */

#include <errno.h>
#include <pfc/jni.h>
#include <pfc/log.h>
#include <pfc/debug.h>
#include <ipcclnt_impl.h>

/*
 * Fully qualified Java class names.
 */
#define	PJNI_CLASS_IpcException				\
	"org/opendaylight/vtn/core/ipc/IpcException"
#define	PJNI_CLASS_IpcLibraryException			\
	"org/opendaylight/vtn/core/ipc/IpcLibraryException"
#define	PJNI_CLASS_IpcBadFileException			\
	"org/opendaylight/vtn/core/ipc/IpcBadFileException"
#define	PJNI_CLASS_IpcBadConnectionException			\
	"org/opendaylight/vtn/core/ipc/IpcBadConnectionException"
#define	PJNI_CLASS_IpcBadPoolException			\
	"org/opendaylight/vtn/core/ipc/IpcBadPoolException"
#define	PJNI_CLASS_IpcPermissionDeniedException				\
	"org/opendaylight/vtn/core/ipc/IpcPermissionDeniedException"
#define	PJNI_CLASS_IpcResourceBusyException			\
	"org/opendaylight/vtn/core/ipc/IpcResourceBusyException"
#define	PJNI_CLASS_IpcClientDisabledException			\
	"org/opendaylight/vtn/core/ipc/IpcClientDisabledException"
#define	PJNI_CLASS_IpcNoEntryException			\
	"org/opendaylight/vtn/core/ipc/IpcNoEntryException"
#define	PJNI_CLASS_IpcTimedOutException				\
	"org/opendaylight/vtn/core/ipc/IpcTimedOutException"
#define	PJNI_CLASS_IpcConnectionRefusedException			\
	"org/opendaylight/vtn/core/ipc/IpcConnectionRefusedException"
#define	PJNI_CLASS_IpcConnectionResetException			\
	"org/opendaylight/vtn/core/ipc/IpcConnectionResetException"
#define	PJNI_CLASS_IpcBrokenPipeException			\
	"org/opendaylight/vtn/core/ipc/IpcBrokenPipeException"
#define	PJNI_CLASS_IpcBadStateException				\
	"org/opendaylight/vtn/core/ipc/IpcBadStateException"
#define	PJNI_CLASS_IpcBadProtocolException			\
	"org/opendaylight/vtn/core/ipc/IpcBadProtocolException"
#define	PJNI_CLASS_IpcTooManyClientsException			\
	"org/opendaylight/vtn/core/ipc/IpcTooManyClientsException"
#define	PJNI_CLASS_IpcTooBigDataException			\
	"org/opendaylight/vtn/core/ipc/IpcTooBigDataException"
#define	PJNI_CLASS_IpcUnknownServiceException			\
	"org/opendaylight/vtn/core/ipc/IpcUnknownServiceException"
#define	PJNI_CLASS_IpcCanceledException				\
	"org/opendaylight/vtn/core/ipc/IpcCanceledException"
#define	PJNI_CLASS_IpcShutdownException				\
	"org/opendaylight/vtn/core/ipc/IpcShutdownException"
#define	PJNI_CLASS_IpcDataIndexOutOfBoundsException			\
	"org/opendaylight/vtn/core/ipc/IpcDataIndexOutOfBoundsException"
#define	PJNI_CLASS_IpcUnknownStructException			\
	"org/opendaylight/vtn/core/ipc/IpcUnknownStructException"
#define	PJNI_CLASS_IpcStructLayoutMismatchException			\
	"org/opendaylight/vtn/core/ipc/IpcStructLayoutMismatchException"
#define	PJNI_CLASS_IpcAlreadyExistsException			\
	"org/opendaylight/vtn/core/ipc/IpcAlreadyExistsException"
#define	PJNI_CLASS_IpcJniException			\
	"org/opendaylight/vtn/core/ipc/IpcJniException"

#define	PJNI_CLASS_IpcInt8	"org/opendaylight/vtn/core/ipc/IpcInt8"
#define	PJNI_CLASS_IpcInt16	"org/opendaylight/vtn/core/ipc/IpcInt16"
#define	PJNI_CLASS_IpcInt32	"org/opendaylight/vtn/core/ipc/IpcInt32"
#define	PJNI_CLASS_IpcInt64	"org/opendaylight/vtn/core/ipc/IpcInt64"
#define	PJNI_CLASS_IpcUint8	"org/opendaylight/vtn/core/ipc/IpcUint8"
#define	PJNI_CLASS_IpcUint16	"org/opendaylight/vtn/core/ipc/IpcUint16"
#define	PJNI_CLASS_IpcUint32	"org/opendaylight/vtn/core/ipc/IpcUint32"
#define	PJNI_CLASS_IpcUint64	"org/opendaylight/vtn/core/ipc/IpcUint64"
#define	PJNI_CLASS_IpcFloat	"org/opendaylight/vtn/core/ipc/IpcFloat"
#define	PJNI_CLASS_IpcDouble	"org/opendaylight/vtn/core/ipc/IpcDouble"
#define	PJNI_CLASS_IpcInetAddress			\
	"org/opendaylight/vtn/core/ipc/IpcInetAddress"
#define	PJNI_CLASS_IpcInet4Address			\
	"org/opendaylight/vtn/core/ipc/IpcInet4Address"
#define	PJNI_CLASS_IpcInet6Address			\
	"org/opendaylight/vtn/core/ipc/IpcInet6Address"
#define	PJNI_CLASS_IpcString	"org/opendaylight/vtn/core/ipc/IpcString"
#define	PJNI_CLASS_IpcBinary	"org/opendaylight/vtn/core/ipc/IpcBinary"
#define	PJNI_CLASS_IpcNull	"org/opendaylight/vtn/core/ipc/IpcNull"
#define	PJNI_CLASS_IpcStruct	"org/opendaylight/vtn/core/ipc/IpcStruct"
#define	PJNI_CLASS_IpcStructField			\
	"org/opendaylight/vtn/core/ipc/IpcStructField"

#define	PJNI_CLASS_TimeSpec	"org/opendaylight/vtn/core/util/TimeSpec"
#define	PJNI_CLASS_HostAddress	"org/opendaylight/vtn/core/util/HostAddress"

#define	PJNI_SIG_IpcInetAddress		PJNI_SIG_CLASS(IpcInetAddress)
#define	PJNI_SIG_HostAddress		PJNI_SIG_CLASS(HostAddress)
#define	PJNI_SIG_IpcException		PJNI_SIG_CLASS(IpcException)

#define	JIPC_SIG_METHOD(clname, name)	JIPC_SIG_##clname##_##name

/*
 * JNI signature of TimeSpec(long, long)
 */
#define	JIPC_CTORSIG_TimeSpec	PJNI_SIG_METHOD2(void, long, long)

/*
 * JNI signature of Object.toString()
 */
#define	JIPC_SIG_Object_toString	PJNI_SIG_METHOD0(String)

/*
 * JNI signature of Thread.interrupted()
 */
#define	JIPC_SIG_Thread_interrupted	PJNI_SIG_METHOD0(boolean)

/*
 * JNI signature of HostAddress.getHostAddress(byte[], int)
 */
#define	JIPC_SIG_HostAddress_getHostAddress	\
	PJNI_SIG_METHOD2(HostAddress, jbyteArray, int)

/*
 * Cast an IPC client session handle.
 */
#define	SESS_PTR(session)	((pfc_ipcsess_t *)(uintptr_t)(session))
#define	SESS_HANDLE(sess)	((jlong)(uint64_t)(uintptr_t)(sess))

/*
 * Exception class table.
 * The last entry must be defined by JIPC_EXCEPT_END().
 */
typedef struct {
	int		jie_errno;		/* error number */
	const char	*jie_class;		/* exception class name */
} jipc_except_t;

typedef const jipc_except_t	jipc_cexcept_t;

#define	JIPC_EXCEPT_DECL(err, class)			\
	{						\
		.jie_errno	= err,			\
		.jie_class	= PJNI_CLASS(class),	\
	}

#define	JIPC_EXCEPT_END()			\
	{					\
		.jie_errno	= 0,		\
		.jie_class	= NULL,		\
	}

/*
 * Buffer size enough to keep a string representation of pfc_ipctype_t.
 */
#define	JIPC_TYPEBUF_SIZE	PFC_CONST_U(20)

/*
 * Internal logging functions.
 */
extern volatile pfc_bool_t	jipc_log_enabled;

#ifdef	__PFC_LOG_GNUC

#define	JIPC_LOG_ERROR(format, ...)				\
	if (jipc_log_enabled) {					\
		pfc_log_error((format), ##__VA_ARGS__);		\
	}

#define	JIPC_LOG_WARN(format, ...)				\
	if (jipc_log_enabled) {					\
		pfc_log_warn((format), ##__VA_ARGS__);		\
	}

#define	JIPC_LOG_DEBUG(format, ...)				\
	if (jipc_log_enabled) {					\
		pfc_log_debug((format), ##__VA_ARGS__);		\
	}

#ifdef	PFC_VERBOSE_DEBUG
#define	JIPC_LOG_VERBOSE(format, ...)				\
	if (jipc_log_enabled) {					\
		pfc_log_verbose((format), ##__VA_ARGS__);	\
	}
#else	/* !PFC_VERBOSE_DEBUG */
#define	JIPC_LOG_VERBOSE(format, ...)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#else	/* !__PFC_LOG_GNUC */

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
JIPC_LOG_ERROR(const char *format, ...)
{
	if (jipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
JIPC_LOG_WARN(const char *format, ...)
{
	if (jipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_warn_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
JIPC_LOG_DEBUG(const char *format, ...)
{
	if (jipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_debug_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
JIPC_LOG_VERBOSE(const char *format, ...)
{
	if (jipc_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_verbose_v(format, ap);
		va_end(ap);
	}
}

#endif	/* __PFC_LOG_GNUC */

extern const char	jipc_str_null[];
extern const char	jipc_str_unknown[];

/*
 * Internal prototypes.
 */
extern void	jipc_throw(JNIEnv *env, int err,
			   jipc_cexcept_t *PFC_RESTRICT extable,
			   const char *PFC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(4, 5);
extern void	jipc_throw_unexpected(JNIEnv *PFC_RESTRICT env,
				      const char *PFC_RESTRICT func, int err);

extern void	jipc_log_fini(void);

extern jobject	jipc_pdu_getmsg(JNIEnv *PFC_RESTRICT env,
				ipc_msg_t *PFC_RESTRICT msg, uint32_t index);
extern jobject	jipc_pdu_create(JNIEnv *PFC_RESTRICT env, pfc_ipctype_t type,
				const uint8_t *PFC_RESTRICT addr,
				uint32_t size);

extern const char	*jipc_pdu_typename(pfc_ipctype_t type, char *buf,
					   size_t size);

/*
 * static inline const char PFC_FATTR_ALWAYS_INLINE *
 * jipc_getstring(JNIEnv *env, jstring jstr, const char *label)
 *	Return a UTF-8 string in the specified Java string.
 *
 *	`label' is a label which will be embedded in an error message.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to UTF-8 string is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	Returned UTF-8 string must be released by pjni_string_release().
 */
static inline const char PFC_FATTR_ALWAYS_INLINE *
jipc_getstring(JNIEnv *env, jstring jstr, const char *label)
{
	const char	*str;

	if (PFC_EXPECT_FALSE(jstr == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "%s is null.", label);
		str = NULL;
	}
	else {
		str = pjni_string_get(env, jstr);
	}

	return str;
}

#endif	/* !_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPCCLNT_JNI_H */
