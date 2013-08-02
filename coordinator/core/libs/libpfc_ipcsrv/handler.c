/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * handler.c - IPC server handler management.
 */

#include <pfc/atomic.h>
#include "ipcsrv_impl.h"

/*
 * How long, in seconds, pfc_ipcsrv_remove_handler() should wait for
 * IPC handler to complete.
 */
#define	IPCSRV_HANDLER_WAIT_TIMEOUT	PFC_CONST_U(10)

/*
 * Internal prototypes.
 */
static int	ipcsrv_handler_create(ipc_handler_t **PFC_RESTRICT hdpp,
				      pfc_refptr_t *PFC_RESTRICT rname,
				      uint32_t nservice, pfc_ipchdlr_t handler,
				      pfc_ptr_t arg, pfc_ipcsvdtor_t dtor);
static int	ipcsrv_handler_get(ipc_channel_t *PFC_RESTRICT chp,
				   const char *PFC_RESTRICT name,
				   pfc_ipcid_t service, pfc_bool_t hold,
				   ipc_handler_t **PFC_RESTRICT hdpp);
static void	ipcsrv_handler_wait(ipc_channel_t *PFC_RESTRICT chp,
				    ipc_handler_t *PFC_RESTRICT hdp);
static void	ipcsrv_handler_node_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
static void	ipcsrv_handler_destroy(ipc_handler_t *hdp);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_handler_hold(ipc_handler_t *hdp)
 *	Hold the IPC handler.
 *
 * Remarks:
 *	This function must be called with holding the handler lock, and
 *	the lock is held on return.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_handler_hold(ipc_handler_t *hdp)
{
	PFC_ASSERT(hdp->ihd_refcnt != 0);
	hdp->ihd_refcnt++;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_handler_release(ipc_handler_t *hdp)
 *	Release the IPC handler held by ipcsrv_handler_hold().
 *
 * Remarks:
 *	This function must be called with holding the handler lock, and the
 *	lock is always released on return.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_handler_release(ipc_handler_t *hdp)
{
	uint32_t	ref = hdp->ihd_refcnt;

	if (ref == 1) {
		/*
		 * This is the last reference.
		 * The handler lock is released by ipcsrv_handler_destroy().
		 */
		ipcsrv_handler_destroy(hdp);
	}
	else {
		ref--;
		hdp->ihd_refcnt = ref;
		PFC_ASSERT(ref != 0);

		/* Wake up the waiter if needed. */
		if (hdp->ihd_waiting && ref == 1) {
			IPC_HANDLER_SIGNAL(hdp);
		}

		IPC_HANDLER_UNLOCK(hdp);
	}
}

#ifdef	PFC_VERBOSE_DEBUG

/*
 * Assertion to detect unexpected pthread_exit() in IPC service handler.
 */

static void
ipcsrv_handler_cleanup(void *arg)
{
	pfc_ipcsrv_t	*srv = (pfc_ipcsrv_t *)arg;

	pfc_log_error("Unexpected exit in IPC service: %s/%u",
		      srv->isv_name, srv->isv_service);
	abort();
	/* NOTREACHED */
}

#define	IPCSRV_HANDLER_ASSERT_PUSH(srv)				\
	pthread_cleanup_push(ipcsrv_handler_cleanup, (srv))

#define	IPCSRV_HANDLER_ASSERT_POP()		\
	pthread_cleanup_pop(0)

#else	/* !PFC_VERBOSE_DEBUG */

#define	IPCSRV_HANDLER_ASSERT_PUSH(srv)		((void)0)
#define	IPCSRV_HANDLER_ASSERT_POP()		((void)0)

#endif	/* PFC_VERBOSE_DEBUG */

/*
 * int
 * pfc_ipcsrv_add_handler_impl(pfc_refptr_t *name, uint32_t nservices,
 *			       pfc_ipchdlr_t handler, pfc_ptr_t arg,
 *			       pfc_ipcsvdtor_t dtor)
 *	Register IPC service handler.
 *
 *	`name' must be a refptr string which represents IPC service name.
 *	`nservices' is the number of IPC services provided by the caller.
 *	From zero to (`nservices' - 1) is considered as valid IPC service ID.
 *
 *	`handler' is a pointer to IPC service handler. It will be called
 *	when the IPC client specifies `name' and valid IPC service ID.
 *	`arg' is an arbitrary pointer to be passed to `handler'.
 *
 *	`dtor' is a function pointer to IPC service destructor.
 *	If a non-NULL pointer is specified to `dtor', it will be called when
 *	the IPC service handler is removed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Registered IPC service handler can not be removed until the system
 *	shutdown.
 */
int
pfc_ipcsrv_add_handler_impl(pfc_refptr_t *name, uint32_t nservices,
			    pfc_ipchdlr_t handler, pfc_ptr_t arg,
			    pfc_ipcsvdtor_t dtor)
{
	ipc_channel_t	*chp = &ipc_channel;
	ipc_handler_t	*hdp;
	int		err;

	if (PFC_EXPECT_FALSE(nservices == 0 || handler == NULL)) {
		return EINVAL;
	}

	/* Verify service name. */
	err = pfc_ipc_check_service_name(pfc_refptr_string_value(name));
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Hold the IPC channel. */
	err = pfc_ipcsrv_channel_hold(chp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Allocate a new IPC handler instance. */
	err = ipcsrv_handler_create(&hdp, name, nservices, handler, arg, dtor);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Register IPC service handler. */
	IPCCH_HANDLER_WRLOCK(chp);
	err = pfc_rbtree_put(&chp->ich_handlers, &hdp->ihd_node);
	IPCCH_HANDLER_UNLOCK(chp);

	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT(err == EEXIST);
		IPCSRV_LOG_ERROR("%s: IPC handler is already registered.",
				 pfc_refptr_string_value(name));
		goto error_free;
	}

	pfc_refptr_get(name);
	pfc_ipcsrv_channel_release(chp);

	return 0;

error_free:
	free(hdp);

error:
	pfc_ipcsrv_channel_release(chp);

	return err;
}

/*
 * int
 * pfc_ipcsrv_add_handler(const char *name, uint32_t nservices,
 *			  pfc_ipchdlr_t handler, pfc_ptr_t arg)
 *	Register IPC service handler.
 *
 *	`name' must be a string which represents IPC service name.
 *	`nservices' is the number of IPC services provided by the caller.
 *	From zero to (`nservices' - 1) is considered as valid IPC service ID.
 *
 *	`handler' is a pointer to IPC service handler. It will be called
 *	when the IPC client specifies `name' and valid IPC service ID.
 *	`arg' is an arbitrary pointer to be passed to `handler'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Registered IPC service handler can not be removed until the system
 *	shutdown.
 */
int
pfc_ipcsrv_add_handler(const char *name, uint32_t nservices,
		       pfc_ipchdlr_t handler, pfc_ptr_t arg)
{
	return pfc_ipcsrv_add_handler_dtor(name, nservices, handler, arg,
					   NULL);
}

/*
 * int
 * pfc_ipcsrv_add_handler_dtor(const char *name, uint32_t nservices,
 *			       pfc_ipchdlr_t handler, pfc_ptr_t arg,
 *			       pfc_ipcsvdtor_t dtor)
 *	Register IPC service handler.
 *
 *	`name' must be a string which represents IPC service name.
 *	`nservices' is the number of IPC services provided by the caller.
 *	From zero to (`nservices' - 1) is considered as valid IPC service ID.
 *
 *	`handler' is a pointer to IPC service handler. It will be called
 *	when the IPC client specifies `name' and valid IPC service ID.
 *	`arg' is an arbitrary pointer to be passed to `handler'.
 *
 *	`dtor' is a function pointer to IPC service destructor.
 *	If a non-NULL pointer is specified to `dtor', it will be called when
 *	the IPC service handler is removed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Registered IPC service handler can not be removed until the system
 *	shutdown.
 */
int
pfc_ipcsrv_add_handler_dtor(const char *name, uint32_t nservices,
			    pfc_ipchdlr_t handler, pfc_ptr_t arg,
			    pfc_ipcsvdtor_t dtor)
{
	pfc_refptr_t	*rname;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	rname = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		IPCSRV_LOG_ERROR("No memory for IPC service name: %s",
				 name);

		return ENOMEM;
	}

	err = pfc_ipcsrv_add_handler_impl(rname, nservices, handler, arg,
					  dtor);
	pfc_refptr_put(rname);

	return err;
}

/*
 * void
 * pfc_ipcsrv_remove_handler(const char *name)
 *	Remove IPC service handler associated with the given service name.
 */
void
pfc_ipcsrv_remove_handler(const char *name)
{
	ipc_channel_t	*chp = &ipc_channel;
	pfc_rbnode_t	*node;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		/* Ignore invalid argument. */
		return;
	}

	if (PFC_EXPECT_FALSE(pfc_ipcsrv_channel_hold(chp) != 0)) {
		/* The IPC channel is not ready. */
		return;
	}

	/* Remove the handler from the handler tree in the channel instance. */
	IPCCH_HANDLER_WRLOCK(chp);
	node = pfc_rbtree_remove(&chp->ich_handlers, name);
	IPCCH_HANDLER_UNLOCK(chp);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		ipc_handler_t	*hdp = IPC_HANDLER_NODE2PTR(node);

		IPC_HANDLER_LOCK(hdp);

		/* Wait for the handler to complete. */
		ipcsrv_handler_wait(chp, hdp);

		IPCSRV_LOG_DEBUG("%s: IPC service has been removed.", name);

		/* Release the handler instance and its lock. */
		ipcsrv_handler_release(hdp);
	}

	pfc_ipcsrv_channel_release(chp);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcsrv_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		     pfc_ipcresp_t *PFC_RESTRICT resultp)
 *	Invoke IPC service.
 *
 *	The caller must set IPC service name and ID to srv->isv_name and
 *	srv->isv_service.
 *
 * Calling/Exit State:
 *	Upon successful completion, response from IPC service is set to
 *	`*resultp', and zero is returned.
 *	ENOSYS is returned if the given IPC service is not found.
 *
 * Remarks:
 *	This function assumes that it is always called on the server session
 *	thread. So this function never holds the IPC channel because the
 *	server session thread should call this function with holding the
 *	IPC channel.
 */
int PFC_ATTR_HIDDEN
pfc_ipcsrv_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
		  pfc_ipcresp_t *PFC_RESTRICT resultp)
{
	ipc_channel_t	*chp = srv->isv_channel;
	ipc_handler_t	*hdp;
	const char	*name = srv->isv_name;
	pfc_ipcid_t	service = srv->isv_service;
	pfc_ipcresp_t	result;
	int		err;

	/*
	 * Search for the IPC handler specified by the given name and
	 * service ID.
	 */
	err = ipcsrv_handler_get(chp, name, service, PFC_TRUE, &hdp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (PFC_EXPECT_FALSE(service >= hdp->ihd_nservices)) {
		err = ENOSYS;
		hdp->ihd_invalid++;
	}
	else {
		/* Call IPC service handler. */
		IPC_HANDLER_UNLOCK(hdp);
		IPCSRV_HANDLER_ASSERT_PUSH(srv);
		result = hdp->ihd_handler(srv, service, hdp->ihd_arg);
		IPCSRV_HANDLER_ASSERT_POP();

		*resultp = result;
		IPC_HANDLER_LOCK(hdp);
		if (PFC_EXPECT_FALSE(result == PFC_IPCRESP_FATAL)) {
			hdp->ihd_failed++;
		}
		else {
			hdp->ihd_succeeded++;
		}
		err = 0;
	}

	/* Release the handler instance and its lock. */
	ipcsrv_handler_release(hdp);

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_remove_all(void)
 *	Remove all registered IPC service handlers.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_remove_all(void)
{
	ipc_channel_t	*chp = &ipc_channel;

	IPCCH_HANDLER_WRLOCK(chp);
	pfc_rbtree_clear(&chp->ich_handlers, ipcsrv_handler_node_dtor, NULL);
	IPCCH_HANDLER_UNLOCK(chp);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_inc_resperror(pfc_ipcsrv_t *srv)
 *	Increment sending response error counter of the current IPC service
 *	handler.
 *
 *	The caller must set IPC service name and ID to srv->isv_name and
 *	srv->isv_service.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_inc_resperror(pfc_ipcsrv_t *srv)
{
	ipc_handler_t	*hdp;

	if (ipcsrv_handler_get(srv->isv_channel, srv->isv_name,
			       srv->isv_service, PFC_FALSE, &hdp) == 0) {
		hdp->ihd_resperror++;
		IPC_HANDLER_UNLOCK(hdp);
	}
}

/*
 * static int
 * ipcsrv_handler_create(ipc_handler_t **PFC_RESTRICT hdpp,
 *			 pfc_refptr_t *PFC_RESTRICT rname, uint32_t nservices,
 *			 pfc_ipchdlr_t handler, pfc_ptr_t arg,
 *			 pfc_ipcsvdtor_t dtor)
 *	Create a new IPC service handler instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IPC service handler
 *	instance is set to the buffer pointed by `hdpp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_handler_create(ipc_handler_t **PFC_RESTRICT hdpp,
		      pfc_refptr_t *PFC_RESTRICT rname, uint32_t nservices,
		      pfc_ipchdlr_t handler, pfc_ptr_t arg,
		      pfc_ipcsvdtor_t dtor)
{
	ipc_handler_t	*hdp;
	int		err;

	/* Allocate a new IPC handler instance. */
	hdp = (ipc_handler_t *)malloc(sizeof(*hdp));
	if (PFC_EXPECT_FALSE(hdp == NULL)) {
		IPCSRV_LOG_ERROR("No memory for IPC handler.");
		err = ENOMEM;
		goto error;
	}

	err = PFC_MUTEX_INIT(&hdp->ihd_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error_free;
	}

	err = pfc_cond_init(&hdp->ihd_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error_free;
	}

	hdp->ihd_name = rname;
	hdp->ihd_handler = handler;
	hdp->ihd_nservices = nservices;
	hdp->ihd_arg = arg;
	hdp->ihd_dtor = dtor;
	hdp->ihd_refcnt = 1;
	hdp->ihd_waiting = PFC_FALSE;
	hdp->ihd_succeeded = 0;
	hdp->ihd_failed = 0;
	hdp->ihd_invalid = 0;
	hdp->ihd_resperror = 0;
	*hdpp = hdp;

	return 0;

error_free:
	free(hdp);

error:
	*hdpp = NULL;

	return err;
}

/*
 * static int
 * ipcsrv_handler_get(ipc_channel_t *PFC_RESTRICT chp,
 *		      const char *PFC_RESTRICT name, pfc_ipcid_t service,
 *		      pfc_bool_t hold, ipc_handler_t **PFC_RESTRICT hdpp)
 *	Obtain the IPC handler instance associated with the IPC service
 *	name and ID on the IPC channel specified by `chp'.
 *
 *	`name' and `service' must be the IPC service name and ID respectively.
 *	If PFC_TRUE is passed to `hold', the IPC handler is held by
 *	ipcsrv_handler_hold().
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to the IPC handler is set to
 *	the buffer pointed by `hdpp', and then zero is returned.
 *
 *	ENOSYS is returned if not found.
 *
 * Remarks:
 *	The IPC handler lock is held on successful return.
 *	The caller is responsible for releasing the lock.
 */
static int
ipcsrv_handler_get(ipc_channel_t *PFC_RESTRICT chp,
		   const char *PFC_RESTRICT name, pfc_ipcid_t service,
		   pfc_bool_t hold, ipc_handler_t **PFC_RESTRICT hdpp)
{
	pfc_rbnode_t	*node;
	ipc_handler_t	*hdp;

	IPCCH_HANDLER_RDLOCK(chp);

	node = pfc_rbtree_get(&chp->ich_handlers, name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		IPCCH_HANDLER_UNLOCK(chp);

		return ENOSYS;
	}

	hdp = IPC_HANDLER_NODE2PTR(node);
	IPC_HANDLER_LOCK(hdp);

	if (hold) {
		/*
		 * Hold handler instance before unlocking the handler tree
		 * lock.
		 */
		ipcsrv_handler_hold(hdp);
	}

	IPCCH_HANDLER_UNLOCK(chp);
	*hdpp = hdp;

	return 0;
}

/*
 * static void
 * ipcsrv_handler_wait(ipc_channel_t *PFC_RESTRICT chp,
 *		       ipc_handler_t *PFC_RESTRICT hdp)
 *	Wait for the given IPC handler to complete within
 *	IPCSRV_HANDLER_WAIT_TIMEOUT seconds.
 *
 * Remarks:
 *	This function must be called with holding the handler lock, and
 *	the lock is held on return.
 */
static void
ipcsrv_handler_wait(ipc_channel_t *PFC_RESTRICT chp,
		    ipc_handler_t *PFC_RESTRICT hdp)
{
	pfc_timespec_t	abstime;
	int		err;

	if (hdp->ihd_refcnt == 1) {
		return;
	}

	PFC_ASSERT(hdp->ihd_refcnt != 0);

	err = pfc_clock_gettime(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCSRV_LOG_ERROR("%s: Failed to determine timeout for removal"
				 ": %s", IPC_HANDLER_NAME(hdp), strerror(err));

		return;
	}

	/* Wait for the handler to complete. */
	abstime.tv_sec += IPCSRV_HANDLER_WAIT_TIMEOUT;
	hdp->ihd_waiting = PFC_TRUE;

	do {
		err = IPC_HANDLER_TIMEDWAIT_ABS(hdp, &abstime);
		if (PFC_EXPECT_TRUE(hdp->ihd_refcnt == 1)) {
			hdp->ihd_waiting = PFC_FALSE;
			return;
		}
	} while (err == 0);

	PFC_ASSERT(err == ETIMEDOUT);
	hdp->ihd_waiting = PFC_FALSE;
	IPCSRV_LOG_ERROR("%s: IPC handler did not complete within %u seconds.",
			 IPC_HANDLER_NAME(hdp),
			 IPCSRV_HANDLER_WAIT_TIMEOUT);
}

/*
 * static void
 * ipcsrv_handler_node_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Destructor of ipc_handler_t specified by Red-Black tree node.
 *	`node' must be a pointer to ihd_node in ipc_handler_t.
 */
static void
ipcsrv_handler_node_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	ipc_handler_t	*hdp = IPC_HANDLER_NODE2PTR(node);

	IPC_HANDLER_LOCK(hdp);
	ipcsrv_handler_release(hdp);
}

/*
 * static void
 * ipcsrv_handler_destroy(ipc_handler_t *hdp)
 *	Destroy IPC handler instance.
 *
 * Remarks:
 *	This function must be called with holding the handler lock.
 */
static void
ipcsrv_handler_destroy(ipc_handler_t *hdp)
{
	PFC_ASSERT(hdp->ihd_refcnt == 1);
	PFC_ASSERT(!hdp->ihd_waiting);

	IPC_HANDLER_UNLOCK(hdp);

	PFC_ASSERT_INT(pfc_mutex_destroy(&hdp->ihd_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_destroy(&hdp->ihd_cond), 0);

	if (hdp->ihd_dtor != NULL) {
		/* Call IPC service destructor. */
		hdp->ihd_dtor(IPC_HANDLER_NAME(hdp), hdp->ihd_arg);
	}

	pfc_refptr_put(hdp->ihd_name);
	free(hdp);
}
