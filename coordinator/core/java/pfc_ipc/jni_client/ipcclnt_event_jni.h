/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPCCLNT_EVENT_JNI_H
#define	_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPCCLNT_EVENT_JNI_H

/*
 * Internal IPC client event definitions for JNI library.
 */

#include "ipcclnt_jni.h"

/*
 * Fully qualified Java class names.
 */
#define	PJNI_CLASS_IpcEventSystemNotReadyException			\
	"org/opendaylight/vtn/core/ipc/IpcEventSystemNotReadyException"
#define	PJNI_CLASS_IpcNoSuchHostSetException			\
	"org/opendaylight/vtn/core/ipc/IpcNoSuchHostSetException"
#define	PJNI_CLASS_IpcNoSuchEventHandlerException			\
	"org/opendaylight/vtn/core/ipc/IpcNoSuchEventHandlerException"

#define	PJNI_CLASS_IpcEvent			\
	"org/opendaylight/vtn/core/ipc/IpcEvent"
#define	PJNI_CLASS_ChannelUpEvent			\
	"org/opendaylight/vtn/core/ipc/ChannelUpEvent"
#define	PJNI_CLASS_ChannelDownEvent			\
	"org/opendaylight/vtn/core/ipc/ChannelDownEvent"
#define	PJNI_CLASS_ChannelStateEvent			\
	"org/opendaylight/vtn/core/ipc/ChannelStateEvent"
#define	PJNI_CLASS_IpcEventHandler			\
	"org/opendaylight/vtn/core/ipc/IpcEventHandler"
#define	PJNI_CLASS_IpcEventConfiguration			\
	"org/opendaylight/vtn/core/ipc/IpcEventConfiguration"

#define	PJNI_SIG_IpcEvent	PJNI_SIG_CLASS(IpcEvent)

/*
 * JNI signature of IpcEvent(long, int, int, long)
 */
#define	JIPC_CTORSIG_IpcEvent				\
	PJNI_SIG_METHOD4(void, long, int, int, long)

/*
 * JNI signature of IpcEvent.invalidate()
 */
#define	JIPC_SIG_IpcEvent_invalidate	PJNI_SIG_METHOD0(boolean)

/*
 * JNI signature of ChannelUpEvent(long, int, long)
 */
#define	JIPC_CTORSIG_ChannelUpEvent		\
	PJNI_SIG_METHOD3(void, long, int, long)

/*
 * JNI signature of ChannelDownEvent(long, int, long, int)
 */
#define	JIPC_CTORSIG_ChannelDownEvent			\
	PJNI_SIG_METHOD4(void, long, int, long, int)

/*
 * JNI signature of ChannelStateEvent(long, int, long, boolean)
 */
#define	JIPC_CTORSIG_ChannelStateEvent			\
	PJNI_SIG_METHOD4(void, long, int, long, boolean)

/*
 * JNI signature of IpcEventHandler.eventReceived(IpcEvent)
 */
#define	JIPC_SIG_IpcEventHandler_eventReceived	\
	PJNI_SIG_METHOD1(void, IpcEvent)

/*
 * Pointer cast macros.
 */
#define	JIPC_EVATTR_PTR(handle)		((pfc_ipcevattr_t *)(uintptr_t)(handle))
#define	JIPC_EVATTR_HANDLE(attr)	((jlong)(uint64_t)(uintptr_t)(attr))
#define	JIPC_EVENT_PTR(handle)		((pfc_ipcevent_t *)(uintptr_t)(handle))
#define	JIPC_EVENT_HANDLE(event)	((jlong)(uint64_t)(uintptr_t)(event))

struct jipc_gref;
typedef struct jipc_gref	jipc_gref_t;

/*
 * Destructor of jipc_gref_t.
 * This function is called with holding the JNI event context lock in
 * reader or writer mode.
 */
typedef void	(*jipc_grefdtor_t)(JNIEnv *PFC_RESTRICT env,
				   jipc_gref_t *PFC_RESTRICT grefp);

/*
 * Common data which keeps a global reference.
 */
struct jipc_gref {
	jobject		jgr_object;	/* global reference */
	void		*jgr_base;	/* base address of this buffer */
	jipc_grefdtor_t	jgr_dtor;	/* destructor */
	jipc_gref_t	*jgr_next;	/* next link */
};

/*
 * Internal prototypes.
 */
extern void	jipc_event_clear(JavaVM *jvm);

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * jipc_gref_init(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp,
 *		  pfc_ptr_t base, jobject obj, jipc_grefdtor_t dtor)
 *	Initialize jipc_gref_t.
 *
 *	`grefp' must be a pointer to jipc_gref_t, and `base' must be a pointer
 *	to base address of the buffer which contains `grefp'. `base' will be
 *	passed to free(3) when the jipc_gref_t buffer is destroyed.
 *
 *	`obj' must be a local reference to a Java object.
 *	This function creates a global reference to it and stores a global
 *	reference to grefp->jgr_object.
 *
 *	`dtor' will be called just before a global reference to `obj' is
 *	deleted.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned.
 *	Otherwise PFC_FALSE is returned with throwing an exception.
 *
 * Remarks:
 *	Note that a local reference specified by `obj' is always deleted.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
jipc_gref_init(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp,
	       pfc_ptr_t base, jobject obj, jipc_grefdtor_t dtor)
{
	jobject	gobj;

	gobj = (*env)->NewGlobalRef(env, obj);
	(*env)->DeleteLocalRef(env, obj);
	if (PFC_EXPECT_FALSE(gobj == NULL)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to create a global reference.");

		return PFC_FALSE;
	}

	grefp->jgr_object = gobj;
	grefp->jgr_dtor = dtor;
	grefp->jgr_base = base;
	grefp->jgr_next = NULL;

	JIPC_LOG_VERBOSE("New global reference: grefp=%p, gobj=%p",
			 grefp, gobj);

	return PFC_TRUE;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * jipc_gref_destroy(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp)
 *	Destroy the specified jipc_gref_t.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
jipc_gref_destroy(JNIEnv *PFC_RESTRICT env, jipc_gref_t *PFC_RESTRICT grefp)
{
	/* Call destructor. */
	grefp->jgr_dtor(env, grefp);

	if (PFC_EXPECT_TRUE(env != NULL)) {
		jobject	gobj = grefp->jgr_object;

		JIPC_LOG_VERBOSE("Delete global reference: grefp=%p, gobj=%p",
				 grefp, gobj);

		/* Delete a global reference. */
		(*env)->DeleteGlobalRef(env, gobj);
	}

	/* Free the buffer. */
	free(grefp->jgr_base);
}

#endif	/* !_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPCCLNT_EVENT_JNI_H */
