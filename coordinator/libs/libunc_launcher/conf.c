/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf.c - Launcher configuration management.
 */

#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unc/liblauncher.h>
#include <pfc/util.h>
#include <pfc/debug.h>
#include <pfc/ctype.h>
#include <pfc/rbtree.h>
#include <pfc/log.h>
#include <pfc/strtoint.h>

extern const pfc_cfdef_t	daemon_conf_defs;
extern const pfc_cfdef_t	launcher_conf_defs;

/*
 * Suffix of daemon configuration file name.
 */
#define	LIBLNC_SUFF_DAEMON	"daemon"

/*
 * Maximum length of the configuration file name, including terminator.
 */
#define	LIBLNC_NAME_MAX		PFC_CONST_U(256)

/*
 * Internal buffer size to read directory entry.
 */
#define	LIBLNC_DIRENT_BUFSIZE					\
	(offsetof(struct dirent, d_name) + LIBLNC_NAME_MAX)

/*
 * Valid signal names.
 */
typedef struct {
	const char	*lns_name;		/* signal name */
	int		lns_signal;		/* signal number */
	pfc_rbnode_t	lns_node;		/* Red-Black tree node */
} lnc_signame_t;

#define	LIBLNC_SIGNAME_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), lnc_signame_t, lns_node)

#define	LIBLNC_SIGNAL_DECL(sig)			\
	{					\
		.lns_name	= #sig,		\
		.lns_signal	= SIG##sig,	\
	}

static lnc_signame_t	launcher_signals[] = {
	LIBLNC_SIGNAL_DECL(HUP),
	LIBLNC_SIGNAL_DECL(INT),
	LIBLNC_SIGNAL_DECL(QUIT),
	LIBLNC_SIGNAL_DECL(ILL),
	LIBLNC_SIGNAL_DECL(TRAP),
	LIBLNC_SIGNAL_DECL(ABRT),
	LIBLNC_SIGNAL_DECL(BUS),
	LIBLNC_SIGNAL_DECL(FPE),
	LIBLNC_SIGNAL_DECL(KILL),
	LIBLNC_SIGNAL_DECL(USR1),
	LIBLNC_SIGNAL_DECL(SEGV),
	LIBLNC_SIGNAL_DECL(USR2),
	LIBLNC_SIGNAL_DECL(PIPE),
	LIBLNC_SIGNAL_DECL(ALRM),
	LIBLNC_SIGNAL_DECL(TERM),
	LIBLNC_SIGNAL_DECL(STKFLT),
	LIBLNC_SIGNAL_DECL(CHLD),
	LIBLNC_SIGNAL_DECL(CONT),
	LIBLNC_SIGNAL_DECL(STOP),
	LIBLNC_SIGNAL_DECL(TSTP),
	LIBLNC_SIGNAL_DECL(TTIN),
	LIBLNC_SIGNAL_DECL(TTOU),
	LIBLNC_SIGNAL_DECL(URG),
	LIBLNC_SIGNAL_DECL(XCPU),
	LIBLNC_SIGNAL_DECL(XFSZ),
	LIBLNC_SIGNAL_DECL(VTALRM),
	LIBLNC_SIGNAL_DECL(PROF),
	LIBLNC_SIGNAL_DECL(WINCH),
	LIBLNC_SIGNAL_DECL(POLL),
	LIBLNC_SIGNAL_DECL(PWR),
	LIBLNC_SIGNAL_DECL(SYS),
};

/*
 * Parameter name in the "daemon" block which determines UINT32 order value.
 */
static const char	*order_params[] = {
	"start_order",		/* LNC_ORDTYPE_START */
	"stop_order",		/* LNC_ORDTYPE_STOP */
	"clevent_order",	/* LNC_ORDTYPE_CLEVENT */
};

/*
 * Context to open daemon configuration files.
 */
struct lnc_confctx
{
	pfc_refptr_t	*lcc_rpath;	/* configuration file directory */
	DIR		*lcc_dir;	/* directory handle of confdir */
	uint32_t	lcc_flags;	/* flags */

	/* Internal buffer to read directory entry. */
	union {
		struct dirent	ent;
		uint8_t		buffer[LIBLNC_DIRENT_BUFSIZE];
	} lcc_entry;
};

/*
 * Internal logging functions.
 */

#ifdef	__PFC_LOG_GNUC

#define	LNCCONF_LOG_DEBUG(ctx, format, ...)		\
	if ((ctx)->lcc_flags & LIBLNC_OFLAG_LOG) {	\
		pfc_log_debug((format), ##__VA_ARGS__);	\
	}						\

#else	/* !__PFC_LOG_GNUC */

static inline void PFC_FATTR_PRINTFLIKE(2, 3)
LNCCONF_LOG_DEBUG(lnc_confctx_t *ctx, const char *format, ...)
{
	if (ctx->lcc_flags & LIBLNC_OFLAG_LOG) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_debug_v(format, ap);
		va_end(ap);
	}
}

#endif	/* __PFC_LOG_GNUC */

/*
 * Internal prototypes.
 */
static int	lnc_openconf(lnc_confctx_t *UNC_RESTRICT ctx,
			     const char *UNC_RESTRICT confdir,
			     char *UNC_RESTRICT name,
			     lnc_conf_t *UNC_RESTRICT cfp);
static int	lnc_path_verify(const char *path);

static pfc_cptr_t	lnc_signame_getkey(pfc_rbnode_t *node);

/*
 * Red-Black tree which keeps valid signal names.
 */
static pfc_rbtree_t	launcher_signal_tree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, lnc_signame_getkey);

/*
 * static void PFC_FATTR_INIT
 * liblnc_init(void)
 *	Initialize liblnc_launcher library.
 */
static void PFC_FATTR_INIT
liblnc_init(void)
{
	pfc_rbtree_t	*tree = &launcher_signal_tree;
	lnc_signame_t	*snp;

	/* Initialize signal name tree. */
	for (snp = launcher_signals; snp < PFC_ARRAY_LIMIT(launcher_signals);
	     snp++) {
		PFC_ASSERT_INT(pfc_rbtree_put(tree, &snp->lns_node), 0);
	}
}

/*
 * const pfc_cfdef_t *
 * liblnc_getcfdef(void)
 *	Return a pointer to pfc_cfdef_t which represents definition of
 *	daemon process configuration file.
 */
const pfc_cfdef_t *
liblnc_getcfdef(void)
{
	return &daemon_conf_defs;
}

/*
 * const pfc_cfdef_t *
 * liblnc_getcfdef(void)
 *	Return a pointer to pfc_cfdef_t which represents definition of
 *	configuration file for the launcher module.
 */
const pfc_cfdef_t *
liblnc_getmodcfdef(void)
{
	return &launcher_conf_defs;
}

/*
 * int
 * liblnc_conf_opendir(lnc_confctx_t **UNC_RESTRICT ctxp,
 *		       const char *UNC_RESTRICT confdir, uint32_t flags)
 *	Open daemon configuration directory handle.
 *
 *	`confdir' must be a path to daemon configuration directory.
 *
 *	`flags' determines behavior of configuration directory handle.
 *	It is either zero or the bitwise OR of one or more of the following
 *	flags:
 *
 *	LIBLNC_OFLAG_LOG
 *	    Enable logging by PFC logging architecture.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to lnc_confctx_t
 *	is set to the buffer pointed by `ctxp', and zero is returned.
 *	The caller must close it by liblnc_conf_closedir().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
liblnc_conf_opendir(lnc_confctx_t **UNC_RESTRICT ctxp,
		    const char *UNC_RESTRICT confdir, uint32_t flags)
{
	lnc_confctx_t	*ctx;
	int		dfd, err;

	/* Create a new context. */
	ctx = (lnc_confctx_t *)malloc(sizeof(*ctx));
	if (PFC_EXPECT_FALSE(ctx == NULL)) {
		return ENOMEM;
	}

	ctx->lcc_rpath = pfc_refptr_string_create(confdir);
	if (PFC_EXPECT_FALSE(ctx->lcc_rpath == NULL)) {
		err = ENOMEM;
		goto error;
	}

	/* Open daemon configuration directory. */
	dfd = pfc_open_cloexec(confdir, O_RDONLY);
	if (PFC_EXPECT_FALSE(dfd == -1)) {
		err = errno;
		goto error;
	}

	ctx->lcc_dir = fdopendir(dfd);
	if (PFC_EXPECT_FALSE(ctx->lcc_dir == NULL)) {
		err = errno;
		(void)close(dfd);
		goto error;
	}

	ctx->lcc_flags = flags;

	*ctxp = ctx;

	return 0;

error:
	if (ctx->lcc_rpath != NULL) {
		pfc_refptr_put(ctx->lcc_rpath);
	}
	free(ctx);

	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * void
 * liblnc_conf_closedir(lnc_confctx_t *ctx)
 *	Close daemon configuration directory handle opened by the call of
 *	liblnc_conf_opendir().
 */
void
liblnc_conf_closedir(lnc_confctx_t *ctx)
{
	pfc_refptr_put(ctx->lcc_rpath);
	closedir(ctx->lcc_dir);
	free(ctx);
}

/*
 * int
 * liblnc_conf_getnext(lnc_confctx_t *UNC_RESTRICT ctx,
 *		       lnc_conf_t *UNC_RESTRICT cfp)
 *	Get next configuration file handle.
 *	This function is used to iterate daemon configuration files under
 *	the directory associated with `ctx'.
 *
 * Calling/Exit State:
 *	Upon successful completion, daemon configuration file handle is set
 *	into the buffer pointed by `cfp', and zero is returned.
 *	-1 is returned if there is no more configuration file to be read.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
liblnc_conf_getnext(lnc_confctx_t *UNC_RESTRICT ctx,
		    lnc_conf_t *UNC_RESTRICT cfp)
{
	const char	*confdir = pfc_refptr_string_value(ctx->lcc_rpath);
	int		err;

	for (;;) {
		struct dirent	*dp;

		err = readdir_r(ctx->lcc_dir, &ctx->lcc_entry.ent, &dp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		if (dp == NULL) {
			err = -1;
			break;
		}

		/* Try to open this file. */
		err = lnc_openconf(ctx, confdir, dp->d_name, cfp);
		if (PFC_EXPECT_TRUE(err >= 0)) {
			/* Succeeded, or fatal error. */
			break;
		}
	}

	return err;
}

/*
 * int
 * liblnc_cmdmap_create(lnc_cmdmap_t **UNC_RESTRICT cmapp,
 *			lnc_conf_t *UNC_RESTRICT cfp,
 *			const char *UNC_RESTRICT name,
 *			lnc_errmsg_t *UNC_RESTRICT emsg)
 *	Create a lnc_cmdmap_t instance which keeps "command" map value
 *	associated with the given name.
 *
 *	`cfp' must be a configuration file handle obtained by
 *	liblnc_conf_getnext().
 *	`name' msut be a non-NULL string which represents "command" map key.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to lnc_cmdmap_t is
 *	set to the buffer pointed by `cmapp', and zero is returned.
 *	The caller must destroy it by liblnc_cmdmap_destroy().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	Detailed error message is stored to the buffer pointed by `emsg'.
 */
int
liblnc_cmdmap_create(lnc_cmdmap_t **UNC_RESTRICT cmapp,
		     lnc_conf_t *UNC_RESTRICT cfp,
		     const char *UNC_RESTRICT name,
		     lnc_errmsg_t *UNC_RESTRICT emsg)
{
	lnc_cmdmap_t	*cmap;
	pfc_cfblk_t	cmd;
	pfc_refptr_t	*rstr;
	pfc_listm_t	argv;
	const char	*path, *proc;
	int		err, i, argc, capacity;

	cmap = (lnc_cmdmap_t *)malloc(sizeof(*cmap));
	if (PFC_EXPECT_FALSE(cmap == NULL)) {
		LIBLNC_ERRMSG_SET(emsg, "Unable to allocate command map");

		return ENOMEM;
	}

	cmd = pfc_conf_get_map(cfp->lcf_conf, "command", name);

	cmap->cm_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(cmap->cm_name == NULL)) {
		LIBLNC_ERRMSG_SET(emsg, "Unable to copy command map name");
		err = ENOMEM;
		goto error;
	}

	/* Fetch executable file path .*/
	path = pfc_conf_get_string(cmd, "path", NULL);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		LIBLNC_ERRMSG_SET(emsg, "Command is not defined");
		err = EINVAL;
		goto error_name;
	}

	cmap->cm_path = pfc_refptr_string_create(path);
	if (PFC_EXPECT_FALSE(cmap->cm_path == NULL)) {
		LIBLNC_ERRMSG_SET(emsg, "Unable to copy executable file path");
		err = ENOMEM;
		goto error_name;
	}

	/* Create a vector which keeps command line arguments. */
	argc = pfc_conf_array_size(cmd, "args");
	capacity = (argc < 0) ? 1 : (argc + 1);
	err = pfc_vector_create_ref(&cmap->cm_args, pfc_refptr_string_ops(),
				    capacity, 10);
	if (PFC_EXPECT_FALSE(err != 0)) {
		LIBLNC_ERRMSG_SET(emsg, "Unable to create argument vector");
		goto error_path;
	}
	argv = cmap->cm_args;

	/* Fetch process name. */
	proc = pfc_conf_get_string(cmd, "name", NULL);
	if (proc == NULL) {
		rstr = cmap->cm_path;
	}
	else {
		rstr = pfc_refptr_string_create(proc);
		if (PFC_EXPECT_FALSE(rstr == NULL)) {
			LIBLNC_ERRMSG_SET(emsg, "Unable to copy process name");
			err = ENOMEM;
			goto error_args;
		}
	}

	err = pfc_listm_push_tail(argv, rstr);
	if (proc != NULL) {
		pfc_refptr_put(rstr);
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		LIBLNC_ERRMSG_SET(emsg, "Unable to set process name");
		goto error_args;
	}

	/* Fetch command line arguments. */
	for (i = 0; i < argc; i++) {
		const char	*arg;

		arg = pfc_conf_array_stringat(cmd, "args", i, NULL);
		PFC_ASSERT(arg != NULL);
		rstr = pfc_refptr_string_create(arg);
		if (PFC_EXPECT_FALSE(rstr == NULL)) {
			LIBLNC_ERRMSG_SET(emsg, "Unable to copy argument");
			err = ENOMEM;
			goto error_args;
		}

		err = pfc_listm_push_tail(argv, rstr);
		pfc_refptr_put(rstr);
		if (PFC_EXPECT_FALSE(err != 0)) {
			LIBLNC_ERRMSG_SET(emsg, "Unable to set argument");
			goto error_args;
		}
	}

	*cmapp = cmap;

	return 0;


error_args:
	pfc_listm_destroy(cmap->cm_args);

error_path:
	pfc_refptr_put(cmap->cm_path);

error_name:
	pfc_refptr_put(cmap->cm_name);

error:
	free(cmap);

	return err;
}

/*
 * void
 * liblnc_cmdmap_destroy(lnc_cmdmap_t *cmap)
 *	Destroy the given command map value.
 */
void
liblnc_cmdmap_destroy(lnc_cmdmap_t *cmap)
{
	pfc_refptr_put(cmap->cm_name);
	pfc_refptr_put(cmap->cm_path);
	pfc_listm_destroy(cmap->cm_args);
	free(cmap);
}

/*
 * int
 * liblnc_getsignal(const char *name)
 *	Return a signal number associated with the given name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a valid signal number associated with the
 *	given name is returned.
 *	Othewise zero is returned.
 */
int
liblnc_getsignal(const char *name)
{
	pfc_rbnode_t	*node;
	uint32_t	num;
	int		err;

	node = pfc_rbtree_get(&launcher_signal_tree, (pfc_cptr_t)name);
	if (node != NULL) {
		lnc_signame_t	*snp = LIBLNC_SIGNAME_NODE2PTR(node);

		return snp->lns_signal;
	}

	/* Try to parse the given string as an unsigned integer. */
	err = pfc_strtou32(name, &num);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return 0;
	}

	if (PFC_EXPECT_FALSE(num >= NSIG)) {
		return 0;
	}

	return (int)num;
}

/*
 * pfc_refptr_t *
 * liblnc_getconfdir(void)
 *	Return a pointer to string refptr object which contains path to
 *	daemon configuration directory. The caller must release the returned
 *	refptr object by pfc_refptr_put().
 *
 *	NULL is returned if the system is low on memory.
 *
 * Remarks:
 *	This function returns default configuration directory path if it
 *	failed to parse module configuration file.
 */
pfc_refptr_t *
liblnc_getconfdir(void)
{
	pfc_conf_t	conf;
	pfc_cfblk_t	options;
	pfc_refptr_t	*primary, *secondary, *rpath;
	const char	*dir;
	int		err;

	/* Determine module directory. */
	options = pfc_sysconf_get_block("options");
	dir = pfc_conf_get_string(options, "module_dir", UNC_MODULEDIR);

	/* Try to open launcher module configuration file. */
	primary = pfc_refptr_sprintf("%s/launcher.conf", UNC_MODCONFDIR);
	if (PFC_EXPECT_FALSE(primary == NULL)) {
		goto defpath;
	}

	secondary = pfc_refptr_sprintf("%s/launcher.conf", dir);
	if (PFC_EXPECT_FALSE(primary == NULL)) {
		pfc_refptr_put(primary);
		goto defpath;
	}

	err = pfc_conf_refopen2(&conf, primary, secondary,
				&launcher_conf_defs);
	pfc_refptr_put(primary);
	pfc_refptr_put(secondary);

	if (PFC_EXPECT_FALSE(err != 0)) {
		goto defpath;
	}

	/*
	 * Derive configuration directory path from the module configuration
	 * file.
	 */
	options = pfc_conf_get_block(conf, "options");
	dir = pfc_conf_get_string(options, "conf_dir", LIBLNC_CONFDIR);
	rpath = pfc_refptr_string_create(dir);
	pfc_conf_close(conf);

	return rpath;

defpath:
	return pfc_refptr_string_create(LIBLNC_CONFDIR);
}

/*
 * const char *
 * liblnc_order_getparamname(lnc_ordtype_t type)
 *	Return the parameter name in the "daemon" block associated with
 *	the given order type.
 */
const char *
liblnc_order_getparamname(lnc_ordtype_t type)
{
	uint32_t	idx = (uint32_t)type;

	PFC_ASSERT(idx < PFC_ARRAY_CAPACITY(order_params));

	return order_params[idx];
}

/*
 * const char *
 * liblnc_order_getindexstr(lnc_ordtype_t type, int index, char *buf,
 *			     size_t size)
 *	Create a string which represents daemon order type index.
 *	This function is used to generate log message.
 */
const char *
liblnc_order_getindexstr(lnc_ordtype_t type, int index, char *buf, size_t size)
{
	const char	*idx;

	if (LNC_ORDTYPE_HASINDEX(type)) {
		snprintf(buf, size, "[%d]", index);
		idx = buf;
	}
	else {
		idx = "";
	}

	return idx;
}

/*
 * static int
 * lnc_openconf(lnc_confctx_t *UNC_RESTRICT ctx,
 *		const char *UNC_RESTRICT confdir, char *UNC_RESTRICT name,
 *		lnc_conf_t *UNC_RESTRICT cfp)
 *	Open the given daemon configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, a daemon configuration file handle is set
 *	to the buffer pointed by `cfp', and zero is returned.
 *	-1 is returned if the given file must be ignored.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
lnc_openconf(lnc_confctx_t *UNC_RESTRICT ctx, const char *UNC_RESTRICT confdir,
	     char *UNC_RESTRICT name, lnc_conf_t *UNC_RESTRICT cfp)
{
	uint8_t		c = *name;
	uint32_t	len;
	pfc_refptr_t	*rpath;
	struct stat	sbuf;
	const char	*path;
	char		*p, *suff;
	int		err;

	/* The first character must be an alphabet. */
	if (PFC_EXPECT_FALSE(c >= 0x7fU || pfc_isalpha_u(c) == 0)) {
		LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Unexpected character in "
				  "filename.", name);

		return -1;
	}

	/* Search for a dot in the filename. */
	for (p = name + 1, len = 1; (c = *p) != '.'; p++, len++) {
		if (PFC_EXPECT_FALSE(c == '\0')) {
			LNCCONF_LOG_DEBUG(ctx,
					  "%s: SKIP: No file name suffix.",
					  name);

			return -1;
		}

		if (PFC_EXPECT_FALSE(len > LNC_DAEMON_NAMELEN_MAX)) {
			LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Too long filename",
					  name);

			return -1;
		}

		/*
		 * Daemon name must consists of alphanumeric characters and
		 * underscores.
		 */
		if (PFC_EXPECT_FALSE(c >= 0x7fU ||
				     (pfc_isalnum_u(c) == 0 && c != '_'))) {
			LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Unexpected character"
					  " in filename.", name);

			return -1;
		}
	}

	/* Check file name suffix. */
	suff = p;
	p++;
	if (PFC_EXPECT_FALSE(strcmp(p, LIBLNC_SUFF_DAEMON) != 0)) {
		LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Unexpected suffix.", name);

		return -1;
	}

	/* "uncd" can not be used as the daemon name. */
	if (suff - name == LIBLNC_UNCD_NAMELEN &&
	    strncmp(name, LIBLNC_UNCD_NAME, LIBLNC_UNCD_NAMELEN) == 0) {
		LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Reserved daemon name.",
				  name);

		return -1;
	}

	/* Create path to daemon configuration file. */
	rpath = pfc_refptr_sprintf("%s/%s", confdir, name);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		return ENOMEM;
	}

	/* Ignore if this file is not a regular file. */
	path = pfc_refptr_string_value(rpath);
	if (PFC_EXPECT_FALSE(stat(path, &sbuf) != 0)) {
		LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: stat(2) failed: %s", name,
				  strerror(errno));
		err = -1;
		goto error;
	}

	if (PFC_EXPECT_FALSE(!S_ISREG(sbuf.st_mode))) {
		LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Not a regular file: 0%o",
				  name, sbuf.st_mode);
		err = -1;
		goto error;
	}

	/* Ensure that the configuration file is safe. */
	err = lnc_path_verify(path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		LNCCONF_LOG_DEBUG(ctx, "%s: SKIP: Unsafe: %s", name,
				  strerror(err));
		err = -1;
		goto error;
	}

	LNCCONF_LOG_DEBUG(ctx, "Configuration file: %s", path);

	/*
	 * Create name of the daemon.
	 * We can chop off file name suffix pointed by `name' because it points
	 * the buffer in lnc_confctx_t.
	 */
	*suff = '\0';
	cfp->lcf_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(cfp->lcf_name == NULL)) {
		err = ENOMEM;
		goto error;
	}

	/* Try to open configuration file. */
	err = pfc_conf_refopen(&cfp->lcf_conf, rpath, &daemon_conf_defs);
	pfc_refptr_put(rpath);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_refptr_put(cfp->lcf_name);
		cfp->lcf_name = NULL;
	}

	return err;

error:
	pfc_refptr_put(rpath);

	return err;
}

/*
 * static int
 * lnc_path_verify(const char *path)
 *	Ensure that the given daemon configuration file path is safe.
 *
 * Calling/Exit State:
 *	Zero is returned if the given configuration file path is safe.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
lnc_path_verify(const char *path)
{
	char	*cpath;
	int	err;

	/*
	 * We need to copy the specified path because pfc_is_safepath()
	 * always breaks the string.
	 */
	cpath = strdup(path);
	if (PFC_EXPECT_FALSE(cpath == NULL)) {
		return ENOMEM;
	}

	err = pfc_is_safepath(cpath);
	free(cpath);

	return err;
}

/*
 * static pfc_cptr_t
 * lnc_signame_getkey(pfc_rbnode_t *node)
 *	Return the Red-Black tree key of lnc_signame_t.
 *	`node' must be a pointer to lns_node in lnc_signame_t.
 */
static pfc_cptr_t
lnc_signame_getkey(pfc_rbnode_t *node)
{
	lnc_signame_t	*snp = LIBLNC_SIGNAME_NODE2PTR(node);

	return (pfc_cptr_t)snp->lns_name;
}
