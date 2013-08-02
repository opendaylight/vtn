/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * client.c - IPC client library management.
 */

#include <org_opendaylight_vtn_core_ipc_ClientLibrary.h>
#include <pfc/atomic.h>
#include <jni_impl.h>
#include "ipcclnt_jni.h"

/*
 * Determine whether internal logging is enabled or not.
 */
volatile pfc_bool_t	jipc_log_enabled PFC_ATTR_HIDDEN;

#define	JIPC_LOG_SET(value)						\
	do {								\
		(void)pfc_atomic_swap_uint8((uint8_t *)&jipc_log_enabled, \
					    (value));			\
	} while (0)
#define	JIPC_LOG_ENABLE()	JIPC_LOG_SET(1)
#define	JIPC_LOG_DISABLE()	JIPC_LOG_SET(0)

/*
 * Internal prototypes.
 */
static void	jipc_log_hook(pfc_bool_t enabled);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientLibrary_cancel(JNIEnv *env, jclass cls)
 *	Cancel all ongoing IPC service requests except PFC_IPCSSF_NOGLOBCANCEL.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientLibrary_cancel(JNIEnv *env, jclass cls)
{
	pfc_ipcclnt_cancel(PFC_FALSE);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientLibrary_disable(JNIEnv *env, jclass cls)
 *	Disable IPC client library.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientLibrary_disable(JNIEnv *env, jclass cls)
{
	pfc_ipcclnt_cancel(PFC_TRUE);
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientLibrary_isDisabled(
 *	JNIEnv *env, jclass cls)
 *
 *	Determine whether the IPC client library is disabled or not.
 *
 * Calling/Exit State:
 *	JNI_TRUE is returned if the IPC client library is disabled.
 *	Otherwise JNI_FALSE is returned.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientLibrary_isDisabled(
	JNIEnv *env, jclass cls)
{
	PFC_ASSERT(PFC_TRUE == JNI_TRUE);
	PFC_ASSERT(PFC_FALSE == JNI_FALSE);

	return (jboolean)pfc_ipcclnt_isdisabled();
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ClientLibrary_setLogEnabled(
 *	JNIEnv *env, jclass cls, jboolean enabled)
 *
 *	Enable or disable the IPC client internal logging.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ClientLibrary_setLogEnabled(
	JNIEnv *env, jclass cls, jboolean enabled)
{
	int	err;

	if (!enabled) {
		jipc_log_fini();

		return;
	}

	/*
	 * Install logging hook and enable libpfc_ipcclnt logging only if
	 * the PFC-Core logging system is initialized by Java bindings.
	 */
	err = pjni_log_hook_install(jipc_log_hook);
	if (PFC_EXPECT_FALSE(err != 0)) {
		const char	*clname = PJNI_CLASS(IllegalStateException);

		if (err == ESHUTDOWN) {
			pjni_throw(env, clname,
				   "PFC-Core logging system is not yet "
				   "initialized.");
		}
		else {
			/* This should never happen. */
			pjni_throw(env, clname,
				   "Failed to enable internal logging: "
				   "err=%d ", err);
		}

		return;
	}

	JIPC_LOG_ENABLE();
}

/*
 * void PFC_ATTR_HIDDEN
 * jipc_log_fini(void)
 *	Finalize the internal logging.
 *	This function is called from library destructor.
 */
void PFC_ATTR_HIDDEN
jipc_log_fini(void)
{
	JIPC_LOG_DISABLE();
	pfc_ipcclnt_enable_log(PFC_FALSE);
	pjni_log_hook_uninstall(jipc_log_hook);
}

/*
 * static void
 * jipc_log_hook(pfc_bool_t enabled)
 *	Enable or disable IPC client internal logging.
 *
 * Remarks:
 *	This function is called with holding the libpfc_jni internal logging
 *	lock.
 */
static void
jipc_log_hook(pfc_bool_t enabled)
{
	JIPC_LOG_DISABLE();
	pfc_ipcclnt_enable_log(enabled);
}
