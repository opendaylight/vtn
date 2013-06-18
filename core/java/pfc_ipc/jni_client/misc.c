/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.c - Miscellaneous routines.
 */

#include "ipcclnt_event_jni.h"

const char	jipc_str_null[] PFC_ATTR_HIDDEN = "<null>";
const char	jipc_str_unknown[] PFC_ATTR_HIDDEN = "<unknown>";

/*
 * Internal buffer size for exception message.
 */
#define	JIPC_EX_BUFSIZE		PFC_CONST_U(128)

static jipc_cexcept_t	jipc_exceptions[] = {
	JIPC_EXCEPT_DECL(EINVAL, IllegalArgumentException),
	JIPC_EXCEPT_DECL(ENOMEM, OutOfMemoryError),
	JIPC_EXCEPT_DECL(EBADF, IpcBadFileException),
	JIPC_EXCEPT_DECL(EPERM, IpcPermissionDeniedException),
	JIPC_EXCEPT_DECL(EBUSY, IpcResourceBusyException),
	JIPC_EXCEPT_DECL(ENOENT, IpcNoEntryException),
	JIPC_EXCEPT_DECL(EEXIST, IpcAlreadyExistsException),
	JIPC_EXCEPT_DECL(ETIMEDOUT, IpcTimedOutException),
	JIPC_EXCEPT_DECL(ECONNREFUSED, IpcConnectionRefusedException),
	JIPC_EXCEPT_DECL(ECONNRESET, IpcConnectionResetException),
	JIPC_EXCEPT_DECL(EPIPE, IpcBrokenPipeException),
	JIPC_EXCEPT_DECL(ECANCELED, IpcCanceledException),
	JIPC_EXCEPT_DECL(ESHUTDOWN, IpcShutdownException),
	JIPC_EXCEPT_END(),
};

/*
 * Signature of IpcLibraryException(int, String).
 */
#define	JIPC_SIG_CTOR_IpcLibraryException	\
	PJNI_SIG_METHOD2(void, int, String)

/*
 * static inline jipc_cexcept_t PFC_FATTR_ALWAYS_INLINE *
 * jipc_exception_lookup(jipc_cexcept_t *extable, int err)
 *	Determine an exception associated with the specified error number.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to jipc_cexcept_t is returned if found.
 *	NULL is returned if not found.
 */
static inline jipc_cexcept_t PFC_FATTR_ALWAYS_INLINE *
jipc_exception_lookup(jipc_cexcept_t *extable, int err)
{
	jipc_cexcept_t	*exp;

	if (extable != NULL) {
		/* Search for an exception in the specified exception table. */
		for (exp = extable; exp->jie_errno != 0; exp++) {
			if (exp->jie_errno == err) {
				return exp;
			}
		}
	}

	/* Search in the global exception table. */
	for (exp = jipc_exceptions; exp->jie_errno != 0; exp++) {
		if (exp->jie_errno == err) {
			return exp;
		}
	}

	return NULL;
}

/*
 * static void PFC_FATTR_FINI
 * jipc_libfini(void)
 *	Destructor of IPC client JNI library.
 */
static void PFC_FATTR_FINI
jipc_libfini(void)
{
	/* Disable internal logging. */
	jipc_log_fini();
}

/*
 * jint
 * JNI_OnLoad(JavaVM *jvm, void *reserved)
 *	Notify required JNI version to the Java VM.
 */
jint
JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	/* Return minimum JNI version. */
	return PJNI_MIN_VERSION;
}

/*
 * void
 * JNI_OnUnload(JavaVM *jvm, void *reserved)
 *	Clean up handler of the native library.
 */
void
JNI_OnUnload(JavaVM *jvm, void *reserved)
{
	/*
	 * Clear context of the IPC event dispatcher thread initialized by
	 * the specified VM.
	 */
	jipc_event_clear(jvm);
}

/*
 * void PFC_ATTR_HIDDEN
 * jipc_throw(JNIEnv *env, int err, jipc_cexcept_t *PFC_RESTRICT extable,
 *	      const char *PFC_RESTRICT fmt, ...)
 *	Throw an exception associated with the specified error number.
 *
 *	`err' must be a non-zero error number defined in errno.h.
 *	`extable' is a pointer to exception table which determines an exception
 *	associated with the error number `err'. If NULL is specified,
 *	the global exception table is used.
 *
 *	The detailed message is specified by `fmt' and the rest of arguments
 *	in printf(3) format.
 */
void PFC_ATTR_HIDDEN
jipc_throw(JNIEnv *env, int err, jipc_cexcept_t *PFC_RESTRICT extable,
	   const char *PFC_RESTRICT fmt, ...)
{
	jipc_cexcept_t	*exp;
	jstring		jmsg;
	va_list		ap;
	char		buf[JIPC_EX_BUFSIZE];

	PFC_ASSERT(err != 0);

	if (err == ECONNABORTED && pfc_ipcclnt_isdisabled()) {
		// IPC client library is disabled.
		pjni_throw(env, PJNI_CLASS(IpcClientDisabledException),
			   "IPC client library is permanently disabled.");

		return;
	}

	/* Determine class name of an exception. */
	exp = jipc_exception_lookup(extable, err);
	if (exp != NULL) {
		/* Throw an exception defined in jipc_except_t. */
		va_start(ap, fmt);
		pjni_vthrow(env, exp->jie_class, fmt, ap);
		va_end(ap);

		return;
	}

	/* Throw an IpcLibraryException. */
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	jmsg = pjni_newstring(env, buf);
	if (!PJNI_EXCEPTION_CHECK(env)) {
		jobject		eobj;

		eobj = pjni_newobject(env, PJNI_CLASS(IpcLibraryException),
				      JIPC_SIG_CTOR_IpcLibraryException,
				      err, jmsg);
		if (PFC_EXPECT_TRUE(eobj != NULL)) {
			(*env)->Throw(env, eobj);
			(*env)->DeleteLocalRef(env, eobj);
		}

		(*env)->DeleteLocalRef(env, jmsg);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * jipc_throw_unexpected(JNIEnv *PFC_RESTRICT env,
 *			 const char *PFC_RESTRICT func, int err)
 *	Throw an exception which indicates the IPC client library returned
 *	an error unexpectedly.
 */
void PFC_ATTR_HIDDEN
jipc_throw_unexpected(JNIEnv *PFC_RESTRICT env, const char *PFC_RESTRICT func,
		      int err)
{
	pjni_throw(env, PJNI_CLASS(IllegalStateException),
		   "%s() failed: err=%d", func, err);
}
