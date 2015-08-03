/*
 * Copyright (c) 2011-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * connection.c - IPC connection handle management.
 */

#include <unistd.h>
#include <pfc/util.h>
#include <iostream_impl.h>
#include "ipcclnt_impl.h"

/*
 * Determine whether the default connection is initialized or not.
 * `cnp' must be a pointer to ipcconn_default.
 */
#define	IPC_CONN_DEFAULT_IS_VALID(cnp)	((cnp)->icn_id == PFC_IPCCONN_DEFAULT)

/*
 * Ping request timeout in seconds.
 */
#define	IPC_CONN_PING_TIMEOUT		PFC_CONST_U(3)

/*
 * Base value of reference counter for the default connection.
 */
#define	IPC_CONN_DEFAULT_REFCNT		PFC_CONST_U(2)

/*
 * Default connection handle.
 */
static ipc_conn_t	ipcconn_default = {
	.icn_id		= PFC_IPCCONN_INVALID,
	.icn_sessions	= PFC_LIST_INITIALIZER(ipcconn_default.icn_sessions),
	.icn_plink	= PFC_LIST_INITIALIZER(ipcconn_default.icn_plink),
};

/*
 * The minimum connection ID for dynamic allocation.
 */
#define	IPC_CONN_ID_MIN		(PFC_IPCCONN_DEFAULT + 1)

/*
 * Global client lock.
 */
pfc_rwlock_t	ipc_client_lock PFC_ATTR_HIDDEN = PFC_RWLOCK_INITIALIZER;

/*
 * Connection ID for the next allocation.
 */
static pfc_ipcconn_t	ipcconn_next_id = IPC_CONN_ID_MIN;

/*
 * The minimum connection pool ID for dynamic allocation.
 */
#define	IPC_CPOOL_ID_MIN	(PFC_IPCCPOOL_GLOBAL + 1)

/*
 * Connection pool ID for the next allocation.
 */
static pfc_ipcconn_t	ipc_cpool_next_id = IPC_CPOOL_ID_MIN;

/*
 * Default capacity of connection pool.
 */
#define	IPC_CPOOL_CAPACITY_DEFAULT	PFC_CONST_U(256)

/*
 * Release default connection handle.
 * This macro is needed to suppress compiler warning.
 */
#define	IPC_CONN_RELEASE_DEFAULT(cnp)					\
	do {								\
		PFC_ASSERT((cnp) == &ipcconn_default);			\
		pfc_atomic_dec_uint32_old(&(cnp)->icn_refcnt);		\
		PFC_ASSERT((cnp)->icn_refcnt >= IPC_CONN_DEFAULT_REFCNT); \
	} while (0)

/*
 * List of dead connections.
 * This list must be synchronized with the client lock.
 */
static pfc_list_t	ipc_conn_dead = PFC_LIST_INITIALIZER(ipc_conn_dead);

/*
 * Internal prototypes.
 */
static pfc_cptr_t	ipcconn_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_cpool_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_cpoolent_getkey(pfc_rbnode_t *node);

static int	ipcconn_register(ipc_conn_t *PFC_RESTRICT cnp,
				 const ipc_chaddr_t *PFC_RESTRICT cap);
static int	ipcconn_init(ipc_conn_t *PFC_RESTRICT cnp,
			     const ipc_chaddr_t *PFC_RESTRICT cap);
static int	ipcconn_init_default(ipc_conn_t *cnp);
static void	ipcconn_altclose(ipc_conn_t *cnp);
static void	ipcconn_unlink(pfc_rbtree_t *PFC_RESTRICT tree,
			       ipc_conn_t *PFC_RESTRICT cnp);
static int	ipcconn_setup(ipc_conn_t *PFC_RESTRICT cnp,
			      pfc_ipcsess_t *PFC_RESTRICT sess,
			      ipc_canceller_t *PFC_RESTRICT clp,
			      ctimespec_t *PFC_RESTRICT abstime);
static int	ipcconn_wait(ipc_conn_t *PFC_RESTRICT cnp,
			     ipc_canceller_t *PFC_RESTRICT clp,
			     ctimespec_t *PFC_RESTRICT abstime);
static int	ipcconn_invoke(ipc_conn_t *PFC_RESTRICT cnp,
			       pfc_ipcsess_t *PFC_RESTRICT sess,
			       pfc_ipcresp_t *PFC_RESTRICT respp,
			       ctimespec_t *PFC_RESTRICT abstime);
static int	ipcconn_handshake(ipc_sess_t *PFC_RESTRICT isp,
				  ipc_clchan_t *PFC_RESTRICT chp,
				  ctimespec_t *PFC_RESTRICT abstime);
static void	ipcconn_fork_prepare(ipc_conn_t *cnp, pfc_ptr_t arg);
static void	ipcconn_fork_parent(ipc_conn_t *cnp, pfc_ptr_t arg);
static void	ipcconn_fork_child(ipc_conn_t *cnp, pfc_ptr_t arg);

static int	ipc_cpool_lookup(pfc_ipccpool_t pool, ipc_cpool_t **cplpp);
static int	ipc_cpool_getconn(pfc_ipccpool_t pool,
				  const ipc_chaddr_t *PFC_RESTRICT cap,
				  ipc_cpool_t **PFC_RESTRICT cplpp,
				  pfc_ipcconn_t *PFC_RESTRICT connp);
static void	ipc_cpool_remove(ipc_cpool_t *PFC_RESTRICT cplp,
				 ipc_cpoolent_t *PFC_RESTRICT cpent,
				 ipc_conn_t *PFC_RESTRICT cnp);
static void	ipc_cpool_iterate(ipc_cpooliter_t iter, pfc_ptr_t arg);
static void	ipc_cpool_reap(ipc_cpool_t *cplp, pfc_ptr_t arg);
static void	ipc_cpool_reap_forced(ipc_cpool_t *cplp, pfc_ptr_t arg);

static pfc_bool_t	ipc_cpool_checksize(ipc_cpool_t *PFC_RESTRICT cplp,
					    ipc_conn_t **PFC_RESTRICT idlep);
static ipc_cpoolent_t	*ipc_cpool_getidle(ipc_cpool_t *cplp);
static ipc_conn_t	*ipc_cpool_purgeidle(ipc_cpool_t *cplp);

/*
 * Red-Black tree which keeps connection handles created by
 * pfc_ipcclnt_altopen().
 */
static pfc_rbtree_t	ipcconn_alttree =
	PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare, ipcconn_getkey);

/*
 * Red-Black tree which keeps alternative connection pool.
 */
static pfc_rbtree_t	ipcconn_pool =
	PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare, ipc_cpool_getkey);

/*
 * Global connection pool, associated with PFC_IPCCPOOL_GLOBAL.
 */
#define	IPCCONN_POOL_INITIALIZER					\
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)pfc_ipcclnt_chaddr_compare,\
			       ipc_cpoolent_getkey)

static ipc_cpool_t	ipc_cpool_global = {
	.icp_id		= PFC_IPCCPOOL_GLOBAL,
	.icp_capacity	= IPC_CPOOL_CAPACITY_DEFAULT,
	.icp_mutex	= PFC_MUTEX_INITIALIZER,
	.icp_pool	= IPCCONN_POOL_INITIALIZER,
	.icp_lrulist	= PFC_LIST_INITIALIZER(ipc_cpool_global.icp_lrulist),
	.icp_uncached	= PFC_LIST_INITIALIZER(ipc_cpool_global.icp_uncached),
};

/*
 * Determine whether a pair of pool handle and instance is valid.
 */
#define	IPC_CPOOL_IS_VALID_DYNAMIC(pool, cplp)				\
	(pfc_rbtree_get(&ipcconn_pool, IPC_CPOOL_KEY(pool)) ==		\
	 &(cplp)->icp_node)

#define	IPC_CPOOL_IS_VALID(pool, cplp)			\
	(((pool) == PFC_IPCCPOOL_GLOBAL)		\
	 ? ((cplp) == &ipc_cpool_global)		\
	 : IPC_CPOOL_IS_VALID_DYNAMIC(pool, cplp))

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_cpool_unlink(ipc_cpool_t *PFC_RESTRICT cplp,
 *		    ipc_conn_t *PFC_RESTRICT cnp)
 *	Unlink the connection specified by `cnp' from the connection list of
 *	the connection pool specified by `cplp'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_cpool_unlink(ipc_cpool_t *PFC_RESTRICT cplp, ipc_conn_t *PFC_RESTRICT cnp)
{
	PFC_ASSERT(!pfc_list_is_empty(&cnp->icn_plink));
	PFC_ASSERT(cnp->icn_pool == cplp);

	pfc_list_remove(&cnp->icn_plink);
#ifdef	PFC_VERBOSE_DEBUG
	pfc_list_init(&cnp->icn_plink);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_cpool_update_lru(ipc_cpool_t *PFC_RESTRICT cplp,
 *			ipc_cpoolent_t *PFC_RESTRICT cpent,
 *			ipc_conn_t *PFC_RESTRICT cnp)
 *	Update the LRU connection list for the specified pool.
 *	This function is called when the IPC client uses the connection
 *	specified by `cnp'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_cpool_update_lru(ipc_cpool_t *PFC_RESTRICT cplp,
		     ipc_cpoolent_t *PFC_RESTRICT cpent,
		     ipc_conn_t *PFC_RESTRICT cnp)
{
	pfc_list_t	*head;
	pfc_list_t	*plink;

	if (cpent == NULL) {
		/* This connection is not cached in the connection pool. */
		return;
	}

	head = &cplp->icp_lrulist;
	plink = &cnp->icn_plink;

	/*
	 * The connection pool lock must be held here because this function
	 * may be called with holding the client lock in reader mode.
	 */
	IPC_CPOOL_LOCK(cplp);

	PFC_ASSERT(cnp != &ipcconn_default);
	PFC_ASSERT(cnp->icn_pool == cplp);
	PFC_ASSERT(!pfc_list_is_empty(plink));
	if (plink->pl_next != head) {
		/* Move this connection to the tail of the LRU list. */
		pfc_list_remove(plink);
		pfc_list_push_tail(head, plink);
	}

	/* Clear aged flag. */
	PFC_ASSERT(cpent->icpe_conn == cnp);
	cpent->icpe_flags &= ~IPC_CPENTF_AGED;

	IPC_CPOOL_UNLOCK(cplp);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcconn_altclose_list(pfc_list_t *head)
 *	Close all connection handles on the list pointed by `head'.
 *
 * Remarks:
 *	This function must be called without holding the client lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcconn_altclose_list(pfc_list_t *head)
{
	pfc_list_t	*elem, *next;

	PFC_LIST_FOREACH_SAFE(head, elem, next) {
		ipc_conn_t	*cnp = IPC_CONN_PLINK2PTR(elem);

#ifdef	PFC_VERBOSE_DEBUG
		pfc_list_init(&cnp->icn_plink);
#endif	/* PFC_VERBOSE_DEBUG */
		ipcconn_altclose(cnp);
	}
}

/*
 * int
 * pfc_ipcclnt_altopen(const char *PFC_RESTRICT name,
 *		       pfc_ipcconn_t *PFC_RESTRICT connp)
 *	Create an alternative IPC connection handle.
 *
 *	If you want to issue IPC service call in parallel, you must create
 *	an alternative IPC connection handle by this function.
 *
 *	`name' is a pointer to the IPC channel address to be connected.
 *
 *	- If `name' is NULL, the IPC channel address of the default connection
 *	  is used.
 *	- If `name' is an empty string, the IPC channel name of the default
 *	  connection and local host address is used as the IPC channel address.
 *	- If IPC channel name part of the IPC channel address is empty,
 *	  the IPC channel name of the default connection is used as the IPC
 *	  channel name.
 *
 * Calling/Exit State:
 *	Upon successful completion, connection handle is set to `*connp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_altopen(const char *PFC_RESTRICT name,
		    pfc_ipcconn_t *PFC_RESTRICT connp)
{
	ipc_conn_t	*cnp;
	ipc_chaddr_t	chaddr;
	int		err;

	if (PFC_EXPECT_FALSE(connp == NULL)) {
		return EINVAL;
	}

	/* Allocate a new connection handle. */
	cnp = (ipc_conn_t *)malloc(sizeof(*cnp));
	if (PFC_EXPECT_TRUE(cnp == NULL)) {
		return ENOMEM;
	}

	IPC_CLIENT_WRLOCK();

	/* Parse IPC channel address. */
	err = pfc_ipcclnt_chaddr_parse(name, &chaddr, PFC_TRUE);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Register this handle. */
		err = ipcconn_register(cnp, &chaddr);
	}

	IPC_CLIENT_UNLOCK();

	if (PFC_EXPECT_FALSE(err != 0)) {
		free(cnp);

		return err;
	}

	*connp = cnp->icn_id;

	return 0;
}

/*
 * int
 * pfc_ipcclnt_altclose(pfc_ipcconn_t conn)
 *	Close the IPC connection handle opened by pfc_ipcclnt_altopen().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_altclose(pfc_ipcconn_t conn)
{
	pfc_rbtree_t	*alttree = &ipcconn_alttree;
	pfc_rbnode_t	*node;
	ipc_conn_t	*cnp;

	IPC_CLIENT_WRLOCK();

	/* Search for the connection associated with the given handle. */
	node = pfc_rbtree_get(alttree, IPC_CONN_KEY(conn));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		IPC_CLIENT_UNLOCK();

		return EBADF;
	}

	cnp = IPC_CONN_NODE2PTR(node);

	if (PFC_EXPECT_FALSE(cnp->icn_pool != NULL)) {
		/* Cached connection can not be removed. */
		IPC_CLIENT_UNLOCK();

		return EPERM;
	}

	/* Remove the connection handle from the connection tree. */
	ipcconn_unlink(alttree, cnp);
	IPC_CLIENT_UNLOCK();

	/* Close the connection handle. */
	ipcconn_altclose(cnp);

	return 0;
}

/*
 * int
 * pfc_ipcclnt_cpool_create(pfc_ipccpool_t *poolp, uint32_t capacity)
 *	Create a new alternative connection pool.
 *
 *	`capacity' specifies the maximum number of connections in the pool.
 *	If `capacity' is zero, default capacity is used.
 *
 * Calling/Exit State:
 *	Upon successful completion, connection pool handle is set to the buffer
 *	pointed by `poolp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_cpool_create(pfc_ipccpool_t *poolp, uint32_t capacity)
{
	ipc_cpool_t	*cplp;
	pfc_ipccpool_t	first;
	int		err;

	if (PFC_EXPECT_FALSE(poolp == NULL)) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(capacity > PFC_IPCCPOOL_MAX_CAPACITY)) {
		return EINVAL;
	}

	/* Allocate a new connection pool. */
	cplp = (ipc_cpool_t *)malloc(sizeof(*cplp));
	if (PFC_EXPECT_FALSE(cplp == NULL)) {
		return ENOMEM;
	}

	/* Initialize connection pool. */
	err = PFC_MUTEX_INIT(&cplp->icp_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		free(cplp);

		return err;
	}

	pfc_rbtree_init(&cplp->icp_pool,
			(pfc_rbcomp_t)pfc_ipcclnt_chaddr_compare,
			ipc_cpoolent_getkey);
	pfc_list_init(&cplp->icp_lrulist);
	pfc_list_init(&cplp->icp_uncached);
	if (capacity == 0) {
		capacity = IPC_CPOOL_CAPACITY_DEFAULT;
	}
	cplp->icp_capacity = capacity;
	cplp->icp_size = 0;

	IPC_CLIENT_WRLOCK();

	/* Assign new pool identifier.*/
	cplp->icp_id = first = ipc_cpool_next_id;

	for (;;) {
		pfc_ipccpool_t	id;

		/* Register this pool to the connection pool tree. */
		err = pfc_rbtree_put(&ipcconn_pool, &cplp->icp_node);

		/* Prepare ID for the next allocation. */
		id = cplp->icp_id + 1;
		if (PFC_EXPECT_FALSE(id < IPC_CPOOL_ID_MIN)) {
			id = IPC_CPOOL_ID_MIN;
		}

		/* Check result. */
		if (PFC_EXPECT_TRUE(err == 0)) {
			ipc_cpool_next_id = id;
			break;
		}

		/* Try another ID. */
		if (PFC_EXPECT_FALSE(id == first)) {
			IPC_CLIENT_UNLOCK();
			free(cplp);

			return ENFILE;
		}

		cplp->icp_id = id;
	}

	*poolp = cplp->icp_id;
	IPC_CLIENT_UNLOCK();

	return 0;
}

/*
 * int
 * pfc_ipcclnt_cpool_destroy(pfc_ipccpool_t pool)
 *	Destroy the connection pool associated with the specified pool handle.
 *	All connections cached in the pool are also closed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_cpool_destroy(pfc_ipccpool_t pool)
{
	ipc_cpool_t	*cplp;
	ipc_conn_t	*cnp;
	pfc_list_t	conns, *elem;
	pfc_rbtree_t	*alttree = &ipcconn_alttree;
	pfc_rbnode_t	*node;

	IPC_CLIENT_WRLOCK();

	if (pool == PFC_IPCCPOOL_GLOBAL) {
		/* Clean up the global connection pool. */
		cplp = &ipc_cpool_global;
	}
	else {
		/* Make the connection pool invisible. */
		node = pfc_rbtree_remove(&ipcconn_pool, IPC_CPOOL_KEY(pool));
		if (PFC_EXPECT_FALSE(node == NULL)) {
			IPC_CLIENT_UNLOCK();

			return ENOENT;
		}

		cplp = IPC_CPOOL_NODE2PTR(node);
	}

	/*
	 * All connections associated with this pool must be removed from
	 * ipcconn_alttree in the same lock transaction.
	 */
	PFC_LIST_FOREACH(&cplp->icp_lrulist, elem) {
		cnp = IPC_CONN_PLINK2PTR(elem);
		PFC_ASSERT(cnp->icn_pool == cplp);
		PFC_ASSERT(cnp->icn_poolent != NULL);
		cnp->icn_pool = NULL;
		ipcconn_unlink(alttree, cnp);
	}

	/* Move all connections to local list. */
	pfc_list_move_all(&cplp->icp_lrulist, &conns);

	/* Uncached connections must be also removed from ipcconn_alttree. */
	while ((elem = pfc_list_pop(&cplp->icp_uncached)) != NULL) {
		pfc_list_push_tail(&conns, elem);
		cnp = IPC_CONN_PLINK2PTR(elem);
		PFC_ASSERT(cnp->icn_pool == cplp);
		PFC_ASSERT(cnp->icn_poolent == NULL);
		cnp->icn_pool = NULL;
		ipcconn_unlink(alttree, cnp);
	}

	if (pool == PFC_IPCCPOOL_GLOBAL) {
		/* Global pool must be cleared here. */
		cplp->icp_size = 0;
		pfc_rbtree_init(&cplp->icp_pool,
				(pfc_rbcomp_t)pfc_ipcclnt_chaddr_compare,
				ipc_cpoolent_getkey);
		pfc_list_init(&cplp->icp_lrulist);
		PFC_ASSERT(pfc_list_is_empty(&cplp->icp_uncached));
	}

	IPC_CLIENT_UNLOCK();

	/* Close all connections. */
	ipcconn_altclose_list(&conns);

	if (pool != PFC_IPCCPOOL_GLOBAL) {
		free(cplp);
	}

	return 0;
}

/*
 * int
 * pfc_ipcclnt_cpool_open(pfc_ipccpool_t pool, const char *PFC_RESTRICT name,
 *			  pfc_ipcconn_t *PFC_RESTRICT connp)
 *	Create an alternative IPC connection handle via connection pool.
 *
 *	If an alternative connection associated with the specified IPC channel
 *	address is cached in the connection pool, the cached connection is
 *	returned.
 *	Otherwise, this function creates a new connection, and caches it in
 *	the pool unless the number of connections exceeds the capacity of
 *	the pool.
 *
 *	`pool' is a connection pool identifier. If `pool' is
 *	PFC_IPCCPOOL_GLOBAL, global connection pool is used.
 *	Otherwise connection pool handle created by pfc_ipcclnt_cpool_create()
 *	must be specified to `pool'.
 *
 *	`name' is a pointer to the IPC channel address to be connected.
 *
 *	- If `name' is NULL, the IPC channel address of the default connection
 *	  is used.
 *	- If `name' is an empty string, the IPC channel name of the default
 *	  connection and local host address is used as the IPC channel address.
 *	- If IPC channel name part of the IPC channel address is empty,
 *	  the IPC channel name of the default connection is used as the IPC
 *	  channel name.
 *
 * Calling/Exit State:
 *	Upon successful completion, connection handle is set to `*connp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_cpool_open(pfc_ipccpool_t pool, const char *PFC_RESTRICT name,
		       pfc_ipcconn_t *PFC_RESTRICT connp)
{
	ipc_cpool_t	*cplp;
	ipc_cpoolent_t	*cpent;
	ipc_chaddr_t	chaddr;
	ipc_conn_t	*cnp, *idle = NULL;
	int		err;

	if (PFC_EXPECT_FALSE(connp == NULL)) {
		return EINVAL;
	}

	IPC_CLIENT_RDLOCK();

	/* Parse IPC channel address. */
	err = pfc_ipcclnt_chaddr_parse(name, &chaddr, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/*
	 * Search for a connection associated with the specified IPC channel
	 * address in the specified connection pool.
	 */
	err = ipc_cpool_getconn(pool, &chaddr, &cplp, connp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Found. */
		goto out;
	}

	if (PFC_EXPECT_FALSE(err > 0)) {
		goto out;
	}

	/*
	 * We need to upgrade the client lock here because a new connection
	 * handle must be opened.
	 */
	IPC_CLIENT_UNLOCK();
	IPC_CLIENT_WRLOCK();

	/*
	 * We need to search for the cached connection again because the
	 * cache state may have been changed while the client lock is
	 * released.
	 */
	err = ipc_cpool_getconn(pool, &chaddr, &cplp, connp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Found. */
		goto out;
	}

	if (PFC_EXPECT_FALSE(err > 0)) {
		goto out;
	}

	/* Allocate a new connection handle. */
	cnp = (ipc_conn_t *)malloc(sizeof(*cnp));
	if (PFC_EXPECT_FALSE(cnp == NULL)) {
		err = ENOMEM;
		goto out;
	}

	/* Allocate a new connection pool entry. */
	cpent = (ipc_cpoolent_t *)malloc(sizeof(*cpent));
	if (PFC_EXPECT_FALSE(cpent == NULL)) {
		err = ENOMEM;
		free(cnp);
		goto out;
	}

	/* Register a new connection handle. */
	err = ipcconn_register(cnp, &chaddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(cnp);
		free(cpent);
		goto out;
	}

	/* Check the capacity of the pool. */
	if (PFC_EXPECT_TRUE(ipc_cpool_checksize(cplp, &idle))) {
		/* Cache the connection into the connection pool. */
		cpent->icpe_addr = chaddr;
		cpent->icpe_conn = cnp;
		cpent->icpe_refcnt = 1;
		cpent->icpe_flags = 0;
		PFC_ASSERT_INT(pfc_rbtree_put(&cplp->icp_pool,
					      &cpent->icpe_node), 0);
		cplp->icp_size++;

		cnp->icn_pool = cplp;
		cnp->icn_poolent = cpent;

		/* Link a new connection to the tail of the LRU list. */
		pfc_list_push_tail(&cplp->icp_lrulist, &cnp->icn_plink);
	}
	else {
		/* The pool is full. This connection can not be cached. */
		cnp->icn_pool = cplp;
		pfc_list_push_tail(&cplp->icp_uncached, &cnp->icn_plink);
		free(cpent);
	}

	*connp = cnp->icn_id;

out:
	IPC_CLIENT_UNLOCK();

	if (idle != NULL) {
		/* Close an idle connection. */
		ipcconn_altclose(idle);
	}

	return err;
}

/*
 * int
 * pfc_ipcclnt_cpool_close(pfc_ipccpool_t pool, pfc_ipcconn_t conn,
 *			   uint32_t flags)
 *	Close the alternative connection specified by `conn' in the connection
 *	pool specified by `pool'.
 *
 *	`flags' determines behavior of this function. It is either zero or
 *	the bitwise OR of one or more of the following flags:
 *
 *	PFC_IPCPLF_C_FORCE
 *	    Force to close connection.
 *	    If bit is set, the connection specified by `conn' is closed and
 *	    removed from the pool, even if it is still opened.
 *	    If not set, the connection specified by `conn' is not closed and
 *	    retained in the pool, as long as it is cached in the pool.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_cpool_close(pfc_ipccpool_t pool, pfc_ipcconn_t conn,
			uint32_t flags)
{
	ipc_cpool_t	*cplp;
	ipc_cpoolent_t	*cpent;
	ipc_conn_t	*cnp;
	pfc_rbtree_t	*alttree = &ipcconn_alttree;
	pfc_rbnode_t	*node;
	int		err;

	if (PFC_EXPECT_FALSE(flags & ~IPC_CPOOL_CLOSE_FLAGS)) {
		return EINVAL;
	}

	IPC_CLIENT_WRLOCK();

	/* Determine connection handle. */
	node = pfc_rbtree_get(alttree, IPC_CONN_KEY(conn));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		err = EBADF;
		goto dontclose;
	}

	cnp = IPC_CONN_NODE2PTR(node);
	cplp = cnp->icn_pool;
	if (PFC_EXPECT_FALSE(cplp == NULL || cplp->icp_id != pool)) {
		/* This connection is not cached in the specified pool. */
		err = ENOENT;
		goto dontclose;
	}
	PFC_ASSERT(IPC_CPOOL_IS_VALID(pool, cplp));

	cpent = cnp->icn_poolent;
	if (PFC_EXPECT_FALSE(cpent == NULL)) {
		/*
		 * This connection is opened from this pool, but not cached.
		 * So it must be closed immediately.
		 */
		ipc_cpool_unlink(cplp, cnp);
		cnp->icn_pool = NULL;
		ipcconn_unlink(alttree, cnp);
	}
	else {
		PFC_ASSERT(cpent->icpe_conn == cnp);
		if (PFC_EXPECT_TRUE((flags & PFC_IPCPLF_C_FORCE) == 0)) {
			uint32_t	refcnt = cpent->icpe_refcnt;

			/*
			 * We don't need to use atomic operation here because
			 * the client lock is held in writer mode.
			 * The reference counter is always incremented with
			 * holding the client lock in reader or writer mode.
			 */
			if (PFC_EXPECT_FALSE(refcnt == 0)) {
				/* This connection is closed too many times. */
				err = EPERM;
			}
			else {
				/*
				 * This connection should be retained in the
				 * pool.
				 */
				cpent->icpe_refcnt = refcnt - 1;
				err = 0;
			}
			goto dontclose;
		}

		/* Remove the connection from the pool. */
		ipc_cpool_remove(cplp, cpent, cnp);
	}

	/* Close the connection. */
	IPC_CLIENT_UNLOCK();
	ipcconn_altclose(cnp);

	return 0;

dontclose:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * uint32_t
 * pfc_ipcclnt_cpool_getsize(pfc_ipccpool_t pool)
 *	Get the number of connections cached in the pool specified by `pool'.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of connections in the pool
 *	is returned.
 *	UINT32_MAX is returned if the connection pool specified by `pool' does
 *	not exist.
 */
uint32_t
pfc_ipcclnt_cpool_getsize(pfc_ipccpool_t pool)
{
	ipc_cpool_t	*cplp;
	uint32_t	size;
	int		err;

	IPC_CLIENT_RDLOCK();
	err = ipc_cpool_lookup(pool, &cplp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		size = cplp->icp_size;
	}
	else {
		size = UINT32_MAX;
	}
	IPC_CLIENT_UNLOCK();

	return size;
}

/*
 * uint32_t
 * pfc_ipcclnt_cpool_getcapacity(pfc_ipccpool_t pool)
 *	Get the capacity of the connection pool specified by `pool'.
 *
 * Calling/Exit State:
 *	Upon successful completion, the capacity of the connection pool is
 *	returned.
 *	UINT32_MAX is returned if the connection pool specified by `pool' does
 *	not exist.
 */
uint32_t
pfc_ipcclnt_cpool_getcapacity(pfc_ipccpool_t pool)
{
	ipc_cpool_t	*cplp;
	uint32_t	capacity;
	int		err;

	IPC_CLIENT_RDLOCK();
	err = ipc_cpool_lookup(pool, &cplp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		capacity = cplp->icp_capacity;
	}
	else {
		capacity = UINT32_MAX;
	}
	IPC_CLIENT_UNLOCK();

	return capacity;
}

/*
 * void
 * pfc_ipcclnt_cpool_reap(pfc_bool_t forced)
 *	Reap unused connections in all connection pools.
 *
 *	If `forced' is PFC_TRUE, this function reaps all unused connections
 *	unconditionally.
 *
 *	If `forced' is PFC_FALSE, this function reaps connections which are
 *	not used from the previous call of this function.
 */
void
pfc_ipcclnt_cpool_reap(pfc_bool_t forced)
{
	pfc_list_t	reaped = PFC_LIST_INITIALIZER(reaped);
	ipc_cpooliter_t	func;

	func = (forced) ? ipc_cpool_reap_forced : ipc_cpool_reap;

	/* Scan all pools and reap unused connections. */
	IPC_CLIENT_WRLOCK();
	ipc_cpool_iterate(func, &reaped);
	IPC_CLIENT_UNLOCK();

	/* Close reaped connections. */
	ipcconn_altclose_list(&reaped);
}

/*
 * int
 * pfc_ipcclnt_conn_cancel(pfc_ipcconn_t conn, pfc_bool_t discard)
 *	Cancel all cancelable client sessions associated with the given
 *	connection.
 *
 *	This function is equivalent to calling pfc_ipcclnt_sess_cancel()
 *	for each client sessions associated with the given connection.
 *
 *	`conn' is the target connection handle.
 *	If PFC_IPCCONN_DEFAULT is specified, this function will cancel
 *	sessions associated with the default connection.
 *
 *	If PFC_TRUE is passed to `discard', the state of the canceled client
 *	session will be changed to DISCARD. In this case further IPC service
 *	request on the canceled client session will get ESHUTDOWN error.
 *
 *	Note that this function never affects client sessions without
 *	PFC_IPCSSF_CANCELABLE flag.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Note that zero is returned if no client session is present on the
 *	given connection.
 *
 *	EBADF is returned if the given connection handle is invalid.
 */
int
pfc_ipcclnt_conn_cancel(pfc_ipcconn_t conn, pfc_bool_t discard)
{
	pfc_rbtree_t	*alttree = &ipcconn_alttree;
	ipc_conn_t	*cnp;
	ipc_clnotify_t	clnotify;
	int		err;

	IPC_CLIENT_RDLOCK();

	/* Determine the connection handle specified by the caller. */
	if (conn == PFC_IPCCONN_DEFAULT) {
		cnp = &ipcconn_default;
		if (PFC_EXPECT_FALSE(!IPC_CONN_DEFAULT_IS_VALID(cnp))) {
			/*
			 * The default connection is not yet initialized.
			 * So no client session is present on the default
			 * connection.
			 */
			err = 0;
			goto out;
		}
	}
	else {
		pfc_rbnode_t	*node;

		node = pfc_rbtree_get(alttree, IPC_CONN_KEY(conn));
		if (PFC_EXPECT_FALSE(node == NULL)) {
			err = EBADF;
			goto out;
		}
		cnp = IPC_CONN_NODE2PTR(node);
	}

	/* Cancel sessions associated with the given connection. */
	IPC_CLNOTIFY_INIT_FORCE(&clnotify, discard);
	pfc_ipcclnt_canceller_conn_notify(cnp, &clnotify);
	err = 0;

out:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * ipc_conn_t PFC_ATTR_HIDDEN *
 * pfc_ipcclnt_getconn(pfc_ipcconn_t conn)
 *	Get connection handle associated with the given connection ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to connection handle
 *	is returned.
 *	NULL is returned if an invalid ID is specified.
 *
 * Remarks:
 *	This function hold the connection handle on successful return.
 *	The caller must release the handle using IPC_CONN_RELEASE().
 */
ipc_conn_t PFC_ATTR_HIDDEN *
pfc_ipcclnt_getconn(pfc_ipcconn_t conn)
{
	pfc_rbnode_t	*node;
	ipc_conn_t	*cnp;

	IPC_CLIENT_RDLOCK();

	node = pfc_rbtree_get(&ipcconn_alttree, IPC_CONN_KEY(conn));
	if (PFC_EXPECT_TRUE(node != NULL)) {
		ipc_cpool_t	*cplp;

		cnp = IPC_CONN_NODE2PTR(node);
		IPC_CONN_HOLD(cnp);

		cplp = cnp->icn_pool;
		if (cplp != NULL) {
			ipc_cpool_update_lru(cplp, cnp->icn_poolent, cnp);
		}
	}
	else {
		cnp = NULL;
	}

	IPC_CLIENT_UNLOCK();

	return cnp;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_default(ipc_conn_t **cnpp)
 *	Get default connection handle.
 *
 * Calling/Exit State:
 *	Upon successful completion, connection ID associated with the default
 *	connection handle is set to `*connp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function hold the connection handle on successful return.
 *	The caller must release the handle using IPC_CONN_RELEASE().
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_default(ipc_conn_t **cnpp)
{
	ipc_conn_t	*cnp = &ipcconn_default;
	int		err;

	*cnpp = cnp;

	IPC_CLIENT_RDLOCK();

	/* Check to see whether the default handle is initialized. */
	if (PFC_EXPECT_TRUE(IPC_CONN_DEFAULT_IS_VALID(cnp))) {
		IPC_CONN_HOLD(cnp);
		IPC_CLIENT_UNLOCK();

		return 0;
	}

	IPC_CLIENT_UNLOCK();

	/* Check again with writer lock. */
	IPC_CLIENT_WRLOCK();
	if (!IPC_CONN_DEFAULT_IS_VALID(cnp)) {
		/* Initialize the default connection handle. */
		err = ipcconn_init_default(cnp);
	}
	else {
		err = 0;
		IPC_CONN_HOLD(cnp);
	}

	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_invoke(ipc_conn_t *PFC_RESTRICT cnp,
 *			   pfc_ipcsess_t *PFC_RESTRICT sess,
 *			   ipc_canceller_t *PFC_RESTRICT clp,
 *			   pfc_ipcresp_t *PFC_RESTRICT respp,
 *			   const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Issue an IPC request on the given IPC connection.
 *
 *	The caller must obtain the session canceller instance in advance,
 *	and must pass it to `clp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, response code sent by the IPC server is
 *	set to `*respp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the connection lock and
 *	  the session lock, and both are always released on return.
 *
 *	- The caller must hold the session canceller specified by `clp'.
 *	  It will be released on error return.
 *
 *	- This function always frees up PDUs in the IPC output stream.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_invoke(ipc_conn_t *PFC_RESTRICT cnp,
			pfc_ipcsess_t *PFC_RESTRICT sess,
			ipc_canceller_t *PFC_RESTRICT clp,
			pfc_ipcresp_t *PFC_RESTRICT respp,
			const pfc_timespec_t *PFC_RESTRICT abstime)
{
	ipc_msg_t	*msg = &sess->icss_msg;
	ipc_stream_t	*stp = &sess->icss_output;
	int		err;

	IPC_CLSESS_SET_BUSY(sess);

	/* Set up connection. */
	err = ipcconn_setup(cnp, sess, clp, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_CANCELLER_RELEASE(clp);
		goto error_output;
	}

	/* Invoke IPC service handler on the IPC server. */
	pfc_ipcmsg_setbflags(msg, cnp->icn_sess.iss_flags);
	pfc_ipcclnt_ipcmsg_assert(msg, cnp->icn_sess.iss_flags);
	err = ipcconn_invoke(cnp, sess, respp, abstime);

	IPC_CONN_LOCK(cnp);
	IPC_CLSESS_LOCK(sess);

	/* IPC stream data is no longer used. */
	pfc_ipcstream_reset(stp, 0);

	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_ipcmsg_reset(msg);
		goto error;
	}

	PFC_ASSERT(sess->icss_state == IPC_SSTATE_BUSY);
	sess->icss_state = IPC_SSTATE_RESULT;
	PFC_ASSERT(cnp->icn_current == sess);
	cnp->icn_current = NULL;
	IPC_CONN_SIGNAL(cnp);
	IPC_CLSESS_CLEAR_BUSY(sess);

	IPC_CLSESS_UNLOCK(sess);
	IPC_CONN_UNLOCK(cnp);

	return err;

error_output:
	pfc_ipcstream_reset(stp, 0);

error:
	PFC_ASSERT(sess->icss_state == IPC_SSTATE_BUSY);

	if (IPC_CLSESS_ISFROZEN(sess)) {
		/* This session must be discarded. */
		sess->icss_state = IPC_SSTATE_DISCARD;
	}
	else {
		sess->icss_state = IPC_SSTATE_READY;
	}

	if (cnp->icn_current == sess) {
		pfc_iostream_t	stream = cnp->icn_stream;

		/*
		 * Discard connection cache unless ENOSYS error.
		 * ENOSYS error means that the IPC server is running.
		 * So the connection can be cached on ENOSYS error.
		 */
		if (err != ENOSYS && stream != NULL) {
			PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);
			cnp->icn_stream = NULL;

			if (cnp->icn_canceller != NULL) {
				IPC_CANCELLER_RELEASE(cnp->icn_canceller);
				cnp->icn_canceller = NULL;
			}
		}

		cnp->icn_current = NULL;
		IPC_CONN_SIGNAL(cnp);
	}

	IPC_CLSESS_CLEAR_BUSY(sess);
	IPC_CLSESS_UNLOCK(sess);
	IPC_CONN_UNLOCK(cnp);

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_destroy(ipc_conn_t *cnp)
 *	Destroy the given connection handle.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_destroy(ipc_conn_t *cnp)
{
	PFC_ASSERT(cnp != &ipcconn_default);
	PFC_ASSERT(pfc_list_is_empty(&cnp->icn_sessions));

	/*
	 * Unlink this connection from the dead connection list if it ls
	 * linked. Note that we can see icn_flags without holding the
	 * connection lock because this connection is no longer visible to
	 * other threads.
	 */
	if (cnp->icn_flags & IPCONNF_DEAD) {
		IPC_CLIENT_WRLOCK();
		pfc_list_remove(&cnp->icn_list);
		IPC_CLIENT_UNLOCK();
	}

	if (cnp->icn_chan != NULL) {
		IPC_CLCHAN_RELEASE(cnp->icn_chan);
	}
	if (cnp->icn_stream != NULL) {
		PFC_ASSERT_INT(pfc_iostream_destroy(cnp->icn_stream), 0);
	}
	if (cnp->icn_canceller != NULL) {
		IPC_CANCELLER_RELEASE(cnp->icn_canceller);
	}

	PFC_ASSERT_INT(pfc_cond_destroy(&cnp->icn_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&cnp->icn_mutex), 0);

	pfc_sockaddr_destroy(&cnp->icn_sockaddr);
	free(cnp);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_setdefault(const char *PFC_RESTRICT name,
 *			       char *PFC_RESTRICT newname,
 *			       pfc_hostaddr_t *PFC_RESTRICT haddr)
 *	Set IPC channel address for the default connection.
 *
 *	`newname' must points the buffer to store new IPC channel name of
 *	the default connection. It must be at least
 *	(IPC_CHANNEL_NAMELEN_MAX + 1) bytes available in the buffer.
 *
 *	`haddr' must points the pfc_hostaddr_t buffer to store new IPC server's
 *	host address of the default connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, new IPC channel name and IPC server's host
 *	address are set to the buffer pointed by `newname' and `haddr'
 *	respectively,  and zero is returned.
 *
 *	EINVAL is returned if an invalid channel address is specified to
 *	`name'.
 *	EBUSY is returned if at least one IPC session exists on the default
 *	connection.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	write mode.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_setdefault(const char *PFC_RESTRICT name,
			    char *PFC_RESTRICT newname,
			    pfc_hostaddr_t *PFC_RESTRICT haddr)
{
	ipc_conn_t	*cnp = &ipcconn_default;
	ipc_clchan_t	*chp, *newchp;
	ipc_canceller_t	*clp;
	ipc_sess_t	sess;
	pfc_iostream_t	stream;
	pfc_hostaddr_t	newhaddr;
	pfc_sockaddr_t	newsa, oldsa, *sap;
	pfc_bool_t	initialized;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	/* Obtain the default connection. */
	if (!IPC_CONN_DEFAULT_IS_VALID(cnp)) {
		/* Initialize the default connection handle. */
		err = ipcconn_init_default(cnp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
		initialized = PFC_TRUE;
	}
	else {
		initialized = PFC_FALSE;
	}

	/* Obtain IPC channel information. */
	err = pfc_ipcclnt_getchan(name, &newchp, &newhaddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Initialize new socket address. */
	sap = &newsa;
	memset(&sess, 0, sizeof(sess));
	err = pfc_ipcclnt_sockaddr_init(sap, newchp, &sess, &newhaddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_CLCHAN_RELEASE(newchp);
		goto out;
	}

	IPC_CONN_LOCK(cnp);

	chp = cnp->icn_chan;
	if (newchp == chp) {
		/* IPC channel address is not changed. */
		PFC_ASSERT(err == 0);
		goto out_unlock;
	}

	if (PFC_EXPECT_FALSE(!pfc_list_is_empty(&cnp->icn_sessions))) {
		err = EBUSY;
		goto out_unlock;
	}

	PFC_ASSERT(cnp->icn_current == NULL);

	/* Destroy connection stream. */
	stream = cnp->icn_stream;
	if (stream != NULL) {
		PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);
		cnp->icn_stream = NULL;
	}

	strcpy(newname, IPC_CLCHAN_NAME(newchp));
	*haddr = newhaddr;

	/* Install new IPC channel. */
	cnp->icn_chan = newchp;
	newchp = chp;
	oldsa = cnp->icn_sockaddr;
	cnp->icn_sockaddr = newsa;
	sap = &oldsa;

	cnp->icn_sess = sess;

	/* Release canceller. */
	clp = cnp->icn_canceller;
	if (clp != NULL) {
		IPC_CANCELLER_RELEASE(clp);
		cnp->icn_canceller = NULL;
	}

out_unlock:
	IPC_CONN_UNLOCK(cnp);

	IPC_CLCHAN_RELEASE(newchp);
	pfc_sockaddr_destroy(sap);

out:
	if (initialized) {
		/*
		 * Revert reference counter incremented by the call of
		 * ipcconn_init_default().
		 */
		IPC_CONN_RELEASE_DEFAULT(cnp);
	}

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_iterate(ipc_conniter_t iter, pfc_ptr_t arg)
 *	Iterate all active IPC connections.
 *
 *	Each connection is passed to the iterator function specified by `iter'.
 *	`arg' is passed to the call of `iter'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	reader or writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_iterate(ipc_conniter_t iter, pfc_ptr_t arg)
{
	pfc_rbnode_t	*node = NULL;
	pfc_list_t	*elem;

	if (ipcconn_default.icn_id != PFC_IPCCONN_INVALID) {
		PFC_ASSERT(IPC_CONN_DEFAULT_IS_VALID(&ipcconn_default));
		(*iter)(&ipcconn_default, arg);
	}

	while ((node = pfc_rbtree_next(&ipcconn_alttree, node)) != NULL) {
		(*iter)(IPC_CONN_NODE2PTR(node), arg);
	}

	/* Iterate dead alternative connections. */
	PFC_LIST_FOREACH(&ipc_conn_dead, elem) {
		(*iter)(IPC_CONN_LIST2PTR(elem), arg);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_cleanup(void)
 *	Clean up the default connection handle.
 *	This function will be called via library destructor.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_cleanup(void)
{
	ipc_conn_t	*cnp = &ipcconn_default;
	pfc_refptr_t	*pname;
	pfc_iostream_t	stream;

	/* Try to acquire the client lock in writer mode. */
	if (IPC_CLIENT_TRYWRLOCK() != 0) {
		return;
	}

	pname = ipcclnt_procname;
	if (pname != NULL) {
		/* Release cached process name. */
		pfc_refptr_put(pname);
		ipcclnt_procname = NULL;
	}

	if (!IPC_CONN_DEFAULT_IS_VALID(cnp)) {
		/* The default connection is not yet initialized. */
		goto unlock_client;
	}

	/* Try to acquire the default connection lock in writer mode. */
	if (IPC_CONN_TRYLOCK(cnp) != 0) {
		goto unlock_client;
	}

	/* Destroy the default connection stream if it is not used. */
	stream = cnp->icn_stream;
	if (stream != NULL && cnp->icn_current == NULL) {
		ipc_canceller_t	*clp = cnp->icn_canceller;

		__pfc_iostream_dispose(stream);
		cnp->icn_stream = NULL;

		if (clp != NULL) {
			IPC_CANCELLER_RELEASE_FINI(clp);
			cnp->icn_canceller = NULL;
		}
	}

	/* Deinitialize the default connection if no one uses it. */
	if (cnp->icn_refcnt == IPC_CONN_DEFAULT_REFCNT) {
		IPC_CLCHAN_RELEASE(cnp->icn_chan);
		cnp->icn_chan = NULL;
		cnp->icn_id = PFC_IPCCONN_INVALID;
		pfc_sockaddr_destroy(&cnp->icn_sockaddr);
	}

	IPC_CONN_UNLOCK(cnp);

unlock_client:
	IPC_CLIENT_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_fork_prepare(void)
{
	/*
	 * Acquire all connection locks and session locks.
	 *
	 * Remarks:
	 *	Connection pool locks don't need to be held on fork(2)
	 *	because they are always held with holding the client lock.
	 */
	pfc_ipcclnt_conn_iterate(ipcconn_fork_prepare, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_fork_parent(void)
{
	/* Release all locks held by pfc_ipcclnt_conn_fork_prepare(). */
	pfc_ipcclnt_conn_iterate(ipcconn_fork_parent, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_conn_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_conn_fork_child(void)
{
	/* Discard IPC connections. */
	pfc_ipcclnt_conn_iterate(ipcconn_fork_child, NULL);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_connect(ipc_sess_t *PFC_RESTRICT isp,
 *		       ipc_clchan_t *PFC_RESTRICT chp,
 *		       pfc_sockaddr_t *PFC_RESTRICT sap, int canceller,
 *		       ctimespec_t *PFC_RESTRICT abstime)
 *	Establish the IPC session between the client and the specified
 *	IPC channel.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function always changes the file descriptor of the session stream
 *	to -1 if connect(2) failed.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_connect(ipc_sess_t *PFC_RESTRICT isp,
		    ipc_clchan_t *PFC_RESTRICT chp,
		    pfc_sockaddr_t *PFC_RESTRICT sap, int canceller,
		    ctimespec_t *PFC_RESTRICT abstime)
{
	pfc_iostream_t	stream = isp->iss_stream;
	csockaddr_t	*saddr = sap->sa_addr;
	int		sock, err;

	/* Connect the socket to the IPC channel. */
	sock = pfc_iostream_getfd(stream);
	err = pfc_sock_connect_abs_c(sock, saddr, sap->sa_addrlen, canceller,
				     abstime, NULL);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Establish the IPC session. */
		return ipcconn_handshake(isp, chp, abstime);
	}

	/* pfc_sock_connect_abs_c() closes the socket on error. */
	PFC_ASSERT_INT(__pfc_iostream_setfd(stream, -1), 0);

	if (saddr->sa_family == AF_UNIX) {
		if (err == ENOENT) {
			/* UNIX domain socket file does not exist. */
			err = ECONNREFUSED;
		}
		else if (err == EAGAIN) {
			/*
			 * Too many connection requests are queued on the
			 * IPC server's listener socket.
			 */
			err = ENOSPC;
		}
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_sockaddr_init(pfc_sockaddr_t *PFC_RESTRICT sap,
 *			     ipc_clchan_t *PFC_RESTRICT chp,
 *			     ipc_sess_t *PFC_RESTRICT sess,
 *			     const pfc_hostaddr_t *PFC_RESTRICT haddr)
 *	Initialize socket address to connect the IPC server specified by
 *	`chp' and `haddr'.
 *
 * Calling/Exit State:
 *	Upon successful completion, the socket address specified by `sap'
 *	is initialized, and the IPC session specified by `sess' is initialized,
 *	and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_sockaddr_init(pfc_sockaddr_t *PFC_RESTRICT sap,
			  ipc_clchan_t *PFC_RESTRICT chp,
			  ipc_sess_t *PFC_RESTRICT sess,
			  const pfc_hostaddr_t *PFC_RESTRICT haddr)
{
	const char	*service;
	struct addrinfo	hints;
	int		gerr, err, family;

	/* Initialize new IPC session. */
	err = pfc_ipc_sess_init(sess, haddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Unable to initialize IPC session: %s",
				  strerror(err));

		return err;
	}

	family = pfc_hostaddr_gettype(haddr);
	if (PFC_EXPECT_FALSE(family != AF_UNIX)) {
		return EAFNOSUPPORT;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	service = pfc_refptr_string_value(chp->ichc_path);
	gerr = pfc_sockaddr_init(sap, haddr, service, &hints);
	if (PFC_EXPECT_TRUE(gerr == 0)) {
		return 0;
	}

	if (gerr == EAI_SYSTEM) {
		err = errno;
		IPCCLNT_LOG_ERROR("Failed to initialize sockaddr: addr=%s "
				  "errno=%d", service, err);
		if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}
	}
	else {
		IPCCLNT_LOG_ERROR("Failed to initialize sockaddr: addr=%s "
				  "%s", service, gai_strerror(gerr));
		err = EINVAL;
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_ping(pfc_iostream_t PFC_RESTRICT stream,
 *		    ctimespec_t *PFC_RESTRICT timeout)
 *	Send a ping message to the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_ping(pfc_iostream_t PFC_RESTRICT stream,
		 ctimespec_t *PFC_RESTRICT timeout)
{
	uint32_t	arg, value;
	uint8_t		cmd = IPC_COMMAND_PING;
	pfc_timespec_t	abstime;
	int		err;

	err = pfc_clock_gettime(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	if (sizeof(abstime.tv_sec) > sizeof(arg)) {
		arg = (uint32_t)(abstime.tv_sec & 0xffffffff);
	}
	else {
		arg = abstime.tv_sec;
	}

	if (timeout == NULL) {
		/* Use default timeout. */
		abstime.tv_sec += IPC_CONN_PING_TIMEOUT;
	}
	else {
		pfc_timespec_add(&abstime, timeout);
	}

	/* Send a PING request. */
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_FALSE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = pfc_ipc_write(stream, &arg, sizeof(arg), PFC_TRUE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Receive response from the server. */
	err = pfc_ipc_read(stream, &value, sizeof(value), &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return (PFC_EXPECT_TRUE(value == arg)) ? 0 : EPROTO;
}

/*
 * static pfc_cptr_t
 * ipcconn_getkey(pfc_rbnode_t *node)
 *	Return the key of the given IPC connection handle.
 *	`node' must be a pointer to icn_node in ipc_conn_t.
 */
static pfc_cptr_t
ipcconn_getkey(pfc_rbnode_t *node)
{
	ipc_conn_t	*cnp = IPC_CONN_NODE2PTR(node);

	return IPC_CONN_KEY(cnp->icn_id);
}

/*
 * static pfc_cptr_t
 * ipc_cpool_getkey(pfc_rbnode_t *node)
 *	Return the key of the given IPC connection pool instance.
 *	`node' must be a pointer to icp_node in ipc_cpool_t.
 */
static pfc_cptr_t
ipc_cpool_getkey(pfc_rbnode_t *node)
{
	ipc_cpool_t	*cplp = IPC_CPOOL_NODE2PTR(node);

	return IPC_CPOOL_KEY(cplp->icp_id);
}

/*
 * static pfc_cptr_t
 * ipc_cpoolent_getkey(pfc_rbnode_t *node)
 *	Return the key of the given IPC connection pool entry.
 *	`node' must be a pointer to icpe_node in ipc_cpoolent_t.
 */
static pfc_cptr_t
ipc_cpoolent_getkey(pfc_rbnode_t *node)
{
	ipc_cpoolent_t	*cpent = IPC_CPOOLENT_NODE2PTR(node);

	return (pfc_cptr_t)&cpent->icpe_addr;
}

/*
 * static int
 * ipcconn_register(ipc_conn_t *PFC_RESTRICT cnp,
 *		    const ipc_chaddr_t *PFC_RESTRICT cap)
 *	Register a new alternative IPC connection handle.
 *
 *	`cap' must points ipc_chaddr_t which contains IPC channel address.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
static int
ipcconn_register(ipc_conn_t *PFC_RESTRICT cnp,
		 const ipc_chaddr_t *PFC_RESTRICT cap)
{
	pfc_ipcconn_t	first;
	int		err;

	/* Initialize connection handle. */
	err = ipcconn_init(cnp, cap);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Assign new identifier. */
	cnp->icn_refcnt = 1;
	cnp->icn_id = first = ipcconn_next_id;
	for (;;) {
		pfc_ipcconn_t	id;

		/*
		 * Register this connection handle to the alternative
		 * connection tree.
		 */
		err = pfc_rbtree_put(&ipcconn_alttree, &cnp->icn_node);

		/* Prepare ID for the next allocation. */
		id = cnp->icn_id + 1;
		if (PFC_EXPECT_FALSE(id < IPC_CONN_ID_MIN)) {
			id = IPC_CONN_ID_MIN;
		}

		/* Check result. */
		if (PFC_EXPECT_TRUE(err == 0)) {
			ipcconn_next_id = id;
			break;
		}

		/* Try another ID. */
		if (PFC_EXPECT_FALSE(id == first)) {
			err = ENFILE;
			IPC_CLCHAN_RELEASE(cnp->icn_chan);
			pfc_sockaddr_destroy(&cnp->icn_sockaddr);
			break;
		}

		cnp->icn_id = id;
	}

	return err;
}

/*
 * static int
 * ipcconn_init(ipc_conn_t *PFC_RESTRICT cnp,
 *		const ipc_chaddr_t *PFC_RESTRICT cap)
 *	Initialize a new IPC connection handle.
 *
 *	`cap' must points ipc_chaddr_t which contains IPC channel address.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in
 *	  writer mode.
 *
 *	- Below fields must be initialized by the caller:
 *	  + icn_id
 *	  + icn_refcnt
 */
static int
ipcconn_init(ipc_conn_t *PFC_RESTRICT cnp,
	     const ipc_chaddr_t *PFC_RESTRICT cap)
{
	ipc_clchan_t	*chp;
	int		err;

	err = PFC_MUTEX_INIT(&cnp->icn_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	err = pfc_cond_init(&cnp->icn_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	/* Fetch IPC channel configuration. */
	err = pfc_ipcclnt_getchanbyname(cap->ica_name, &cnp->icn_chan);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Initialize socket address. */
	chp = cnp->icn_chan;
	err = pfc_ipcclnt_sockaddr_init(&cnp->icn_sockaddr, chp,
					&cnp->icn_sess, &cap->ica_host);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_CLCHAN_RELEASE(cnp->icn_chan);

		return err;
	}

	cnp->icn_flags = 0;
	cnp->icn_pool = NULL;
	cnp->icn_poolent = NULL;
	cnp->icn_stream = NULL;
	cnp->icn_current = NULL;
	cnp->icn_canceller = NULL;
	pfc_list_init(&cnp->icn_sessions);
	pfc_list_init(&cnp->icn_plink);

	return 0;
}

/*
 * static int
 * ipcconn_init_default(ipc_conn_t *cnp)
 *	Initialize the default connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 *	The caller must release the handle using IPC_CONN_RELEASE().
 */
static int
ipcconn_init_default(ipc_conn_t *cnp)
{
	ipc_chaddr_t	chaddr;
	int		err;

	/* Fetch IPC channel address of the default connection. */
	PFC_ASSERT_INT(pfc_ipcclnt_chaddr_parse(NULL, &chaddr, PFC_TRUE), 0);

	/* Initialize the handle with the default IPC channel. */
	err = ipcconn_init(cnp, &chaddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	cnp->icn_id = PFC_IPCCONN_DEFAULT;

	/*
	 * Initialize reference counter with (IPC_CONN_DEFAULT_REFCNT + 1)
	 * in order not to be released.
	 */
	cnp->icn_refcnt = IPC_CONN_DEFAULT_REFCNT + 1;

	return 0;
}

/*
 * static void
 * ipcconn_altclose(ipc_conn_t *cnp)
 *	Close the given alternative connection instance.
 *
 * Remarks:
 *	- This function must be called without holding the client lock.
 *
 *	- The specified connection instance must be removed from
 *	  ipcconn_alttree in advance.
 */
static void
ipcconn_altclose(ipc_conn_t *cnp)
{
	pfc_list_t	*elem;
	ipc_cpoolent_t	*cpent = cnp->icn_poolent;

	PFC_ASSERT(cnp->icn_pool == NULL);
	PFC_ASSERT(pfc_list_is_empty(&cnp->icn_plink));

	if (cpent != NULL) {
		free(cpent);
	}

	IPC_CONN_LOCK(cnp);

	/*
	 * Invalidate sessions on the connection handle except for ongoing
	 * session.
	 */
	PFC_LIST_FOREACH(&cnp->icn_sessions, elem) {
		pfc_ipcsess_t	*sess = IPC_CLSESS_LIST2PTR(elem);

		IPC_CLSESS_LOCK(sess);
		pfc_ipcclnt_sess_freeze(sess);
		IPC_CLSESS_UNLOCK(sess);
	}

	/* Set closed flag. */
	cnp->icn_flags |= IPCONNF_CLOSED;
	IPC_CONN_BROADCAST(cnp);
	IPC_CONN_UNLOCK(cnp);

	IPC_CONN_RELEASE(cnp);
}

/*
 * static void
 * ipcconn_unlink(pfc_rbtree_t *PFC_RESTRICT tree,
 *		  ipc_conn_t *PFC_RESTRICT cnp)
 *	Unlink the given connection from the alternative connection tree.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static void
ipcconn_unlink(pfc_rbtree_t *PFC_RESTRICT tree, ipc_conn_t *PFC_RESTRICT cnp)
{
	PFC_ASSERT(tree == &ipcconn_alttree);

	/* Remove the connection from the alternative connection tree. */
	pfc_rbtree_remove_node(tree, &cnp->icn_node);

	if (cnp->icn_refcnt == 1) {
		/*
		 * We can say that no other thread refers this connection.
		 * So there is nothing to do because this connection will be
		 * destroyed by succeeding IPC_CONN_RELEASE().
		 */
	}
	else {
		/*
		 * This connection is still used.
		 * Link this connection to the dead connection list.
		 */
		IPC_CONN_LOCK(cnp);
		cnp->icn_flags |= IPCONNF_DEAD;
		pfc_list_push_tail(&ipc_conn_dead, &cnp->icn_list);
		IPC_CONN_UNLOCK(cnp);
	}
}

/*
 * static int
 * ipcconn_setup(ipc_conn_t *PFC_RESTRICT cnp, pfc_ipcsess_t *PFC_RESTRICT sess,
 *		 ipc_canceller_t *PFC_RESTRICT clp,
 *		 ctimespec_t *PFC_RESTRICT abstime)
 *	Set up IPC connection to issue an IPC request.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	The caller must obtain the session canceller instance in advance,
 *	and must pass it to `clp'.
 *
 * Remarks:
 *	- This function must be called with holding the connection lock and
 *	  the session lock. Both locks are released on successful return,
 *	  and held on error return.
 *
 *	- The caller must hold the session canceller specified by `clp', and
 *	  the caller must release it on error return.
 */
static int
ipcconn_setup(ipc_conn_t *PFC_RESTRICT cnp, pfc_ipcsess_t *PFC_RESTRICT sess,
	      ipc_canceller_t *PFC_RESTRICT clp,
	      ctimespec_t *PFC_RESTRICT abstime)
{
	pfc_sockaddr_t	*sap = &cnp->icn_sockaddr;
	pfc_iostream_t	stream;
	int		sock, err = 0;

	PFC_ASSERT(sess->icss_state == IPC_SSTATE_READY);

	/* Ensure that the given connection is valid. */
	if (PFC_EXPECT_FALSE(cnp->icn_flags & IPCONNF_CLOSED)) {
		return ESHUTDOWN;
	}

	sess->icss_state = IPC_SSTATE_BUSY;
	IPC_CLSESS_UNLOCK(sess);

	PFC_ASSERT(cnp->icn_current != sess);
	if (cnp->icn_current != NULL) {
		/* Wait for completion of another session. */
		err = ipcconn_wait(cnp, clp, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
	}

	cnp->icn_current = sess;

	/* Determine whether we can use cached connection. */
	stream = cnp->icn_stream;
	if (stream != NULL) {
		ipc_canceller_t	*oldclp = cnp->icn_canceller;

		/* New canceller must be used for ping request. */
		if (oldclp != NULL) {
			IPC_CANCELLER_RELEASE(oldclp);
		}
		cnp->icn_canceller = clp;
		if (oldclp != clp) {
			err = pfc_iostream_setcanceller(stream,
							clp->icl_watch_fd);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error_stream;
			}
		}

		/* Ensure that this connection is still valid. */
		err = pfc_ipcclnt_ping(stream, NULL);
		if (PFC_EXPECT_TRUE(err == 0)) {
			IPC_CONN_UNLOCK(cnp);

			return 0;
		}

		/* Discard broken connection. */
		PFC_ASSERT_INT(pfc_iostream_destroy(cnp->icn_stream), 0);
		cnp->icn_stream = NULL;

		if (err == ECANCELED) {
			/* Canceled by user request. */
			goto error_canceller;
		}
	}

	/* Create a new socket. */
	sock = pfc_sockaddr_open(sap, PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		err = errno;
		goto error_current;
	}

	/* Create a new session stream. */
	err = pfc_ipc_iostream_create(&cnp->icn_sess, sock, clp->icl_watch_fd,
				      &ipcclnt_option);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_current;
	}

	stream = cnp->icn_stream;
	cnp->icn_canceller = clp;
	IPC_CONN_UNLOCK(cnp);

	/* Establish connection. */
	err = pfc_ipcclnt_connect(&cnp->icn_sess, cnp->icn_chan, sap,
				  clp->icl_watch_fd, abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return 0;
	}

	IPC_CONN_LOCK(cnp);

error_stream:
	PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);
	cnp->icn_stream = NULL;

error_canceller:
	cnp->icn_canceller = NULL;

error_current:
	cnp->icn_current = NULL;
	IPC_CONN_SIGNAL(cnp);

error:
	IPC_CLSESS_LOCK(sess);

	return err;
}

/*
 * static int
 * ipcconn_wait(ipc_conn_t *PFC_RESTRICT cnp,
 *		ipc_canceller_t *PFC_RESTRICT clp,
 *		ctimespec_t *PFC_RESTRICT abstime)
 *	Wait for completion of another session on the connection specified
 *	by `cnp'.
 *
 *	This is an internal function of ipcconn_setup().
 *	`clp' and `abstime' passed to ipcconn_setup() must be passed
 *	to this function.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the connection lock.
 *	And the caller must ensure that cnp->icn_current is not NULL.
 */
static int
ipcconn_wait(ipc_conn_t *PFC_RESTRICT cnp, ipc_canceller_t *PFC_RESTRICT clp,
	     ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_clops_t	*clops = clp->icl_ops;
	int		err;

	do {
		/* Wait for completion of another session. */
		err = IPC_CONN_TIMEDWAIT_ABS(cnp, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/*
			 * Although cnp->icn_current may have been changed to
			 * NULL, we don't need to check it because succeeding
			 * communication will get ETIMEDOUT error anyway.
			 */
			PFC_ASSERT(err == ETIMEDOUT);
			break;
		}

		/*
		 * Connection wait must be terminated if the given canceller
		 * is canceled.
		 */
		err = clops->clops_testcancel(clp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* Canceled by user request. */
			break;
		}

		/*
		 * The connection may be closed while the connection lock
		 * is released.
		 */
		if (PFC_EXPECT_FALSE(cnp->icn_flags & IPCONNF_CLOSED)) {
			err = ESHUTDOWN;
			break;
		}
	} while (cnp->icn_current != NULL);

	return err;
}

/*
 * static int
 * ipcconn_invoke(ipc_conn_t *PFC_RESTRICT cnp,
 *		  pfc_ipcsess_t *PFC_RESTRICT sess,
 *		  pfc_ipcresp_t *PFC_RESTRICT respp,
 *		  ctimespec_t *PFC_RESTRICT abstime)
 *	Invoke an IPC service.
 *
 * Calling/Exit State:
 *	Upon successful completion, response code sent by the IPC server is
 *	set to `*respp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcconn_invoke(ipc_conn_t *PFC_RESTRICT cnp, pfc_ipcsess_t *PFC_RESTRICT sess,
	       pfc_ipcresp_t *PFC_RESTRICT respp,
	       ctimespec_t *PFC_RESTRICT abstime)
{
	pfc_iostream_t	stream = cnp->icn_stream;
	pfc_refptr_t	*svname = sess->icss_name;
	ipc_msg_t	*msg = &sess->icss_msg;
	ipc_stream_t	*stp = &sess->icss_output;
	ipc_sess_t	*isp = &cnp->icn_sess;
	ipc_svreq_t	request;
	ipc_svresp_t	response;
	pfc_ipcresp_t	res;
	int32_t		ipcres;
	uint32_t	service = sess->icss_service;
	uint32_t	namelen;
	uint8_t		cmd = IPC_COMMAND_INVOKE;
	int		err;

	PFC_ASSERT(svname != NULL);

	/* Send INVOKE command. */
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Send service request header. */
	namelen = pfc_refptr_string_length(svname);
	request.isrq_namelen = namelen;
	request.isrq_service = service;
	err = pfc_ipc_write(stream, &request, sizeof(request), PFC_FALSE,
			    abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Send module name. */
	err = pfc_ipc_write(stream, pfc_refptr_string_value(svname),
			    namelen, PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Send arguments for IPC service. */
	err = pfc_ipcstream_send(isp, stp, abstime);

	/* IPC stream data is no longer used. */
	IPC_CLSESS_LOCK(sess);
	PFC_ASSERT(sess->icss_state == IPC_SSTATE_BUSY);
	pfc_ipcstream_reset(stp, 0);
	IPC_CLSESS_UNLOCK(sess);

	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Read response code. */
	err = pfc_ipc_read(stream, &response, sizeof(response), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	ipcres = response.isrs_response;
	if (PFC_EXPECT_FALSE(ipcres != 0)) {
		IPC_SESS_BSWAP_INT(isp, ipcres);

		return ipcres;
	}

	res = response.isrs_result;
	IPC_SESS_BSWAP_INT(isp, res);
	*respp = res;

	if (PFC_EXPECT_FALSE(res == PFC_IPCRESP_FATAL)) {
		/* No IPC message should not be sent on fatal error. */
		return 0;
	}

	/* Receive IPC message sent by the IPC server. */
	return pfc_ipcmsg_recv(isp, msg, abstime);
}

/*
 * static int
 * ipcconn_handshake(ipc_sess_t *PFC_RESTRICT isp,
 *		     ipc_clchan_t *PFC_RESTRICT chp,
 *		     ctimespec_t *PFC_RESTRICT abstime)
 *	Send a handshake message to the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcconn_handshake(ipc_sess_t *PFC_RESTRICT isp,
		  ipc_clchan_t *PFC_RESTRICT chp,
		  ctimespec_t *PFC_RESTRICT abstime)
{
	pfc_iostream_t	stream = isp->iss_stream;
	ipc_hshake_t	hshake;
	uint8_t		magic;
	size_t		sz = sizeof(hshake);
	int		err;

	IPC_HSHAKE_INIT(&hshake);

	/* Send a handshake message and credentials to the server. */
	err = __pfc_iostream_sendcred_abs(stream, &hshake, &sz, NULL, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	if (PFC_EXPECT_FALSE(sz != sizeof(hshake))) {
		return EPIPE;
	}

	/* Read response of handshake. */
	err = pfc_ipc_read(stream, &hshake, sizeof(hshake), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	magic = hshake.ih_magic;
	if (PFC_EXPECT_FALSE(magic != IPC_PROTO_MAGIC)) {
		if (magic == IPC_PROTO_MAGIC_TOOMANY) {
			/* Too many clients or session threads. */
			return ENOSPC;
		}

		return EPROTO;
	}

	if (PFC_EXPECT_FALSE(!IPC_ORDER_IS_VALID(hshake.ih_order) ||
			     !IPC_ORDER_IS_VALID(hshake.ih_forder))) {
		return EPROTO;
	}

	IPC_SESS_PROTO_INIT(isp, &hshake);

	return 0;
}

/*
 * static void
 * ipcconn_fork_prepare(ipc_conn_t *cnp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	fork(2) prepare handler for the given connection handle.
 */
static void
ipcconn_fork_prepare(ipc_conn_t *cnp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	pfc_list_t	*elem;

	IPC_CONN_LOCK(cnp);

	PFC_LIST_FOREACH(&cnp->icn_sessions, elem) {
		pfc_ipcsess_t	*sess = IPC_CLSESS_LIST2PTR(elem);

		IPC_CLSESS_LOCK(sess);
	}
}

/*
 * static void
 * ipcconn_fork_parent(ipc_conn_t *cnp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	fork(2) parent handler for the given connection handle.
 *
 * Remarks:
 *	This function must be called with holding the connection lock.
 */
static void
ipcconn_fork_parent(ipc_conn_t *cnp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	pfc_list_t	*elem;

	PFC_LIST_FOREACH(&cnp->icn_sessions, elem) {
		pfc_ipcsess_t	*sess = IPC_CLSESS_LIST2PTR(elem);

		IPC_CLSESS_UNLOCK(sess);
	}

	IPC_CONN_UNLOCK(cnp);
}

/*
 * static void
 * ipcconn_fork_child(ipc_conn_t *cnp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	fork(2) child handler for the given connection handle.
 */
static void
ipcconn_fork_child(ipc_conn_t *cnp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	pfc_iostream_t	stream;
	pfc_list_t	*elem;

	/* Initialize connection lock and condition variable. */
	PFC_ASSERT_INT(PFC_MUTEX_INIT(&cnp->icn_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&cnp->icn_cond), 0);

	/* Discard active connection. */
	stream = cnp->icn_stream;
	if (stream != NULL) {
		__pfc_iostream_dispose(stream);
		cnp->icn_stream = NULL;
	}

	/* Invalidate current session on the connection. */
	cnp->icn_current = NULL;

	/*
	 * Set NULL to icn_canceller.
	 *
	 * - All valid cancellers will be destroyed by
	 *   succeeding call pfc_ipcclnt_canceller_fork_child().
	 *
	 * - If a session-specific canceller is set to icn_canceller,
	 *   it will be destroyed by succeeding call of
	 *   pfc_ipcclnt_canceller_sess_destroy().
	 */
	cnp->icn_canceller = NULL;

	PFC_LIST_FOREACH(&cnp->icn_sessions, elem) {
		pfc_ipcsess_t	*sess = IPC_CLSESS_LIST2PTR(elem);
		ipc_sessclr_t	*sclp = sess->icss_canceller;

		/* Initialize session lock. */
		PFC_ASSERT_INT(PFC_MUTEX_INIT(&sess->icss_mutex), 0);

		/* Invalidate client session. */
		sess->icss_state = IPC_SSTATE_DISCARD;
		sess->icss_busy = 0;
		sess->icss_canceller = NULL;
		if (sclp != NULL) {
			pfc_ipcclnt_canceller_sess_destroy(sclp);
		}
		pfc_ipcclnt_sess_resetmsg(sess);
	}
}

/*
 * static int
 * ipc_cpool_lookup(pfc_ipccpool_t pool, ipc_cpool_t **cplpp)
 *	Search for the connection pool associated with `pool'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to connection pool
 *	instance is set to the buffer pointed by `cplpp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static int
ipc_cpool_lookup(pfc_ipccpool_t pool, ipc_cpool_t **cplpp)
{
	if (pool == PFC_IPCCPOOL_GLOBAL) {
		/* Global pool is specified. */
		*cplpp = &ipc_cpool_global;
		PFC_ASSERT((*cplpp)->icp_id == PFC_IPCCPOOL_GLOBAL);
	}
	else {
		pfc_rbnode_t	*node;

		node = pfc_rbtree_get(&ipcconn_pool, IPC_CPOOL_KEY(pool));
		if (PFC_EXPECT_FALSE(node == NULL)) {
			return ENOENT;
		}

		*cplpp = IPC_CPOOL_NODE2PTR(node);
		PFC_ASSERT((*cplpp)->icp_id != PFC_IPCCPOOL_GLOBAL);
	}

	return 0;
}

/*
 * static int
 * ipc_cpool_getconn(pfc_ipccpool_t pool, const ipc_chaddr_t *PFC_RESTRICT cap,
 *		     ipc_cpool_t **PFC_RESTRICT cplpp,
 *		     pfc_ipcconn_t *PFC_RESTRICT connp)
 *	Search for the alternative connection associated with the IPC channel
 *	address `cap' in the connection pool `pool'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to connection pool
 *	instance is set to the buffer pointed by `cplpp', and a cached
 *	connection handle associated with `cap' is set to the buffer pointed
 *	by `connp', and zero is returned. In this case, the reference counter
 *	of pool entry is incremented.
 *
 *	-1 is returned if a connection associated with `cap' is not cached.
 *	In this case, a non-NULL pointer to connection pool instance is set
 *	to the buffer pointed by `cplpp'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static int
ipc_cpool_getconn(pfc_ipccpool_t pool, const ipc_chaddr_t *PFC_RESTRICT cap,
		  ipc_cpool_t **PFC_RESTRICT cplpp,
		  pfc_ipcconn_t *PFC_RESTRICT connp)
{
	ipc_cpool_t	*cplp;
	pfc_rbnode_t	*node;
	int		err;

	/* Determine connection pool. */
	err = ipc_cpool_lookup(pool, cplpp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	cplp = *cplpp;

	/*
	 * Search for the connection associated with the specified IPC channel
	 * address.
	 */
	node = pfc_rbtree_get(&cplp->icp_pool, (pfc_cptr_t)cap);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		ipc_cpoolent_t	*cpent = IPC_CPOOLENT_NODE2PTR(node);
		ipc_conn_t	*cnp = cpent->icpe_conn;

		/* Return this connection to the caller. */
		ipc_cpool_update_lru(cplp, cpent, cnp);

		/*
		 * The reference counter must be incremented by atomic
		 * operation because this function may be called from
		 * shared client lock section.
		 */
		pfc_atomic_inc_uint32(&cpent->icpe_refcnt);
		*connp = cnp->icn_id;

		return 0;
	}

	return -1;
}

/*
 * static void
 * ipc_cpool_remove(ipc_cpool_t *PFC_RESTRICT cplp,
 *		    ipc_cpoolent_t *PFC_RESTRICT cpent,
 *		    ipc_conn_t *PFC_RESTRICT cnp)
 *	Remove the connection cache entry specified by `cpent' from the
 *	connection pool specified by `cpent'.
 *
 *	The buffer pointed by `cpent' is always freed.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in writer
 *	  mode.
 *
 *	- The connection handle in `cnp' must be closed by the caller.
 */
static void
ipc_cpool_remove(ipc_cpool_t *PFC_RESTRICT cplp,
		 ipc_cpoolent_t *PFC_RESTRICT cpent,
		 ipc_conn_t *PFC_RESTRICT cnp)
{
	PFC_ASSERT(cpent->icpe_conn == cnp);

	/* Remove the connection from the pool. */
	ipc_cpool_unlink(cplp, cnp);
	pfc_rbtree_remove_node(&cplp->icp_pool, &cpent->icpe_node);
	free(cpent);

	PFC_ASSERT(cplp->icp_size != 0);
	cplp->icp_size--;
	cnp->icn_pool = NULL;
	cnp->icn_poolent = NULL;

	/* Remove the connection handle from the connection tree. */
	ipcconn_unlink(&ipcconn_alttree, cnp);
}

/*
 * static void
 * ipc_cpool_iterate(ipc_cpooliter_t iter, pfc_ptr_t arg)
 *	Iterate all connection pools.
 *
 *	Each connection pool and an arbitrary pointer specified by `arg' are
 *	passed to the iterator function specified by `iter'
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	reader or writer mode.
 */
static void
ipc_cpool_iterate(ipc_cpooliter_t iter, pfc_ptr_t arg)
{
	pfc_rbnode_t	*node = NULL;

	(*iter)(&ipc_cpool_global, arg);

	while ((node = pfc_rbtree_next(&ipcconn_pool, node)) != NULL) {
		(*iter)(IPC_CPOOL_NODE2PTR(node), arg);
	}
}

/*
 * static void
 * ipc_cpool_reap(ipc_cpool_t *cplp, pfc_ptr_t arg)
 *	Reap recently unused connections in the pool specified by `cplp'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static void
ipc_cpool_reap(ipc_cpool_t *cplp, pfc_ptr_t arg)
{
	pfc_list_t	*reaped = (pfc_list_t *)arg;
	pfc_list_t	*elem, *next;
	uint32_t	size = cplp->icp_size, cnt;

	PFC_LIST_FOREACH_SAFE(&cplp->icp_lrulist, elem, next) {
		ipc_conn_t	*cnp = IPC_CONN_PLINK2PTR(elem);
		ipc_cpoolent_t	*cpent = cnp->icn_poolent;
		pfc_bool_t	empty;

		if (cpent == NULL || cpent->icpe_refcnt != 0) {
			/* This connection is not cached, or still opened. */
			continue;
		}

		IPC_CONN_LOCK(cnp);
		empty = pfc_list_is_empty(&cnp->icn_sessions);
		IPC_CONN_UNLOCK(cnp);
		if (!empty) {
			/*
			 * At least one client session exists on this
			 * connection.
			 */
			continue;
		}

		if (cpent->icpe_flags & IPC_CPENTF_AGED) {
			/* Reap this connection. */
			ipc_cpool_remove(cnp->icn_pool, cpent, cnp);
			PFC_ASSERT(pfc_list_is_empty(&cnp->icn_plink));
			pfc_list_push_tail(reaped, &cnp->icn_plink);
		}
		else {
			/* Set aged flag. */
			cpent->icpe_flags |= IPC_CPENTF_AGED;
		}
	}

	cnt = size - cplp->icp_size;
	if (cnt != 0) {
		IPCCLNT_LOG_DEBUG("Reaped unused connections: "
				  "pool=%u, count=%u", cplp->icp_id, cnt);
	}
}

/*
 * static void
 * ipc_cpool_reap_forced(ipc_cpool_t *cplp, pfc_ptr_t arg)
 *	Reap all unused connections in the pool specified by `cplp'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static void
ipc_cpool_reap_forced(ipc_cpool_t *cplp, pfc_ptr_t arg)
{
	pfc_list_t	*reaped = (pfc_list_t *)arg;
	pfc_list_t	*elem, *next;
	uint32_t	size = cplp->icp_size, cnt;

	PFC_LIST_FOREACH_SAFE(&cplp->icp_lrulist, elem, next) {
		ipc_conn_t	*cnp = IPC_CONN_PLINK2PTR(elem);
		ipc_cpoolent_t	*cpent = cnp->icn_poolent;

		if (cpent == NULL || cpent->icpe_refcnt != 0) {
			/* This connection is not cached, or still opened. */
			continue;
		}

		/* Reap this connection. */
		ipc_cpool_remove(cnp->icn_pool, cpent, cnp);
		PFC_ASSERT(pfc_list_is_empty(&cnp->icn_plink));
		pfc_list_push_tail(reaped, &cnp->icn_plink);
	}

	cnt = size - cplp->icp_size;
	if (cnt != 0) {
		IPCCLNT_LOG_DEBUG("Reaped unused connections by force: "
				  "pool=%u, count=%u", cplp->icp_id, cnt);
	}
}

/*
 * static pfc_bool_t
 * ipc_cpool_checksize(ipc_cpool_t *PFC_RESTRICT cplp,
 *		       ipc_conn_t **PFC_RESTRICT idlep)
 *	Check the current size of the pool specified by `cplp'.
 *
 *	If the pool is full, this function choose one idle connection in the
 *	pool, and purge it from the pool.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if a new connection can be cached in the pool.
 *	A non-NULL pointer to idle connection handle is set to the buffer
 *	pointed by `idlep' if an idle connection was purged. NULL is set if
 *	no connection was purged.
 *
 *	PFC_FALSE is returned if no more connection can not be cached in the
 *	pool.
 */
static pfc_bool_t
ipc_cpool_checksize(ipc_cpool_t *PFC_RESTRICT cplp,
		    ipc_conn_t **PFC_RESTRICT idlep)
{
	ipc_conn_t	*idle;

	/* Check the capacity of the pool. */
	if (PFC_EXPECT_TRUE(cplp->icp_size < cplp->icp_capacity)) {
		return PFC_TRUE;
	}

	/* Choose one idle connection, and purge it from the pool. */
	idle = ipc_cpool_purgeidle(cplp);
	if (PFC_EXPECT_FALSE(idle == NULL)) {
		return PFC_FALSE;
	}

	PFC_ASSERT(cplp->icp_size < cplp->icp_capacity);
	*idlep = idle;

	return PFC_TRUE;
}

/*
 * static ipc_cpoolent_t *
 * ipc_cpool_getidle(ipc_cpool_t *cplp)
 *	Choose one idle connection in the connection pool specified by `cplp'.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to connection cache entry is returned if found.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static ipc_cpoolent_t *
ipc_cpool_getidle(ipc_cpool_t *cplp)
{
	pfc_list_t	*elem;
	ipc_cpoolent_t	*closed = NULL;

	PFC_LIST_FOREACH(&cplp->icp_lrulist, elem) {
		ipc_conn_t	*cnp = IPC_CONN_PLINK2PTR(elem);
		ipc_cpoolent_t	*cpent = cnp->icn_poolent;
		pfc_bool_t	empty;

		if (cpent == NULL || cpent->icpe_refcnt != 0) {
			/* This connection is not cached, or still opened. */
			continue;
		}
		if (closed == NULL) {
			closed = cpent;
		}

		/*
		 * A connection which has no client session should be
		 * preceded.
		 */
		cnp = cpent->icpe_conn;
		IPC_CONN_LOCK(cnp);
		empty = pfc_list_is_empty(&cnp->icn_sessions);
		IPC_CONN_UNLOCK(cnp);

		if (empty) {
			return cpent;
		}
	}

	/* Return closed connection which has at least one client session. */
	return closed;
}

/*
 * static ipc_conn_t *
 * ipc_cpool_purgeidle(ipc_cpool_t *cplp)
 *	Purge one idle connection in the pool specified by `cplp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to purged connection
 *	handle is returned. Note that it must be closed by the caller.
 *
 *	NULL is returned if there is no idle connection in the pool.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static ipc_conn_t *
ipc_cpool_purgeidle(ipc_cpool_t *cplp)
{
	ipc_cpoolent_t	*cpent = ipc_cpool_getidle(cplp);
	ipc_conn_t	*cnp;

	if (PFC_EXPECT_FALSE(cpent == NULL)) {
		return NULL;
	}

	/* Remove this entry from the pool. */
	cnp = cpent->icpe_conn;
	ipc_cpool_remove(cplp, cpent, cnp);

	return cnp;
}
