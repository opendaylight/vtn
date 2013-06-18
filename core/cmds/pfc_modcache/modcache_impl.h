/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODCACHE_MODCACHE_IMPL_H
#define	_PFC_MODCACHE_MODCACHE_IMPL_H

/*
 * Common definitions for pfc_modcache command.
 */

#include <errno.h>
#include <pfc/base.h>
#include <pfc/util.h>
#include <pfc/exstatus.h>
#include <pfc/debug.h>
#include <cmdutil.h>
#include <modcache.h>

/*
 * String table instance.
 */
struct strtable;
typedef struct strtable		strtable_t;

/*
 * Path to module directory.
 */
extern const char	*module_dir;

/*
 * Path to module cache directory.
 */
extern const char	*cache_dir;

/*
 * Determine we should wait for the module directory lock to be released.
 */
extern pfc_bool_t	module_lock_wait;

/*
 * Prototypes.
 */
extern void	fatal(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2) PFC_FATTR_NORETURN;
extern void	warning(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	debug_printf(int level, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);

extern void	path_verify(const char *path);
extern void	not_allowed(void) PFC_FATTR_NORETURN;

extern pfc_flock_t	module_dir_lock(pfc_bool_t write_mode);
extern void		module_dir_unlock(pfc_flock_t lock);

extern void	cache_update(void);
extern void	cache_dump(void);

extern void		strtable_create(strtable_t **tblpp);
extern uint32_t		strtable_add(strtable_t *PFC_RESTRICT tblp,
				     pfc_refptr_t *PFC_RESTRICT rstr);
extern const char	*strtable_finalize(strtable_t *PFC_RESTRICT tblp,
					   uint32_t *PFC_RESTRICT sizep);

#endif	/* !_PFC_MODCACHE_MODCACHE_IMPL_H */
