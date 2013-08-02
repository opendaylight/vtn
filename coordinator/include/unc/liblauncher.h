/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_LIBLAUNCHER_H
#define	_UNC_LIBLAUNCHER_H

/*
 * APIs provided by libunc_launcher library, which handles UNC daemon launcher.
 */

#include <signal.h>
#include <unc/base.h>
#include <unc/lnc_types.h>
#include <pfc/refptr.h>
#include <pfc/conf.h>
#include <pfc/conf_parser.h>
#include <pfc/util.h>
#include <pfc/listmodel.h>

UNC_C_BEGIN_DECL

/*
 * Daemon name associated with the UNC daemon.
 */
#define	LIBLNC_UNCD_NAME	"uncd"
#define	LIBLNC_UNCD_NAMELEN	PFC_CONST_U(4)

/*
 * Default path to daemon configuration directory.
 */
#define	LIBLNC_CONFDIR		UNC_SYSCONFDIR "/launcher.d"

/*
 * Context to open daemon configuration files.
 */
struct lnc_confctx;
typedef struct lnc_confctx	lnc_confctx_t;

/*
 * Daemon configuration file handle.
 */
typedef struct {
	pfc_refptr_t	*lcf_name;	/* name of this configuration */
	pfc_conf_t	lcf_conf;	/* configuration file handle */
} lnc_conf_t;

/*
 * Contents of the "command" map value.
 */
typedef struct {
	pfc_refptr_t	*cm_name;	/* map name */
	pfc_refptr_t	*cm_path;	/* executable file path */
	pfc_listm_t	cm_args;	/* arguments, including argv[0] */
} lnc_cmdmap_t;

#define	LIBLNC_CMDMAP_NAME(cmap)			\
	pfc_refptr_string_value((cmap)->cm_name)
#define	LIBLNC_CMDMAP_PATH(cmap)			\
	pfc_refptr_string_value((cmap)->cm_path)

/*
 * Error message buffer.
 */
#define	LIBLNC_ERRBUFSIZE	PFC_CONST_U(128)

typedef struct {
	char	le_message[LIBLNC_ERRBUFSIZE];
} lnc_errmsg_t;

#define	LIBLNC_ERRMSG_SET(errmsg, message)				\
	pfc_strlcpy((errmsg)->le_message, (message),			\
		    sizeof((errmsg)->le_message))

/*
 * Default daemon attributes.
 */
#define	LIBLNC_DMCONF_DEF_command		NULL
#define	LIBLNC_DMCONF_DEF_process_type		LNC_PROCTYPE_UNSPEC
#define	LIBLNC_DMCONF_DEF_uncd			PFC_FALSE
#define	LIBLNC_DMCONF_DEF_start_wait		PFC_FALSE
#define	LIBLNC_DMCONF_DEF_start_timeout		PFC_CONST_U(10000)
#define	LIBLNC_DMCONF_DEF_stop			NULL
#define	LIBLNC_DMCONF_DEF_stop_signal		"TERM"
#define	LIBLNC_DMCONF_DEF_stop_timeout		PFC_CONST_U(10000)
#define	LIBLNC_DMCONF_DEF_stderr_rotate		10

/*
 * Helper macros to fetch daemon attributes.
 */
#define	LIBLNC_DMCONF_GET(blk, type, name)				\
	pfc_conf_get_##type((blk), #name, LIBLNC_DMCONF_DEF_##name)

/*
 * Flags for liblnc_conf_opendir().
 */
#define	LIBLNC_OFLAG_LOG	PFC_CONST_U(0x1)	/* enable log */

/*
 * Prototypes.
 */
extern const pfc_cfdef_t	*liblnc_getcfdef(void);
extern const pfc_cfdef_t	*liblnc_getmodcfdef(void);

extern pfc_refptr_t	*liblnc_getconfdir(void);
extern const char	*liblnc_order_getparamname(lnc_ordtype_t type);
extern const char	*liblnc_order_getindexstr(lnc_ordtype_t type,
						  int index, char *buf,
						  size_t size);

extern int	liblnc_conf_opendir(lnc_confctx_t **UNC_RESTRICT ctxp,
				    const char *UNC_RESTRICT confdir,
				    uint32_t flags);
extern void	liblnc_conf_closedir(lnc_confctx_t *ctx);
extern int	liblnc_conf_getnext(lnc_confctx_t *UNC_RESTRICT ctx,
				    lnc_conf_t *UNC_RESTRICT cfp);
extern int	liblnc_cmdmap_create(lnc_cmdmap_t **UNC_RESTRICT cmapp,
				     lnc_conf_t *UNC_RESTRICT cfp,
				     const char *UNC_RESTRICT name,
				     lnc_errmsg_t *UNC_RESTRICT emsg);
extern void	liblnc_cmdmap_destroy(lnc_cmdmap_t *cmap);
extern int	liblnc_getsignal(const char *name);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * liblnc_conf_close(lnc_conf_t *cfp)
 *	Close the given daemon configuration file handle.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
liblnc_conf_close(lnc_conf_t *cfp)
{
	pfc_refptr_put(cfp->lcf_name);
	pfc_conf_close(cfp->lcf_conf);
}

UNC_C_END_DECL

#endif	/* !_UNC_LIBLAUNCHER_H */
