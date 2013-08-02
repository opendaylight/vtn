/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CMDUTIL_H
#define	_PFC_CMDUTIL_H

/*
 * Miscellaneous utilities for PFC commands.
 */

#include <errno.h>
#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/conf.h>

PFC_C_BEGIN_DECL

/*
 * PID file handle.
 */
typedef int	pfc_pidf_t;

/*
 * Error message handler.
 */
typedef void	(*pfccmd_err_t)(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);

/*
 * Prototypes.
 */
extern int		pidfile_open(pfc_pidf_t *PFC_RESTRICT pfp,
				     const char *PFC_RESTRICT path);
extern int		pidfile_open_rdonly(pfc_pidf_t *PFC_RESTRICT pfp,
					    const char *PFC_RESTRICT path);
extern void		pidfile_close(pfc_pidf_t pf);
extern int		pidfile_install(pfc_pidf_t pf, pid_t *ownerp);
extern int		pidfile_getowner(pfc_pidf_t pf, pid_t *ownerp);
extern void		pidfile_unlink(const char *path);
extern const char	*pidfile_default_path(void);

extern pfc_refptr_t	*progname_init(const char *PFC_RESTRICT arg0,
				       const char **PFC_RESTRICT namepp);

extern int	pfccmd_switchuser(pfc_cfblk_t options, pfc_bool_t daemon,
				  pfccmd_err_t errfunc);

PFC_C_END_DECL

#endif	/* !_PFC_CMDUTIL_H */
