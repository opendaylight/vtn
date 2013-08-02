/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * log.c - Internal logging functions.
 */

#include <pfc/log.h>
#include <pfc/synch.h>
#include "jni_impl.h"

/*
 * Determine whether internal logging is enabled or not.
 */
static pfc_bool_t	jni_log_enabled = PFC_FALSE;

/*
 * Internal logging lock.
 */
static pfc_mutex_t	jni_log_lock = PFC_MUTEX_INITIALIZER;

#define	PJNI_LOG_LOCK()		pfc_mutex_lock(&jni_log_lock)
#define	PJNI_LOG_UNLOCK()	pfc_mutex_unlock(&jni_log_lock)

/*
 * Logging configuration hook.
 */
static pjni_loghook_t	jni_log_hook;

/*
 * void
 * pjni_log_enable(pfc_bool_t enable)
 *	Enable or disable internal logging by the PFC-Core logging system.
 *
 *	If PFC_TRUE is specified to `enable', internal logging is enabled.
 *
 * Remarks:
 *	This function must be called only from Java bindings of the PFC-Core
 *	logging system.
 */
void
pjni_log_enable(pfc_bool_t enable)
{
	PJNI_LOG_LOCK();

	jni_log_enabled = enable;

	if (!enable) {
		pjni_loghook_t	hook = jni_log_hook;

		if (hook != NULL) {
			/* Uninstall the hook function, and call it. */
			jni_log_hook = NULL;
			(*hook)(enable);
		}
	}

	PJNI_LOG_UNLOCK();
}

/*
 * int
 * pjni_log_hook_install(pjni_loghook_t hook)
 *	Install logging configuration hook.
 *
 *	Logging hook is callback function which is called when the logging
 *	is disabled.
 *
 *	If the logging is enabled by Java bindings, this function calls
 *	the hook function specified by `hook' with specifying PFC_TRUE to
 *	the argument.
 *
 *	When the logging is disabled by Java bindings after successful
 *	return of this function, the hook function will be called with
 *	specifying PFC_FALSE to the argument, and the hook function is
 *	automatically uninstalled.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ESHUTDOWN is returned if the logging is not enabled by Java bindings.
 *	EEXIST is returned if another hook is already registered.
 *
 * Remarks:
 *	- Note that the hook function is called with holding the internal
 *	  logging lock.
 *
 *	- Note that this function does nothing if the specified function
 *	  is already registered.
 */
int
pjni_log_hook_install(pjni_loghook_t hook)
{
	pfc_bool_t	enabled;
	int		err;

	PJNI_LOG_LOCK();

	enabled = jni_log_enabled;
	if (enabled) {
		if (PFC_EXPECT_TRUE(jni_log_hook == NULL)) {
			jni_log_hook = hook;
			(*hook)(enabled);
			err = 0;
		}
		else if (PFC_EXPECT_TRUE(jni_log_hook == hook)) {
			/* Already installed. */
			err = 0;
		}
		else {
			err = EEXIST;
		}
	}
	else {
		err = ESHUTDOWN;
	}

	PJNI_LOG_UNLOCK();

	return err;
}

/*
 * void
 * pjni_log_hook_uninstall(pjni_loghook_t hook)
 *	Uninstall logging configuration hook.
 *
 *	Note that this function does nothing if the specified hook is not
 *	installed.
 */
void
pjni_log_hook_uninstall(pjni_loghook_t hook)
{
	PJNI_LOG_LOCK();
	if (PFC_EXPECT_TRUE(jni_log_hook == hook)) {
		jni_log_hook = NULL;
	}
	PJNI_LOG_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pjni_log_error(const char *format, ...)
 *	Log the specified message to the PFC-Core logging system.
 */
void PFC_ATTR_HIDDEN
pjni_log_error(const char *format, ...)
{
	PJNI_LOG_LOCK();

	if (jni_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}

	PJNI_LOG_UNLOCK();
}
