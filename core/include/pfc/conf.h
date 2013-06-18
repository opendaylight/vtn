/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CONF_H
#define	_PFC_CONF_H

/*
 * Definitions for PFC configuration file API.
 */

#include <pfc/conf_parser.h>
#include <pfc/refptr.h>
#include <pfc/listmodel.h>
#include <stdarg.h>
#include <errno.h>

PFC_C_BEGIN_DECL

/*
 * Configuration file handle.
 */
struct __pfc_conf;
typedef struct __pfc_conf	*pfc_conf_t;

/*
 * Parameter block handle.
 */
typedef uint32_t	pfc_cfblk_t;

/*
 * Invalid handle.
 */
#define	PFC_CONF_INVALID	((pfc_conf_t)NULL)
#define	PFC_CFBLK_INVALID	(0U)

/*
 * Error handler for configuration file parser.
 * This function must take the same arguments as vprintf(3).
 */
typedef void	(*pfc_conf_err_t)(const char *fmt, va_list ap);

/*
 * Prototypes.
 */
extern int	pfc_conf_open(pfc_conf_t *PFC_RESTRICT confp,
			      const char *PFC_RESTRICT path,
			      const pfc_cfdef_t *PFC_RESTRICT defs);
extern int	pfc_conf_open_ex(pfc_conf_t *PFC_RESTRICT confp,
				 const char *PFC_RESTRICT path,
				 const pfc_cfdef_t *PFC_RESTRICT defs,
				 pfc_conf_err_t errfunc);
extern int	pfc_conf_refopen(pfc_conf_t *PFC_RESTRICT confp,
				 pfc_refptr_t *PFC_RESTRICT rpath,
				 const pfc_cfdef_t *PFC_RESTRICT defs);
extern int	pfc_conf_refopen_ex(pfc_conf_t *PFC_RESTRICT confp,
				    pfc_refptr_t *PFC_RESTRICT rpath,
				    const pfc_cfdef_t *PFC_RESTRICT defs,
				    pfc_conf_err_t errfunc);
extern int	pfc_conf_refopen2(pfc_conf_t *PFC_RESTRICT confp,
				  pfc_refptr_t *PFC_RESTRICT primary,
				  pfc_refptr_t *PFC_RESTRICT secondary,
				  const pfc_cfdef_t *PFC_RESTRICT defs);
extern int	pfc_conf_reload(pfc_conf_t conf);
extern void	pfc_conf_close(pfc_conf_t conf);

extern void	pfc_conf_error(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);
extern void	pfc_conf_verror(const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(1, 0);

extern const char	*pfc_conf_get_path(pfc_conf_t conf);
extern const char	*pfc_conf_get_secondary(pfc_conf_t conf);
extern pfc_bool_t	pfc_conf_is_primary_loaded(pfc_conf_t conf);
extern pfc_bool_t	pfc_conf_is_secondary_loaded(pfc_conf_t conf);

extern pfc_cfblk_t	pfc_conf_get_block(pfc_conf_t PFC_RESTRICT conf,
					   const char *PFC_RESTRICT bname);
extern pfc_cfblk_t	pfc_conf_get_map(pfc_conf_t PFC_RESTRICT conf,
					 const char *mname,
					 const char *key);

extern int	pfc_conf_get_mapkeys(pfc_conf_t PFC_RESTRICT conf,
				     const char *PFC_RESTRICT mname,
				     pfc_listm_t *PFC_RESTRICT keysp);

extern uint8_t		pfc_conf_get_byte(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name,
					  uint8_t defvalue);
extern const char	*pfc_conf_get_string(pfc_cfblk_t block,
					     const char *name,
					     const char *defvalue);
extern pfc_bool_t	pfc_conf_get_bool(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name,
					  pfc_bool_t defvalue);
extern int32_t		pfc_conf_get_int32(pfc_cfblk_t block,
					   const char *PFC_RESTRICT name,
					   int32_t defvalue);
extern uint32_t		pfc_conf_get_uint32(pfc_cfblk_t block,
					    const char *PFC_RESTRICT name,
					    uint32_t defvalue);
extern int64_t		pfc_conf_get_int64(pfc_cfblk_t block,
					   const char *PFC_RESTRICT name,
					   int64_t defvalue);
extern uint64_t		pfc_conf_get_uint64(pfc_cfblk_t block,
					    const char *PFC_RESTRICT name,
					    uint64_t defvalue);
extern pfc_long_t	pfc_conf_get_long(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name,
					  pfc_long_t defvalue);
extern pfc_ulong_t	pfc_conf_get_ulong(pfc_cfblk_t block,
					   const char *PFC_RESTRICT name,
					   pfc_ulong_t defvalue);

extern void	pfc_conf_copy_string(pfc_cfblk_t block,
				     const char *name, const char *defvalue,
				     char *PFC_RESTRICT buffer,
				     uint32_t bufsize);

extern pfc_bool_t	pfc_conf_is_defined(pfc_cfblk_t block,
					    const char *PFC_RESTRICT name);
extern pfc_bool_t	pfc_conf_is_array(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name);
extern int		pfc_conf_array_size(pfc_cfblk_t block,
					    const char *name);

extern uint8_t		pfc_conf_array_byteat(pfc_cfblk_t block,
					      const char *PFC_RESTRICT name,
					      uint32_t index,
					      uint8_t defvalue);
extern const char	*pfc_conf_array_stringat(pfc_cfblk_t block,
						 const char *name,
						 uint32_t index,
						 const char *defvalue);
extern pfc_bool_t	pfc_conf_array_boolat(pfc_cfblk_t block,
					      const char *PFC_RESTRICT name,
					      uint32_t index,
					      pfc_bool_t defvalue);
extern int32_t		pfc_conf_array_int32at(pfc_cfblk_t block,
					       const char *PFC_RESTRICT name,
					       uint32_t index,
					       int32_t defvalue);
extern uint32_t		pfc_conf_array_uint32at(pfc_cfblk_t block,
						const char *PFC_RESTRICT name,
						uint32_t index,
						uint32_t defvalue);
extern int64_t		pfc_conf_array_int64at(pfc_cfblk_t block,
					       const char *PFC_RESTRICT name,
					       uint32_t index,
					       int64_t defvalue);
extern uint64_t		pfc_conf_array_uint64at(pfc_cfblk_t block,
						const char *PFC_RESTRICT name,
						uint32_t index,
						uint64_t defvalue);
extern pfc_long_t	pfc_conf_array_longat(pfc_cfblk_t block,
					      const char *PFC_RESTRICT name,
					      uint32_t index,
					      pfc_long_t defvalue);
extern pfc_ulong_t	pfc_conf_array_ulongat(pfc_cfblk_t block,
					       const char *PFC_RESTRICT name,
					       uint32_t index,
					       pfc_ulong_t defvalue);

extern void	pfc_conf_array_copy_string(pfc_cfblk_t block,
					   const char *name, uint32_t index,
					   const char *defvalue,
					   char *PFC_RESTRICT buffer,
					   uint32_t bufsize);

extern int	pfc_conf_array_byte_range(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name,
					  uint32_t start, uint32_t nelems,
					  uint8_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_string_range(pfc_cfblk_t block,
					    const char *PFC_RESTRICT name,
					    uint32_t start, uint32_t nelems,
					    const char **PFC_RESTRICT buffer);
extern int	pfc_conf_array_bool_range(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name,
					  uint32_t start, uint32_t nelems,
					  pfc_bool_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_int32_range(pfc_cfblk_t block,
					   const char *PFC_RESTRICT name,
					   uint32_t start, uint32_t nelems,
					   int32_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_uint32_range(pfc_cfblk_t block,
					    const char *PFC_RESTRICT name,
					    uint32_t start, uint32_t nelems,
					    uint32_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_int64_range(pfc_cfblk_t block,
					   const char *PFC_RESTRICT name,
					   uint32_t start, uint32_t nelems,
					   int64_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_uint64_range(pfc_cfblk_t block,
					    const char *PFC_RESTRICT name,
					    uint32_t start, uint32_t nelems,
					    uint64_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_long_range(pfc_cfblk_t block,
					  const char *PFC_RESTRICT name,
					  uint32_t start, uint32_t nelems,
					  pfc_long_t *PFC_RESTRICT buffer);
extern int	pfc_conf_array_ulong_range(pfc_cfblk_t block,
					   const char *PFC_RESTRICT name,
					   uint32_t start, uint32_t nelems,
					   pfc_ulong_t *PFC_RESTRICT buffer);

extern pfc_conf_t	pfc_sysconf_open(void);
extern const char	*pfc_sysconf_get_path(void);
extern pfc_cfblk_t	pfc_sysconf_get_block(const char *bname);
extern pfc_cfblk_t	pfc_sysconf_get_map(const char *mname, const char *key);

extern int	pfc_sysconf_get_mapkeys(const char *PFC_RESTRICT mname,
					pfc_listm_t *PFC_RESTRICT keysp);

PFC_C_END_DECL

#endif	/* !_PFC_CONF_H */
