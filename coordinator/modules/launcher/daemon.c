/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * daemon.c - Daemon process management.
 */

#include <string.h>
#include <unc/lnc_ipc.h>
#include <pfc/property.h>
#include "launcher_impl.h"

/*
 * Name of stderr logging directory.
 */
#define	LNC_STDERR_LOGDIR		"stderr"

/*
 * Name of stderr log file.
 */
#define	LNC_STDERR_LOGFILE_DAEMON	"daemon.log"
#define	LNC_STDERR_LOGFILE_STOP		"stop.log"

/*
 * Permission bits for stderr logging directory.
 */
#define	LNC_STDERR_LOGDIR_PERM		(S_IRWXU)		/* 0700 */
/*
 * Permission bits for stderr log file.
 */
#define	LNC_STDERR_LOGFILE_PERM		(S_IRUSR | S_IWUSR)	/* 0600 */

/*
 * open(2) flags to open stderr log file.
 */
#define	LNC_STDERR_OFLAGS		(O_WRONLY | O_TRUNC | O_CREAT)

/*
 * Timeout of child process clean up.
 */
#define	LNC_CLEANUP_TIMEOUT		PFC_CONST_U(5)		/* 5 seconds */

/*
 * Internal prototypes.
 */
static int	daemon_create(lnc_conf_t *cfp);
static int	daemon_setup(lnc_daemon_t *ldp);
static void	daemon_destroy(pfc_rbnode_t *node, pfc_ptr_t arg);
static int	daemon_start(lnc_daemon_t *ldp);
static int	daemon_start_wait(lnc_daemon_t *UNC_RESTRICT ldp,
				  lnc_proc_t *UNC_RESTRICT daemon);
static int	daemon_stop(lnc_daemon_t *ldp);
static int	daemon_stop_exec(lnc_daemon_t *UNC_RESTRICT ldp,
				 const pfc_timespec_t *UNC_RESTRICT abstime);
static int	daemon_killall(lnc_ctx_t *ctx, int sig);
static int	daemon_waitall(lnc_ctx_t *UNC_RESTRICT ctx,
			       const pfc_timespec_t *UNC_RESTRICT abstime);
static void	daemon_getinfo(lnc_daemon_t *UNC_RESTRICT ldp,
			       lnc_dminfo_t *UNC_RESTRICT dip);

static void	daemon_conf_error(lnc_conf_t *UNC_RESTRICT cfp,
				  const char *UNC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);

static int	daemon_proc_create(lnc_proc_t **UNC_RESTRICT lprpp,
				   lnc_daemon_t *UNC_RESTRICT ldp,
				   lnc_conf_t *UNC_RESTRICT cfp,
				   const char *UNC_RESTRICT name,
				   const char *UNC_RESTRICT key);
static int	daemon_proc_setup(lnc_daemon_t *UNC_RESTRICT ldp,
				  lnc_proc_t *UNC_RESTRICT lprp);
static int	daemon_proc_setup_uncd(lnc_daemon_t *UNC_RESTRICT ldp,
				       lnc_proc_t *UNC_RESTRICT lprp,
				       pfc_extcmd_t ecmd);
static int	daemon_proc_ioinit(lnc_proc_t *lprp);
static int	daemon_proc_destroy(lnc_proc_t *lprp);
static int	daemon_proc_start_l(lnc_ctx_t *UNC_RESTRICT ctx,
				    lnc_daemon_t *UNC_RESTRICT ldp,
				    const char *UNC_RESTRICT dname,
				    lnc_proc_t *UNC_RESTRICT lprp);
static int	daemon_proc_wait(lnc_daemon_t *UNC_RESTRICT ldp,
				 lnc_proc_t *UNC_RESTRICT lprp,
				 const pfc_timespec_t *UNC_RESTRICT abstime);
static void	daemon_proc_callback(pfc_extcmd_t ecmd,
				     const pfc_extcmd_proc_t *proc,
				     pfc_ptr_t arg);

static int	daemon_stderr_init(lnc_daemon_t *UNC_RESTRICT ldp,
				   lnc_proc_t *UNC_RESTRICT lprp,
				   lnc_conf_t *UNC_RESTRICT cfp,
				   const char *UNC_RESTRICT fname, int rotate);
static void	daemon_stderr_destroy(lnc_stderr_t *lsep);
static int	daemon_stderr_rotate(lnc_proc_t *UNC_RESTRICT lprp,
				     lnc_stderr_t *UNC_RESTRICT lsep);

static int	daemon_order_put(lnc_conf_t *UNC_RESTRICT cfp,
				 pfc_cfblk_t daemon,
				 lnc_daemon_t *UNC_RESTRICT ldp,
				 const char *UNC_RESTRICT param,
				 lnc_clevfunc_t clevent, lnc_ordtype_t type,
				 int index);
static void	daemon_order_destroy(pfc_rbnode_t *node, pfc_ptr_t arg);

static int	daemon_clevent_init(void);

static pfc_cptr_t	daemon_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	daemon_order_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	daemon_proc_getkey(pfc_rbnode_t *node);

/*
 * Red-Black tree which keeps daemon instances indexed by name.
 */
pfc_rbtree_t	daemon_tree PFC_ATTR_HIDDEN;

/*
 * Red-Black tree which keeps daemon instances indexed by UINT32 order value.
 * Array index must be lnc_ordtype_t value.
 */
pfc_rbtree_t	daemon_order_tree[LNC_ORDER_NTYPES] PFC_ATTR_HIDDEN;

/*
 * Red-Black tree which keeps processes spawned by the launcher module.
 * Any access to this tree must be serialized by the launcher lock.
 */
static pfc_rbtree_t	daemon_proc_tree;

/*
 * True if no child process is running, and no child process callback function
 * is being called.
 */
#define	LNC_DAEMON_HAS_NO_CHILD(ctx)					\
	((ctx)->lc_ncallbacks == 0 && pfc_rbtree_isempty(&daemon_proc_tree))

/*
 * List of daemons per process type.
 * Array index must be process type.
 */
static lnc_idmlist_t	daemon_type_list[LNC_NPROCTYPES];

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * daemon_proc_assert_empty(void)
 *	Assertion which ensures that no child process is active.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
daemon_proc_assert_empty(void)
{
#ifdef	UNC_VERBOSE_DEBUG
	lnc_ctx_t	*ctx = &launcher_ctx;

	LNC_LOCK(ctx);
	PFC_ASSERT(LNC_DAEMON_HAS_NO_CHILD(ctx));
	LNC_UNLOCK(ctx);
#endif	/* UNC_VERBOSE_DEBUG */
}

/*
 * static inline lnc_idmlist_t PFC_FATTR_ALWAYS_INLINE *
 * daemon_type_getlist(lnc_proctype_t type)
 *	Return the daemon list associated with the given process type.
 *	NULL is returned if the given type is invalid.
 */
static inline lnc_idmlist_t PFC_FATTR_ALWAYS_INLINE *
daemon_type_getlist(lnc_proctype_t type)
{
	if (PFC_EXPECT_FALSE(type >= PFC_ARRAY_CAPACITY(daemon_type_list))) {
		return NULL;
	}

	return &daemon_type_list[type];
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * daemon_type_addlist(lnc_daemon_t *ldp)
 * 	Add the daemon instance to the daemon type list.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
daemon_type_addlist(lnc_daemon_t *ldp)
{
	lnc_idmlist_t	*idlp = daemon_type_getlist(ldp->ld_type);

	PFC_ASSERT(idlp != NULL);
	idlp->idml_count++;
	pfc_list_push_tail(&idlp->idml_list, &ldp->ld_typelist);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * daemon_proc_start(lnc_daemon_t *UNC_RESTRICT ldp,
 *		     const char *UNC_RESTRICT dname,
 *		     lnc_proc_t *UNC_RESTRICT lprp)
 *	Start the given process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
daemon_proc_start(lnc_daemon_t *UNC_RESTRICT ldp,
		  const char *UNC_RESTRICT dname,
		  lnc_proc_t *UNC_RESTRICT lprp)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	int		err;

	LNC_LOCK(ctx);
	LNC_PROC_LOCK(lprp);
	err = daemon_proc_start_l(ctx, ldp, dname, lprp);
	LNC_PROC_UNLOCK(lprp);
	LNC_UNLOCK(ctx);

	return err;
}

/*
 * int
 * lncapi_dmlist_create(lnc_proctype_t type, lnc_dmlist_t **UNC_RESTRICT listpp,
 *			const pfc_timespec_t *UNC_RESTRICT tout)
 *	Get information of daemons associated with the given process type.
 *
 *	This function will block until all daemon process are launched by
 *	the launcher module. `tout' specifies an upper limit of amount of
 *	time for which lncapi_dmlist_create() will block. Specifying a NULL to
 *	`tout' means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to lnc_dmlist_t
 *	is set to the buffer pointed by `listpp', and zero is returned.
 *	Otherwise error number which indicates cause of error is returned.
 *
 * Remarks:
 *	A pointer to lnc_dmlist_t must be freed by the call of
 *	lncapi_dmlist_destroy().
 */
int
lncapi_dmlist_create(lnc_proctype_t type, lnc_dmlist_t **UNC_RESTRICT listpp,
		     const pfc_timespec_t *UNC_RESTRICT tout)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_dmlist_t	*listp;
	lnc_dminfo_t	*dip;
	lnc_idmlist_t	*idlp;
	pfc_timespec_t	abstime, *abs;
	pfc_list_t	*elem;
	uint32_t	count;
	int		err;

	if (PFC_EXPECT_FALSE(listpp == NULL)) {
		return EINVAL;
	}

	idlp = daemon_type_getlist(type);
	if (PFC_EXPECT_FALSE(idlp == NULL)) {
		return EINVAL;
	}

	listp = (lnc_dmlist_t *)malloc(sizeof(*listp));
	if (PFC_EXPECT_FALSE(listp == NULL)) {
		return ENOMEM;
	}

	count = idlp->idml_count;
	if (PFC_EXPECT_TRUE(count > 0)) {
		size_t	sz = count * sizeof(*dip);

		sz = count * sizeof(*dip);
		dip = (lnc_dminfo_t *)malloc(sz);
		if (PFC_EXPECT_FALSE(dip == NULL)) {
			free(listp);

			return ENOMEM;
		}
	}
	else {
		dip = NULL;
	}

	listp->dml_count = count;
	listp->dml_info = dip;

	if (tout == NULL) {
		/* Infinite timeout. */
		abs = NULL;
	}
	else {
		abs = &abstime;
		PFC_ASSERT_INT(pfc_clock_abstime(abs, tout), 0);
	}

	LNC_LOCK(ctx);

	/* Wait for all daemons to be launched. */
	err = lnc_wait(ctx, abs);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Copy daemon information. */
	PFC_LIST_FOREACH(&idlp->idml_list, elem) {
		lnc_daemon_t	*ldp = LNC_DAEMON_LIST2PTR(elem);

		daemon_getinfo(ldp, dip);
		dip++;
	}

	LNC_UNLOCK(ctx);

	PFC_ASSERT(dip == listp->dml_info + listp->dml_count);

	*listpp = listp;

	return 0;

error:
	LNC_UNLOCK(ctx);
	free(listp->dml_info);
	free(listp);

	return err;
}

/*
 * void
 * lncapi_dmlist_destroy(lnc_dmlist_t *listp)
 *	Free lnc_dmlist_t created by the call of lncapi_dmlist_create().
 */
void
lncapi_dmlist_destroy(lnc_dmlist_t *listp)
{
	if (PFC_EXPECT_TRUE(listp != NULL)) {
		PFC_ASSERT(listp->dml_count != 0 || listp->dml_info == NULL);
		free(listp->dml_info);
		free(listp);
	}
}

/*
 * int
 * lncapi_dmlist_get(lnc_dmlist_t *UNC_RESTRICT listp, uint32_t index,
 *		     lnc_dminfo_t *UNC_RESTRICT infop)
 *	Fetch daemon information in the daemon list at the given index.
 *
 *	`listp' must be a pointer to lnc_dmlist_t obtained by the call of
 *	lncapi_dmlist_create().
 *	`index' is a list index, starts from zero, which specifies daemon
 *	information in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, daemon information specified by `listp'
 *	and `index' is copied to the buffer pointed by `infop', and zero is
 *	returned.
 *	Otherwise error number which indicates cause of error is returned.
 */
int
lncapi_dmlist_get(lnc_dmlist_t *UNC_RESTRICT listp, uint32_t index,
		  lnc_dminfo_t *UNC_RESTRICT infop)
{
	if (PFC_EXPECT_FALSE(listp == NULL || infop == NULL ||
			     index >= listp->dml_count)) {
		return EINVAL;
	}

	*infop = *(listp->dml_info + index);

	return 0;
}

/*
 * uint32_t
 * lncapi_dmlist_getsize(lnc_dmlist_t *listp)
 *	Return the number of daemons in the given daemon list.
 *	UINT32_MAX is returned if NULL is specified to `listp'.
 */
uint32_t
lncapi_dmlist_getsize(lnc_dmlist_t *listp)
{
	return (PFC_EXPECT_FALSE(listp == NULL))
		? UINT32_MAX
		: listp->dml_count;
}

/*
 * int
 * lncapi_getdminfo(const char *UNC_RESTRICT name,
 *		    lnc_dminfo_t *UNC_RESTRICT infop,
 *		    const pfc_timespec_t *UNC_RESTRICT tout)
 *	Get daemon information specified by the daemon name.
 *
 *	This function will block until all daemon process are launched by
 *	the launcher module. `tout' specifies an upper limit of amount of
 *	time for which lncapi_getdminfo() will block. Specifying a NULL to
 *	`tout' means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, daemon information assocaited with the
 *	daemon name `name' is copied to the buffer pointed by `infop',
 *	and zero is returned.
 *	Otherwise error number which indicates cause of error is returned.
 */
int
lncapi_getdminfo(const char *UNC_RESTRICT name,
		 lnc_dminfo_t *UNC_RESTRICT infop,
		 const pfc_timespec_t *UNC_RESTRICT tout)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_daemon_t	*ldp;
	pfc_timespec_t	abstime, *abs;
	pfc_rbnode_t	*node;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL || infop == NULL)) {
		return EINVAL;
	}

	if (tout == NULL) {
		/* Infinite timeout. */
		abs = NULL;
	}
	else {
		abs = &abstime;
		PFC_ASSERT_INT(pfc_clock_abstime(abs, tout), 0);
	}

	LNC_LOCK(ctx);

	/* Wait for all daemons to be launched. */
	err = lnc_wait(ctx, abs);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Search for a daemon instance associated with the given name. */
	node = pfc_rbtree_get(&daemon_tree, name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		err = ENOENT;
		goto out;
	}

	ldp = LNC_DAEMON_NODE2PTR(node);
	daemon_getinfo(ldp, infop);

out:
	LNC_UNLOCK(ctx);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_daemon_boot(void)
 *	Load daemon configurations.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_daemon_boot(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_confctx_t	*conf;
	lnc_idmlist_t	*idlp;
	pfc_rbtree_t	*tree;
	uint32_t	count;
	int		err;

	/* Initialize Red-Black trees. */
	pfc_rbtree_init(&daemon_tree, (pfc_rbcomp_t)strcmp, daemon_getkey);
	pfc_rbtree_init(&daemon_proc_tree, pfc_rbtree_uint32_compare,
			daemon_proc_getkey);

	for (tree = daemon_order_tree;
	     tree < PFC_ARRAY_LIMIT(daemon_order_tree); tree++) {
		pfc_rbtree_init(tree, pfc_rbtree_uint32_compare,
				daemon_order_getkey);
	}

	/*
	 * Put UNC daemon itself to daemon_order_tree associated with
	 * cluster state event order.
	 */
	err = daemon_clevent_init();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Initialize daemon list per process type. */
	for (idlp = daemon_type_list; idlp < PFC_ARRAY_LIMIT(daemon_type_list);
	     idlp++) {
		idlp->idml_count = 0;
		pfc_list_init(&idlp->idml_list);
	}

	/* Open daemon configuration directory. */
	err = liblnc_conf_opendir(&conf, ctx->lc_confdir, LIBLNC_OFLAG_LOG);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_conf_error("launcher: Failed to open daemon configuration "
			       "directory: %s", ctx->lc_confdir);
		goto error;
	}

	count = 0;
	for (;;) {
		lnc_conf_t	dcf;

		err = liblnc_conf_getnext(conf, &dcf);
		if (err < 0) {
			/* No more daemon. */
			liblnc_conf_closedir(conf);
			ctx->lc_ndaemons = count;

			pfc_log_debug("%u daemon%s been loaded.", count,
				      (count > 1) ? "s have" : " has");

			return 0;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_conf_error("launcher: Failed to read daemon "
				       "configuration directory: %s",
				       ctx->lc_confdir);
			break;
		}

		/* Load daemon configuration. */
		err = daemon_create(&dcf);
		liblnc_conf_close(&dcf);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		count++;
	}

	liblnc_conf_closedir(conf);

error:
	/* Rollback all daemon instances. */
	PFC_ASSERT(err != 0);
	lnc_daemon_destroy();

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_daemon_init(void)
 *	Initialize daemon command execution environment.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_daemon_init(void)
{
	pfc_rbnode_t	*node = NULL;
	int		err = 0;

	while ((node = pfc_rbtree_next(&daemon_tree, node)) != NULL) {
		lnc_daemon_t	*ldp = LNC_DAEMON_NODE2PTR(node);

		err = daemon_setup(ldp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_daemon_destroy(void)
 *	Destroy all daemon configurations.
 *
 * Remarks:
 *	The caller must ensure that no child process is running.
 */
void PFC_ATTR_HIDDEN
lnc_daemon_destroy(void)
{
	pfc_rbtree_t	*tree;

	daemon_proc_assert_empty();

	for (tree = daemon_order_tree;
	     tree < PFC_ARRAY_LIMIT(daemon_order_tree); tree++) {
		pfc_rbtree_clear(tree, daemon_order_destroy, NULL);
	}

	pfc_rbtree_clear(&daemon_tree, daemon_destroy, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_daemon_start(void *arg)
 *	Start all daemons.
 *	This function is called on a task queue thread.
 */
void PFC_ATTR_HIDDEN
lnc_daemon_start(void *arg)
{
	lnc_ctx_t	*ctx = (lnc_ctx_t *)arg;
	pfc_rbtree_t	*tree;
	pfc_rbnode_t	*node = NULL;
	int		err;

	PFC_ASSERT(ctx == &launcher_ctx);

	pfc_log_info("Start all daemons.");
	tree = lnc_daemon_order_gettree(LNC_ORDTYPE_START, 0);

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_ordnode_t	*onp = LNC_ORDNODE_NODE2PTR(node);
		lnc_daemon_t	*ldp = onp->lo_daemon;

		if (PFC_EXPECT_FALSE(lnc_is_finalized(ctx))) {
			pfc_log_notice("Start up sequence has been "
				       "terminated.");

			return;
		}

		pfc_log_debug("%s[order=%u]: Starting.",
			      LNC_DAEMON_NAME(ldp), onp->lo_order);
		err = daemon_start(ldp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			if (err == ECANCELED) {
				pfc_log_verbose("Start up sequence has been "
						"canceled.");
			}
			else {
				lnc_log_fatal("Failed to start daemon.");
			}

			return;
		}
	}

	LNC_LOCK(ctx);

	if (PFC_EXPECT_TRUE(!LNC_IS_FINI(ctx))) {
		pfc_log_info("All daemons have been started.");
		LNC_SETFLAG_L(ctx, LNC_CF_INIT);

		/* Post an INIT event. */
		err = lnc_event_post(LNC_EVTYPE_INIT);
		if (PFC_EXPECT_FALSE(err != 0)) {
			lnc_log_fatal_l("Failed to post INIT event.");
		}
	}

	LNC_UNLOCK(ctx);

	return;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_daemon_stop(uint32_t maxorder, const pfc_timespec_t *abstime)
 *	Stop all daemons.
 *
 *	`maxorder' is the maximum number of "stop_order".
 *	This function stops all daemons which are assigned "stop_order" value
 *	less than `maxorder'. If `maxorder' is UINT32_MAX, all daemons will be
 *	stopped.
 *
 *	`abstime' must be a pointer to pfc_timespec_t which represents deadline
 *	of stopping all daemons. Note that `abstime' is ignored unless
 *	`maxorder' is UINT32_MAX.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_daemon_stop(uint32_t maxorder, const pfc_timespec_t *abstime)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_rbtree_t	*tree;
	pfc_rbnode_t	*node = NULL;
	int		err = 0;

	pfc_log_burst_info_begin();
	if (maxorder == UINT32_MAX) {
		pfc_log_burst_write("Stop all daemons.");
	}
	else {
		pfc_log_burst_write("Stop daemons: maxorder=%u", maxorder);
	}
	pfc_log_burst_end();

	/* Finalize the launcher before terminating daemons. */
	lnc_finalize(ctx);

	tree = lnc_daemon_order_gettree(LNC_ORDTYPE_STOP, 0);

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_ordnode_t	*onp = LNC_ORDNODE_NODE2PTR(node);
		lnc_daemon_t	*ldp = onp->lo_daemon;
		uint32_t	order = onp->lo_order;
		int		e;

		if (order > maxorder) {
			break;
		}

		pfc_log_debug("%s[order=%u]: Terminating.",
			      LNC_DAEMON_NAME(ldp), order);
		e = daemon_stop(ldp);
		if (PFC_EXPECT_FALSE(e != 0)) {
			err = e;
			/* FALLTHROUGH */
		}
	}

	if (maxorder == UINT32_MAX && PFC_EXPECT_TRUE(err == 0)) {
		/* Wait for all callback functions to return. */
		LNC_LOCK(ctx);
		err = daemon_waitall(ctx, abstime);
		LNC_UNLOCK(ctx);

		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("At least one callback function did not "
				      "return.");
		}
		else {
			pfc_log_info("All daemons have been stopped.");
		}
	}

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_daemon_kill(void)
 *	Kill all children spawned by the launcher module by force.
 */
void PFC_ATTR_HIDDEN
lnc_daemon_kill(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;

	if (PFC_EXPECT_FALSE(daemon_killall(ctx, SIGABRT) != 0)) {
		(void)daemon_killall(ctx, SIGKILL);
	}
}

/*
 * pfc_bool_t PFC_ATTR_HIDDEN
 * lnc_daemon_haschild(void)
 *	Return PFC_TRUE only if at least one child process is running.
 */
pfc_bool_t PFC_ATTR_HIDDEN
lnc_daemon_haschild(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_bool_t	ret;

	LNC_LOCK(ctx);
	ret = !LNC_DAEMON_HAS_NO_CHILD(ctx);
	LNC_UNLOCK(ctx);

	return ret;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_daemon_notify(pid_t pid, const char *channel)
 *	Notify that the specified process becomes ready.
 *
 *	`channel' must be an IPC channel name provided by the specified
 *	process. NULL must be set if the process does not provide IPC service.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_daemon_notify(pid_t pid, const char *channel)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_daemon_t	*ldp;
	lnc_proc_t	*lprp;
	pfc_rbnode_t	*node;
	pfc_refptr_t	*rchannel, *old;
	uint32_t	flags;
	int		err;

	if (channel == NULL) {
		rchannel = NULL;
	}
	else {
		rchannel = pfc_refptr_string_create(channel);
		if (PFC_EXPECT_FALSE(rchannel == NULL)) {
			pfc_log_error("notify: Failed to copy IPC channel "
				      "name.");

			return ENOMEM;
		}
	}

	LNC_LOCK(ctx);

	/* Search for a process information associated with the given PID. */
	node = pfc_rbtree_get(&daemon_proc_tree, LNC_PROC_KEY(pid));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		pfc_log_error("notify: Unexpected process ID: %u", pid);
		err = ESRCH;
		goto error;
	}

	lprp = LNC_PROC_NODE2PTR(node);
	ldp = lprp->lp_daemon;
	if (PFC_EXPECT_FALSE(ldp->ld_command != lprp)) {
		pfc_log_error("notify: Not a daemon process: pid=%u", pid);
		err = ESRCH;
		goto error;
	}

	LNC_PROC_LOCK(lprp);

	/*
	 * Ensure that the launcher module is waiting for this process to
	 * start.
	 */
	flags = lprp->lp_flags;
	if (PFC_EXPECT_FALSE((flags & LNC_PF_START_WAIT) == 0)) {
		pfc_log_error("nofity: Already started: pid=%u", pid);
		err = ECHILD;
		LNC_PROC_UNLOCK(lprp);
		goto error;
	}

	/* Install IPC channel name to the daemon instance. */
	old = ldp->ld_channel;
	ldp->ld_channel = rchannel;

	/* Wake up start-up thread. */
	pfc_log_info("%s.%s: Daemon has become ready.",
		     LNC_DAEMON_NAME(ldp), lprp->lp_name);
	lprp->lp_flags = (flags & ~LNC_PF_START_WAIT);
	LNC_PROC_BROADCAST(lprp);

	LNC_PROC_UNLOCK(lprp);
	LNC_UNLOCK(ctx);

	if (old != NULL) {
		pfc_refptr_put(old);
	}

	return 0;

error:
	LNC_UNLOCK(ctx);

	if (rchannel != NULL) {
		pfc_refptr_put(rchannel);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_daemon_clevent_ack(pid_t pid, pfc_bool_t result)
 *	Receive acknowledgement of CLACT event from the process specified by
 *	the process ID `pid'.
 *
 *	`result' is a boolean value which represents the result of cluster
 *	state transition. PFC_TRUE means successful completion.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_daemon_clevent_ack(pid_t pid, pfc_bool_t result)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_daemon_t	*ldp;
	lnc_proc_t	*lprp;
	pfc_rbnode_t	*node;
	int		err;

	LNC_LOCK(ctx);

	/* Search for a process information associated with the given PID. */
	node = pfc_rbtree_get(&daemon_proc_tree, LNC_PROC_KEY(pid));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		pfc_log_error("CLEVACK: Unexpected process ID: %u", pid);
		err = ESRCH;
		goto out;
	}

	lprp = LNC_PROC_NODE2PTR(node);
	ldp = lprp->lp_daemon;
	if (PFC_EXPECT_FALSE(ldp->ld_command != lprp)) {
		pfc_log_error("CLEVACK: Not a daemon process: pid=%u",
			      pid);
		err = ESRCH;
		goto out;
	}

	LNC_PROC_LOCK(lprp);

	if (PFC_EXPECT_FALSE((lprp->lp_flags & LNC_PF_CLEVENT) == 0)) {
		pfc_log_error("CLEVACK: %s.%s: ACK from unexpected daemon: "
			      "pid=%u", LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      pid);
		err = EINVAL;
		goto out_proc;
	}

	pfc_log_verbose("CLEVACK: %s.%s: Received: pid=%u, result=%u",
			LNC_DAEMON_NAME(ldp), lprp->lp_name, pid, result);

	lprp->lp_flags &= ~LNC_PF_CLEVENT;
	if (PFC_EXPECT_TRUE(result)) {
		lprp->lp_flags |= LNC_PF_CLEVOK;
	}
	LNC_PROC_BROADCAST(lprp);
	err = 0;

out_proc:
	LNC_PROC_UNLOCK(lprp);

out:
	LNC_UNLOCK(ctx);

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_daemon_fini(void)
 *	Finalize all daemon configurations.
 *	All threads waiting for server-ready notification will be woken up.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
void PFC_ATTR_HIDDEN
lnc_daemon_fini(void)
{
	pfc_rbtree_t	*tree = &daemon_proc_tree;
	pfc_rbnode_t	*node = NULL;

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_proc_t	*lprp = LNC_PROC_NODE2PTR(node);

		LNC_PROC_LOCK(lprp);
		lprp->lp_flags |= LNC_PF_FINI;
		LNC_PROC_BROADCAST(lprp);
		LNC_PROC_UNLOCK(lprp);
	}
}

/*
 * static int
 * daemon_create(lnc_conf_t *cfp)
 *	Create a daemon instance associated with the given configuration file
 *	handle.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_create(lnc_conf_t *cfp)
{
	lnc_daemon_t	*ldp;
	lnc_ordtype_t	type;
	pfc_cfblk_t	blk;
	const char	*str, *name, *param;
	uint8_t		cltype;
	int		err, rotate;

	ldp = (lnc_daemon_t *)malloc(sizeof(*ldp));
	if (PFC_EXPECT_FALSE(ldp == NULL)) {
		daemon_conf_error(cfp, "Failed to allocate daemon instance.");

		return ENOMEM;
	}

	ldp->ld_name = cfp->lcf_name;
	pfc_refptr_get(ldp->ld_name);
	name = LNC_DAEMON_NAME(ldp);

	blk = pfc_conf_get_block(cfp->lcf_conf, "daemon");

	ldp->ld_type = LIBLNC_DMCONF_GET(blk, uint32, process_type);
	ldp->ld_uncd = LIBLNC_DMCONF_GET(blk, bool, uncd);
	ldp->ld_start_wait = LIBLNC_DMCONF_GET(blk, bool, start_wait);
	ldp->ld_start_timeout = LIBLNC_DMCONF_GET(blk, uint32, start_timeout);
	ldp->ld_stop_timeout = LIBLNC_DMCONF_GET(blk, uint32, stop_timeout);
	
	str = LIBLNC_DMCONF_GET(blk, string, stop_signal);
	ldp->ld_stop_sig = liblnc_getsignal(str);
	if (PFC_EXPECT_FALSE(ldp->ld_stop_sig == 0)) {
		daemon_conf_error(cfp, "Unknown signal name: %s", str);
		err = EINVAL;
		goto error;
	}

	ldp->ld_start_time.tv_sec = 0;
	ldp->ld_start_time.tv_nsec = 0;
	ldp->ld_channel = NULL;

	/* Set NULL for error handling. */
	ldp->ld_command = NULL;
	ldp->ld_stop_cmd = NULL;
	ldp->ld_logdir = NULL;

	/* Register the daemon instance to daemon_tree. */
	err = pfc_rbtree_put(&daemon_tree, &ldp->ld_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		PFC_ASSERT(err == EEXIST);
		daemon_conf_error(cfp, "Duplicated daemon name.");
		goto error;
	}

	/* Insert daemon instance to process type list. */
	daemon_type_addlist(ldp);

	/*
	 * From here, no rollback is required on error.
	 * All daemon instances in daemon_tree and daemon_order_tree will be
	 * destroyed by the caller.
	 */

	/* Initialize stderr logging configuration. */
	rotate = LIBLNC_DMCONF_GET(blk, int32, stderr_rotate);
	if (rotate >= 0) {
		lnc_ctx_t	*ctx = &launcher_ctx;
		pfc_refptr_t	*logdir;

		/* Determine logging directory path. */
		logdir = pfc_refptr_sprintf("%s/%s/%s", LNC_WORKDIR(ctx),
					    name, LNC_STDERR_LOGDIR);
		if (PFC_EXPECT_FALSE(logdir == NULL)) {
			daemon_conf_error(cfp, "Failed to create logging "
					  "directory path.");

			return ENOMEM;
		}

		ldp->ld_logdir = logdir;
	}

	/* Fetch executable path and arguments for the daemon. */
	err = daemon_proc_create(&ldp->ld_command, ldp, cfp, "daemon",
				 LIBLNC_DMCONF_GET(blk, string, command));
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = daemon_stderr_init(ldp, ldp->ld_command, cfp,
				 LNC_STDERR_LOGFILE_DAEMON, rotate);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/*
	 * Fetch executable path and arguments for the command to stop the
	 * daemon.
	 */
	str = LIBLNC_DMCONF_GET(blk, string, stop);
	if (str != NULL) {
		err = daemon_proc_create(&ldp->ld_stop_cmd, ldp, cfp, "stop",
					 str);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		/* Disable stderr log file rotation for the stop command. */
		err = daemon_stderr_init(ldp, ldp->ld_stop_cmd, cfp,
					 LNC_STDERR_LOGFILE_STOP, 0);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	/* Register the daemon instance to daemon_order_tree. */
	for (type = __LNC_ORDTYPE_MIN; type < __LNC_ORDTYPE_INDEX_MIN;
	     type++) {
		param = liblnc_order_getparamname(type);
		err = daemon_order_put(cfp, blk, ldp, param, NULL, type, 0);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	param = liblnc_order_getparamname(LNC_ORDTYPE_CLEVENT);
	for (cltype = 0; cltype < CLSTAT_NEVTYPES; cltype++) {
		err = daemon_order_put(cfp, blk, ldp, param,
				       lnc_cluster_notify, LNC_ORDTYPE_CLEVENT,
				       cltype);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	pfc_log_debug("%s: Loaded.", name);

	return 0;

error:
	pfc_refptr_put(ldp->ld_name);
	free(ldp);

	return err;
}

/*
 * static int
 * daemon_setup(lnc_daemon_t *ldp)
 *	Set up command execution environment for the given daemon instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_setup(lnc_daemon_t *ldp)
{
	lnc_proc_t	*lprp;
	pfc_refptr_t	*logdir = ldp->ld_logdir;
	int		err;

	if (logdir != NULL) {
		const char	*path = pfc_refptr_string_value(logdir);

		/* Create stderr logging directory if it does not exist. */
		err = pfc_mkdir(path, LNC_STDERR_LOGDIR_PERM);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s: Failed to create stderr logging "
				      "directory: %s", LNC_DAEMON_NAME(ldp),
				      strerror(err));

			return err;
		}
	}

	lprp = ldp->ld_command;
	err = daemon_proc_setup(ldp, lprp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	lprp = ldp->ld_stop_cmd;
	if (lprp != NULL) {
		err = daemon_proc_setup(ldp, lprp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

/*
 * void
 * daemon_destroy(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destroy the daemon instance.
 *	This function will be called via pfc_rbtree_clear().
 *
 *	`node' must be a pointer to ld_node in lnc_daemon_t.
 *
 * Remarks:
 *	The caller must ensure that any process associated with the given
 *	daemon instance is not running.
 */
static void
daemon_destroy(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	int		err = 0;
	lnc_daemon_t	*ldp = LNC_DAEMON_NODE2PTR(node);
	lnc_proc_t	**lprpp, *procs[] = {
		ldp->ld_command,
		ldp->ld_stop_cmd,
	};
	pfc_refptr_t	**rstrp, *rstrings[] = {
		ldp->ld_name,
		ldp->ld_logdir,
		ldp->ld_channel,
	};

	/* Destory command instances in the given daemon instance. */
	for (lprpp = procs; lprpp < PFC_ARRAY_LIMIT(procs); lprpp++) {
		lnc_proc_t	*lprp = *lprpp;

		if (lprp != NULL) {
			int	e;

			e = daemon_proc_destroy(lprp);
			if (PFC_EXPECT_FALSE(e != 0)) {
				err = e;
			}
		}
	}

	if (PFC_EXPECT_FALSE(err != 0)) {
		return;
	}

#ifdef	UNC_VERBOSE_DEBUG
	PFC_ASSERT_INT(pfc_cond_destroy(&ldp->ld_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&ldp->ld_mutex), 0);
#endif	/* UNC_VERBOSE_DEBUG */

	/* Free up refptr strings. */
	for (rstrp = rstrings; rstrp < PFC_ARRAY_LIMIT(rstrings); rstrp++) {
		pfc_refptr_t	*rstr = *rstrp;

		if (rstr != NULL) {
			pfc_refptr_put(rstr);
		}
	}

	free(ldp);
}

/*
 * static int
 * daemon_start(lnc_daemon_t *ldp)
 *	Start daemon process associated with the given daemon instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_start(lnc_daemon_t *ldp)
{
	lnc_proc_t	*lprp = ldp->ld_command;
	int		err;

	err = daemon_proc_start(ldp, LNC_DAEMON_NAME(ldp), lprp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (ldp->ld_start_wait) {
		/* Wait for a service-ready notification. */
		err = daemon_start_wait(ldp, lprp);
	}

	return err;
}

/*
 * static int
 * daemon_start_wait(lnc_daemon_t *UNC_RESTRICT ldp,
 *		     lnc_proc_t *UNC_RESTRICT daemon)
 *	Wait for a service-ready notification issued by the given daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_start_wait(lnc_daemon_t *UNC_RESTRICT ldp,
		  lnc_proc_t *UNC_RESTRICT daemon)
{
	pfc_timespec_t	abstime, to;
	const char	*dname = LNC_DAEMON_NAME(ldp);
	int		wait_err = 0;

	PFC_ASSERT(ldp->ld_start_wait);

	/* Determine timeout of daemon bootstrap. */
	pfc_clock_msec2time(&to, ldp->ld_start_timeout);
	PFC_ASSERT_INT(pfc_clock_abstime(&abstime, &to), 0);

	pfc_log_info("%s.%s: Waiting for the daemon to be ready.",
		     dname, daemon->lp_name);

	LNC_PROC_LOCK(daemon);

	for (;;) {
		uint32_t	flags = daemon->lp_flags;

		if ((flags & LNC_PF_START_WAIT) == 0) {
			wait_err = 0;
			break;
		}

		if (PFC_EXPECT_FALSE(daemon->lp_pid == 0)) {
			LNC_PROC_UNLOCK(daemon);
			lnc_log_fatal("%s.%s: Daemon died without sending "
				      "notification.", dname, daemon->lp_name);

			return ESRCH;
		}

		if (PFC_EXPECT_FALSE(flags & LNC_PF_FINI)) {
			pfc_log_notice("%s.%s: Notification wait has been "
				       "canceled.", dname, daemon->lp_name);
			wait_err = ECANCELED;
			break;
		}

		if (PFC_EXPECT_FALSE(wait_err != 0)) {
			PFC_ASSERT(wait_err == ETIMEDOUT);
			pfc_log_error("%s.%s: Daemon did not become ready "
				      "within %u milliseconds.",
				      dname, daemon->lp_name,
				      ldp->ld_start_timeout);
			break;
		}

		wait_err = LNC_PROC_TIMEDWAIT_ABS(daemon, &abstime);
	}

	LNC_PROC_UNLOCK(daemon);

	return wait_err;
}

/*
 * static int
 * daemon_stop(lnc_daemon_t *ldp)
 *	Stop daemon process associated with the given daemon instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_stop(lnc_daemon_t *ldp)
{
	lnc_proc_t	*daemon = ldp->ld_command;
	pfc_bool_t	do_kill;
	const char	*dname = LNC_DAEMON_NAME(ldp);
	pfc_timespec_t	abstime, to;
	pid_t		pid;
	int		err;

	/* Determine timeout of process shutdown. */
	pfc_clock_msec2time(&to, ldp->ld_stop_timeout);
	PFC_ASSERT_INT(pfc_clock_abstime(&abstime, &to), 0);

	LNC_PROC_LOCK(daemon);
	pid = daemon->lp_pid;
	LNC_PROC_UNLOCK(daemon);

	if (PFC_EXPECT_FALSE(pid == 0)) {
		/* Daemon is not running. */
		return 0;
	}

	/* Try to execute stop command. */
	do_kill = PFC_TRUE;
	err = daemon_stop_exec(ldp, &abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		do_kill = PFC_FALSE;
	}

	if (do_kill) {
		int	sig;

		/* Check whether the daemon is still alive. */
		LNC_PROC_LOCK(daemon);
		pid = daemon->lp_pid;
		LNC_PROC_UNLOCK(daemon);
		if (PFC_EXPECT_FALSE(pid == 0)) {
			return 0;
		}

		/* Send stop signal. */
		sig = ldp->ld_stop_sig;
		pfc_log_info("%s: Sending signal: pid=%u, sig=%d",
			     dname, pid, sig);
		if (PFC_EXPECT_FALSE(kill(pid, sig) != 0)) {
			err = errno;
			if (PFC_EXPECT_TRUE(err == ESRCH)) {
				/* Process does not exist. */
				return 0;
			}

			pfc_log_error("%s: Failed to send signal: %s",
				      dname, strerror(err));

			if (PFC_EXPECT_FALSE(err == 0)) {
				err = EIO;
			}

			return err;
		}
	}

	/* Wait for the daemon process to stop. */
	return daemon_proc_wait(ldp, daemon, &abstime);
}

/*
 * static int
 * daemon_stop_exec(lnc_daemon_t *UNC_RESTRICT ldp,
 *		    const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Execute stop command defined by the daemon configuration.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the stop command is not defined.
 *	EIO is returned if the stop command exited abnormally.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_stop_exec(lnc_daemon_t *UNC_RESTRICT ldp,
		 const pfc_timespec_t *UNC_RESTRICT abstime)
{
	lnc_proc_t	*stop = ldp->ld_stop_cmd;
	int		err;

	if (stop == NULL) {
		return -1;
	}

	/* Execute stop command. */
	err = daemon_proc_start(ldp, LNC_DAEMON_NAME(ldp), stop);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Wait for the stop command to exit. */
	err = daemon_proc_wait(ldp, stop, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Check exit status. */
	if (PFC_EXPECT_FALSE(stop->lp_status != 0)) {
		err = EIO;
	}

	LNC_PROC_RESET_STATUS(stop);

	return err;
}

/*
 * static int
 * daemon_killall(lnc_ctx_t *ctx, int sig)
 *	Kill all children by sending the given signal.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error numbern which indicates the cause of error is returned.
 */
static int
daemon_killall(lnc_ctx_t *ctx, int sig)
{
	pfc_rbtree_t	*tree = &daemon_proc_tree;
	pfc_rbnode_t	*node = NULL;
	pfc_timespec_t	abstime;
	int		err;

	LNC_LOCK(ctx);
	if (LNC_DAEMON_HAS_NO_CHILD(ctx)) {
		LNC_UNLOCK(ctx);

		return 0;
	}

	PFC_ASSERT_INT(pfc_clock_gettime(&abstime), 0);
	abstime.tv_sec += LNC_CLEANUP_TIMEOUT;

	pfc_log_warn("Send signal %d to all children.", sig);

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_proc_t	*lprp = LNC_PROC_NODE2PTR(node);
		pid_t		pid;

		LNC_PROC_LOCK(lprp);
		pid = lprp->lp_pid;
		if (PFC_EXPECT_TRUE(pid != 0)) {
			if (PFC_EXPECT_FALSE(kill(pid, sig) != 0 &&
					     (err = errno) != ESRCH)) {
				pfc_log_error("Failed to send signal %d to "
					      "pid %u: %s", sig, pid,
					      strerror(err));
			}
		}
		LNC_PROC_UNLOCK(lprp);
	}

	err = daemon_waitall(ctx, &abstime);
	LNC_UNLOCK(ctx);

	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("At least one child process did not exit "
			      "within %u seconds.", LNC_CLEANUP_TIMEOUT);
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * static int
 * daemon_waitall(lnc_ctx_t *UNC_RESTRICT ctx,
 *		  const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Wait for all children to exit, and all callback functions to return.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error numbern which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 *	Note that it may be released for a while.
 */
static int
daemon_waitall(lnc_ctx_t *UNC_RESTRICT ctx,
	       const pfc_timespec_t *UNC_RESTRICT abstime)
{
	int	err = 0;

	for (;;) {
		if (PFC_EXPECT_TRUE(LNC_DAEMON_HAS_NO_CHILD(ctx))) {
			return 0;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		err = LNC_TIMEDWAIT_ABS(ctx, abstime);
	}

	return err;
}

/*
 * static void
 * daemon_getinfo(lnc_daemon_t *UNC_RESTRICT ldp,
 *		  lnc_dminfo_t *UNC_RESTRICT dip)
 *	Copy daemon information to the buffer pointed by `dip.
 */
static void
daemon_getinfo(lnc_daemon_t *UNC_RESTRICT ldp, lnc_dminfo_t *UNC_RESTRICT dip)
{
	lnc_proc_t	*lprp = ldp->ld_command;

	LNC_PROC_LOCK(lprp);
	lnc_daemon_setdminfo(dip, LNC_DAEMON_NAME(ldp), ldp->ld_channel,
			     ldp->ld_type, lprp->lp_pid);
	LNC_PROC_UNLOCK(lprp);
}

/*
 * static void
 * daemon_conf_error(lnc_conf_t *UNC_RESTRICT cfp,
 *		     const char *UNC_RESTRICT fmt, ...)
 *	Print error message that indicates daemon configuration file error.
 *	Error message must be specified in printf(3) format.
 *
 * Remarks:
 *	This function uses pfc_conf_verror() to print error message in order
 *	to make error message visible to configuration file check mode.
 */
static void
daemon_conf_error(lnc_conf_t *UNC_RESTRICT cfp, const char *UNC_RESTRICT fmt,
		  ...)
{
	const char	*path;
	pfc_refptr_t	*rfmt;
	va_list		ap;

	/* Try to prepend configuration file path to the format string. */
	path = pfc_conf_get_path(cfp->lcf_conf);
	PFC_ASSERT(path != NULL);

	va_start(ap, fmt);

	rfmt = pfc_refptr_sprintf("%s: %s", path, fmt);
	if (PFC_EXPECT_TRUE(rfmt != NULL)) {
		fmt = pfc_refptr_string_value(rfmt);
		pfc_conf_verror(fmt, ap);
		pfc_refptr_put(rfmt);
	}
	else {
		pfc_conf_verror(fmt, ap);
	}

	va_end(ap);
}

/*
 * static int
 * daemon_proc_create(lnc_proc_t **UNC_RESTRICT lprpp,
 *		      lnc_daemon_t *UNC_RESTRICT ldp,
 *		      lnc_conf_t *UNC_RESTRICT cfp,
 *		      const char *UNC_RESTRICT name,
 *		      const char *UNC_RESTRICT key)
 *	Create a new process instance.
 *
 *	`cfp' must be a configuration file obtained by liblnc_conf_getnext().
 *	`name' must be the name of the process. It must points static string
 *	literal.
 *	`key' is key of "command" map in the configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to lnc_proc_t is
 *	set to the buffer pointed by `lprpp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_proc_create(lnc_proc_t **UNC_RESTRICT lprpp,
		   lnc_daemon_t *UNC_RESTRICT ldp,
		   lnc_conf_t *UNC_RESTRICT cfp, const char *UNC_RESTRICT name,
		   const char *UNC_RESTRICT key)
{
	lnc_proc_t	*lprp;
	lnc_errmsg_t	emsg;
	lnc_cmdmap_t	*cmap;
	pfc_extcmd_t	ecmd;
	const char	*path;
	int		err;

	PFC_ASSERT(key != NULL);

	lprp = (lnc_proc_t *)malloc(sizeof(*lprp));
	if (PFC_EXPECT_FALSE(lprp == NULL)) {
		daemon_conf_error(cfp, "Failed to allocate process instance.");

		return ENOMEM;
	}

	PFC_ASSERT_INT(PFC_MUTEX_INIT(&lprp->lp_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&lprp->lp_cond), 0);

	/*
	 * pfc_extcmd_create_async() can not be called here because it is
	 * not initialized yet. So the specified command map must be preserved.
	 */
	err = liblnc_cmdmap_create(&cmap, cfp, key, &emsg);
	if (PFC_EXPECT_FALSE(err != 0)) {
		daemon_conf_error(cfp, "command[%s]: %s", key,
				  emsg.le_message);
		free(lprp);

		return err;
	}

	/*
	 * Try to create dummy command environment in order to verify
	 * executable file path.
	 */
	path = LIBLNC_CMDMAP_PATH(cmap);
	err = pfc_extcmd_create(&ecmd, path, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		const char	*emsg;

		if (err == EINVAL) {
			emsg = "Invalid command path";
		}
		else if (err == ENOENT) {
			emsg = "Command not found";
		}
		else {
			emsg = "Failed to create command environment";
		}

		daemon_conf_error(cfp, "command[%s]: %s: path=\"%s\"",
				  key, emsg, path);
		liblnc_cmdmap_destroy(cmap);
		free(lprp);

		return err;
	}

	PFC_ASSERT_INT(pfc_extcmd_destroy(ecmd), 0);

	/*
	 * Initialize lp_daemon as NULL so that we can distinguish value
	 * in lp_cmd union.
	 */
	lprp->lp_daemon = NULL;

	LNC_PROC_CONF(lprp) = cmap;
	lprp->lp_name = name;
	lprp->lp_stderr = NULL;
	lprp->lp_flags = 0;
	LNC_PROC_RESET_STATUS(lprp);

	*lprpp = lprp;

	return 0;
}

/*
 * static int
 * daemon_proc_setup(lnc_daemon_t *UNC_RESTRICT ldp,
 *		    lnc_proc_t *UNC_RESTRICT lprp)
 *	Set up process environment.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_proc_setup(lnc_daemon_t *UNC_RESTRICT ldp, lnc_proc_t *UNC_RESTRICT lprp)
{
	lnc_cmdmap_t	*cmap = LNC_PROC_CONF(lprp);
	pfc_listm_t	args = cmap->cm_args;
	pfc_extcmd_t	ecmd;
	pfc_refptr_t	*rstr;
	const char	*path, *proc;
	int		err, argc, i;

	PFC_ASSERT(lprp->lp_daemon == NULL);

	argc = pfc_listm_get_size(args);
	PFC_ASSERT(argc > 0);

	path = LIBLNC_CMDMAP_PATH(cmap);
	PFC_ASSERT_INT(pfc_listm_getat(args, 0, (pfc_cptr_t *)&rstr), 0);
	proc = pfc_refptr_string_value(rstr);

	/* Create a command execution environment. */
	err = pfc_extcmd_create_async(&ecmd, path, proc, daemon_proc_callback);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to create command environment: "
			      "%s", LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));

		return err;
	}

	if (ldp->ld_command == lprp && ldp->ld_uncd) {
		/* Add UNC daemon specific arguments. */
		err = daemon_proc_setup_uncd(ldp, lprp, ecmd);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT_INT(pfc_extcmd_destroy(ecmd), 0);

			return err;
		}
	}

	/* Set up arguments. */
	for (i = 1; i < argc; i++) {
		const char	*arg;

		PFC_ASSERT_INT(pfc_listm_getat(args, i, (pfc_cptr_t *)&rstr),
			       0);
		arg = pfc_refptr_string_value(rstr);
		err = pfc_extcmd_add_arguments(ecmd, arg, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s.%s: Failed to add command argument: "
				      "%s", LNC_DAEMON_NAME(ldp),
				      lprp->lp_name, strerror(err));
			PFC_ASSERT_INT(pfc_extcmd_destroy(ecmd), 0);

			return err;
		}
	}

	/* Install command environment to the process instance. */
	LNC_PROC_ENV(lprp) = ecmd;
	lprp->lp_daemon = ldp;
	liblnc_cmdmap_destroy(cmap);

	/* Set up I/O handles for the daemon process. */
	return daemon_proc_ioinit(lprp);
}

/*
 * static int
 * daemon_proc_setup_uncd(lnc_daemon_t *UNC_RESTRICT ldp,
 *			  lnc_proc_t *UNC_RESTRICT lprp, pfc_extcmd_t ecmd)
 *	Set up command line arguments specific to UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_proc_setup_uncd(lnc_daemon_t *UNC_RESTRICT ldp,
		       lnc_proc_t *UNC_RESTRICT lprp, pfc_extcmd_t ecmd)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	char		buf[32];
	int		err;

	/* Set core.uncd.pid property. */
	snprintf(buf, sizeof(buf), "-P%s=%u", LNC_PROP_PID, getpid());
	err = pfc_extcmd_add_arguments(ecmd, buf, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to add PID property argument: %s",
			      LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));

		return err;
	}

	if (ldp->ld_start_wait) {
		pfc_refptr_t	*rprop;
		const char	*prop;

		/*
		 * Let the UNC daemon to send notification IPC service
		 * request.
		 */
		rprop = pfc_refptr_sprintf("-P%s=%s,%s,%u",
					   PFC_PROP_IPC_NOTIFY,
					   ctx->lc_channel,
					   PFC_MODULE_THIS_NAME,
					   LNC_IPC_SVID_NOTIFY);
		if (PFC_EXPECT_FALSE(rprop == NULL)) {
			pfc_log_error("%s.%s: Failed to create notification "
				      "property.", LNC_DAEMON_NAME(ldp),
				      lprp->lp_name);

			return ENOMEM;
		}

		prop = pfc_refptr_string_value(rprop);
		err = pfc_extcmd_add_arguments(ecmd, prop, NULL);
		pfc_refptr_put(rprop);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s.%s: Failed to add notification "
				      "property argument: %s",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name,
				      strerror(err));

			return err;
		}
	}

	return 0;
}

/*
 * static int
 * daemon_proc_ioinit(lnc_proc_t *lprp)
 *	Initialize I/O handles of the given process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_proc_ioinit(lnc_proc_t *lprp)
{
	const pfc_extcmd_iodesc_t	*iodp, *iodlimit, iodescs[] = {
		PFC_EXTCMD_STDIN,
		PFC_EXTCMD_STDOUT,
		PFC_EXTCMD_STDERR,
	};
	pfc_extcmd_t	ecmd = LNC_PROC_ENV(lprp);
	lnc_daemon_t	*ldp = lprp->lp_daemon;
	lnc_stderr_t	*lsep = lprp->lp_stderr;
	int		err;

	PFC_ASSERT(ldp != NULL);

	iodlimit = PFC_ARRAY_LIMIT(iodescs);
	if (lsep != NULL) {
		pfc_refptr_t	*rpath;
		const char	*path;

		/* Construct stderr log file path. */
		rpath = pfc_refptr_sprintf("%s/%s", LNC_DAEMON_LOGDIR(ldp),
					   lsep->lse_fname);
		if (PFC_EXPECT_FALSE(rpath == NULL)) {
			pfc_log_error("%s.%s: "
				      "Failed to create log file path.",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name);

			return ENOMEM;
		}

		/* Enable standard error logging. */
		iodlimit--;
		path = pfc_refptr_string_value(rpath);
		err = pfc_extcmd_setio_file(ecmd, PFC_EXTCMD_STDERR, path,
					    LNC_STDERR_OFLAGS,
					    LNC_STDERR_LOGFILE_PERM);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(rpath);
			pfc_log_error("%s.%s: Failed to bind stderr to log "
				      "file: %s",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name,
				      strerror(err));

			return err;
		}

		pfc_log_verbose("%s.%s: stderr logging: path=%s, rotate=%d",
				LNC_DAEMON_NAME(ldp), lprp->lp_name,
				path, lsep->lse_rotate);
		pfc_refptr_put(rpath);
	}

	/* Disable standard I/O handles associated with the given process. */
	for (iodp = iodescs; iodp < iodlimit; iodp++) {
		err = pfc_extcmd_setio_discard(ecmd, *iodp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s.%s: Failed to disable I/O handle"
				      "[%u]: %s",
				      LNC_DAEMON_NAME(lprp->lp_daemon),
				      lprp->lp_name, *iodp,
				      strerror(err));

			return err;
		}
	}

	return 0;
}

/*
 * static int
 * daemon_proc_destroy(lnc_proc_t *lprp)
 *	Destory the given process instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must ensure that the given proces is not running.
 */
static int
daemon_proc_destroy(lnc_proc_t *lprp)
{
	lnc_daemon_t	*ldp = lprp->lp_daemon;
	lnc_stderr_t	*lsep = lprp->lp_stderr;

	if (ldp == NULL) {
		liblnc_cmdmap_destroy(LNC_PROC_CONF(lprp));
	}
	else {
		int	err;

		err = pfc_extcmd_destroy(LNC_PROC_ENV(lprp));
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s.%s: Failed to destroy command "
				      "environment: %s",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name,
				      strerror(err));

			return err;
		}
	}

	if (lsep != NULL) {
		daemon_stderr_destroy(lsep);
	}

	free(lprp);

	return 0;
}

/*
 * static int
 * daemon_proc_start_l(lnc_ctx_t *UNC_RESTRICT ctx,
 *		       lnc_daemon_t *UNC_RESTRICT ldp,
 *		       const char *UNC_RESTRICT dname,
 *		       lnc_proc_t *UNC_RESTRICT lprp)
 *	Start the given process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding both the launcher lock and
 *	the process lock.
 */
static int
daemon_proc_start_l(lnc_ctx_t *UNC_RESTRICT ctx,
		    lnc_daemon_t *UNC_RESTRICT ldp,
		    const char *UNC_RESTRICT dname,
		    lnc_proc_t *UNC_RESTRICT lprp)
{
	lnc_stderr_t	*lsep = lprp->lp_stderr;
	pfc_extcmd_t	ecmd = LNC_PROC_ENV(lprp);
	pid_t		pid;
	int		err;

	PFC_ASSERT(lprp->lp_pid == 0);
	if (ldp->ld_command == lprp) {
		if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx))) {
			/* Already finalized. */
			return ECANCELED;
		}

		/* Update process start time. */
		PFC_ASSERT_INT(pfc_clock_gettime(&ldp->ld_start_time), 0);
	}

	if (lsep != NULL) {
		/* Rotate stderr log files. */
		err = daemon_stderr_rotate(lprp, lsep);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	/* Initialize process state flags. */
	lprp->lp_flags &= ~LNC_PF_CLEVENT;
	if (ldp->ld_start_wait) {
		lprp->lp_flags |= LNC_PF_START_WAIT;
	}
	else {
		PFC_ASSERT((lprp->lp_flags & LNC_PF_START_WAIT) == 0);
	}

	/* Start the process. */
	err = pfc_extcmd_start(ecmd, &pid, lprp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to start process: %s",
			      dname, lprp->lp_name, strerror(err));
	}
	else {
		/* Register the process instance to the process tree. */
		lprp->lp_pid = pid;
		PFC_ASSERT_INT(pfc_rbtree_put(&daemon_proc_tree,
					      &lprp->lp_node), 0);

		pfc_log_info("%s.%s[%u]: Process has been started.",
			     dname, lprp->lp_name, pid);
	}

	return err;
}

/*
 * static int
 * daemon_proc_wait(lnc_daemon_t *UNC_RESTRICT ldp,
 *		    lnc_proc_t *UNC_RESTRICT lprp,
 *		    const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Wait for the specified process to exit.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_proc_wait(lnc_daemon_t *UNC_RESTRICT ldp, lnc_proc_t *UNC_RESTRICT lprp,
		 const pfc_timespec_t *UNC_RESTRICT abstime)
{
	int	wait_err = 0;

	LNC_PROC_LOCK(lprp);

	while (lprp->lp_pid != 0) {
		if (PFC_EXPECT_FALSE(wait_err != 0)) {
			LNC_PROC_UNLOCK(lprp);
			pfc_log_error("%s.%s: Process did not stop: %s",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name,
				      strerror(wait_err));

			return wait_err;
		}

		wait_err = LNC_PROC_TIMEDWAIT_ABS(lprp, abstime);
	}

	LNC_PROC_UNLOCK(lprp);

	return 0;
}

/*
 * static void
 * daemon_proc_callback(pfc_extcmd_t ecmd, const pfc_extcmd_proc_t *proc,
 *		       pfc_ptr_t arg)
 *	Callback function which will be called when a process terminates.
 */
static void
daemon_proc_callback(pfc_extcmd_t ecmd, const pfc_extcmd_proc_t *proc,
		     pfc_ptr_t arg)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_proc_t	*lprp = (lnc_proc_t *)arg;
	lnc_daemon_t	*ldp = lprp->lp_daemon;
	pfc_rbtree_t	*tree = &daemon_proc_tree;
	const char	*dname = LNC_DAEMON_NAME(ldp);
	pid_t		pid;
	int		sig, status;

	LNC_LOCK(ctx);
	LNC_PROC_LOCK(lprp);

	ctx->lc_ncallbacks++;

	/* Update process status. */
	pid = proc->pep_pid;
	PFC_ASSERT(pid == lprp->lp_pid);
	pfc_rbtree_remove_node(tree, &lprp->lp_node);
	lprp->lp_pid = 0;

	status = proc->pep_status;
	sig = proc->pep_signal;
	lprp->lp_status = status;

	if (sig == PFC_EXTCMD_ERR_UNSPEC) {
		if (PFC_EXPECT_TRUE(status == 0)) {
			pfc_log_info("%s.%s[%u]: "
				     "Process has exited successfully.",
				     dname, lprp->lp_name, pid);
		}
		else {
			pfc_log_error("%s.%s[%u]: "
				      "Process has exited abnormally: %d",
				      dname, lprp->lp_name, pid, status);
		}
	}
	else {
		const char	*fmt =
			"%s.%s[%u]: Process has been killed by signal: %d";

		if (PFC_EXPECT_TRUE(LNC_IS_FINI(ctx))) {
			pfc_log_info(fmt, dname, lprp->lp_name, pid, sig);
		}
		else {
			pfc_log_error(fmt, dname, lprp->lp_name, pid, sig);
		}
	}

	LNC_PROC_BROADCAST(lprp);

	if (PFC_EXPECT_FALSE(!LNC_IS_FINI(ctx) && ldp->ld_command == lprp)) {
		lnc_log_fatal_l("%s: Mandatory daemon has terminated.", dname);
	}

	ctx->lc_ncallbacks--;
	if (LNC_DAEMON_HAS_NO_CHILD(ctx)) {
		LNC_BROADCAST(ctx);
	}

	LNC_PROC_UNLOCK(lprp);
	LNC_UNLOCK(ctx);
}

/*
 * static int
 * daemon_stderr_init(lnc_daemon_t *UNC_RESTRICT ldp,
 *		      lnc_proc_t *UNC_RESTRICT lprp,
 *		      lnc_conf_t *UNC_RESTRICT cfp,
 *		      const char *UNC_RESTRICT fname, int rotate)
 *	Initialize stderr logging configuration for the given daemon.
 *
 *	`fname' must be the name of the stderr log file. It must be a static
 *	string literal.
 *	`rotate' must be a value of "stderr_rotate" parameter.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_stderr_init(lnc_daemon_t *UNC_RESTRICT ldp,
		   lnc_proc_t *UNC_RESTRICT lprp, lnc_conf_t *UNC_RESTRICT cfp,
		   const char *UNC_RESTRICT fname, int rotate)
{
	lnc_stderr_t	*lsep;

	if (rotate >= 0) {
		lsep = (lnc_stderr_t *)malloc(sizeof(*lsep));
		if (PFC_EXPECT_FALSE(lsep == NULL)) {
			daemon_conf_error(cfp, "Failed to allocate stderr "
					  "logging configuration.");

			return ENOMEM;
		}

		lsep->lse_fname = fname;
		lsep->lse_rotate = rotate;
	}
	else {
		lsep = NULL;
	}

	lprp->lp_stderr = lsep;

	return 0;
}

/*
 * static void
 * daemon_stderr_destroy(lnc_stderr_t *lsep)
 *	Destroy stderr logging configuration.
 */
static void
daemon_stderr_destroy(lnc_stderr_t *lsep)
{
	free(lsep);
}

/*
 * static int
 * daemon_stderr_rotate(lnc_proc_t *UNC_RESTRICT lprp,
 *			lnc_stderr_t *UNC_RESTRICT lsep)
 *	Rotate stderr log files.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_stderr_rotate(lnc_proc_t *UNC_RESTRICT lprp,
		     lnc_stderr_t *UNC_RESTRICT lsep)
{
	lnc_daemon_t	*ldp = lprp->lp_daemon;
	const char	*logdir = LNC_DAEMON_LOGDIR(ldp);
	int		err, dfd, fd, rotate = lsep->lse_rotate;

	if (rotate == 0) {
		/* Nothing to do. */
		return 0;
	}

	/* Open logging directory. */
	dfd = pfc_open_cloexec(logdir, O_RDONLY);
	if (PFC_EXPECT_FALSE(dfd == -1)) {
		err = errno;
		pfc_log_error("%s.%s: Failed to stderr logging directory: %s",
			      LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));
		goto error;
	}

	/* Rotate log files. */
	fd = pfc_frotate_openat(dfd, lsep->lse_fname, LNC_STDERR_LOGFILE_PERM,
				0, rotate);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		pfc_log_error("%s.%s: Failed to rotate stderr log files: %s",
			      LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));
		(void)close(dfd);
		goto error;
	}

	(void)close(dfd);
	(void)close(fd);

	return 0;

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static int
 * daemon_order_put(lnc_conf_t *UNC_RESTRICT cfp, pfc_cfblk_t daemon,
 *		    lnc_daemon_t *UNC_RESTRICT ldp,
 *		    const char *UNC_RESTRICT param, lnc_clevfunc_t clevent,
 *		    lnc_ordtype_t type, int index)
 *	Register the given daemon instance into the daemon order tree.
 *
 *	The target tree is determined by the order type `type' and
 *	type index `index'. `index' is always ignored if `type' is
 *	either LNC_ORDTYPE_START or LNC_ORDTYPE_STOP.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
daemon_order_put(lnc_conf_t *UNC_RESTRICT cfp, pfc_cfblk_t daemon,
		 lnc_daemon_t *UNC_RESTRICT ldp,
		 const char *UNC_RESTRICT param, lnc_clevfunc_t clevent,
		 lnc_ordtype_t type, int index)
{
	pfc_rbtree_t	*tree = lnc_daemon_order_gettree(type, index);
	lnc_ordnode_t	*onp;
	uint32_t	order;
	char		buf[32];
	int		err;

	/* Fetch UINT32 order value. */
	if (LNC_ORDTYPE_HASINDEX(type)) {
		order = pfc_conf_array_uint32at(daemon, param, index,
						LNC_ORDER_INVALID);
		if (order == LNC_ORDER_INVALID) {
			/*
			 * This daemon does not require this type of cluster
			 * event.
			 */
			return 0;
		}
	}
	else {
		order = pfc_conf_get_uint32(daemon, param, LNC_ORDER_INVALID);
		if (PFC_EXPECT_FALSE(order == LNC_ORDER_INVALID)) {
			/* This should never happen. */
			daemon_conf_error(cfp, "\"%s\" is not defined.",
					  param);

			return EINVAL;
		}
	}

	/* Allocate a new node. */
	onp = (lnc_ordnode_t *)malloc(sizeof(*onp));
	if (PFC_EXPECT_FALSE(onp == NULL)) {
		daemon_conf_error(cfp, "Failed to allocate order node.");

		return ENOMEM;
	}

	onp->lo_order= order;
	onp->lo_daemon = ldp;
	onp->lo_name = ldp->ld_name;
	onp->lo_clevent = clevent;

	err = pfc_rbtree_put(tree, &onp->lo_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_rbnode_t	*node;
		lnc_ordnode_t	*conflict;
		const char	*idx;

		PFC_ASSERT(err == EEXIST);
		node = pfc_rbtree_get(tree, LNC_ORDNODE_KEY(order));
		PFC_ASSERT(node != NULL);

		conflict = LNC_ORDNODE_NODE2PTR(node);
		PFC_ASSERT(conflict->lo_order == order);

		idx = liblnc_order_getindexstr(type, index, buf, sizeof(buf));
		daemon_conf_error(cfp, "\"%s%s\" conflicts with \"%s\": "
				  "value=%u", param, idx,
				  LNC_ORDNODE_NAME(conflict), order);
		free(onp);

		return EINVAL;
	}

	pfc_refptr_get(onp->lo_name);
	pfc_log_verbose("%s: %s%s = %u", LNC_DAEMON_NAME(ldp), param,
			liblnc_order_getindexstr(type, index, buf,
						 sizeof(buf)),
			order);

	return 0;
}

/*
 * static void
 * daemon_order_destroy(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Destroy the given UINT32 order node.
 *	This function will be called via pfc_rbtree_clear().
 *
 *	`node' must be a pointer to lo_node in lnc_ordnode_t.
 *
 * Remarks:
 *	The caller must ensure that any process associated with the given
 *	order node is not running.
 */
static void
daemon_order_destroy(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	lnc_ordnode_t	*onp = LNC_ORDNODE_NODE2PTR(node);

	pfc_refptr_put(onp->lo_name);
	free(onp);
}

/*
 * static int
 * daemon_clevent_init(void)
 *	Initialize daemon_order_tree which manages order of cluster event
 *	delivery.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
w */
static int
daemon_clevent_init(void)
{
	pfc_refptr_t	*rname;
	uint8_t		type;
	int		err = 0;

	rname = pfc_refptr_string_create(LIBLNC_UNCD_NAME);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		pfc_conf_error("launcher: Failed to copy UNC daemon name.");

		return ENOMEM;
	}

	/*
	 * Register the UNC daemon itself to daemon_order_tree associated
	 * with cluster event delivery order.
	 */
	for (type = 0; type < CLSTAT_NEVTYPES; type++) {
		pfc_rbtree_t	*tree;
		lnc_ordnode_t	*onp;

		/* Allocate a new order node. */
		onp = (lnc_ordnode_t *)malloc(sizeof(*onp));
		if (PFC_EXPECT_FALSE(onp == NULL)) {
			pfc_conf_error("launcher: Failed to allocate order "
				       "node for uncd.");
			err = ENOMEM;
			break;
		}

		onp->lo_order = LNC_ORDER_UNCD;
		onp->lo_daemon = NULL;
		onp->lo_name = rname;
		onp->lo_clevent = lnc_cluster_notify_uncd;
		pfc_refptr_get(rname);

		tree = lnc_daemon_order_gettree(LNC_ORDTYPE_CLEVENT, type);
		PFC_ASSERT_INT(pfc_rbtree_put(tree, &onp->lo_node), 0);
	}

	pfc_refptr_put(rname);

	return err;
}

/*
 * static pfc_cptr_t
 * daemon_getkey(pfc_rbnode_t *node)
 *	Return the key of daemon instance.
 *	`node' must be a pointer to ld_node in lnc_daemon_t.
 */
static pfc_cptr_t
daemon_getkey(pfc_rbnode_t *node)
{
	lnc_daemon_t	*ldp = LNC_DAEMON_NODE2PTR(node);

	return (pfc_cptr_t)LNC_DAEMON_NAME(ldp);
}

/*
 * static pfc_cptr_t
 * daemon_order_getkey(pfc_rbnode_t *node)
 *	Return the key of daemon order node.
 *	`node' must be a pointer to lo_order in lnc_ordnode_t.
 */
static pfc_cptr_t
daemon_order_getkey(pfc_rbnode_t *node)
{
	lnc_ordnode_t	*onp = LNC_ORDNODE_NODE2PTR(node);

	return LNC_ORDNODE_KEY(onp->lo_order);
}

/*
 * static pfc_cptr_t
 * daemon_proc_getkey(pfc_rbnode_t *node)
 *	Return the key of process node.
 *	`node' must be a pointer to lp_node in lnc_proc_t.
 */
static pfc_cptr_t
daemon_proc_getkey(pfc_rbnode_t *node)
{
	lnc_proc_t	*lprp = LNC_PROC_NODE2PTR(node);

	return LNC_PROC_KEY(lprp->lp_pid);
}
