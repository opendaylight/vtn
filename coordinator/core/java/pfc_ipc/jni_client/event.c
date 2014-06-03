/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - IPC event subsystem.
 */

#include <pfc/synch.h>
#include <pfc/util.h>
#include <pfc/atomic.h>
#include <pfc/hostaddr.h>
#include <pfc/refptr.h>
#include <ipcclnt_event.h>
#include <org_opendaylight_vtn_core_ipc_IpcHostSet.h>
#include <org_opendaylight_vtn_core_ipc_IpcEvent.h>
#include <org_opendaylight_vtn_core_ipc_IpcEventSystem.h>
#include <org_opendaylight_vtn_core_ipc_IpcEventAttribute.h>
#include "ipcclnt_event_jni.h"

/*
 * Operation for IpcHostSet.handleHostAddress().
 */
#define	HOSTSET_OP_ADD						\
	org_opendaylight_vtn_core_ipc_IpcHostSet_HOSTADDR_ADD
#define	HOSTSET_OP_REMOVE					\
	org_opendaylight_vtn_core_ipc_IpcHostSet_HOSTADDR_REMOVE
#define	HOSTSET_OP_CONTAINS					\
	org_opendaylight_vtn_core_ipc_IpcHostSet_HOSTADDR_CONTAINS

/*
 * Return value of IpcEventSystem.getChannelState(String, byte[], int).
 */
#define	EVSYS_STATE_DOWN					\
	org_opendaylight_vtn_core_ipc_IpcEventSystem_STATE_DOWN
#define	EVSYS_STATE_UP						\
	org_opendaylight_vtn_core_ipc_IpcEventSystem_STATE_UP
#define	EVSYS_STATE_IN_PROGRESS						\
	org_opendaylight_vtn_core_ipc_IpcEventSystem_STATE_IN_PROGRESS

/*
 * Determine whether the stack trace dump on the IPC event dispatcher thread
 * is enabled or not.
 */
static volatile pfc_bool_t	jipc_stacktrace_enabled = PFC_FALSE;

/*
 * Dump Java stack trace recorded in an exception.
 * Note that this macro may clear a pending exception.
 */
#define	JIPC_EXCEPTION_DESCRIBE(env)				\
	do {							\
		if (jipc_stacktrace_enabled) {			\
			(*(env))->ExceptionDescribe(env);	\
		}						\
	} while (0)

/*
 * Java classes to be cached.
 */
static const char	*jipc_cached_classes[] = {
	PJNI_CLASS(IpcEvent),
	PJNI_CLASS(ChannelUpEvent),
	PJNI_CLASS(ChannelDownEvent),
	PJNI_CLASS(ChannelStateEvent),
	PJNI_CLASS(TimeSpec),
	PJNI_CLASS(HostAddress),
	PJNI_CLASS(Thread),
};

#define	JIPC_EVCTX_NCLASSES	PFC_ARRAY_CAPACITY(jipc_cached_classes)

#define	JIPC_EVCTX_CLIDX_IpcEvent		0
#define	JIPC_EVCTX_CLIDX_ChannelUpEvent		1
#define	JIPC_EVCTX_CLIDX_ChannelDownEvent	2
#define	JIPC_EVCTX_CLIDX_ChannelStateEvent	3
#define	JIPC_EVCTX_CLIDX_TimeSpec		4
#define	JIPC_EVCTX_CLIDX_HostAddress		5
#define	JIPC_EVCTX_CLIDX_Thread			6

#define	JIPC_EVCTX_CLASS(ectx, name)			\
	((ectx)->je_classes[JIPC_EVCTX_CLIDX_##name])

/*
 * Method IDs to be cached.
 */
typedef struct {
	const char	*jms_name;		/* method name */
	const char	*jms_sig;		/* JNI signature */
	pfc_bool_t	jms_static;		/* true if static method */
	pfc_bool_t	jms_cached;		/* true if class is cached */

	union {
		uint32_t	id;		/* class cache ID */
		const char	*name;		/* class name */
	} jms_un;
} jipc_midspec_t;

typedef const jipc_midspec_t	jipc_cmidspec_t;

#define	JIPC_MIDSPEC_DECL(clname, method, isstatic)			\
	{								\
		.jms_name	= #method,				\
		.jms_sig	= JIPC_SIG_METHOD(clname, method),	\
		.jms_static	= isstatic,				\
		.jms_cached	= PFC_FALSE,				\
		.jms_un		= {					\
			.name	= PJNI_CLASS(clname),			\
		},							\
	}

#define	JIPC_MIDSPEC_CACHED_DECL(clname, method, isstatic)		\
	{								\
		.jms_name	= #method,				\
		.jms_sig	= JIPC_SIG_METHOD(clname, method),	\
		.jms_static	= isstatic,				\
		.jms_cached	= PFC_TRUE,				\
		.jms_un		= {					\
			.id	= JIPC_EVCTX_CLIDX_##clname,		\
		},							\
	}

#define	JIPC_MIDSPEC_CTOR_DECL(clname)					\
	{								\
		.jms_name	= PJNI_METHOD_CTOR,			\
		.jms_sig	= JIPC_CTORSIG_##clname,		\
		.jms_static	= PFC_FALSE,				\
		.jms_cached	= PFC_TRUE,				\
		.jms_un		= {					\
			.id	= JIPC_EVCTX_CLIDX_##clname,		\
		},							\
	}

static jipc_cmidspec_t	jipc_cached_methods[] = {
	JIPC_MIDSPEC_CTOR_DECL(IpcEvent),
	JIPC_MIDSPEC_CTOR_DECL(ChannelUpEvent),
	JIPC_MIDSPEC_CTOR_DECL(ChannelDownEvent),
	JIPC_MIDSPEC_CTOR_DECL(ChannelStateEvent),
	JIPC_MIDSPEC_CTOR_DECL(TimeSpec),
	JIPC_MIDSPEC_CACHED_DECL(IpcEvent, invalidate, PFC_FALSE),
	JIPC_MIDSPEC_CACHED_DECL(HostAddress, getHostAddress, PFC_TRUE),
	JIPC_MIDSPEC_DECL(IpcEventHandler, eventReceived, PFC_FALSE),
	JIPC_MIDSPEC_DECL(Object, toString, PFC_FALSE),
	JIPC_MIDSPEC_CACHED_DECL(Thread, interrupted, PFC_TRUE),
};

#define	JIPC_EVCTX_NMETHODS	PFC_ARRAY_CAPACITY(jipc_cached_methods)

#define	JIPC_EVCTX_MID_IpcEvent				0
#define	JIPC_EVCTX_MID_ChannelUpEvent			1
#define	JIPC_EVCTX_MID_ChannelDownEvent			2
#define	JIPC_EVCTX_MID_ChannelStateEvent		3
#define	JIPC_EVCTX_MID_TimeSpec				4
#define	JIPC_EVCTX_MID_IpcEvent_invalidate		5
#define	JIPC_EVCTX_MID_HostAddress_getHostAddress	6
#define	JIPC_EVCTX_MID_IpcEventHandler_eventReceived	7
#define	JIPC_EVCTX_MID_Object_toString			8
#define	JIPC_EVCTX_MID_Thread_interrupted		9

#define	JIPC_EVCTX_MID_CTOR(ectx, clname)		\
	((ectx)->je_methods[JIPC_EVCTX_MID_##clname])

#define	JIPC_EVCTX_MID(ectx, clname, method)				\
	((ectx)->je_methods[JIPC_EVCTX_MID_##clname##_##method])

/*
 * Global context of event handling.
 */
typedef struct {
	JavaVM		*je_jvm;	/* Java VM */
	JNIEnv		*je_env;	/* environment for dispatcher thread */
	pfc_mutex_t	je_mutex;	/* mutex */
	pfc_cond_t	je_cond;	/* condition variable */
	jipc_gref_t	*je_grefs;	/* GC list for global references */
	uint8_t		je_finicnt;	/* counter for finalization */
	uint8_t		je_state;	/* state of JNI event context */
	uint16_t	je_unloading;	/* number of unloading threads */
	uint32_t	je_hold;	/* VM hold counter */

	/*
	 * Caches for fast optimization.
	 */

	/* Global references to class objects. */
	jclass		je_classes[JIPC_EVCTX_NCLASSES];

	/* Method IDs. */
	jmethodID	je_methods[JIPC_EVCTX_NMETHODS];
} jipc_evctx_t;

/*
 * State of JNI event context. (je_state)
 */
#define	JIPC_EVSTATE_VALID		PFC_CONST_U(0x00)
#define	JIPC_EVSTATE_SHUTDOWN		PFC_CONST_U(0x01)
#define	JIPC_EVSTATE_FINALIZED		PFC_CONST_U(0x02)

static jipc_evctx_t	jipc_event_ctx = {
	.je_mutex	= PFC_MUTEX_INITIALIZER,
};

#define	JIPC_EVCTX_LOCK(ectx)		pfc_mutex_lock(&(ectx)->je_mutex)
#define	JIPC_EVCTX_UNLOCK(ectx)		pfc_mutex_unlock(&(ectx)->je_mutex)

#define	JIPC_EVCTX_WAIT(ectx)					\
	pfc_cond_wait(&(ectx)->je_cond, &(ectx)->je_mutex)
#define	JIPC_EVCTX_BROADCAST(ectx)	pfc_cond_broadcast(&(ectx)->je_cond)

/*
 * Private fields in IpcEventConfiguration.
 */
typedef struct {
	const char	*jcd_name;		/* field name */
	uint32_t	jcd_off;		/* offset of structure field */
} jipc_confdef_t;

typedef const jipc_confdef_t	jipc_cconfdef_t;

#define	JIPC_CONFDEF_DECL(name, member)					\
	{								\
		.jcd_name	= name,					\
		.jcd_off	= offsetof(pfc_ipcevopts_t, member),	\
	}

static jipc_cconfdef_t	jipc_conf_fields[] = {
	JIPC_CONFDEF_DECL("_idleTimeout", evopt_idle_timeout),
	JIPC_CONFDEF_DECL("_maxThreads", evopt_maxthreads),
	JIPC_CONFDEF_DECL("_connectInterval", evopt_conn_interval),
	JIPC_CONFDEF_DECL("_timeout", evopt_timeout),
};

/*
 * Private data for an IPC event handler.
 */
typedef struct {
	jipc_gref_t	jeh_gref;	/* global reference to handler */
	pfc_refptr_t	*jeh_name;	/* handler's name */
} jipc_evhdlr_t;

/*
 * Private data for an IPC event object.
 */
typedef struct {
	jipc_gref_t	jev_gref;	/* global reference to event */
	pfc_ipcevent_t	*jev_event;	/* event object */
} jipc_event_t;

/*
 * Buffer for raw IP address.
 */
typedef union {
	struct in_addr	jip_addr4;	/* IPv4 address */
	struct in6_addr	jip_addr6;	/* IPv6 address */
	jbyte		jip_addr;	/* base address */
} jipc_ipaddr_t;

/*
 * Exception table for operations which requires the IPC event subsystem to
 * be initialized.
 */
static jipc_cexcept_t	extable_event[] = {
	JIPC_EXCEPT_DECL(ENXIO, IpcEventSystemNotReadyException),
	JIPC_EXCEPT_END(),
};

/*
 * Exception table for host set operations.
 */
static jipc_cexcept_t	extable_hostset[] = {
	JIPC_EXCEPT_DECL(ENODEV, IpcNoSuchHostSetException),
	JIPC_EXCEPT_DECL(ENXIO, IpcEventSystemNotReadyException),
	JIPC_EXCEPT_END(),
};

/*
 * Exception table for event handler operations.
 */
static jipc_cexcept_t	extable_handler[] = {
	JIPC_EXCEPT_DECL(ENOENT, IpcNoSuchEventHandlerException),
	JIPC_EXCEPT_DECL(ENXIO, IpcEventSystemNotReadyException),
	JIPC_EXCEPT_END(),
};

static const char	emsg_already_initialized[] =
	"The IPC event subsystem is already initialized.";
static const char	emsg_shutdown[] =
	"The IPC event subsystem is shutting down.";
static const char	emsg_already_finalized[] =
	"The IPC event subsystem is already finalized.";
static const char	emsg_not_initialized[] =
	"The IPC event subsystem is not yet initialized.";
static const char	emsg_another_vm[] =
	"The IPC event subsystem is initialized by another VM.";

/*
 * Internal prototypes.
 */
static pfc_bool_t	jipc_fetchconf(JNIEnv *PFC_RESTRICT env, jobject conf,
				       uint8_t *PFC_RESTRICT ops);
static pfc_bool_t	jipc_evctx_init(JNIEnv *PFC_RESTRICT env,
					jipc_evctx_t *PFC_RESTRICT ectx);
static jmethodID	jipc_evctx_getmid(JNIEnv *PFC_RESTRICT env,
					  jipc_evctx_t *PFC_RESTRICT ectx,
					  jipc_cmidspec_t *PFC_RESTRICT spec);
static void		jipc_evctx_fail(JNIEnv *PFC_RESTRICT env,
					jipc_evctx_t *PFC_RESTRICT ectx)
	PFC_FATTR_NOINLINE;
static void		jipc_evctx_fail_rt(JNIEnv *PFC_RESTRICT env,
					   jipc_evctx_t *PFC_RESTRICT ectx)
	PFC_FATTR_NOINLINE;
static void		jipc_evctx_destroy(JNIEnv *PFC_RESTRICT env,
					   jipc_evctx_t *PFC_RESTRICT ectx);
static void		jipc_evctx_gc_clear(JNIEnv *PFC_RESTRICT env,
					    jipc_evctx_t *PFC_RESTRICT ectx)
	PFC_FATTR_NOINLINE;
static int		jipc_evdisp_ctor(void);
static void		jipc_evdisp_dtor(void);
static pfc_bool_t	jipc_gethostaddr(JNIEnv *PFC_RESTRICT env,
					 jbyteArray rawaddr, int scope,
					 pfc_hostaddr_t *PFC_RESTRICT haddr);
static void		jipc_hostaddr_tostring(const pfc_hostaddr_t
					       *PFC_RESTRICT haddr,
					       char *PFC_RESTRICT buf,
					       size_t bufsize);
static void		jipc_event_handler(pfc_ipcevent_t *event,
					   pfc_ptr_t arg);
static void		jipc_event_catch(JNIEnv *PFC_RESTRICT env,
					 jipc_evctx_t *PFC_RESTRICT ectx,
					 jipc_evhdlr_t *PFC_RESTRICT jhp);
static const char	*jipc_event_tostring(JNIEnv *PFC_RESTRICT env,
					     jipc_evctx_t *PFC_RESTRICT ectx,
					     jobject obj,
					     jstring *PFC_RESTRICT jstrp);
static jobject		jipc_event_create(jipc_evctx_t *PFC_RESTRICT ectx,
					  pfc_ipcevent_t *PFC_RESTRICT event);
static jobject		jipc_event_error(JNIEnv *PFC_RESTRICT env,
					 jipc_evctx_t *PFC_RESTRICT ectx,
					 pfc_ipcevent_t *PFC_RESTRICT event,
					 const char *PFC_RESTRICT msg,
					 jipc_event_t *PFC_RESTRICT jep)
	PFC_FATTR_NOINLINE;
static jobject		jipc_event_new(JNIEnv *PFC_RESTRICT env,
				       jipc_evctx_t *PFC_RESTRICT ectx,
				       pfc_ipcevent_t *PFC_RESTRICT event,
				       pfc_ipcevtype_t type);
static void		jipc_event_dtor(JNIEnv *PFC_RESTRICT env,
					jipc_gref_t *PFC_RESTRICT grefp);
static void		jipc_evhdlr_dtor(JNIEnv *PFC_RESTRICT env,
					 jipc_gref_t *PFC_RESTRICT grefp);
static void		jipc_gref_dtor(pfc_ptr_t arg);

/*
 * Operations for the IPC event subsystem.
 */
static ipc_cevsysops_t	jipc_evsys_ops = {
	.eop_disp_ctor		= jipc_evdisp_ctor,
	.eop_disp_dtor		= jipc_evdisp_dtor,
};

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_gc(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
 *	Free up global references on the GC list.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_evctx_gc(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	if (PFC_EXPECT_FALSE(ectx->je_grefs != NULL)) {
		jipc_evctx_gc_clear(env, ectx);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_release_l(JNIEnv *PFC_RESTRICT env,
 *		      jipc_evctx_t *PFC_RESTRICT ectx)
 *	Release the JNI event context.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_evctx_release_l(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	jipc_evctx_gc(env, ectx);

	PFC_ASSERT(ectx->je_hold > 0);
	ectx->je_hold--;
	if (PFC_EXPECT_FALSE(ectx->je_hold == 0 && ectx->je_unloading != 0)) {
		JIPC_EVCTX_BROADCAST(ectx);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_release(JNIEnv *PFC_RESTRICT env,
 *		      jipc_evctx_t *PFC_RESTRICT ectx)
 *	Release the JNI event context.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_evctx_release(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	JIPC_EVCTX_LOCK(ectx);
	jipc_evctx_release_l(env, ectx);
	JIPC_EVCTX_UNLOCK(ectx);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_fini(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
 *	Finalize the JNI event context.
 *
 * Remarks:
 *	- This function must be called with holding the JNI event context lock.
 *
 *	- The caller must hold the JNI event context in advance, and the
 *	  context is released on return.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_evctx_fini_l(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	PFC_ASSERT(ectx->je_finicnt != 0);
	ectx->je_finicnt--;

	jipc_evctx_release_l(env, ectx);

	if (ectx->je_finicnt == 0) {
		/* Destroy the context. */
		jipc_evctx_destroy(env, ectx);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_fini(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
 *	Finalize the JNI event context.
 *
 * Remarks:
 *	The caller must hold the JNI event context in advance, and the
 *	context is released on return.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_evctx_fini(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	JIPC_EVCTX_LOCK(ectx);
	jipc_evctx_fini_l(env, ectx);
	JIPC_EVCTX_UNLOCK(ectx);
}

/*
 * static inline JNIEnv PFC_FATTR_ALWAYS_INLINE *
 * jipc_evctx_getenv_l(jipc_evctx_t *ectx)
 *	Return a pointer to JNI environment for the IPC event dispatcher
 *	thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to JNI environment is
 *	returned.
 *	Otherwise NULL is returned.
 *
 * Remarks:
 *	- This function must be called on the IPC event dispatcher thread.
 *
 *	- This function must be called with holding the JNI event context lock.
 *
 *	- This function holds the JNI event context on successful return.
 *	  The caller must release the context by jipc_evctx_release().
 */
static inline JNIEnv PFC_FATTR_ALWAYS_INLINE *
jipc_evctx_getenv_l(jipc_evctx_t *ectx)
{
	JavaVM	*jvm;
	JNIEnv	*env;

	jvm = ectx->je_jvm;
	if (PFC_EXPECT_FALSE(jvm == NULL)) {
		env = NULL;
	}
	else {
		PFC_ASSERT((*jvm)->GetEnv(jvm, (void **)&env,
					  PJNI_MIN_VERSION) == JNI_OK);
		PFC_ASSERT(env == ectx->je_env);
		env = ectx->je_env;
		ectx->je_hold++;
	}

	return env;
}

/*
 * static inline JNIEnv PFC_FATTR_ALWAYS_INLINE *
 * jipc_evctx_getenv(jipc_evctx_t *ectx)
 *	Return a pointer to JNI environment for the IPC event dispatcher
 *	thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to JNI environment is
 *	returned.
 *	Otherwise NULL is returned.
 *
 * Remarks:
 *	- This function must be called on the IPC event dispatcher thread.
 *
 *	- This function holds the JNI event context on successful return.
 *	  The caller must release the context by jipc_evctx_release().
 */
static inline JNIEnv PFC_FATTR_ALWAYS_INLINE *
jipc_evctx_getenv(jipc_evctx_t *ectx)
{
	JNIEnv	*env;

	JIPC_EVCTX_LOCK(ectx);
	env = jipc_evctx_getenv_l(ectx);
	JIPC_EVCTX_UNLOCK(ectx);

	return env;
}

/*
 * Determine whether the state of the JNI event context is valid or not.
 * This macro must be used with holding the JNI event context lock.
 */
#define	JIPC_EVCTX_STATE_IS_VALID(ectx, state)				\
	((ectx)->je_state == JIPC_EVSTATE_VALID ||			\
	 ((state) == JIPC_EVSTATE_FINALIZED &&				\
	  (ectx)->je_state == JIPC_EVSTATE_SHUTDOWN))

/*
 * Body of jipc_evctx_check(), jipc_evctx_check_rt(), and
 * jipc_evctx_changestate().
 */
#define	JIPC_EVCTX_CHECK_DECL(env, ectx, state, errfunc)		\
	JavaVM		*jvm;						\
	pfc_bool_t	ret;						\
									\
	jvm = pjni_getvm(env);						\
	if (PFC_EXPECT_FALSE(jvm == NULL)) {				\
		/* This should never happen. */				\
		return PFC_FALSE;					\
	}								\
									\
	JIPC_EVCTX_LOCK(ectx);						\
									\
	if (PFC_EXPECT_FALSE((ectx)->je_jvm != jvm ||			\
			     !JIPC_EVCTX_STATE_IS_VALID(ectx, state))) { \
		errfunc(env, ectx);					\
		ret = PFC_FALSE;					\
	}								\
	else {								\
		(ectx)->je_hold++;					\
		if ((state) != JIPC_EVSTATE_VALID) {			\
			PFC_ASSERT((state) == JIPC_EVSTATE_SHUTDOWN ||	\
				   (state) == JIPC_EVSTATE_FINALIZED);	\
			(ectx)->je_state = (state);			\
		}							\
		ret = PFC_TRUE;						\
	}								\
									\
	JIPC_EVCTX_UNLOCK(ectx);					\
									\
	return ret

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_check(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
 *	Ensure that the specified context is initialized by the Java VM
 *	which contains the specified JNI environment.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned with holding the
 *	JNI event context. The caller must release the context by
 *	jipc_evctx_release().
 *
 *	Otherwise PFC_FALSE is returned.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_evctx_check(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	JIPC_EVCTX_CHECK_DECL(env, ectx, JIPC_EVSTATE_VALID, jipc_evctx_fail);
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_check_rt(JNIEnv *PFC_RESTRICT env,
 *		       jipc_evctx_t *PFC_RESTRICT ectx)
 *	Ensure that the specified context is initialized by the Java VM
 *	which contains the specified JNI environment.
 *
 *	Unlike jipc_evctx_check(), this function always throws
 *	IllegalStateException on error.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned with holding the
 *	JNI event context. The caller must release the context by
 *	jipc_evctx_release().
 *
 *	Otherwise PFC_FALSE is returned.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_evctx_check_rt(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	JIPC_EVCTX_CHECK_DECL(env, ectx, JIPC_EVSTATE_VALID,
			      jipc_evctx_fail_rt);
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_evctx_changestate(JNIEnv *PFC_RESTRICT env,
 *			  jipc_evctx_t *PFC_RESTRICT ectx, uint8_t state)
 *	Change the state of the JNI event context.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned with holding the
 *	JNI event context. The caller must release the context by
 *	jipc_evctx_release().
 *
 *	Otherwise PFC_FALSE is returned.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_evctx_changestate(JNIEnv *PFC_RESTRICT env,
		       jipc_evctx_t *PFC_RESTRICT ectx, uint8_t state)
{
	JIPC_EVCTX_CHECK_DECL(env, ectx, state, jipc_evctx_fail);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_event_destroy(pfc_ipcevent_t *event)
 *	Destroy the IPC event object.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_event_destroy(pfc_ipcevent_t *event)
{
	JIPC_LOG_VERBOSE("Destroy event: event=%p", event);
	pfc_ipcevent_destroy_impl(event);
}

/*
 * static inline const char PFC_FATTR_ALWAYS_INLINE *
 * jipc_evhdlr_name(jipc_evhdlr_t *jhp)
 *	Return the name of the given event handler.
 */
static inline const char PFC_FATTR_ALWAYS_INLINE *
jipc_evhdlr_name(jipc_evhdlr_t *jhp)
{
	pfc_refptr_t	*rname = jhp->jeh_name;

	if (rname == NULL) {
		return jipc_str_null;
	}

	return pfc_refptr_string_value(rname);
}

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEvent_getTime(
 *	JNIEnv *env, jobject this, jlong jevent)
 *
 *	Return a TimeSpec instance which contains the creation time of the
 *	event.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to a TimeSpec instance
 *	is returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEvent_getTime(
	JNIEnv *env, jobject this, jlong jevent)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	pfc_ipcevent_t	*event = JIPC_EVENT_PTR(jevent);
	ipc_evproto_t	*iep = &event->ie_proto;
	jmethodID	mid;
	jclass		cls;
	jobject		obj;

	PFC_ASSERT(event != NULL);

	if (PFC_EXPECT_FALSE(!jipc_evctx_check_rt(env, ectx))) {
		return NULL;
	}

	/* Construct a TimeSpec instance. */
	cls = JIPC_EVCTX_CLASS(ectx, TimeSpec);
	mid = JIPC_EVCTX_MID_CTOR(ectx, TimeSpec);
	PFC_ASSERT(cls != NULL);
	PFC_ASSERT(mid != NULL);

	obj = (*env)->NewObject(env, cls, mid, (jlong)iep->iev_time_sec,
				(jlong)iep->iev_time_nsec);
	if (PFC_EXPECT_FALSE(obj == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to create TimeSpec object.");
		/* FALLTHROUGH */
	}

	jipc_evctx_release(env, ectx);

	return obj;
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEvent_getChannelName(
 *	JNIEnv *env, jobject this, jlong jevent)
 *
 *	Return an IPC channel name in the specified IPC event.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to a string is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEvent_getChannelName(
	JNIEnv *env, jobject this, jlong jevent)
{
	pfc_ipcevent_t	*event = JIPC_EVENT_PTR(jevent);

	return pjni_newstring(env, pfc_refptr_string_value(event->ie_chan));
}

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEvent_getHostAddress(
 *	JNIEnv *env, jobject this, jlong jevent)
 *
 *	Return a host address in the specified IPC event.
 *
 * Calling/Exit State:
 *	If the specified event contains an IPv4 or IPv6 host address,
 *	a non-NULL reference to a HostAddress instance is returned.
 *
 *	Otherwise NULL is returned. An exception is thrown on error.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEvent_getHostAddress(
	JNIEnv *env, jobject this, jlong jevent)
{
	const pfc_hostaddr_t	*haddr;
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	pfc_ipcevent_t	*event = JIPC_EVENT_PTR(jevent);
	jmethodID	mid;
	jclass		cls;
	jobject		obj = NULL;
	jbyteArray	array;
	jipc_ipaddr_t	buf;
	size_t		addrlen;
	uint32_t	scope;
	int		err;

	PFC_ASSERT(event != NULL);

	if (PFC_EXPECT_FALSE(!jipc_evctx_check_rt(env, ectx))) {
		return NULL;
	}

	/* Fetch raw IP address in the host address. */
	haddr = &event->ie_addr;
	addrlen = sizeof(buf);
	err = pfc_hostaddr_getaddr(haddr, &buf.jip_addr, &addrlen);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to fetch host address: err=%d", err);
		goto out;
	}

	if (addrlen == 0) {
		/* Return NULL because the host address is local address. */
		PFC_ASSERT(pfc_hostaddr_gettype(haddr) == AF_UNIX);
		goto out;
	}

	PFC_ASSERT(addrlen == sizeof(buf.jip_addr4) ||
		   addrlen == sizeof(buf.jip_addr6));

	/* Obtain IPv6 scope ID. */
	err = pfc_hostaddr_getscope(haddr, &scope);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to fetch scope ID in host address: err=%d",
			   err);
		goto out;
	}

	/* Construct a byte array which contains this address. */
	array = pjni_bytearray_new(env, &buf.jip_addr, addrlen);
	if (PFC_EXPECT_FALSE(array == NULL)) {
		goto out;
	}

	/* Construct a HostAddress instance. */
	cls = JIPC_EVCTX_CLASS(ectx, HostAddress);
	mid = JIPC_EVCTX_MID(ectx, HostAddress, getHostAddress);
	PFC_ASSERT(cls != NULL);
	PFC_ASSERT(mid != NULL);

	obj = (*env)->CallStaticObjectMethod(env, cls, mid, array, scope);
	(*env)->DeleteLocalRef(env, array);
	if (PJNI_EXCEPTION_CHECK(env)) {
		obj = NULL;
	}
	else if (PFC_EXPECT_FALSE(obj == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to create TimeSpec object.");
		/* FALLTHROUGH */
	}

out:
	jipc_evctx_release(env, ectx);

	return obj;
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEvent_getServiceName(
 *	JNIEnv *env, jobject this, jlong jevent)
 *
 *	Return an IPC service name in the specified IPC event.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to a string is
 *	returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEvent_getServiceName(
	JNIEnv *env, jobject this, jlong jevent)
{
	pfc_ipcevent_t	*event = JIPC_EVENT_PTR(jevent);

	PFC_ASSERT(pfc_refptr_string_length(event->ie_sess.icss_name) ==
		   IPC_EVENT_SERVICE_NAMELEN(event));

	return pjni_newstring(env, IPC_EVENT_SERVICE(event));
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEvent_destroy(
 *	JNIEnv *env, jobject this, jlong jevent)
 *
 *	Destroy the IPC event object.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEvent_destroy(
	JNIEnv *env, jobject this, jlong jevent)
{
	pfc_ipcevent_t	*event = JIPC_EVENT_PTR(jevent);

	PFC_ASSERT(event != NULL);
	jipc_event_destroy(event);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_initialize(
 *	JNIEnv *env, jobject this, jobject conf)
 *
 *	Initialize the IPC event subsystem.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_initialize(
	JNIEnv *env, jobject this, jobject conf)
{
	pfc_ipcevopts_t	*ops, buf;
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	int		err;

	/* Initialize the JNI event context. */
	if (PFC_EXPECT_FALSE(!jipc_evctx_init(env, ectx))) {
		return;
	}

	/* Install IPC event subsystem operations. */
	err = pfc_ipcclnt_evsysops_install(&jipc_evsys_ops);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to install event subsystem operations: "
			   "err=%d", err);
		goto error;
	}

	if (conf == NULL) {
		ops = NULL;
	}
	else {
		uint8_t		*ptr = (uint8_t *)&buf;

		/* Fetch parameters in IpcEventConfiguration. */
		memset(&buf, 0, sizeof(buf));
		ops = &buf;
		if (PFC_EXPECT_FALSE(!jipc_fetchconf(env, conf, ptr))) {
			goto error;
		}
	}

	/* Initialize the IPC event subsystem. */
	err = pfc_ipcclnt_event_init(ops);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err < 0) {
			pjni_throw(env, PJNI_CLASS(IpcJniException),
				   "Unable to attach the IPC event dispatcher "
				   "thread to the Java VM: err=%d", err);
		}
		else {
			jipc_throw(env, err, NULL,
				   "Failed to initialize the IPC event "
				   "subsystem: err=%d", err);
		}
		goto error;
	}

	jipc_evctx_release(env, ectx);

	return;

error:
	pfc_ipcclnt_evsysops_uninstall(&jipc_evsys_ops);

	JIPC_EVCTX_LOCK(ectx);
	jipc_evctx_release_l(env, ectx);
	jipc_evctx_destroy(env, ectx);
	JIPC_EVCTX_UNLOCK(ectx);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_shutdown(
 *	JNIEnv *env, jobject this)
 *
 *	Start shutdown sequence of the IPC event subsystem.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_shutdown(
	JNIEnv *env, jobject this)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	const uint8_t	state = JIPC_EVSTATE_SHUTDOWN;
	int		err;

	if (PFC_EXPECT_FALSE(!jipc_evctx_changestate(env, ectx, state))) {
		return;
	}

	err = pfc_ipcclnt_event_shutdown();
	if (PFC_EXPECT_FALSE(err != 0)) {
		/*
		 * We don't restore the state of the JNI event context because
		 * this should be unrecoverable.
		 */
		jipc_throw(env, err, extable_event,
			   "Failed to shutdown the IPC event subsystem: "
			   "err=%d", err);
		/* FALLTHROUGH */
	}

	jipc_evctx_release(env, ectx);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_disable(
 *	JNIEnv *env, jobject this)
 *
 *	Finalize the IPC event subsystem.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_disable(
	JNIEnv *env, jobject this)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	const uint8_t	state = JIPC_EVSTATE_FINALIZED;
	int		err;

	if (PFC_EXPECT_FALSE(!jipc_evctx_changestate(env, ectx, state))) {
		return;
	}

	/* Finalize the IPC event subsystem. */
	err = pfc_ipcclnt_event_fini();

	/* Finalize the JNI event context. */
	jipc_evctx_fini(env, ectx);

	if (PFC_EXPECT_FALSE(err != 0)) {
		/*
		 * We don't restore the state of the JNI event context because
		 * this should be unrecoverable.
		 */
		jipc_throw(env, err, extable_event,
			   "Failed to disable the IPC event subsystem: "
			   "err=%d", err);
		/* FALLTHROUGH */
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_removeHandler(
 *	JNIEnv *env, jobject this, jint id)
 *
 *	Remove the IPC event handler associated with the specified ID.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_removeHandler(
	JNIEnv *env, jobject this, jint id)
{
	int	err;

	err = pfc_ipcevent_remove_handler((pfc_ipcevhdlr_t)id);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_handler,
			   "Failed to remove IPC event handler: "
			   "id=%u, err=%d ", id, err);
		/* FALLTHROUGH */
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_setStackTraceEnabled(
 *	JNIEnv *env, jobject this, jboolean enabled)
 *
 *	Enable or disable the stack trace dump on the IPC event dispatcher
 *	thread.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_setStackTraceEnabled(
	JNIEnv *env, jobject this, jboolean enabled)
{
	pfc_atomic_swap_uint8((uint8_t *)&jipc_stacktrace_enabled, enabled);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_notifyState(
 *	JNIEnv *env, jobject this, jint id)
 *	Raise an IPC channel state notification event and deliver it to the
 *	specified IPC event handler.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_notifyState(
	JNIEnv *env, jobject this, jint id)
{
	int	err;

	err = pfc_ipcevent_notifystate(id);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == EINVAL) {
			pjni_throw(env,
				   PJNI_CLASS(IpcNoSuchEventHandlerException),
				   "Channel state notification event is not "
				   "targeted: id=%u", id);
		}
		else {
			jipc_throw(env, err, extable_handler,
				   "Failed to post channel state notification "
				   "event: id=%u, err=%d", id, err);
		}
		/* FALLTHROUGH */
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_com_nec_jp_pflow_core_ipc_IpcEventSystem_setAutoCancelEnabled(
 *	JNIEnv *env, jobject this, jboolean value)
 *
 *	Enable or disable auto-cancellation of IPC client session.
 */
JNIEXPORT void JNICALL
Java_com_nec_jp_pflow_core_ipc_IpcEventSystem_setAutoCancelEnabled(
	JNIEnv *env, jobject this, jboolean value)
{
	pfc_ipcclnt_event_setautocancel(value);
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_com_nec_jp_pflow_core_ipc_IpcEventSystem_isAutoCancelEnabled(
 *	JNIEnv *env, jobject this)
 *
 *	Determine whether auto-cancellation of IPC client session is
 *	enabled or not.
 *
 * Calling/Exit State:
 *	JNI_TRUE is returned if enabled.
 *	JNI_FALSE is returned if disabled.
 */
JNIEXPORT jboolean JNICALL
Java_com_nec_jp_pflow_core_ipc_IpcEventSystem_isAutoCancelEnabled(
	JNIEnv *env, jobject this)
{
	return (jboolean)pfc_ipcclnt_event_getautocancel();
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_addHandler(
 *	JNIEnv *env, jobject this, jstring jchannel, jobject handler,
 *	jlong ahandle, jstring jname)
 *
 *	Add the specified IPC event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, an identifier assigned to the specified
 *	IPC event handler is returned.
 *	Otherwise PFC_IPCEVHDLR_INVALID is returned with throwing an exception.
 *
 * Remarks:
 *	This method is called with holding the IpcEventSystem object monitor.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_addHandler(
	JNIEnv *env, jobject this, jstring jchannel, jobject handler,
	jlong ahandle, jstring jname)
{
	pfc_ipcevhdlr_t	id;
	pfc_ipcevattr_t	*attr, attrbuf;
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	jipc_evhdlr_t	*jhp;
	jipc_gref_t	*grefp;
	const char	*channel = NULL, *name = NULL;
	int		err;

	if (PFC_EXPECT_FALSE(!jipc_evctx_check(env, ectx))) {
		return PFC_IPCEVHDLR_INVALID;
	}

	if (PFC_EXPECT_FALSE(handler == NULL)) {
		pjni_throw(env, PJNI_CLASS(NullPointerException),
			   "handler is null.");
		goto error;
	}

	if (ahandle == 0) {
		/* Use local attributes */
		attr = &attrbuf;
		PFC_ASSERT_INT(pfc_ipcevent_attr_init(attr), 0);
	}
	else {
		attr = JIPC_EVATTR_PTR(ahandle);
	}

	/* Fetch the IPC channel name. */
	if (jchannel != NULL) {
		channel = pjni_string_get(env, jchannel);
		if (PFC_EXPECT_FALSE(channel == NULL)) {
			goto error_attr;
		}
	}

	/* Fetch the name of handler. */
	if (jname != NULL) {
		name = pjni_string_get(env, jname);
		if (PFC_EXPECT_FALSE(name == NULL)) {
			goto error_string;
		}
	}

	/* Allocate a private data for the handler. */
	jhp = (jipc_evhdlr_t *)malloc(sizeof(*jhp));
	if (PFC_EXPECT_FALSE(jhp == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to allocate private data for the handler.");
		goto error_string;
	}

	/* Preserve the name of the handler. */
	if (name == NULL) {
		jhp->jeh_name = NULL;
	}
	else {
		jhp->jeh_name = pfc_refptr_string_create(name);
		if (PFC_EXPECT_FALSE(jhp->jeh_name == NULL)) {
			pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
				   "Unable to copy handler's name.");
			goto error_jhp;
		}
	}

	/* Create a global reference to the handler object. */
	grefp = &jhp->jeh_gref;
	if (PFC_EXPECT_FALSE(!jipc_gref_init(env, grefp, jhp, handler,
					     jipc_evhdlr_dtor))) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to create reference to the handler.");
		goto error_jhp;
	}

	/* Set an arbitrary data to the event attributes. */
#ifdef	PFC_VERBOSE_DEBUG
	{
		pfc_ptr_t	arg;
		pfc_ipcevdtor_t	dtor;

		PFC_ASSERT_INT(pfc_ipcevent_attr_getarg(attr, &arg), 0);
		PFC_ASSERT_INT(pfc_ipcevent_attr_getargdtor(attr, &dtor), 0);
		PFC_ASSERT(arg == NULL);
		PFC_ASSERT(dtor == NULL);
	}
#endif	/* PFC_VERBOSE_DEBUG */

	/*
	 * This should never happen as long as valid pointer to attributes is
	 * specified.
	 */
	PFC_ASSERT_INT(pfc_ipcevent_attr_setarg(attr, grefp), 0);
	PFC_ASSERT_INT(pfc_ipcevent_attr_setargdtor(attr, jipc_gref_dtor), 0);

	/* Register the specified event handler. */
	err = pfc_ipcevent_add_handler(&id, channel, jipc_event_handler,
				       attr, name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_event,
			   "Failed to register IPC event handler: err=%d",
			   err);
		id = PFC_IPCEVHDLR_INVALID;
		/* FALLTHROUGH */
	}

	if (ahandle == 0) {
		pfc_ipcevent_attr_destroy(attr);
	}
	else {
		PFC_ASSERT_INT(pfc_ipcevent_attr_setarg(attr, NULL), 0);
		PFC_ASSERT_INT(pfc_ipcevent_attr_setargdtor(attr, NULL), 0);
	}

	pjni_string_release(env, jchannel, channel);
	pjni_string_release(env, jname, name);

	jipc_evctx_release(env, ectx);

	return id;

error_jhp:
	if (jhp->jeh_name != NULL) {
		pfc_refptr_put(jhp->jeh_name);
	}
	free(jhp);

error_string:
	pjni_string_release(env, jchannel, channel);
	pjni_string_release(env, jname, name);

error_attr:
	if (ahandle == 0) {
		pfc_ipcevent_attr_destroy(attr);
	}

error:
	jipc_evctx_release(env, ectx);

	return PFC_IPCEVHDLR_INVALID;
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_getChannelState(
 *	JNIEnv *env, jobject this, jstring jchannel, jbyteArray rawaddr,
 *	jint scope)
 *
 *	Return the state of the specified IPC event listener session.
 *
 * Calling/Exit State:
 *	Upon successful completion, one of EVSYS_STATE_DOWN, EVSYS_STATE_UP,
 *	EVSYS_STATE_IN_PROGRESS is returned.
 *	Otherwise -1 is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventSystem_getChannelState(
	JNIEnv *env, jobject this, jstring jchannel, jbyteArray rawaddr,
	jint scope)
{
	pfc_hostaddr_t	haddr;
	const char	*channel;
	jint		state = -1;
	int		err;

	if (jchannel == NULL) {
		/* Use IPC channel name of the default channel. */
		channel = NULL;
	}
	else {
		channel = pjni_string_get(env, jchannel);
		if (PFC_EXPECT_FALSE(channel == NULL)) {
			return -1;
		}
	}

	/* Fetch the host address. */
	if (PFC_EXPECT_FALSE(!jipc_gethostaddr(env, rawaddr, scope, &haddr))) {
		goto out;
	}

	/* Get the current state of the specified event listener session. */
	err = pfc_ipcevent_isconnected(channel, &haddr);
	if (err == 0) {
		state = EVSYS_STATE_UP;
	}
	else if (err == ECONNRESET) {
		state = EVSYS_STATE_DOWN;
	}
	else if (PFC_EXPECT_TRUE(err == EINPROGRESS)) {
		state = EVSYS_STATE_IN_PROGRESS;
	}
	else {
		char	str[PFC_HOSTADDR_STRSIZE];

		jipc_hostaddr_tostring(&haddr, str, sizeof(str));
		jipc_throw(env, err, extable_handler,
			   "Failed to get state of the IPC event listener "
			   "session: addr=%s@%s, err=%d",
			   (channel == NULL) ? jipc_str_null : channel,
			   str, err);
	}

out:
	pjni_string_release(env, jchannel, channel);

	return state;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcHostSet_create(
 *	JNIEnv *env, jclass cls, jstring jname)
 *
 *	Create a new IPC host set.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcHostSet_create(
	JNIEnv *env, jclass cls, jstring jname)
{
	const char	*name;

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_TRUE(name != NULL)) {
		int	err = pfc_ipcclnt_hostset_create(name);

		if (PFC_EXPECT_FALSE(err != 0)) {
			jipc_throw(env, err, extable_hostset,
				   "Failed to create IPC host set: name=%s, "
				   "err=%d", name, err);
			/* FALLTHROUGH */
		}
		pjni_string_release(env, jname, name);
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcHostSet_destroy(
 *	JNIEnv *env, jclass cls, jstring jname)
 *
 *	Destroy the specified IPC host set.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcHostSet_destroy(
	JNIEnv *env, jclass cls, jstring jname)
{
	const char	*name;

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_TRUE(name != NULL)) {
		int	err = pfc_ipcclnt_hostset_destroy(name);

		if (PFC_EXPECT_FALSE(err != 0)) {
			jipc_throw(env, err, extable_hostset,
				   "Failed to destroy IPC host set: name=%s, "
				   "err=%d", name, err);
			/* FALLTHROUGH */
		}
		pjni_string_release(env, jname, name);
	}
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcHostSet_exists(
 *	JNIEnv *env, jclass cls, jstring jname)
 *
 *	Return JNI_TRUE only if the specified IPC host set exists.
 *	Otherwise JNI_FALSE is returned.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcHostSet_exists(
	JNIEnv *env, jclass cls, jstring jname)
{
	const char	*name;
	jboolean	ret = JNI_FALSE;

	name = jipc_getstring(env, jname, "name");
	if (PFC_EXPECT_TRUE(name != NULL)) {
		int	err = pfc_ipcclnt_hostset_exists(name);

		if (PFC_EXPECT_TRUE(err == 0)) {
			ret = JNI_TRUE;
		}
		else if (err != ENODEV) {
			jipc_throw(env, err, extable_hostset,
				   "Failed to test IPC host set: name=%s, "
				   "err=%d", name, err);
			/* FALLTHROUGH */
		}
		pjni_string_release(env, jname, name);
	}

	return ret;
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcHostSet_handleHostAddress(
 *	JNIEnv *env, jobject this, jstring jname, jint op, jbyteArray rawaddr,
 *	jint scope)
 *
 *	Invoke an operation which handles a HostAddress instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, JNI_TRUE is returned.
 *	Otherwise JNI_FALSE is returned. An exception is also thrown on
 *	fatal error.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcHostSet_handleHostAddress(
	JNIEnv *env, jobject this, jstring jname, jint op, jbyteArray rawaddr,
	jint scope)
{
	const char	*name;
	pfc_hostaddr_t	haddr;
	int		err, non_fatal;
	jboolean	ret = JNI_FALSE;

	PFC_ASSERT(jname != NULL);
	name = pjni_string_get(env, jname);
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return ret;
	}

	/* Fetch the host address. */
	if (PFC_EXPECT_FALSE(!jipc_gethostaddr(env, rawaddr, scope, &haddr))) {
		goto out;
	}

	if (op == HOSTSET_OP_ADD) {
		/* Add the specified host address to the host set. */
		err = pfc_ipcclnt_hostset_add(name, &haddr);
		non_fatal = EEXIST;
	}
	else if (op == HOSTSET_OP_REMOVE) {
		/* Remove the specified host address from the host set. */
		err = pfc_ipcclnt_hostset_remove(name, &haddr);
		non_fatal = ENOENT;
	}
	else if (op == HOSTSET_OP_CONTAINS) {
		/*
		 * Check whether the host set contains the specified host
		 * address.
		 */
		err = pfc_ipcclnt_hostset_contains(name, &haddr);
		non_fatal = ENOENT;
	}
	else {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unexpected operation: %u", op);
		goto out;
	}

	if (PFC_EXPECT_TRUE(err == 0)) {
		ret = JNI_TRUE;
	}
	else if (PFC_EXPECT_FALSE(err != non_fatal)) {
		char	str[PFC_HOSTADDR_STRSIZE];

		jipc_hostaddr_tostring(&haddr, str, sizeof(str));
		jipc_throw(env, err, extable_hostset,
			   "Failed to %s host address: hostset=%s, addr=%s, "
			   "err=%d",
			   (op == HOSTSET_OP_ADD) ? "add"
			   : (op == HOSTSET_OP_REMOVE) ? "remove" : "test",
			   name, str, err);
	}

out:
	pjni_string_release(env, jname, name);

	return ret;
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_createHandle(
 *	JNIEnv *env, jclass cls)
 *
 *	Create a new IPC event attributes.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-zero value which represents IPC
 *	event attributes handle is returned.
 *	Otherwise zero is returned with throwing an exception.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_createHandle(
	JNIEnv *env, jclass cls)
{
	pfc_ipcevattr_t	*attrp;

	/* Allocate buffer for pfc_ipcevattr_t. */
	attrp = (pfc_ipcevattr_t *)malloc(sizeof(*attrp));
	if (PFC_EXPECT_FALSE(attrp == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to allocate pfc_ipcevattr_t.");

		return 0;
	}

	/* This should never fail as long as a valid pointer is specified. */
	PFC_ASSERT_INT(pfc_ipcevent_attr_init(attrp), 0);

	return JIPC_EVATTR_HANDLE(attrp);
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_getHostSet(
 *	JNIEnv *env, jobject this, jlong handle)
 *
 *	Return the name of IPC host set in the specified IPC event attributes.
 *
 * Calling/Exit State:
 *	If the IPC host set is set, a non-NULL Java string which keeps the
 *	name of IPC host set is returned. NULL is returned if not set.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_getHostSet(
	JNIEnv *env, jobject this, jlong handle)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	const char	*name;
	int		err;

	err = pfc_ipcevent_attr_gethostset(attr, &name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_gethostset",
				      err);

		return NULL;
	}

	if (name == NULL) {
		/* No IPC host set is set. */
		return NULL;
	}

	return pjni_newstring(env, name);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_setHostSet(
 *	JNIEnv *env, jobject this, jlong handle, jstring jname)
 *
 *	Set the IPC host set to this IPC event attributes.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_setHostSet(
	JNIEnv *env, jobject this, jlong handle, jstring jname)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	const char	*name;
	int		err;

	if (jname == NULL) {
		/* Reset IPC host set. */
		name = NULL;
	}
	else {
		name = pjni_string_get(env, jname);
		if (PFC_EXPECT_FALSE(name == NULL)) {
			return;
		}
	}

	err = pfc_ipcevent_attr_sethostset(attr, name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, extable_hostset,
			   "Failed to set IPC host set: name=%s, err=%d",
			   (name == NULL) ? jipc_str_null : name, err);
		/* FALLTHROUGH */
	}

	pjni_string_release(env, jname, name);
}

/*
 * JNIEXPORT jlong JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_getTarget(
 *	JNIEnv *env, jobject this, jlong handle, jstring jservice)
 *
 *	Return the IPC event mask value associated with the specified IPC
 *	service name in the target event set.
 *
 * Calling/Exit State:
 *	On error, zero is returned with throwing an exception.
 */
JNIEXPORT jlong JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_getTarget(
	JNIEnv *env, jobject this, jlong handle, jstring jservice)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	const char	*service;
	int		err;
	jlong		mask;

	if (jservice == NULL) {
		/* Server state change event is specified. */
		service = NULL;
	}
	else {
		service = pjni_string_get(env, jservice);
		if (PFC_EXPECT_FALSE(service == NULL)) {
			return 0;
		}
	}

	err = pfc_ipcevent_attr_gettarget(attr, service,
					  (pfc_ipcevmask_t *)&mask);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_gettarget",
				      err);
		mask = 0;
		/* FALLTHROUGH */
	}

	pjni_string_release(env, jservice, service);

	return mask;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_addTarget(
 *	JNIEnv *env, jobject this, jlong handle, jstring jservice, jlong mask)
 *
 *	Add a target event into the target event set in the specified IPC
 *	event attributes.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_addTarget(
	JNIEnv *env, jobject this, jlong handle, jstring jservice, jlong mask)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	const char	*service;
	int		err;

	if (jservice == NULL) {
		/* Server state change event is specified. */
		service = NULL;
	}
	else {
		service = pjni_string_get(env, jservice);
		if (PFC_EXPECT_FALSE(service == NULL)) {
			return;
		}
	}

	err = pfc_ipcevent_attr_addtarget(attr, service,
					  (pfc_ipcevmask_t *)&mask);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to add a target event: service=%s, mask=0x%"
			   PFC_PFMT_x64,
			   (service == NULL) ? jipc_str_null : service, mask);
	}

	pjni_string_release(env, jservice, service);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_resetTarget(
 *	JNIEnv *env, jobject this, jlong handle)
 *
 *	Reset the target event set in the specified IPC event attributes
 *	to initial state.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_resetTarget(
	JNIEnv *env, jobject this, jlong handle)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	int		err;

	err = pfc_ipcevent_attr_resettarget(attr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_resettarget",
				      err);
		/* FALLTHROUGH */
	}
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_getPriority(
 *	JNIEnv *env, jobject this, jlong handle)
 *
 *	Return an IPC event handler's priority value in this IPC event
 *	attributes.
 *
 * Calling/Exit State:
 *	Upon successful completion, a priority value is returned.
 *	Otherwise zero is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_getPriority(
	JNIEnv *env, jobject this, jlong handle)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	uint32_t	pri;
	int		err;

	err = pfc_ipcevent_attr_getpriority(attr, &pri);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_getpriority",
				      err);
		pri = 0;
		/* FALLTHROUGH */
	}

	return (jint)pri;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_setPriority(
 *	JNIEnv *env, jobject this, jlong handle, jint priority)
 *
 *	Set an IPC event handler's priority value to this IPC event attributes.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_setPriority(
	JNIEnv *env, jobject this, jlong handle, jint priority)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	int		err;

	err = pfc_ipcevent_attr_setpriority(attr, (uint32_t)priority);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_setpriority",
				      err);
		/* FALLTHROUGH */
	}
}

/*
 * JNIEXPORT jboolean JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_isLogEnabled(
 *	JNIEnv *env, jobject this, jlong handle)
 *
 *	Determine whether the event delivery logging is enabled or not.
 *
 * Calling/Exit State:
 *	JNI_TRUE is returned if the event delivery logging is enabled in this
 *	IPC event attributes.
 *	Otherwise JNI_FALSE is returned. Note that an exception is thrown on
 *	fatal error.
 */
JNIEXPORT jboolean JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_isLogEnabled(
	JNIEnv *env, jobject this, jlong handle)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	pfc_bool_t	enabled;
	int		err;

	err = pfc_ipcevent_attr_getlog(attr, &enabled);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_getlog", err);
		enabled = PFC_FALSE;
		/* FALLTHROUGH */
	}

	return (jboolean)enabled;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_setLogEnabled(
 *	JNIEnv *env, jobject this, jlong handle, jboolean enabled)
 *
 *	Enable or disable the event delivery logging in this IPC event
 *	attributes.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_setLogEnabled(
	JNIEnv *env, jobject this, jlong handle, jboolean enabled)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);
	int		err;

	err = pfc_ipcevent_attr_setlog(attr, (pfc_bool_t)enabled);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		jipc_throw_unexpected(env, "pfc_ipcevent_attr_setlog", err);
		/* FALLTHROUGH */
	}
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_finalize(
 *	JNIEnv *env, jobject this, jlong handle)
 *
 *	Finalize the IPC event attributes.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_IpcEventAttribute_finalize(
	JNIEnv *env, jobject this, jlong handle)
{
	pfc_ipcevattr_t	*attr = JIPC_EVATTR_PTR(handle);

	if (PFC_EXPECT_TRUE(attr != NULL)) {
		pfc_ipcevent_attr_destroy(attr);
		free(attr);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * jipc_event_clear(JavaVM *jvm)
 *	Clear context for the IPC event dispatcher thread.
 *
 *	A pointer to JavaVM must be specified to `jvm'.
 */
void PFC_ATTR_HIDDEN
jipc_event_clear(JavaVM *jvm)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;

	PFC_ASSERT(jvm != NULL);

	JIPC_EVCTX_LOCK(ectx);

	if (jvm == ectx->je_jvm) {
		while (ectx->je_hold != 0) {
			ectx->je_unloading++;
			JIPC_EVCTX_WAIT(ectx);
			ectx->je_unloading--;
		}

		ectx->je_jvm = NULL;
		ectx->je_env = NULL;
	}

	JIPC_EVCTX_UNLOCK(ectx);
}

/*
 * static pfc_bool_t
 * jipc_fetchconf(JNIEnv *PFC_RESTRICT env, jobject conf,
 *		  uint8_t *PFC_RESTRICT ops)
 *	Fetch parameters in IpcEventConfiguration instance.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on success.
 *	Otherwise PFC_FALSE is returned with throwing an exception.
 */
static pfc_bool_t
jipc_fetchconf(JNIEnv *PFC_RESTRICT env, jobject conf,
	      uint8_t *PFC_RESTRICT ops)
{
	jclass		cls;
	pfc_bool_t	ret = PFC_TRUE;
	jipc_cconfdef_t	*def;

	/* Determine class. */
	cls = pjni_findclass(env, PJNI_CLASS(IpcEventConfiguration));
	if (PFC_EXPECT_FALSE(cls == NULL)) {
		return PFC_FALSE;
	}

	/* Fetch parameters in IpcEventConfiguration fields. */
	for (def = jipc_conf_fields; def < PFC_ARRAY_LIMIT(jipc_conf_fields);
	     def++) {
		jfieldID	fid;
		uint32_t	value;
		uint32_t	*dstp = (uint32_t *)(ops + def->jcd_off);

		fid = pjni_getfield(env, cls, def->jcd_name, PJNI_SIG_int);
		if (PFC_EXPECT_FALSE(fid == NULL)) {
			ret = PFC_FALSE;
			break;
		}

		value = (uint32_t)(*env)->GetIntField(env, conf, fid);
		if (PJNI_EXCEPTION_CHECK(env)) {
			ret = PFC_FALSE;
			break;
		}

		IPC_STORE_INT(dstp, value);
	}

	(*env)->DeleteLocalRef(env, cls);

	return ret;
}

/*
 * static pfc_bool_t
 * jipc_evctx_init(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
 *	Initialize the JNI event context.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned.
 *	Otherwise PFC_FALSE is returned with throwing an exception.
 *
 * Remarks:
 *	This function holds the JNI event context on successful return.
 *	The caller must release the context by jipc_evctx_release().
 */
static pfc_bool_t
jipc_evctx_init(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	JavaVM		*jvm;
	jclass		cls;
	uint32_t	i;

	jvm = pjni_getvm(env);
	if (PFC_EXPECT_FALSE(jvm == NULL)) {
		/* This should never happen. */
		return PFC_FALSE;
	}

	JIPC_EVCTX_LOCK(ectx);

	if (PFC_EXPECT_FALSE(ectx->je_jvm != NULL ||
			     ectx->je_state != JIPC_EVSTATE_VALID)) {
		const char	*clname, *msg;

		if (ectx->je_state == JIPC_EVSTATE_SHUTDOWN) {
			clname = PJNI_CLASS(IpcCanceledException);
			msg = emsg_shutdown;
		}
		else if (ectx->je_state == JIPC_EVSTATE_FINALIZED) {
			clname = PJNI_CLASS(IpcCanceledException);
			msg = emsg_already_finalized;
		}
		else if (ectx->je_jvm == jvm) {
			clname = PJNI_CLASS(IpcResourceBusyException);
			msg = emsg_already_initialized;
		}
		else {
			clname = PJNI_CLASS(IllegalStateException);
			msg = emsg_another_vm;
		}
		JIPC_EVCTX_UNLOCK(ectx);
		pjni_throw(env, clname, msg);

		return PFC_FALSE;
	}

	PFC_ASSERT(ectx->je_hold == 0);

	PFC_ASSERT_INT(pfc_cond_init(&ectx->je_cond), 0);

	/* Keep global references to class objects. */
	for (i = 0; i < PFC_ARRAY_CAPACITY(ectx->je_classes); i++) {
		const char	*clname = jipc_cached_classes[i];

		PFC_ASSERT(ectx->je_classes[i] == NULL);
		PFC_ASSERT(i < PFC_ARRAY_CAPACITY(jipc_cached_classes));
		ectx->je_classes[i] = pjni_loadclass(env, clname);
		if (PFC_EXPECT_FALSE(ectx->je_classes[i] == NULL)) {
			goto error;
		}
	}

	/* Cache method IDs. */
	for (i = 0; i < PFC_ARRAY_CAPACITY(ectx->je_methods); i++) {
		jipc_cmidspec_t	*spec = &jipc_cached_methods[i];

		PFC_ASSERT(ectx->je_methods[i] == NULL);
		PFC_ASSERT(i < PFC_ARRAY_CAPACITY(jipc_cached_methods));
		ectx->je_methods[i] = jipc_evctx_getmid(env, ectx, spec);
		if (PFC_EXPECT_FALSE(ectx->je_methods[i] == NULL)) {
			goto error;
		}
	}

	ectx->je_jvm = jvm;
	ectx->je_hold++;

	JIPC_EVCTX_UNLOCK(ectx);

	return PFC_TRUE;

error:
	for (i = 0; i < PFC_ARRAY_CAPACITY(ectx->je_classes); i++) {
		cls = ectx->je_classes[i];
		if (cls != NULL) {
			(*env)->DeleteGlobalRef(env, cls);
			ectx->je_classes[i] = NULL;
		}
	}

	memset(ectx->je_methods, 0, sizeof(ectx->je_methods));

	JIPC_EVCTX_UNLOCK(ectx);

	return PFC_FALSE;
}

/*
 * static jmethodID
 * jipc_evctx_getmid(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
 *		     jipc_cmidspec_t *PFC_RESTRICT spec)
 *	Return the method ID for the specified method.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL method ID is returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
static jmethodID
jipc_evctx_getmid(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
		  jipc_cmidspec_t *PFC_RESTRICT spec)
{
	jmethodID	mid;
	jclass		cls, loaded;
	const char	*method, *sig;

	if (spec->jms_cached) {
		/* Class object is cached. */
		cls = ectx->je_classes[spec->jms_un.id];
		PFC_ASSERT(cls != NULL);
		loaded = NULL;
	}
	else {
		cls = pjni_findclass(env, spec->jms_un.name);
		if (PFC_EXPECT_FALSE(cls == NULL)) {
			return NULL;
		}

		loaded = cls;
	}

	method = spec->jms_name;
	sig = spec->jms_sig;

	if (spec->jms_static) {
		mid = (*env)->GetStaticMethodID(env, cls, method, sig);
	}
	else {
		mid = (*env)->GetMethodID(env, cls, method, sig);
	}

	if (loaded != NULL) {
		(*env)->DeleteLocalRef(env, loaded);
	}

	if (PFC_EXPECT_FALSE(mid == NULL)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to find method ID: %s, %s", method, sig);
		/* FALLTHROUGH */
	}

	return mid;
}

/*
 * static void
 * jipc_evctx_fail(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
 *	Throw an exception which indicates the IPC event subsystem is not
 *	initialized correctly.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock.
 */
static void
jipc_evctx_fail(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	const char	*clname, *msg;

	if (ectx->je_state == JIPC_EVSTATE_SHUTDOWN) {
		clname = PJNI_CLASS(IpcCanceledException);
		msg = emsg_shutdown;
	}
	else if (ectx->je_state == JIPC_EVSTATE_FINALIZED) {
		clname = PJNI_CLASS(IpcCanceledException);
		msg = emsg_already_finalized;
	}
	else if (ectx->je_jvm == NULL) {
		clname = PJNI_CLASS(IpcEventSystemNotReadyException);
		msg = emsg_not_initialized;
	}
	else {
		clname = PJNI_CLASS(IllegalStateException);
		msg = emsg_another_vm;
	}

	pjni_throw(env, clname, msg);
}

/*
 * static void
 * jipc_evctx_fail_rt(JNIEnv *PFC_RESTRICT env,
 *		      jipc_evctx_t *PFC_RESTRICT ectx)
 *	Throw an exception which indicates the IPC event subsystem is not
 *	initialized correctly.
 *
 *	Unlike jipc_evctx_fail(), this function always throws
 *	IllegalStateException.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock.
 */
static void
jipc_evctx_fail_rt(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	const char	*msg;

	if (ectx->je_state == JIPC_EVSTATE_SHUTDOWN) {
		msg = emsg_shutdown;
	}
	else if (ectx->je_state == JIPC_EVSTATE_FINALIZED) {
		msg = emsg_already_finalized;
	}
	else if (ectx->je_jvm == NULL) {
		msg = emsg_not_initialized;
	}
	else {
		msg = emsg_another_vm;
	}

	pjni_throw(env, PJNI_CLASS(IllegalStateException), msg);
}

/*
 * static void
 * jipc_evctx_destroy(JNIEnv *PFC_RESTRICT env,
 *		      jipc_evctx_t *PFC_RESTRICT ectx)
 *	Destroy the JNI event context.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock.
 */
static void
jipc_evctx_destroy(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	jclass	*clsp;

	PFC_ASSERT(env != NULL);
	PFC_ASSERT(pjni_getvm(env) == ectx->je_jvm);
	PFC_ASSERT(ectx->je_hold == 0);

	/* Clean up global references. */
	jipc_evctx_gc(env, ectx);

	/* Destroy Java object caches. */
	for (clsp = ectx->je_classes; clsp < PFC_ARRAY_LIMIT(ectx->je_classes);
	     clsp++) {
		jclass	cls = *clsp;

		if (PFC_EXPECT_TRUE(cls != NULL)) {
			(*env)->DeleteLocalRef(env, cls);
			*clsp = NULL;
		}
	}

	ectx->je_jvm = NULL;
	ectx->je_env = NULL;
	ectx->je_finicnt = 0;

	memset(ectx->je_methods, 0, sizeof(ectx->je_methods));

	if (PFC_EXPECT_FALSE(ectx->je_unloading != 0)) {
		JIPC_EVCTX_BROADCAST(ectx);
	}
}

/*
 * static void
 * jipc_evctx_gc_clear(JNIEnv *PFC_RESTRICT env,
 *		       jipc_evctx_t *PFC_RESTRICT ectx)
 *	Free up global references on the GC list.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock.
 */
static void
jipc_evctx_gc_clear(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx)
{
	jipc_gref_t	*grefp, *next;

	/* This should never happen. */
	for (grefp = ectx->je_grefs; grefp != NULL; grefp = next) {
		next = grefp->jgr_next;
		jipc_gref_destroy(env, grefp);
	}

	ectx->je_grefs = NULL;
}

/*
 * static int
 * jipc_evdisp_ctor(void)
 *	Constructor of the IPC event dispatcher thread.
 *
 *	This function is called on the IPC event dispatcher thread when it
 *	has been started.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	A negative integer value is returned if the calling thread could not
 *	be attached to the Java VM.
 */
static int
jipc_evdisp_ctor(void)
{
	JavaVMAttachArgs	args;
	JavaVM		*jvm;
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	int		err;

	args.version = PJNI_MIN_VERSION;
	args.name = "IPC Event Dispatcher";
	args.group = NULL;

	JIPC_EVCTX_LOCK(ectx);

	jvm = ectx->je_jvm;
	PFC_ASSERT(jvm != NULL);

	/*
	 * Attach the IPC event dispatch thread to the Java VM as daemon
	 * thread.
	 */
	err = (*jvm)->AttachCurrentThreadAsDaemon(jvm, (void **)&ectx->je_env,
						  &args);
	if (PFC_EXPECT_TRUE(err == JNI_OK)) {
		PFC_ASSERT(ectx->je_env != NULL);
		PFC_ASSERT(JNI_OK == 0);

		/*
		 * Initialize the finalization counter with 2.
		 * 1 for the event dispatcher thread, and 1 for
		 * IpcEventSystem.disable().
		 */
		ectx->je_finicnt = 2;
		ectx->je_state = JIPC_EVSTATE_VALID;

		JIPC_LOG_DEBUG("IPC event dispatcher thread has been "
			       "initialized.");
	}
	else {
		ectx->je_env = NULL;
	}

	JIPC_EVCTX_UNLOCK(ectx);

	return err;
}

/*
 * static void
 * jipc_evdisp_dtor(void)
 *	Destructor of the IPC event dispatcher thread.
 */
static void
jipc_evdisp_dtor(void)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	JNIEnv		*env;

	JIPC_EVCTX_LOCK(ectx);

	env = jipc_evctx_getenv_l(ectx);
	if (PFC_EXPECT_TRUE(env != NULL)) {
		JavaVM	*jvm = ectx->je_jvm;
		jint	ret;

		PFC_ASSERT(jvm != NULL);

		/* Destroy the JNI event context. */
		jipc_evctx_fini_l(env, ectx);

		/* Detach the IPC event dispatcher thread from the Java VM. */
		ret = (*jvm)->DetachCurrentThread(jvm);
		if (PFC_EXPECT_FALSE(ret != JNI_OK)) {
			JIPC_LOG_WARN("Unable to detach dispatcher thread: "
				      "ret=%d", ret);
		}
	}

	JIPC_EVCTX_UNLOCK(ectx);
}

/*
 * static pfc_bool_t
 * jipc_gethostaddr(JNIEnv *PFC_RESTRICT env, jbyteArray rawaddr, int scope,
 *		    pfc_hostaddr_t *PFC_RESTRICT haddr)
 *	Fetch pfc_hostaddr_t value specified by Java code.
 *
 * Calling/Exit State:
 *	Upon successful completion, a value of pfc_hostaddr_t is set into the
 *	buffer pointed by `haddr', and PFC_TRUE is returned.
 *	Otherwise PFC_FALSE is returned with throwing an exception.
 */
static pfc_bool_t
jipc_gethostaddr(JNIEnv *PFC_RESTRICT env, jbyteArray rawaddr, int scope,
		 pfc_hostaddr_t *PFC_RESTRICT haddr)
{
	jsize		alen;
	jipc_ipaddr_t	buf;

	PFC_ASSERT((void *)&buf.jip_addr4 == (void *)&buf.jip_addr);
	PFC_ASSERT((void *)&buf.jip_addr6 == (void *)&buf.jip_addr);

	if (rawaddr == NULL) {
		/* Local address. */
		PFC_ASSERT_INT(pfc_hostaddr_init_local(haddr), 0);

		return PFC_TRUE;
	}

	alen = (*env)->GetArrayLength(env, rawaddr);
	if (PFC_EXPECT_FALSE((uint32_t)alen != sizeof(buf.jip_addr4) &&
			     (uint32_t)alen != sizeof(buf.jip_addr6))) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unexpected IP address length: %u", alen);

		return PFC_FALSE;
	}

	/* Fetch raw IP address in a byte array. */
	(*env)->GetByteArrayRegion(env, rawaddr, 0, alen, &buf.jip_addr);
	if (PJNI_EXCEPTION_CHECK(env)) {
		return PFC_FALSE;
	}

	if (alen == sizeof(buf.jip_addr4)) {
		/* IPv4 address. */
		PFC_ASSERT(scope == 0);
		PFC_ASSERT_INT(pfc_hostaddr_init_inet4(haddr, &buf.jip_addr4),
			       0);
	}
	else {
		/* IPv6 address. */
		PFC_ASSERT(alen == sizeof(buf.jip_addr6));
		PFC_ASSERT_INT(pfc_hostaddr_init_inet6(haddr, &buf.jip_addr6,
						       scope), 0);
	}

	return PFC_TRUE;
}

/*
 * static void
 * jipc_hostaddr_tostring(const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *			  char *PFC_RESTRICT buf, size_t bufsize)
 *	Convert the specified pfc_hostaddr_t into a string.
 */
static void
jipc_hostaddr_tostring(const pfc_hostaddr_t *PFC_RESTRICT haddr,
		       char *PFC_RESTRICT buf, size_t bufsize)
{
	int	err;

	err = pfc_hostaddr_tostring(haddr, buf, bufsize, PFC_HA2STR_TYPE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_strlcpy(buf, jipc_str_unknown, bufsize);
	}
}

/*
 * static void
 * jipc_event_handler(pfc_ipcevent_t *event, pfc_ptr_t arg)
 *	Invoke an IPC event handler.
 *	This function is always called on the IPC event dispatcher thread.
 */
static void
jipc_event_handler(pfc_ipcevent_t *event, pfc_ptr_t arg)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	jipc_gref_t	*grefp = (jipc_gref_t *)arg;
	JNIEnv		*env;
	jobject		eobj;
	jmethodID	mid;

	env = jipc_evctx_getenv(ectx);
	if (PFC_EXPECT_FALSE(env == NULL)) {
		return;
	}

	/* Create an IpcEvent instance associated with this event. */
	eobj = jipc_event_create(ectx, event);
	if (PFC_EXPECT_TRUE(eobj != NULL)) {
		jclass	cls;

		mid = JIPC_EVCTX_MID(ectx, IpcEventHandler, eventReceived);
		PFC_ASSERT(mid != NULL);

		/* Invoke IpcEventHandler.eventReceived(IpcEvent). */
		PFC_ASSERT(grefp->jgr_object != NULL);
		(*env)->CallVoidMethod(env, grefp->jgr_object, mid, eobj);

		jipc_event_catch(env, ectx, (jipc_evhdlr_t *)grefp->jgr_base);

		/* Clear interrupted status of the current thread. */
		cls = JIPC_EVCTX_CLASS(ectx, Thread);
		mid = JIPC_EVCTX_MID(ectx, Thread, interrupted);
		PFC_ASSERT(cls != NULL);
		PFC_ASSERT(mid != NULL);
		(void)(*env)->CallStaticBooleanMethod(env, cls, mid);
		(*env)->ExceptionClear(env);
	}

	jipc_evctx_release(env, ectx);
}

/*
 * static void
 * jipc_event_catch(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
 *		    jipc_evhdlr_t *PFC_RESTRICT jhp)
 *	Clean up an exception thrown by the event handler.
 */
static void
jipc_event_catch(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
		 jipc_evhdlr_t *PFC_RESTRICT jhp)
{
	jthrowable	t;

	t = (*env)->ExceptionOccurred(env);
	if (PFC_EXPECT_TRUE(t == NULL)) {
		/* No exception is pending. */
		return;
	}

	JIPC_EXCEPTION_DESCRIBE(env);

	/* Clear exception. */
	(*env)->ExceptionClear(env);

	if (jipc_log_enabled) {
		jstring		jstr;
		const char	*cause;

		/* Convert a throwable into a string. */
		cause = jipc_event_tostring(env, ectx, t, &jstr);
		if (PFC_EXPECT_FALSE(cause == NULL)) {
			cause = jipc_str_unknown;
			jstr = NULL;
		}

		JIPC_LOG_ERROR("Uncaught exception in event handler: "
			       "name=%s, cause=%s", jipc_evhdlr_name(jhp),
			       cause);
		if (jstr != NULL) {
			(*env)->ReleaseStringUTFChars(env, jstr, cause);
			(*env)->DeleteLocalRef(env, jstr);
		}

		(*env)->ExceptionClear(env);
	}

	(*env)->DeleteLocalRef(env, t);
}

/*
 * static const char *
 * jipc_event_tostring(JNIEnv *PFC_RESTRICT env,
 *		       jipc_evctx_t *PFC_RESTRICT ectx, jobject obj,
 *		       jstring *PFC_RESTRICT jstrp)
 *	Convert a Java object into a string.
 *
 * Calling/Exit State:
 *	Upon successful completion, a string representation of the specified
 *	object is returned. In this case, a local reference to the Java
 *	string is set to the buffer pointed by `jstrp'. Returned string must
 *	be released by ReleaseStringUTFChars().
 *
 *	Otherwise NULL is returned.
 *
 * Remarks:
 *	This function must be called on the IPC event dispatcher thread.
 */
static const char *
jipc_event_tostring(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
		    jobject obj, jstring *PFC_RESTRICT jstrp)
{
	jmethodID	mid;
	jstring		jstr;
	const char	*str;

	/* Call toString(). */
	mid = JIPC_EVCTX_MID(ectx, Object, toString);
	PFC_ASSERT(mid != NULL);
	jstr = (*env)->CallObjectMethod(env, obj, mid);
	if (PFC_EXPECT_FALSE(jstr == NULL || PJNI_EXCEPTION_CHECK(env))) {
		JIPC_LOG_ERROR("Unable to convert an object into a string.");
		JIPC_EXCEPTION_DESCRIBE(env);
		if (jstr != NULL) {
			(*env)->DeleteLocalRef(env, jstr);
		}

		return NULL;
	}

	str = (*env)->GetStringUTFChars(env, jstr, NULL);
	if (PFC_EXPECT_FALSE(str == NULL)) {
		(*env)->DeleteLocalRef(env, jstr);
		JIPC_LOG_ERROR("Unable to fetch a Java string.");
		JIPC_EXCEPTION_DESCRIBE(env);

		return NULL;
	}

	*jstrp = jstr;

	return str;
}

/*
 * static jobject
 * jipc_event_create(jipc_evctx_t *PFC_RESTRICT ectx,
 *		     pfc_ipcevent_t *PFC_RESTRICT event)
 *	Create an IpcEvent instance associated with the specified IPC event.
 *
 * Calling/Exit State:
 *	Upon successful completion, a global reference to an IpcEvent instance
 *	is returned.
 *	NULL is returned on failure.
 */
static jobject
jipc_event_create(jipc_evctx_t *PFC_RESTRICT ectx,
		  pfc_ipcevent_t *PFC_RESTRICT event)
{
	jipc_gref_t	*grefp = (jipc_gref_t *)event->ie_data;

	if (grefp == NULL) {
		jobject		obj;
		JNIEnv		*env;
		jipc_event_t	*jep;
		const char	*emsg;
		pfc_ipcevtype_t	type = IPC_EVENT_TYPE(event);

		PFC_ASSERT(event->ie_dtor == NULL);
		env = ectx->je_env;
		PFC_ASSERT(env != NULL);

		/* Allocate a private data for the event. */
		jep = (jipc_event_t *)malloc(sizeof(*jep));
		if (PFC_EXPECT_FALSE(jep == NULL)) {
			emsg = "Unable to allocate private data";

			return jipc_event_error(env, ectx, event, emsg, jep);
		}

		/* Create an IpcEvent instance. */
		obj = jipc_event_new(env, ectx, event, type);
		if (PFC_EXPECT_FALSE(obj == NULL)) {
			emsg = "Unable to create IpcEvent object";

			return jipc_event_error(env, ectx, event, emsg, jep);
		}

		/* Create a global reference to the IpcEvent instance. */
		jep->jev_event = event;
		grefp = &jep->jev_gref;
		if (PFC_EXPECT_FALSE(!jipc_gref_init(env, grefp, jep, obj,
						     jipc_event_dtor))) {
			emsg = "Unable to create a global reference to "
				"IpcEvent";

			return jipc_event_error(env, ectx, event, emsg, jep);
		}

		/* Set private destructor for this event. */
		event->ie_dtor = jipc_gref_dtor;
		event->ie_data = grefp;

		JIPC_LOG_VERBOSE("New IpcEvent: obj=%p, event=%p",
				 grefp->jgr_object, event);
	}
	else {
		PFC_ASSERT(event->ie_dtor == jipc_gref_dtor);
	}

	return grefp->jgr_object;
}

/*
 * static jobject
 * jipc_event_error(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
 *		    pfc_ipcevent_t *PFC_RESTRICT event,
 *		    const char *PFC_RESTRICT msg,
 *		    jipc_event_t *PFC_RESTRICT jep)
 *	Logs an error record generated by jipc_event_create().
 *
 * Calling/Exit State:
 *	NULL is always returned.
 */
static jobject
jipc_event_error(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
		 pfc_ipcevent_t *PFC_RESTRICT event,
		 const char *PFC_RESTRICT msg, jipc_event_t *PFC_RESTRICT jep)
{
	jthrowable	t;
	char		str[PFC_HOSTADDR_STRSIZE];

	if (jep != NULL) {
		free(jep);
	}

	if (!jipc_log_enabled) {
		JIPC_EXCEPTION_DESCRIBE(env);
		(*env)->ExceptionClear(env);

		return NULL;
	}

	/*
	 * A local reference to pending exception must be obtained before
	 * JIPC_EXCEPTION_DESCRIBE(env) because it may clear exception.
	 */
	t = (*env)->ExceptionOccurred(env);
	JIPC_EXCEPTION_DESCRIBE(env);

	jipc_hostaddr_tostring(&event->ie_addr, str, sizeof(str));
	pfc_log_error("%s: serial=%u, server=%s@%s, service=%s/%u",
		      msg, IPC_EVENT_SERIAL(event),
		      pfc_refptr_string_value(event->ie_chan), str,
		      IPC_EVENT_SERVICE(event), IPC_EVENT_TYPE(event));

	if (t != NULL) {
		jstring		jstr;
		const char	*cause;

		(*env)->ExceptionClear(env);
		cause = jipc_event_tostring(env, ectx, t, &jstr);
		if (PFC_EXPECT_FALSE(cause == NULL)) {
			cause = jipc_str_unknown;
			jstr = NULL;
		}

		pfc_log_error("Java exception=%s", cause);
		if (jstr != NULL) {
			(*env)->ReleaseStringUTFChars(env, jstr, cause);
			(*env)->DeleteLocalRef(env, jstr);
		}
	}

	(*env)->ExceptionClear(env);
	(*env)->DeleteLocalRef(env, t);

	return NULL;
}

/*
 * static jobject
 * jipc_event_new(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
 *		  pfc_ipcevent_t *PFC_RESTRICT event, pfc_ipcevtype_t type)
 *	Construct a new IpcEvent instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL reference to an IpcEvent
 *	instance is returned.
 *	Otherwise NULL is returned.
 */
static jobject
jipc_event_new(JNIEnv *PFC_RESTRICT env, jipc_evctx_t *PFC_RESTRICT ectx,
	       pfc_ipcevent_t *PFC_RESTRICT event, pfc_ipcevtype_t type)
{
	jclass		cls;
	jmethodID	mid;
	jlong		jevent;
	jint		serial;
	jboolean	up;
	pfc_bool_t	chstate;
	pfc_ipcsess_t	*sess = &event->ie_sess;
	uint32_t	u32;
	int		err;

	chstate = pfc_ipcevent_isstatechange(event);
	jevent = JIPC_EVENT_HANDLE(event);
	serial = (jint)IPC_EVENT_SERIAL(event);
	type = IPC_EVENT_TYPE(event);

	if (!chstate) {
		/* Create an IpcEvent instance. */
		cls = JIPC_EVCTX_CLASS(ectx, IpcEvent);
		mid = JIPC_EVCTX_MID_CTOR(ectx, IpcEvent);
		PFC_ASSERT(cls != NULL);
		PFC_ASSERT(mid != NULL);

		return (*env)->NewObject(env, cls, mid, jevent, serial,
					 (jint)(uint32_t)type,
					 SESS_HANDLE(sess));
	}

	if (type == PFC_IPCCHSTATE_UP) {
		/* Create a ChannelUpEvent instance. */
		cls = JIPC_EVCTX_CLASS(ectx, ChannelUpEvent);
		mid = JIPC_EVCTX_MID_CTOR(ectx, ChannelUpEvent);
		PFC_ASSERT(cls != NULL);
		PFC_ASSERT(mid != NULL);

		return (*env)->NewObject(env, cls, mid, jevent, serial,
					 SESS_HANDLE(sess));
	}

	/* Derive an UINT32 data from this event. */
	err = pfc_ipcclnt_getres_uint32(sess, 0, &u32);
	if (PFC_EXPECT_FALSE(err != 0)) {
		JIPC_LOG_ERROR("Unable to get UINT32 data from channel state "
			       "change event: err=%d", err);

		return NULL;
	}

	if (type == PFC_IPCCHSTATE_DOWN) {
		/* Create a ChannelDownEvent instance. */

		if (PFC_EXPECT_FALSE(u32 < PFC_IPCCHDOWN_REFUSED ||
				     u32 > PFC_IPCCHDOWN_ERROR)) {
			/* This should never happen. */
			JIPC_LOG_WARN("Bogus cause in channel down event: %u",
				      u32);
			u32 = PFC_IPCCHDOWN_ERROR;
		}

		cls = JIPC_EVCTX_CLASS(ectx, ChannelDownEvent);
		mid = JIPC_EVCTX_MID_CTOR(ectx, ChannelDownEvent);
		PFC_ASSERT(cls != NULL);
		PFC_ASSERT(mid != NULL);

		return (*env)->NewObject(env, cls, mid, jevent, serial,
					 SESS_HANDLE(sess), u32);
	}

	if (PFC_EXPECT_FALSE(type != PFC_IPCCHSTATE_NOTIFY)) {
		JIPC_LOG_ERROR("Bogus channel state change event: type=%u",
			       type);

		return NULL;
	}

	/* Create a ChannelStateEvent instance. */
	if (u32 == PFC_IPCCHNOTIFY_DOWN) {
		up = JNI_FALSE;
	}
	else {
		PFC_ASSERT(u32 == PFC_IPCCHNOTIFY_UP);
		up = JNI_TRUE;
	}

	cls = JIPC_EVCTX_CLASS(ectx, ChannelStateEvent);
	mid = JIPC_EVCTX_MID_CTOR(ectx, ChannelStateEvent);
	PFC_ASSERT(cls != NULL);
	PFC_ASSERT(mid != NULL);

	return (*env)->NewObject(env, cls, mid, jevent, serial,
				 SESS_HANDLE(sess), up);
}

/*
 * static void
 * jipc_event_dtor(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp)
 *	Destructor of jipc_event_t.
 *	`grefp' must be a pointer to jev_gref in jipc_gref_t.
 *
 * Remarks:
 *	This function must be called with holding the JNI event context lock
 *	in reader or writer mode.
 */
static void
jipc_event_dtor(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp)
{
	jipc_evctx_t	*ectx;
	jmethodID	mid;
	jobject		eobj;
	jboolean	inval;

	if (PFC_EXPECT_FALSE(env == NULL)) {
		return;
	}

	ectx = &jipc_event_ctx;
	mid = JIPC_EVCTX_MID(ectx, IpcEvent, invalidate);
	eobj = grefp->jgr_object;

	PFC_ASSERT(mid != NULL);

	/* Invalidate IpcEvent instance associated with this event. */
	inval = (*env)->CallBooleanMethod(env, eobj, mid);
	if (PJNI_EXCEPTION_CHECK(env)) {
		/* This should never happen. */
		(*env)->ExceptionClear(env);
		JIPC_LOG_ERROR("Unable to invalidate IpcEvent: obj=%p", eobj);

		return;
	}

	if (PFC_EXPECT_TRUE(inval)) {
		jipc_event_t	*jep = (jipc_event_t *)grefp->jgr_base;

		/*
		 * This IpcEvent instance is no longer used.
		 * So pfc_ipcevent_t can be destroyed here.
		 */
		jipc_event_destroy(jep->jev_event);
	}
	else {
		/* IPC event will be destroyed by IpcEvent.destroy(). */
		JIPC_LOG_VERBOSE("IpcEvent.invalidate() returned false: "
				 "obj=%p, event=%p", eobj,
				 ((jipc_event_t *)grefp->jgr_base)->jev_event);
	}
}

/*
 * static void
 * jipc_evhdlr_dtor(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp)
 *	Destructor of jipc_evhdlr_t.
 *	`grefp' must be a pointer to jeh_gref in jipc_evhdlr_t.
 */
static void
jipc_evhdlr_dtor(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp)
{
	jipc_evhdlr_t	*jhp = (jipc_evhdlr_t *)grefp->jgr_base;
	pfc_refptr_t	*rname = jhp->jeh_name;

	if (rname != NULL) {
		jhp->jeh_name = NULL;
		pfc_refptr_put(rname);
	}
}

/*
 * static void
 * jipc_gref_dtor(pfc_ptr_t arg)
 *	Destructor of jipc_gref_t which keeps a global reference to a Java
 *	object.
 *	`arg' must be a pointer to jipc_gref_t.
 */
static void
jipc_gref_dtor(pfc_ptr_t arg)
{
	jipc_evctx_t	*ectx = &jipc_event_ctx;
	jipc_gref_t	*grefp = (jipc_gref_t *)arg;
	JavaVM		*jvm;

	JIPC_LOG_VERBOSE("jipc_gref_dtor: %p", grefp);

	JIPC_EVCTX_LOCK(ectx);

	/* Determine the Java VM. */
	jvm = ectx->je_jvm;
	if (PFC_EXPECT_TRUE(jvm != NULL)) {
		JNIEnv	*env;

		/*
		 * This method is usually called on the event dispatcher
		 * thread or a Java thread. In other words, the calling thread
		 * should be already attached to the Java VM. But we tries to
		 * attach the calling thread to the Java VM here.
		 * This is just for safe.
		 */
		PJNI_ATTACH(jvm, env);
		jipc_gref_destroy(env, grefp);
		grefp = NULL;
		PJNI_DETACH(jvm);

		if (PFC_EXPECT_FALSE(grefp != NULL)) {
			/* Put this reference into the GC list. */
			JIPC_LOG_WARN("Put a reference to the GC list: "
				      "grefp=%p, gobj=%p", grefp,
				      grefp->jgr_object);
			grefp->jgr_next = ectx->je_grefs;
			ectx->je_grefs = grefp;
		}
	}
	else {
		/*
		 * The Java VM is already unloaded. Only thing to do here is
		 * to free jipc_gref_t buffer.
		 */
		jipc_gref_destroy(NULL, grefp);
	}

	JIPC_EVCTX_UNLOCK(ectx);
}
