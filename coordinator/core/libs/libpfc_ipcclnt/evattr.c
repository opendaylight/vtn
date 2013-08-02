/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * evattr.c - IPC event handler's attributes.
 *
 * Remarks:
 *	All functions defined in this file are not thread safe.
 *	pfc_ipcevattr_t is designed to be instantiated as local variable,
 *	and it must be used by only one thread.
 */

#include "ipcclnt_event.h"

/*
 * Default attributes.
 */
#define	EVATTR_DEF_eva_hostset		NULL
#define	EVATTR_DEF_eva_arg		NULL
#define	EVATTR_DEF_eva_argdtor		NULL
#define	EVATTR_DEF_eva_mask		PFC_IPC_EVENT_MASK_FILL
#define	EVATTR_DEF_eva_priority		PFC_CONST_U(100)
#define	EVATTR_DEF_eva_flags		PFC_CONST_U(0)

#define	EVATTR_DEF(attr, name)				\
	do {						\
		(attr)->name = EVATTR_DEF_##name;	\
	} while (0)

#define	EVATTR_DECL(name)	.name	= EVATTR_DEF_##name

/*
 * Default event attributes.
 */
ipc_evattr_t	ipc_evattr_default PFC_ATTR_HIDDEN = {
	EVATTR_DECL(eva_hostset),
	EVATTR_DECL(eva_arg),
	EVATTR_DECL(eva_argdtor),
	EVATTR_DECL(eva_priority),
	EVATTR_DECL(eva_flags),
};

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcevent_attr_sysinit(void)
 *	Initialize default event attributes object.
 */
void PFC_ATTR_HIDDEN
pfc_ipcevent_attr_sysinit(void)
{
	pfc_ipc_evset_init(&ipc_evattr_default.eva_target);
}

/*
 * int
 * pfc_ipcevent_attr_init(pfc_ipcevattr_t *attr)
 *	Initialize IPC event handler's attributes object.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_init(pfc_ipcevattr_t *attr)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	EVATTR_DEF(eattr, eva_hostset);
	EVATTR_DEF(eattr, eva_arg);
	EVATTR_DEF(eattr, eva_argdtor);
	EVATTR_DEF(eattr, eva_priority);
	EVATTR_DEF(eattr, eva_flags);

	pfc_ipc_evset_init(&eattr->eva_target);

	return 0;
}

/*
 * void
 * pfc_ipcevent_attr_destroy(pfc_ipcevattr_t *attr)
 *	Free up resources held by the specified event handler's attribute
 *	object.
 *
 *	This function does nothing if NULL is specified.
 */
void
pfc_ipcevent_attr_destroy(pfc_ipcevattr_t *attr)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_TRUE(eattr != NULL)) {
		ipc_hostset_t	*hset = eattr->eva_hostset;

		if (hset != NULL) {
			eattr->eva_hostset = NULL;
			IPC_HOSTSET_RELEASE(hset);
		}

		pfc_ipc_evset_destroy(&eattr->eva_target);
	}
}

/*
 * int
 * pfc_ipcevent_attr_gethostset(const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *				const char **PFC_RESTRICT namep)
 *	Get the name of IPC host set in the specified event attributes object.
 *
 * Calling/Exit State:
 *	Upon successful completion, the name of IPC host set is set to the
 *	buffer pointed by `name', and zero is returned. NULL is set to `namep'
 *	if no IPC host set is set in the event attributes.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_gethostset(const pfc_ipcevattr_t *PFC_RESTRICT attr,
			     const char **PFC_RESTRICT namep)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);
	ipc_hostset_t	*hset;

	if (PFC_EXPECT_FALSE(eattr == NULL || namep == NULL)) {
		return EINVAL;
	}

	hset = eattr->eva_hostset;
	if (hset == NULL) {
		*namep = NULL;
	}
	else {
		*namep = pfc_refptr_string_value(hset->hs_name);
	}

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_sethostset(pfc_ipcevattr_t *PFC_RESTRICT attr,
 *				const char *PFC_RESTRICT name)
 *	Set the name of IPC host set into the specified event attributes
 *	object.
 *
 *	Events from hosts in the specified host set will be received by
 *	event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function never ensure that the IPC host set associated with
 *	the specified name actually exists.
 */
int
pfc_ipcevent_attr_sethostset(pfc_ipcevattr_t *PFC_RESTRICT attr,
			     const char *PFC_RESTRICT name)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);
	ipc_hostset_t	*old;
	int		err;

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	old = eattr->eva_hostset;
	if (name == NULL) {
		/* Reset to default value. */
		EVATTR_DEF(eattr, eva_hostset);
		err = 0;
	}
	else {
		ipc_hostset_t	*hset;

		/* Determine new host set associated with the given name. */
		IPC_CLIENT_RDLOCK();
		err = pfc_ipcclnt_hostset_lookup(name, &hset);
		if (PFC_EXPECT_TRUE(err == 0) && hset != old) {
			IPC_HOSTSET_HOLD(hset);
			eattr->eva_hostset = hset;
		}
		else {
			old = NULL;
		}
		IPC_CLIENT_UNLOCK();
	}

	if (old != NULL) {
		IPC_HOSTSET_RELEASE(old);
	}

	return err;
}

/*
 * int
 * pfc_ipcevent_attr_gettarget(const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *			       const char *PFC_RESTRICT service,
 *			       pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Get target event mask bits.
 *
 *	This function searches for the target IPC event mask bits associated
 *	with the IPC service name `service' in the specified event attributes
 *	object.
 *
 *	If `service' is NULL, this function searches for the event mask bits
 *	associated with IPC channel state change event.
 *
 * Calling/Exit State:
 *	Upon successful completion, event mask bits associated with the
 *	specified IPC service name is set to the buffer pointed by `mask',
 *	and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_gettarget(const pfc_ipcevattr_t *PFC_RESTRICT attr,
			    const char *PFC_RESTRICT service,
			    pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);
	ipc_evmask_t	*emp;

	if (PFC_EXPECT_FALSE(eattr == NULL || mask == NULL)) {
		return EINVAL;
	}

	if (service == NULL) {
		service = ipc_str_svname_chstate;
	}

	if (pfc_ipc_evset_isempty(&eattr->eva_target)) {
		/* All events are targeted. */
		*mask = PFC_IPC_EVENT_MASK_FILL;
	}
	else {
		emp = pfc_ipc_evset_lookup(&eattr->eva_target, service,
					   IPC_EVTYPE_NONE);
		if (emp == NULL) {
			/* The specified IPC service is not targeted. */
			*mask = PFC_IPC_EVENT_MASK_EMPTY;
		}
		else {
			PFC_ASSERT(emp->iem_mask != 0);
			*mask = emp->iem_mask;
		}
	}

	return 0;
}
/*
 * int
 * pfc_ipcevent_attr_addtarget(pfc_ipcevattr_t *PFC_RESTRICT attr,
 *			       const char *PFC_RESTRICT service,
 *			       const pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Add the target event mask specified by `mask' to the event mask
 *	associated with the IPC service name specified by `service'.
 *
 *	If the service name specified by `service' already exists, the event
 *	mask specified by `mask' is merged to existing event mask.
 *
 *	Specifying NULL to `mask' means all events generated by `service'
 *	should be targeted.
 *
 *	If `service' is NULL, this function adds `mask' to the event mask
 *	associated with IPC channel state change event.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Specifying empty mask to `mask' causes EINVAL error.
 */
int
pfc_ipcevent_attr_addtarget(pfc_ipcevattr_t *PFC_RESTRICT attr,
			    const char *PFC_RESTRICT service,
			    const pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);
	int		err;

	if (PFC_EXPECT_FALSE(eattr == NULL || (mask != NULL && *mask == 0))) {
		return EINVAL;
	}

	if (service == NULL) {
		service = ipc_str_svname_chstate;
	}
	else {
		err = pfc_ipc_check_service_name(service);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return pfc_ipc_evset_add(&eattr->eva_target, service, mask);
}

/*
 * int
 * pfc_ipcevent_attr_resettarget(pfc_ipcevattr_t *attr)
 *	Reset target IPC event mask to initial state.
 *
 *	This function can be used to target all IPC events, including IPC
 *	channel state change events.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_resettarget(pfc_ipcevattr_t *attr)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	pfc_ipc_evset_destroy(&eattr->eva_target);

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_getpriority(const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *				 uint32_t *PFC_RESTRICT prip);
 *	Get a priority value for event handler in the specified IPC event
 *	attributes object.
 *
 * Calling/Exit State:
 *	Upon successful completion, a priority value is set to the buffer
 *	pointed by `prip', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_getpriority(const pfc_ipcevattr_t *PFC_RESTRICT attr,
			      uint32_t *PFC_RESTRICT prip)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL || prip == NULL)) {
		return EINVAL;
	}

	*prip = eattr->eva_priority;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_setpriority(pfc_ipcevattr_t *attr, uint32_t pri)
 *	Set a priority value for event handler.
 *	Event handlers are called in ascending order of priority value.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_setpriority(pfc_ipcevattr_t *attr, uint32_t pri)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	eattr->eva_priority = pri;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_getarg(const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *			    pfc_ptr_t *PFC_RESTRICT argp)
 *	Get an event handler's argument in the specified attributes object.
 *
 * Calling/Exit State:
 *	Upon successful completion, an event handler's argument is set to
 *	the buffer pointed by `argp', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_getarg(const pfc_ipcevattr_t *PFC_RESTRICT attr,
			 pfc_ptr_t *PFC_RESTRICT argp)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL || argp == NULL)) {
		return EINVAL;
	}

	*argp = eattr->eva_arg;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_setarg(pfc_ipcevattr_t *PFC_RESTRICT attr, pfc_ptr_t arg)
 *	Set an arbitrary data to be passed to the event handler call.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_setarg(pfc_ipcevattr_t *PFC_RESTRICT attr, pfc_ptr_t arg)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	eattr->eva_arg = arg;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_getargdtor(const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *				pfc_ipcevdtor_t *PFC_RESTRICT dtorp)
 *	Get a pointer to event handler's argument destructor function in the
 *	specified event attributes object.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to event handler's argument
 *	destructor is set to the buffer pointed by `dtorp'. NULL is set if
 *	no destructor is set.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_getargdtor(const pfc_ipcevattr_t *PFC_RESTRICT attr,
			     pfc_ipcevdtor_t *PFC_RESTRICT dtorp)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL || dtorp == NULL)) {
		return EINVAL;
	}

	*dtorp = eattr->eva_argdtor;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_setargdtor(pfc_ipcevattr_t *PFC_RESTRICT attr,
 *				pfc_ipcevdtor_t dtor)
 *	Set an argument destructor function to the event attributes object.
 *	The specified function will be called when the event handler is
 *	removed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_setargdtor(pfc_ipcevattr_t *PFC_RESTRICT attr,
			     pfc_ipcevdtor_t dtor)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	eattr->eva_argdtor = dtor;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_getlog(const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *			    pfc_bool_t *PFC_RESTRICT logp)
 *	Get a delivery logging flag in the specified IPC event attributes
 *	object.
 *
 * Calling/Exit State:
 *	Upon successful completion, a delivery logging flag in the event
 *	attributes object is set to the buffer pointed by `logp', and zero is
 *	returned. PFC_TRUE means delivery logging is enabled, and PFC_FALSE
 *	means disabled.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_getlog(const pfc_ipcevattr_t *PFC_RESTRICT attr,
			 pfc_bool_t *PFC_RESTRICT logp)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL || logp == NULL)) {
		return EINVAL;
	}

	*logp = (eattr->eva_flags & IPC_EVATTRF_LOG) ? PFC_TRUE : PFC_FALSE;

	return 0;
}

/*
 * int
 * pfc_ipcevent_attr_setlog(pfc_ipcevattr_t *PFC_RESTRICT attr, pfc_bool_t log)
 *	Set a delivery logging flag to the specified event attributes object.
 *
 *	If `log' is PFC_TRUE, delivery logging is enabled. If PFC_FALSE,
 *	delivery logging is disabled.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_attr_setlog(pfc_ipcevattr_t *PFC_RESTRICT attr, pfc_bool_t log)
{
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);

	if (PFC_EXPECT_FALSE(eattr == NULL)) {
		return EINVAL;
	}

	if (log) {
		eattr->eva_flags |= IPC_EVATTRF_LOG;
	}
	else {
		eattr->eva_flags &= ~IPC_EVATTRF_LOG;
	}

	return 0;
}
