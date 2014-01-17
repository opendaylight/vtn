/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * log.c - Java bindings for the PFC-Core logging system.
 */

#include <stdlib.h>
#include <string.h>
#include <org_opendaylight_vtn_core_util_LogSystem.h>
#include <org_opendaylight_vtn_core_util_TraceLogImpl.h>
#include <org_opendaylight_vtn_core_util_Logger_TraceLogger.h>
#include <pfc/log.h>
#include <pfc/rbtree.h>
#include <pfc/synch.h>
#include <pfc/debug.h>
#include <jni_impl.h>
#include "pfc_util_jni.h"

/*
 * Logging configuration per VM.
 */
typedef struct {
	JavaVM		*pl_jvm;	/* Java VM handle */
	jobject		pl_fatal;	/* FATAL log handler */
	uint32_t	pl_refcnt;	/* reference counter */
} pjni_log_t;

static pjni_log_t	log_conf;

/*
 * Prototype of logging function.
 */
typedef void	(*log_func_t)(pfc_log_level_t level, const char *modname,
			      const char *format, ...);

/*
 * Internal prototypes.
 */
static void		log_fatal_handler(void);
static void		log_send(JNIEnv *env, log_func_t func,
				 pfc_log_level_t level, jstring module,
				 jstring msg);
static void		log_send_impl(JNIEnv *env, log_func_t func,
				      pfc_log_level_t level, jstring module,
				      jstring msg);
static void		log_destroy(JNIEnv *env, pjni_log_t *lp);

/*
 * Hold/Release reference counter of the logging configuration.
 * These macros must be used with holding the log configuration lock.
 */
#define	PJNI_LOG_HOLD(lp)				\
	do {						\
		PFC_ASSERT((lp)->pl_refcnt != 0);	\
		(lp)->pl_refcnt++;			\
	} while (0)

#define	PJNI_LOG_RELEASE(env, lp)					\
	do {								\
		uint32_t	__ref = (lp)->pl_refcnt;		\
									\
		(lp)->pl_refcnt = __ref - 1;				\
		if (__ref == 1) {					\
			log_destroy(env, lp);				\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

/*
 * Log configuration lock.
 */
static pfc_mutex_t	log_conf_lock = PFC_MUTEX_INITIALIZER;

#define	LOG_CONF_LOCK()		pfc_mutex_lock(&log_conf_lock)
#define	LOG_CONF_UNLOCK()	pfc_mutex_unlock(&log_conf_lock)

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_util_LogSystem_initializeLog(
 *	JNIEnv *env, jobject this, jstring ident, jint facility, jint level,
 *	jint defLevel, jstring logDir, jstring logName, jstring levelDir,
 *	jstring levelName, jint rcount, jint rsize, jobject fatalHandler)
 *
 *	Initialize the PFC-Core logging system.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_util_LogSystem_initializeLog(
	JNIEnv *env, jobject this, jstring ident, jint facility, jint level,
	jint defLevel, jstring logDir, jstring logName, jstring levelDir,
	jstring levelName, jint rcount, jint rsize, jobject fatalHandler)
{
	const char	*identstr;
	const char	*ldir = NULL, *lname = NULL;
	const char	*lvldir = NULL, *lvlname = NULL;
	JavaVM		*jvm;
	pjni_log_t	*lp = &log_conf;
	pfc_log_conf_t	conf;
	pfc_log_fatal_t	handler;

	/* Obtain UTF-8 strings. */
	identstr = pjni_string_get(env, ident);
	if (PFC_EXPECT_FALSE(identstr == NULL)) {
		return;
	}

	if (logDir != NULL) {
		ldir = pjni_string_get(env, logDir);
		if (PFC_EXPECT_FALSE(ldir == NULL)) {
			goto out;
		}

		PFC_ASSERT(logName != NULL);
		lname = pjni_string_get(env, logName);
		if (PFC_EXPECT_FALSE(lname == NULL)) {
			goto out;
		}
	}

	if (levelDir != NULL) {
		lvldir = pjni_string_get(env, levelDir);
		if (PFC_EXPECT_FALSE(lvldir == NULL)) {
			goto out;
		}

		PFC_ASSERT(levelName != NULL);
		lvlname = pjni_string_get(env, levelName);
		if (PFC_EXPECT_FALSE(lvlname == NULL)) {
			goto out;
		}
	}

	jvm = pjni_getvm(env);
	if (PFC_EXPECT_FALSE(jvm == NULL)) {
		/* This should never happen. */
		goto out;
	}

	/* Register logging configuration. */
	LOG_CONF_LOCK();
	if (PFC_EXPECT_FALSE(lp->pl_jvm != NULL)) {
		const char *msg = (jvm == lp->pl_jvm)
			? ""
			: " by another Java VM";

		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "The logging system is already initialized%s.",
			   msg);
		goto out_unlock;
	}

	if (fatalHandler == NULL) {
		handler = NULL;
		lp->pl_fatal = NULL;
	}
	else {
		jobject	ghandler;

		/* Create global reference to this handler. */
		ghandler = (*env)->NewGlobalRef(env, fatalHandler);
		if (PFC_EXPECT_FALSE(ghandler == NULL)) {
			pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
				   "Unable to create reference to FATAL log "
				   "handler.");
			goto out_unlock;
		}

		handler = log_fatal_handler;
		lp->pl_fatal = ghandler;
	}

	lp->pl_jvm = jvm;
	lp->pl_refcnt = 1;

	/* Initialize the PFC-Core logging system. */
	pfc_logconf_init(&conf, PFC_CFBLK_INVALID, identstr, handler);
	pfc_logconf_setsyslog(&conf, PFC_FALSE);
	pfc_logconf_setfacility(&conf, facility);
	if (ldir != NULL) {
		pfc_logconf_setpath(&conf, ldir, strlen(ldir), NULL, 0,
				    lname, strlen(lname));
	}

	if (lvldir != NULL) {
		pfc_logconf_setlvlpath(&conf, lvldir, strlen(lvldir),
				       lvlname, strlen(lvlname));
	}

	if (level != PFC_LOGLVL_NONE) {
		pfc_logconf_setlevel(&conf, level);
	}
	pfc_logconf_setdeflevel(&conf, defLevel);
	pfc_logconf_setrotate(&conf, rcount, rsize);

	pfc_log_sysinit(&conf);

	/* Enable internal logging for libpfc_jni.so. */
	pjni_log_enable(PFC_TRUE);

out_unlock:
	LOG_CONF_UNLOCK();

out:
	pjni_string_release(env, ident, identstr);
	pjni_string_release(env, logDir, ldir);
	pjni_string_release(env, logName, lname);
	pjni_string_release(env, levelDir, lvldir);
	pjni_string_release(env, levelName, lvlname);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_util_LogSystem_finalizeLog(
 *	JNIEnv *env, jobject this)
 *
 *	Finalize the PFC-Core logging system.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_util_LogSystem_finalizeLog(
	JNIEnv *env, jobject this)
{
	JavaVM		*jvm = pjni_getvm(env);
	pjni_log_t	*lp = &log_conf;

	if (PFC_EXPECT_FALSE(jvm == NULL)) {
		/* This should never happen. */
		return;
	}

	LOG_CONF_LOCK();

	/* Delete logging configuration. */
	if (PFC_EXPECT_TRUE(lp->pl_jvm == jvm)) {
		PJNI_LOG_RELEASE(env, lp);
	}

	LOG_CONF_UNLOCK();
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_util_TraceLogImpl_getLogLevel(
 *	JNIEnv *env, jobject this)
 *
 *	Return the current logging level of the trace log.
 *
 * Calling/Exit State:
 *	The current logging level of the trace log is returned.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_util_TraceLogImpl_getLogLevel(
	JNIEnv *env, jobject this)
{
	return pfc_log_current_level;
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_util_TraceLogImpl_setLogLevel(
 *	JNIEnv *env, jobject this, jint level)
 *
 *	Change the trace log level to the specified value.
 *
 * Calling/Exit State:
 *	JNI_TRUE is returned if the trace log level has been actually changed.
 *	JNI_FALSE is returned if the current trace log level is identical
 *	to the specified value.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_util_TraceLogImpl_setLogLevel(
	JNIEnv *env, jobject this, jint level)
{
	int	err;

	err = pfc_log_set_level((pfc_log_level_t)level);
	if (err == 0) {
		return JNI_TRUE;
	}
	if (PFC_EXPECT_TRUE(err > 0)) {
		return JNI_FALSE;
	}

	/* This should never happen. */
	pjni_throw(env, PJNI_CLASS(IllegalStateException),
		   "Unable to change log level: level=%d, err=%d", level, err);

	return JNI_FALSE;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_util_Logger_00024TraceLogger_logImpl(
 *	JNIEnv *env, jobject this, jint level, jstring module, jstring msg)
 *
 *	Log a trace log message.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_util_Logger_00024TraceLogger_logImpl(
	JNIEnv *env, jobject this, jint level, jstring module, jstring msg)
{
	pfc_log_level_t	lvl = (pfc_log_level_t)level;

	if (PFC_EXPECT_TRUE(lvl <= pfc_log_current_level)) {
		log_send(env, __pfc_log_common, lvl, module, msg);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * log_cleanup(JavaVM *jvm)
 *	Clean up handler which will be called when the specified Java VM
 *	unloads this library.
 */
void PFC_ATTR_HIDDEN
log_cleanup(JavaVM *jvm)
{
	pjni_log_t	*lp = &log_conf;

	LOG_CONF_LOCK();

	if (lp->pl_jvm == jvm) {
		/* Clean up logging configuration. */
		lp->pl_jvm = NULL;
		lp->pl_fatal = NULL;
		lp->pl_refcnt = 0;
	}

	LOG_CONF_UNLOCK();
}

/*
 * static void
 * log_fatal_handler(void)
 *	FATAL log handler which will be called when a FATAL message is logged.
 */
static void
log_fatal_handler(void)
{
	JNIEnv		*env;
	JavaVM		*jvm;
	jobject		handler;
	pjni_log_t	*lp = &log_conf;

	/* Determine Java VM. */
	LOG_CONF_LOCK();
	jvm = lp->pl_jvm;
	if (PFC_EXPECT_FALSE(jvm == NULL)) {
		goto out;
	}

	/* Check whether FATAL log handler is registered or not. */
	handler = lp->pl_fatal;
	if (PFC_EXPECT_FALSE(handler == NULL)) {
		goto out;
	}

	PJNI_LOG_HOLD(lp);
	LOG_CONF_UNLOCK();

	/* Call FATAL log handler. */
	PJNI_ATTACH(jvm, env);
	pjni_call_void(env, handler, "fatalError", PJNI_SIG_METHOD0(void));
	PJNI_DETACH(jvm);

	LOG_CONF_LOCK();
	PJNI_LOG_RELEASE(env, lp);

out:
	LOG_CONF_UNLOCK();

	return;
}

/*
 * static void
 * log_send(JNIEnv *env, log_func_t func, pfc_log_level_t level,
 *	    jstring module, jstring msg)
 *	Send a log message to the logging system.
 *
 * Remarks:
 *	Current logging level must be checked by the caller.
 */
static void
log_send(JNIEnv *env, log_func_t func, pfc_log_level_t level, jstring module,
	 jstring msg)
{
	JavaVM		*jvm = pjni_getvm(env);
	pjni_log_t	*lp = &log_conf;

	if (PFC_EXPECT_FALSE(jvm == NULL)) {
		/* This should never happen. */
		return;
	}

	LOG_CONF_LOCK();

	if (PFC_EXPECT_TRUE(lp->pl_jvm == jvm)) {
		PJNI_LOG_HOLD(lp);
		LOG_CONF_UNLOCK();
		log_send_impl(env, func, level, module, msg);
		LOG_CONF_LOCK();
		PJNI_LOG_RELEASE(env, lp);
	}

	LOG_CONF_UNLOCK();
}

/*
 * static void
 * log_send_impl(JNIEnv *env, log_func_t func, pfc_log_level_t level,
 *		 jstring module, jstring msg)
 *	Send a log message to the logging system.
 *	This is an internal function of log_send().
 */
static void
log_send_impl(JNIEnv *env, log_func_t func, pfc_log_level_t level,
	      jstring module, jstring msg)
{
	const char	*modname = NULL, *message = NULL;

	if (PFC_EXPECT_FALSE(msg == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "message is null.");

		return;
	}

	if (module != NULL) {
		modname = pjni_string_get(env, module);
		if (PFC_EXPECT_FALSE(modname == NULL)) {
			return;
		}
	}

	message = pjni_string_get(env, msg);
	if (PFC_EXPECT_TRUE(message != NULL)) {
		/* Call PFC-Core log function. */
		(*func)(level, modname, "%s", message);
		pjni_string_release(env, msg, message);
	}

	pjni_string_release(env, module, modname);
}

/*
 * static void
 * log_destroy(JNIEnv *env, pjni_log_t *lp)
 *	Destroy the logging configuration.
 *
 * Remarks:
 *	This function must be called with holding the log configuration lock.
 */
static void
log_destroy(JNIEnv *env, pjni_log_t *lp)
{
	PFC_ASSERT(lp->pl_refcnt == 0);

	/* Disable internal logging for libpfc_jni.so. */
	pjni_log_enable(PFC_FALSE);

	pfc_log_fini();

	lp->pl_jvm = NULL;
	if (lp->pl_fatal != NULL) {
		(*env)->DeleteGlobalRef(env, lp->pl_fatal);
		lp->pl_fatal= NULL;
	}
}
