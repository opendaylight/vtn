/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - IPC event object management.
 */

#include <pfc/util.h>
#include "ipcclnt_event.h"

/*
 * Event serial number for the next channel state change event.
 */
static pfc_ipcevid_t	ipc_chstate_serial_next = IPC_EVENT_SERIAL_INITIAL;

/*
 * Internal prototypes.
 */
static int	ipc_event_create(pfc_ipcevent_t **PFC_RESTRICT eventp,
				 ipc_elsess_t *PFC_RESTRICT elsp,
				 pfc_refptr_t *PFC_RESTRICT svname);

/*
 * static inline pfc_ipcevid_t PFC_FATTR_ALWAYS_INLINE
 * ipc_event_serial_alloc(void)
 *	Allocate a new IPC event serial ID.
 *
 * Remarks:
 *	We assume that an event object is destroyed in very short term.
 *	So we don't consider serial ID overflow.
 */
static inline pfc_ipcevid_t PFC_FATTR_ALWAYS_INLINE
ipc_event_serial_alloc(void)
{
	uint32_t	*nextp = &ipc_chstate_serial_next;
	pfc_ipcevid_t	id;

	id = pfc_atomic_inc_uint32_old(nextp);
	if (PFC_EXPECT_FALSE(id == PFC_IPC_EVSERIAL_INVALID)) {
		id = pfc_atomic_inc_uint32_old(nextp);
		PFC_ASSERT(id != PFC_IPC_EVSERIAL_INVALID);
	}

	return id;
}

/*
 * pfc_ipcevid_t
 * pfc_ipcevent_getserial(pfc_ipcevent_t *event)
 *	Return serial ID of the specified IPC event.
 */
pfc_ipcevid_t
pfc_ipcevent_getserial(pfc_ipcevent_t *event)
{
	return IPC_EVENT_SERIAL(event);
}

/*
 * pfc_ipcevtype_t
 * pfc_ipcevent_gettype(pfc_ipcevent_t *event)
 *	Return IPC event type of the specified IPC event.
 */
pfc_ipcevtype_t
pfc_ipcevent_gettype(pfc_ipcevent_t *event)
{
	return IPC_EVENT_TYPE(event);
}

/*
 * void
 * pfc_ipcevent_gettime(pfc_ipcevent_t *PFC_RESTRICT event,
 *			pfc_timespec_t *PFC_RESTRICT tsp)
 *	Copy creation time of the specified IPC event to the buffer pointed
 *	by `tsp'.
 */
void
pfc_ipcevent_gettime(pfc_ipcevent_t *PFC_RESTRICT event,
		     pfc_timespec_t *PFC_RESTRICT tsp)
{
	ipc_evproto_t	*iep = &event->ie_proto;

	tsp->tv_sec = iep->iev_time_sec;
	tsp->tv_nsec = iep->iev_time_nsec;
}

/*
 * const char *
 * pfc_ipcevent_getchannelname(pfc_ipcevent_t *event)
 *	Return the name of IPC channel which generated the specified IPC event.
 */
const char *
pfc_ipcevent_getchannelname(pfc_ipcevent_t *event)
{
	return pfc_refptr_string_value(event->ie_chan);
}

/*
 * const pfc_hostaddr_t *
 * pfc_ipcevent_gethostaddr(pfc_ipcevent_t *event)
 *	Return a pointer to pfc_hostaddr_t which represents the IPC server's
 *	host address which generated the specified IPC event.
 */
const pfc_hostaddr_t *
pfc_ipcevent_gethostaddr(pfc_ipcevent_t *event)
{
	return &event->ie_addr;
}

/*
 * const char *
 * pfc_ipcevent_getservicename(pfc_ipcevent_t *event)
 *	Return the name of IPC service associated with the specified IPC event.
 */
const char *
pfc_ipcevent_getservicename(pfc_ipcevent_t *event)
{
	PFC_ASSERT(pfc_refptr_string_length(event->ie_sess.icss_name) ==
		   IPC_EVENT_SERVICE_NAMELEN(event));

	return IPC_EVENT_SERVICE(event);
}

/*
 * pfc_ipcsess_t *
 * pfc_ipcevent_getsess(pfc_ipcevent_t *event)
 *	Return a pointer to pseudo IPC client session which contains additional
 *	data of the specified IPC event.
 *
 *	Additional data of the IPC event can be derived by calling
 *	pfc_ipcclnt_getres_XXX() with specifying the returned session.
 */
pfc_ipcsess_t *
pfc_ipcevent_getsess(pfc_ipcevent_t *event)
{
	return &event->ie_sess;
}

/*
 * pfc_bool_t
 * pfc_ipcevent_isstatechange(pfc_ipcevent_t *event)
 *	Return PFC_TRUE if the specified event is an IPC channel state change
 *	event.
 */
pfc_bool_t
pfc_ipcevent_isstatechange(pfc_ipcevent_t *event)
{
	return (event->ie_sess.icss_name == ipc_svname_chstate)
		? PFC_TRUE : PFC_FALSE;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcevent_create(pfc_ipcevent_t **PFC_RESTRICT eventp,
 *		       ipc_elsess_t *PFC_RESTRICT elsp,
 *		       const char *PFC_RESTRICT svname)
 *	Create a new IPC event object associated with the given event listener
 *	session and the IPC service name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to an IPC event object
 *	is set to the buffer pointed by `eventp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	ie_proto in the event object must be initialized by the caller.
 */
int PFC_ATTR_HIDDEN
pfc_ipcevent_create(pfc_ipcevent_t **PFC_RESTRICT eventp,
		    ipc_elsess_t *PFC_RESTRICT elsp,
		    const char *PFC_RESTRICT svname)
{
	pfc_refptr_t	*rname;
	int		err;

	rname = pfc_refptr_string_create(svname);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		IPCCLNT_LOG_ERROR("%s: Failed to duplicate event service "
				  "name.", IPC_CLCHAN_NAME(elsp->els_chan));

		return ENOMEM;
	}

	err = ipc_event_create(eventp, elsp, rname);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_refptr_put(rname);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcevent_create_chstate(pfc_ipcevent_t **PFC_RESTRICT eventp,
 *			       ipc_elsess_t *PFC_RESTRICT elsp,
 *			       pfc_ipcevtype_t type)
 *	Create a new IPC channel state change event object associated with the
 *	given event listener session.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to an IPC event object
 *	is set to the buffer pointed by `eventp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
pfc_ipcevent_create_chstate(pfc_ipcevent_t **PFC_RESTRICT eventp,
			    ipc_elsess_t *PFC_RESTRICT elsp,
			    pfc_ipcevtype_t type)
{
	pfc_timespec_t	cur;
	pfc_refptr_t	*rname = ipc_svname_chstate;
	ipc_evproto_t	*iep;
	int		err;

	/* Determine creation time of the event. */
	err = pfc_clock_get_realtime(&cur);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("%s: Failed to obtain system realtime "
				  "clock: %s", IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	err = ipc_event_create(eventp, elsp, rname);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	pfc_refptr_get(rname);

	/* Initialize common event data. */
	iep = &(*eventp)->ie_proto;
	iep->iev_type = type;
	iep->iev_namelen = pfc_refptr_string_length(rname);
	iep->iev_resv = 0;
	iep->iev_serial = ipc_event_serial_alloc();
	iep->iev_time_sec = cur.tv_sec;
	iep->iev_time_nsec = cur.tv_nsec;

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcevent_destroy(pfc_ipcevent_t *event)
 *	Destroy the specified IPC event object.
 */
void PFC_ATTR_HIDDEN
pfc_ipcevent_destroy(pfc_ipcevent_t *event)
{
	ipc_eventdtor_t	dtor = event->ie_dtor;

	if (dtor != NULL) {
		pfc_ptr_t	data = event->ie_data;

		/* Rest of work will be done by the destructor. */
		event->ie_dtor = NULL;
		event->ie_data = NULL;
		(*dtor)(data);
	}
	else {
		pfc_ipcevent_destroy_impl(event);
	}
}

/*
 * static int
 * ipc_event_create(pfc_ipcevent_t **PFC_RESTRICT eventp,
 *		    ipc_elsess_t *PFC_RESTRICT elsp,
 *		    pfc_refptr_t *PFC_RESTRICT svname)
 *	Allocate an IPC event object associated with the given event listener
 *	session and the IPC service name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to an IPC event object
 *	is set to the buffer pointed by `eventp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- ie_proto in the event object must be initialized by the caller.
 *
 *	- The caller must increment reference counter of `svname' in advance.
 */
static int
ipc_event_create(pfc_ipcevent_t **PFC_RESTRICT eventp,
		 ipc_elsess_t *PFC_RESTRICT elsp,
		 pfc_refptr_t *PFC_RESTRICT svname)
{
	pfc_ipcevent_t	*event;
	pfc_refptr_t	*chname;
	pfc_ipcsess_t	*sess;
	ipc_clchan_t	*chp = elsp->els_chan;
	int		err;

	event = (pfc_ipcevent_t *)malloc(sizeof(*event));
	if (PFC_EXPECT_FALSE(event == NULL)) {
		IPCCLNT_LOG_ERROR("%s: Failed to allocate an event.",
				  IPC_CLCHAN_NAME(chp));

		return ENOMEM;
	}

	/* Initialize pseudo IPC client session. */
	sess = &event->ie_sess;

	err = PFC_MUTEX_INIT(&sess->icss_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("%s: Failed to initialize mutex: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		free(event);

		return err;
	}

	/* The state of IPC client session must be fixed to "RESULT". */
	sess->icss_state = IPC_SSTATE_RESULT;
	sess->icss_service = 0;
	sess->icss_flags = IPC_CLSSF_FROZEN;
	sess->icss_cflags = IPC_CLSESS_CF_EVENT;
	sess->icss_busy = 0;
	sess->icss_canceller = NULL;
	sess->icss_conn = NULL;
	sess->icss_name = svname;
	sess->icss_timeout.tv_sec = 0;
	sess->icss_timeout.tv_nsec = 0;
	pfc_list_init(&sess->icss_list);
	pfc_ipcmsg_init(&sess->icss_msg, 0);
	pfc_ipcstream_init(&sess->icss_output, 0);

	chname = chp->ichc_name;
	PFC_ASSERT(chname != NULL);
	pfc_refptr_get(chname);
	event->ie_chan = chname;
	event->ie_addr = elsp->els_addr;
	event->ie_refcnt = 1;
	event->ie_dtor = NULL;
	event->ie_data = NULL;

	*eventp = event;

	return 0;
}
