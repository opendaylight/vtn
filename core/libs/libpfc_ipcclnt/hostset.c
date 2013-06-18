/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * hostset.c - Management of IPC server address set.
 */

#include <string.h>
#include <netdb.h>
#include <pfc/util.h>
#include "ipcclnt_event.h"

/*
 * Valid creation flags.
 */
#define	HOSTSET_VALID_FLAGS	(PFC_IPCHSET_NEW | PFC_IPCHSET_EXCL)

#define	HOSTSET_FLAGS_IS_VALID(flags)				\
	PFC_EXPECT_TRUE(((flags) & ~HOSTSET_VALID_FLAGS) == 0)

/*
 * Internal prototypes.
 */
static int	ipc_hostset_lookup(const char *PFC_RESTRICT name,
				   ipc_hostset_t **PFC_RESTRICT hsetp);
static void	ipc_hostset_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static void	ipc_hostent_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static pfc_cptr_t	ipc_hostset_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_hostent_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_chref_getkey(pfc_rbnode_t *node);

/*
 * Set of ipc_hostset_t.
 * This tree must be synchronized with holding the global client lock.
 */
static pfc_rbtree_t	ipc_hostset_tree =
	IPC_RBTREE_INITIALIZER(strcmp, ipc_hostset_getkey);

/*
 * Static host set which contains only local host address.
 */
static ipc_hostent_t	ipc_hostent_local;

ipc_hostset_t		ipc_hostset_local PFC_ATTR_HIDDEN = {
	.hs_lock	= PFC_RWLOCK_INITIALIZER,
	.hs_host	= IPC_RBTREE_INITIALIZER(pfc_hostaddr_compare,
						 ipc_hostent_getkey),
	.hs_chref	= IPC_RBTREE_INITIALIZER(pfc_rbtree_ulong_compare,
						 ipc_chref_getkey),
	.hs_refcnt	= 1,
};

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_hostset_init(void)
 *	Initialize the static local host set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_hostset_init(void)
{
	ipc_hostset_t	*hset = &ipc_hostset_local;
	ipc_hostent_t	*hep = &ipc_hostent_local;
	int		err;

	if (PFC_EXPECT_FALSE(!pfc_rbtree_isempty(&hset->hs_host))) {
		/* Already initialized. */
		return 0;
	}

	PFC_ASSERT_INT(pfc_hostaddr_init_local(&hep->he_addr), 0);

	/*
	 * Create dummy name of static host set.
	 * Although static host set is not visible, it must be named.
	 */
	hset->hs_name = pfc_refptr_string_create("<local>");
	if (PFC_EXPECT_FALSE(hset->hs_name == NULL)) {
		IPCCLNT_LOG_ERROR("Unable to create local host set name.");

		return ENOMEM;
	}

	err = pfc_rbtree_put(&hset->hs_host, &hep->he_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Unable to initialize local host set: %s",
				  strerror(err));
		pfc_refptr_put(hset->hs_name);
		hset->hs_name = NULL;
	}

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_hostset_fini(void)
 *	Destroy all host set.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_hostset_fini(void)
{
	pfc_refptr_t	*rname = ipc_hostset_local.hs_name;

	/* Destroy all host set if possible. */
	pfc_rbtree_clear(&ipc_hostset_tree, ipc_hostset_dtor, NULL);

	/* Destroy static local host set. */
	if (rname != NULL) {
		pfc_refptr_put(rname);
		ipc_hostset_local.hs_name = NULL;
	}
}

/*
 * int
 * pfc_ipcclnt_hostset_create(const char *name)
 *	Create a new host set, and associated it with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_hostset_create(const char *name)
{
	ipc_hostset_t	*hset;
	int		err;

	/* Verify name. */
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	err = pfc_ipc_checkname(IPC_NAME_HOSTSET, name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Allocate buffer for a new host set. */
	hset = (ipc_hostset_t *)malloc(sizeof(*hset));
	if (PFC_EXPECT_FALSE(hset == NULL)) {
		return ENOMEM;
	}

	hset->hs_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(hset->hs_name == NULL)) {
		err = ENOMEM;
		goto error;
	}

	err = pfc_rwlock_init(&hset->hs_lock);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	pfc_rbtree_init(&hset->hs_host, (pfc_rbcomp_t)pfc_hostaddr_compare,
			ipc_hostent_getkey);
	pfc_rbtree_init(&hset->hs_chref, pfc_rbtree_ulong_compare,
			ipc_chref_getkey);
	hset->hs_refcnt = 1;

	IPC_CLIENT_WRLOCK();

	err = ipc_event_is_running();
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_CLIENT_UNLOCK();
		goto error;
	}

	err = pfc_rbtree_put(&ipc_hostset_tree, &hset->hs_node);
	IPC_CLIENT_UNLOCK();

	if (PFC_EXPECT_TRUE(err == 0)) {
		return 0;
	}

	PFC_ASSERT(err == EEXIST);

error:
	if (hset->hs_name != NULL) {
		pfc_refptr_put(hset->hs_name);
	}
	free(hset);

	return err;
}

/*
 * int
 * pfc_ipcclnt_hostset_destroy(const char *name)
 *	Destroy IPC host set associated with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_hostset_destroy(const char *name)
{
	ipc_hostset_t	*hset;
	int		err;

	IPC_CLIENT_WRLOCK();

	err = ipc_event_is_available();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	err = ipc_hostset_lookup(name, &hset);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	pfc_rbtree_remove_node(&ipc_hostset_tree, &hset->hs_node);
	IPC_CLIENT_UNLOCK();

	/* Release the host set. */
	IPC_HOSTSET_RELEASE(hset);

	return 0;

error:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcclnt_hostset_exists(const char *name)
 *	Check whether the IPC host set associated with the given name exists
 *	or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the host set is associated with the given name.
 *	ENODEV is returned if no host set is associated with the given name.
 *	EINVAL is returned if NULL is specified to `name'.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_hostset_exists(const char *name)
{
	ipc_hostset_t	*hset;
	int		err;

	IPC_CLIENT_RDLOCK();

	err = ipc_event_is_available();
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = ipc_hostset_lookup(name, &hset);
	}

	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcclnt_hostset_add(const char *PFC_RESTRICT name,
 *			   const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Add a host entry to the host set specified by the given name.
 *
 *	A new IPC event listener session will be started if the specified host
 *	set is used by event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_hostset_add(const char *PFC_RESTRICT name,
			const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	ipc_hostset_t	*hset;
	ipc_hostent_t	*hep;
	ipc_clchan_t	*chp_error = NULL;
	pfc_rbnode_t	*node;
	int		err, family;

	family = pfc_hostaddr_gettype(addr);
	if (PFC_EXPECT_FALSE(family == AF_UNSPEC)) {
		return EINVAL;
	}

	if (PFC_EXPECT_FALSE(family != AF_UNIX)) {
		/* Currently remote host is not supported. */
		return EAFNOSUPPORT;
	}

	/* Allocate a new host entry. */
	hep = (ipc_hostent_t *)malloc(sizeof(*hep));
	if (PFC_EXPECT_FALSE(hep == NULL)) {
		return ENOMEM;
	}

	hep->he_addr = *addr;

	IPC_CLIENT_WRLOCK();

	err = ipc_event_is_running();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Search for the host set associated with the given name. */
	err = ipc_hostset_lookup(name, &hset);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Add the host address to the host set. */
	IPC_HOSTSET_WRLOCK(hset);
	err = pfc_rbtree_put(&hset->hs_host, &hep->he_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_HOSTSET_UNLOCK(hset);
		PFC_ASSERT(err == EEXIST);
		goto error;
	}
	IPC_HOSTSET_UNLOCK(hset);

	/* Set up event listener sessions for a new host. */
	node = NULL;
	while ((node = pfc_rbtree_next(&hset->hs_chref, node)) != NULL) {
		ipc_chref_t	*crp = IPC_CHREF_CHAN_NODE2PTR(node);
		ipc_clchan_t	*chp = crp->cr_chan;

		PFC_ASSERT(crp->cr_hostset == hset);
		PFC_ASSERT(crp->cr_refcnt != 0);
		err = pfc_ipcclnt_evchan_addhost(chp, hset, addr);
		if (PFC_EXPECT_FALSE(err != 0)) {
			chp_error = chp;
			goto error_sess;
		}
	}

	IPC_CLIENT_UNLOCK();

	return 0;

error_sess:
	pfc_log_burst_error_begin();
	if (ipcclnt_log_enabled) {
		char	addrbuf[PFC_HOSTADDR_STRSIZE];
		int	e;

		e = pfc_hostaddr_tostring(addr, addrbuf, sizeof(addrbuf),
					  PFC_HA2STR_TYPE);
		if (PFC_EXPECT_FALSE(e != 0)) {
			(void)pfc_strlcpy(addrbuf, "???", sizeof(addrbuf));
		}

		pfc_log_burst_write("%s@%s: Failed to set up session for new "
				    "host: hostset=%s: %s",
				    IPC_CLCHAN_NAME(chp_error),
				    addrbuf, name, strerror(err));
	}
	pfc_log_burst_end();

	/* Revert changes to host set. */
	IPC_HOSTSET_WRLOCK(hset);
	pfc_rbtree_remove_node(&hset->hs_host, &hep->he_node);
	IPC_HOSTSET_UNLOCK(hset);

	/* Revert listener sessions. */
	node = NULL;
	while ((node = pfc_rbtree_next(&hset->hs_chref, node)) != NULL) {
		ipc_chref_t	*crp = IPC_CHREF_CHAN_NODE2PTR(node);
		ipc_clchan_t	*chp = crp->cr_chan;

		if (chp == chp_error) {
			break;
		}
		pfc_ipcclnt_evchan_removehost(chp, hset, addr);
	}

error:
	IPC_CLIENT_UNLOCK();

	free(hep);

	return err;
}

/*
 * int
 * pfc_ipcclnt_hostset_remove(const char *PFC_RESTRICT name,
 *			      const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Remove a host entry from the host set specified by the given name.
 *
 *	The IPC event listener session specified by `addr' will be stopped
 *	if the specified host set is used by event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_hostset_remove(const char *PFC_RESTRICT name,
			   const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	ipc_hostset_t	*hset;
	pfc_rbnode_t	*node, *enode;
	int		err;

	if (PFC_EXPECT_FALSE(pfc_hostaddr_gettype(addr) == AF_UNSPEC)) {
		return EINVAL;
	}

	IPC_CLIENT_RDLOCK();

	err = ipc_event_is_available();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Search for the host set associated with the given name. */
	err = ipc_hostset_lookup(name, &hset);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	IPC_HOSTSET_WRLOCK(hset);

	/* Remove the host entry. */
	enode = pfc_rbtree_remove(&hset->hs_host, (pfc_cptr_t)addr);
	if (PFC_EXPECT_FALSE(enode == NULL)) {
		IPC_HOSTSET_UNLOCK(hset);
		err = ENOENT;
		goto error;
	}
	IPC_HOSTSET_UNLOCK(hset);

	/* Destroy the listener session associated with this host. */
	node = NULL;
	while ((node = pfc_rbtree_next(&hset->hs_chref, node)) != NULL) {
		ipc_chref_t	*crp = IPC_CHREF_CHAN_NODE2PTR(node);
		ipc_clchan_t	*chp = crp->cr_chan;

		PFC_ASSERT(crp->cr_hostset == hset);
		PFC_ASSERT(crp->cr_refcnt != 0);
		pfc_ipcclnt_evchan_removehost(chp, hset, addr);
	}

	IPC_CLIENT_UNLOCK();

	ipc_hostent_dtor(enode, NULL);

	return 0;

error:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcclnt_hostset_contains(const char *PFC_RESTRICT name,
 *				const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Check whether the host address specified by `addr' exists in the
 *	host set associated with `name'.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified address is contained by the host
 *	set associated with `name'.
 *
 *	ENODEV is returned if no host set is associated with `name'.
 *	ENOENT is returned if `addr' is not contained by the host set.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_hostset_contains(const char *PFC_RESTRICT name,
			     const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	ipc_hostset_t	*hset;
	int		err;

	if (PFC_EXPECT_FALSE(pfc_hostaddr_gettype(addr) == AF_UNSPEC)) {
		return EINVAL;
	}

	IPC_CLIENT_RDLOCK();

	err = ipc_event_is_available();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Search for the host set associated with the given name. */
	err = ipc_hostset_lookup(name, &hset);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = ipc_hostset_contains(hset, addr);
	}

out:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_hostset_lookup(const char *name, ipc_hostset_t **hsetp)
 *	Search for the IPC host set associated with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to host set instance
 *	is set to the buffer pointed by `hsetp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or write mode.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_hostset_lookup(const char *name, ipc_hostset_t **hsetp)
{
	return ipc_hostset_lookup(name, hsetp);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_hostset_destroy_impl(ipc_hostset_t *hset)
 *	Destroy the specified IPC host set instance.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_hostset_destroy_impl(ipc_hostset_t *hset)
{
	IPC_HOSTSET_WRLOCK(hset);
	pfc_rbtree_clear(&hset->hs_host, ipc_hostent_dtor, NULL);
	IPC_HOSTSET_UNLOCK(hset);

	pfc_refptr_put(hset->hs_name);
	free(hset);
}

/*
 * static int
 * ipc_hostset_lookup(const char *PFC_RESTRICT name,
 *		      ipc_hostset_t **PFC_RESTRICT hsetp)
 *	Search for the IPC host set associated with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to host set instance
 *	is set to the buffer pointed by `hsetp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or write mode.
 */
static int
ipc_hostset_lookup(const char *PFC_RESTRICT name,
		   ipc_hostset_t **PFC_RESTRICT hsetp)
{
	pfc_rbnode_t	*node;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	node = pfc_rbtree_get(&ipc_hostset_tree, (pfc_cptr_t)name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ENODEV;
	}

	*hsetp = IPC_HOSTSET_NODE2PTR(node);

	return 0;
}

/*
 * static void
 * ipc_hostset_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of IPC host set.
 *	`node' must be a pointer to hs_node in ipc_hostset_t.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static void
ipc_hostset_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	ipc_hostset_t	*hset = IPC_HOSTSET_NODE2PTR(node);

	IPC_HOSTSET_RELEASE(hset);
}

/*
 * static void
 * ipc_hostent_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of IPC host entry.
 *	`node' must be a pointer to hn_node in ipc_hostent_t.
 */
static void
ipc_hostent_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	ipc_hostent_t	*hep = IPC_HOSTENT_NODE2PTR(node);

	free(hep);
}

/*
 * static pfc_cptr_t
 * ipc_hostset_getkey(pfc_rbnode_t *node)
 *	Return the key of IPC host set.
 *	`node' must be a pointer to hs_node in ipc_hostset_t.
 */
static pfc_cptr_t
ipc_hostset_getkey(pfc_rbnode_t *node)
{
	ipc_hostset_t	*hset = IPC_HOSTSET_NODE2PTR(node);

	return (pfc_cptr_t)pfc_refptr_string_value(hset->hs_name);
}

/*
 * static pfc_cptr_t
 * ipc_hostent_getkey(pfc_rbnode_t *node)
 *	Return the key of IPC host entry.
 *	`node' must be a pointer to hn_node in ipc_hostent_t.
 */
static pfc_cptr_t
ipc_hostent_getkey(pfc_rbnode_t *node)
{
	ipc_hostent_t	*hep = IPC_HOSTENT_NODE2PTR(node);

	return (pfc_cptr_t)&hep->he_addr;
}

/*
 * static pfc_cptr_t
 * ipc_chref_getkey(pfc_rbnode_t *node)
 *	Return the key of IPC channel reference.
 *	`node' must be a pointer to cr_chnode in ipc_chref_t.
 */
static pfc_cptr_t
ipc_chref_getkey(pfc_rbnode_t *node)
{
	ipc_chref_t	*crp = IPC_CHREF_CHAN_NODE2PTR(node);

	return (pfc_cptr_t)crp->cr_chan;
}
