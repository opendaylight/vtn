/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * channel.c - IPC channel management for client.
 */

#include <string.h>
#include <pfc/ipc_pfcd.h>
#include <pfc/util.h>
#include <iostream_impl.h>
#include "ipcclnt_impl.h"
#include "ipcclnt_event.h"

/*
 * Internal prototypes.
 */
static int		channel_create(const char *PFC_RESTRICT name,
				       ipc_clchan_t **PFC_RESTRICT chpp);
static int		channel_get(const char *PFC_RESTRICT name,
				    ipc_clchan_t **PFC_RESTRICT chpp);
static ipc_clchan_t	*channel_lookup(const char *name, pfc_bool_t do_hold);
static void		channel_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
static pfc_cptr_t	channel_getkey(pfc_rbnode_t *node);

/*
 * IPC channels.
 */
static pfc_rbtree_t	channel_tree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, channel_getkey);

/*
 * Default IPC channel name.
 */
static char		channel_default[IPC_CHANNEL_NAMELEN_MAX + 1];

/*
 * IPC server's host address of the default channel.
 */
static pfc_hostaddr_t	hostaddr_default;

/*
 * int
 * pfc_ipcclnt_setdefault(const char *name)
 *	Set IPC channel name for the default connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EINVAL is returned if an invalid channel name is specified to `name'.
 *	EBUSY is returned if at least one IPC session exists on the default
 *	connection.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_setdefault(const char *name)
{
	int	err;

	IPC_CLIENT_WRLOCK();
	err = pfc_ipcclnt_conn_setdefault(name, channel_default,
					  &hostaddr_default);
	PFC_ASSERT(strlen(channel_default) < sizeof(channel_default));
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_channel_init(void)
 *	Initialize default channel name.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_channel_init(void)
{
	strcpy(channel_default, PFCD_IPC_CHANNEL);
	PFC_ASSERT_INT(pfc_hostaddr_init_local(&hostaddr_default), 0);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_getchan(const char *PFC_RESTRICT name,
 *		       ipc_clchan_t **PFC_RESTRICT chpp,
 *		       pfc_hostaddr_t *PFC_RESTRICT haddr)
 *	Get channel information associated with the channel address specified
 *	by `name'.
 *
 *	If NULL is specified to `name', information of the default IPC channel
 *	is obtained.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_channel_t is
 *	set to `*chpp', and zero is returned. If `haddr' is not NULL, IPC
 *	server's host address is also set to the buffer pointed by `haddr'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in
 *	  writer mode.
 *
 *	- ipc_clchan_t instance set to `*chpp' must be released using
 *	  IPC_CLCHAN_RELEASE() by the caller.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_getchan(const char *PFC_RESTRICT name,
		    ipc_clchan_t **PFC_RESTRICT chpp,
		    pfc_hostaddr_t *PFC_RESTRICT haddr)
{
	ipc_chaddr_t	chaddr;
	pfc_bool_t	use_hostaddr;
	int		err;

	/* Parse IPC channel address. */
	use_hostaddr = (haddr != NULL) ? PFC_TRUE : PFC_FALSE;
	err = pfc_ipcclnt_chaddr_parse(name, &chaddr, use_hostaddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/*
	 * Search for an IPC channel information, and create new one
	 * if not found.
	 */
	err = channel_get(chaddr.ica_name, chpp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (use_hostaddr) {
		*haddr = chaddr.ica_host;
	}

	return 0;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_getchanbyname(const char *PFC_RESTRICT name,
 *			     ipc_clchan_t **PFC_RESTRICT chpp)
 *	Get channel information associated with the channel name.
 *
 *	If NULL is specified to `name', information of the default IPC channel
 *	is obtained.
 *
 *	Unlike pfc_ipcclnt_getchan(), this function takes IPC channel name,
 *	not IPC channel address.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_channel_t is
 *	set to `*chpp', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in
 *	  writer mode.
 *
 *	- ipc_clchan_t instance set to `*chpp' must be released using
 *	  IPC_CLCHAN_RELEASE() by the caller.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_getchanbyname(const char *PFC_RESTRICT name,
			  ipc_clchan_t **PFC_RESTRICT chpp)
{
	return channel_get(name, chpp);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_chaddr_parse(const char *PFC_RESTRICT addr,
 *			    ipc_chaddr_t *PFC_RESTRICT cap,
 *			    pfc_bool_t use_hostaddr)
 *	Parse the IPC channel address specified `addr'.
 *
 *	If `use_hostaddr' is PFC_FALSE, IPC server's address part in the IPC
 *	channel address is not parsed.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed IPC channel address information is
 *	set into the buffer pointed by `cap', and zero is returned.
 *
 *	Otherwise EINVAL is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_chaddr_parse(const char *PFC_RESTRICT addr,
			 ipc_chaddr_t *PFC_RESTRICT cap,
			 pfc_bool_t use_hostaddr)
{
	const char	*ap, *textaddr = NULL;
	char		*cp;

	if (addr == NULL) {
		/* Use IPC channel address of the default connection. */
		strcpy(cap->ica_name, channel_default);
		PFC_ASSERT(strlen(cap->ica_name) < sizeof(cap->ica_name));

		if (use_hostaddr) {
			cap->ica_host = hostaddr_default;
		}

		return 0;
	}

	/* Parse IPC channel name part in the IPC channel address. */
	for (cp = cap->ica_name, ap = addr; PFC_TRUE; cp++, ap++) {
		char	c;

		if (PFC_EXPECT_FALSE(cp >= PFC_ARRAY_LIMIT(cap->ica_name))) {
			/* Too long IPC channel name. */
			return EINVAL;
		}

		c = *ap;
		if (c == IPC_CHANNEL_ADDR_SEP) {
			*cp = '\0';
			textaddr = ap + 1;
			break;
		}

		*cp = c;
		if (c == '\0') {
			break;
		}
	}

	if (cap->ica_name[0] == '\0') {
		/* Use IPC channel name of the default connection. */
		strcpy(cap->ica_name, channel_default);
		PFC_ASSERT(strlen(cap->ica_name) < sizeof(cap->ica_name));
	}

	if (use_hostaddr) {
		int	gerr;

		/*
		 * Parse IPC server's address part in the IPC channel
		 * address.
		 */
		gerr = pfc_hostaddr_fromstring(&cap->ica_host, textaddr);
		if (PFC_EXPECT_FALSE(gerr != 0)) {
			return EINVAL;
		}
	}

	return 0;
}

/*
 * int
 * pfc_ipcclnt_chaddr_compare(const ipc_chaddr_t *cap1,
 *			      const ipc_chaddr_t *cap2)
 *	Compare the given two IPC channel addresses.
 *
 * Calling/Exit State:
 *	If the given two addresses are identical, zero is returned.
 *	If the first argument is less than the second, a negative value is
 *	returned.
 *	If the first argument is greater than the second, a positive value
 *	is returned.
 *
 * Remarks:
 *	Specifying invalid pointer to IPC channel address, including NULL,
 *	to any argument results in undefined behavior.
 */
int
pfc_ipcclnt_chaddr_compare(const ipc_chaddr_t *cap1, const ipc_chaddr_t *cap2)
{
	int	ret;

	/* Compare IPC channel name. */
	ret = strcmp(cap1->ica_name, cap2->ica_name);
	if (ret == 0) {
		/* Compare IPC host address. */
		ret = pfc_hostaddr_compare(&cap1->ica_host, &cap2->ica_host);
	}

	return ret;
}

/*
 * ipc_clchan_t *
 * pfc_ipcclnt_chan_lookup(const char *name, pfc_bool_t do_hold)
 *	Search for an IPC channel information associated with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non NULL pointer to ipc_clchan_t is
 *	returned.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	- This function must be called with holding the client lock.
 *	  + If `do_hold' is PFC_TRUE, it must be held in writer mode.
 *	  + If `do_hold' is PFC_FALSE, it must be held in reader or writer
 *	    mode.
 *
 *	- Returned ipc_clchan_t is held by IPC_CLCHAN_HOLD() if PFC_TRUE is
 *	  specified to `do_hold'. The caller is responsible for releasing
 *	  the channel.
 */
ipc_clchan_t *
pfc_ipcclnt_chan_lookup(const char *name, pfc_bool_t do_hold)
{
	if (name == NULL) {
		/* Use default channel name. */
		name = channel_default;
	}

	return channel_lookup(name, do_hold);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_chan_destroy(ipc_clchan_t *chp)
 *	Destroy IPC channel information.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_chan_destroy(ipc_clchan_t *chp)
{
	pfc_refptr_put(chp->ichc_name);
	pfc_refptr_put(chp->ichc_path);

	free(chp);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_chan_iterate(ipc_clchiter_t iter, pfc_ptr_t arg)
 *	Iterate all IPC channel cached in the channel_tree.
 *
 *	`arg' is an arbitrary pointer to be passed to the iterator function
 *	specified by `iter'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	reader or writer mode except in case of fork(2) child handler.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_chan_iterate(ipc_clchiter_t iter, pfc_ptr_t arg)
{
	pfc_rbnode_t	*node = NULL;

	while ((node = pfc_rbtree_next(&channel_tree, node)) != NULL) {
		(*iter)(IPC_CLCHAN_NODE2PTR(node), arg);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_chan_cleanup(void)
 *	Clean up IPC channel cache.
 *	This function will be called via library destructor.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_chan_cleanup(void)
{
	int	lock_err = IPC_CLIENT_TRYWRLOCK();

	if (lock_err == 0) {
		pfc_rbtree_clear(&channel_tree, channel_dtor, NULL);
		IPC_CLIENT_UNLOCK();
	}
}

/*
 * static pfc_cptr_t
 * ipcclnt_channel_getkey(pfc_rbnode_t *node)
 *	Return the key of the IPC channel information node.
 *	`node' must be a pointer to ichc_node in ipc_clchan_t.
 */
static pfc_cptr_t
channel_getkey(pfc_rbnode_t *node)
{
	ipc_clchan_t	*chp = IPC_CLCHAN_NODE2PTR(node);

	return IPC_CLCHAN_NAME(chp);
}

/*
 * static void
 * channel_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Destructor of ipc_clchan_t.
 *	`node' must be a pointer to ichc_node in ipc_clchan_t.
 */
static void
channel_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	ipc_clchan_t	*chp = IPC_CLCHAN_NODE2PTR(node);

	IPC_CLCHAN_RELEASE(chp);
}

/*
 * static int
 * channel_create(const char *PFC_RESTRICT name,
 *		  ipc_clchan_t **PFC_RESTRICT chpp)
 *	Create a new IPC channel information.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_channel_t is
 *	set to `*chpp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
static int
channel_create(const char *PFC_RESTRICT name,
	       ipc_clchan_t **PFC_RESTRICT chpp)
{
	ipc_clchan_t	*chp;
	ipc_chconf_t	chconf;
	int		err;

	chp = (ipc_clchan_t *)malloc(sizeof(*chp));
	if (PFC_EXPECT_FALSE(chp == NULL)) {
		return ENOMEM;
	}

	chp->ichc_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(chp->ichc_name == NULL)) {
		err = ENOMEM;
		goto error;
	}

	PFC_ASSERT_INT(pfc_rwlock_init(&chp->ichc_lock), 0);

	/*
	 * Reference counter must be initialized as 2. One is for reference
	 * from channel_tree, and another is reference from the caller.
	 */
	chp->ichc_refcnt = 2;

	/* Determine socket file path and temporary directory path. */
	err = pfc_ipc_conf_getpath(name, &chp->ichc_path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Load channel configuration for client. */
	pfc_ipc_conf_getcliconf(name, &chconf);
	chp->ichc_timeout = chconf.icc_timeout;

	pfc_ipcclnt_evchan_init(chp);

	/* Register channel information. */
	err = pfc_rbtree_put(&channel_tree, &chp->ichc_node);
	if (PFC_EXPECT_TRUE(err == 0)) {
		*chpp = chp;

		return 0;
	}

	PFC_ASSERT(err == EEXIST);

error:
	if (chp->ichc_name != NULL) {
		pfc_refptr_put(chp->ichc_name);
	}
	free(chp);

	return err;
}

/*
 * static int
 * channel_get(const char *PFC_RESTRICT name, ipc_clchan_t **PFC_RESTRICT chpp)
 *	Search for an IPC channel information associated with the given name.
 *	If not found, create a new one.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_channel_t is
 *	set to `*chpp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in
 *	  writer mode.
 *
 *	- ipc_clchan_t instance set to `*chpp' must be released using
 *	  IPC_CLCHAN_RELEASE() by the caller.
 */
static int
channel_get(const char *PFC_RESTRICT name, ipc_clchan_t **PFC_RESTRICT chpp)
{
	ipc_clchan_t	*chp;
	int		err;

	/* Try to use cached information. */
	chp = channel_lookup(name, PFC_TRUE);
	if (chp == NULL) {
		/* Create a new channel information. */
		err = channel_create(name, chpp);
	}
	else {
		*chpp = chp;
		err = 0;
	}

	return err;
}

/*
 * static ipc_clchan_t *
 * channel_lookup(const char *name, pfc_bool_t do_hold)
 *	Search for an IPC channel information associated with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non NULL pointer to ipc_clchan_t is
 *	returned.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	- This function must be called with holding the client lock.
 *	  + If `do_hold' is PFC_TRUE, it must be held in writer mode.
 *	  + If `do_hold' is PFC_FALSE, it must be held in reader or writer
 *	    mode.
 *
 *	- Returned ipc_clchan_t is held by IPC_CLCHAN_HOLD() if PFC_TRUE is
 *	  specified to `do_hold'. The caller is responsible for releasing
 *	  the channel.
 */
static ipc_clchan_t *
channel_lookup(const char *name, pfc_bool_t do_hold)
{
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(&channel_tree, name);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		ipc_clchan_t	*chp = IPC_CLCHAN_NODE2PTR(node);

		if (do_hold) {
			IPC_CLCHAN_HOLD(chp);
		}

		return chp;
	}

	return NULL;
}
