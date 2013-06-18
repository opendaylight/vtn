/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_CMD_MODCACHE_H
#define	_PFC_LIBPFC_CMD_MODCACHE_H

/*
 * Definitions for PFC module information cache.
 */

#include <elf.h>
#include <sys/stat.h>
#include <pfc/base.h>
#include <pfc/module.h>
#include <pfc/byteorder.h>

PFC_C_BEGIN_DECL

/*
 * The following is the format of module cache file.
 *
 *    +--------------------+ offset: 0x0
 *    |                    |
 *    |    Cache Header    |
 *    |                    |
 *    +--------------------+ offset: mch_modoff
 *    |                    |
 *    |   Module Section   |
 *    |                    |
 *    +--------------------+ offset: mch_depoff
 *    |                    |
 *    | Dependency Section |
 *    |                    |
 *    +--------------------+ offset: mch_rdepoff
 *    |                    |
 *    | Reverse Dependency |
 *    |       Section      |
 *    |                    |
 *    +--------------------+ offset: mch_stroff
 *    |                    |
 *    |    String Table    |
 *    |                    |
 *    +--------------------+
 *
 * - Cache Header
 *   The cache header defines attributes of the cache file.
 *   It contains all attributes required to retrieve cached data, such as
 *   byte order type, and file offset to each sections.
 *
 * - Module Section
 *   This section contains information about all installed modules.
 *   This section is implemented as an array of modch_ent_t, and each element
 *   represents one module. modch_ent_t entries are sorted in module load
 *   order.
 *
 * - Dependency Section
 *   This section contains module dependencies.
 *   This section is implemented as an array of modch_dep_t. Each element
 *   represents one module depended by other module.
 *
 *   modch_dep_t array is sorted in the same order of module section entries.
 *   If a module depends on two or more modules, same number of modch_dep_t
 *   elements are associated. modch_dep_t element with MODCH_DEP_NONE in
 *   mcd_module is used as terminator of depended module list.
 *
 *   The following example defines module dependencies for three modules.
 *
 *   + The module associated with the first modch_ent_t element depends on
 *     two modules. So two modch_dep_t elements are assigned. modch_dep_t[2]
 *     element has a MODCH_DEP_NONE as mcd_module, so it means the end of
 *     module dependencies for the first modch_ent_t element.
 *
 *   + Only one modch_dep_t[3] element is assigned for modch_ent_t[1], and
 *     mcd_module of modch_dep_t[3] is MODCH_DEP_NONE. It means the module
 *     associated with modch_ent_t[1] element has no module dependency.
 *
 *   + modch_dep_t[4] and modch_dep_t[5] are assigned to the module associated
 *     with modch_ent_t[2]. It means the module depends on one module
 *     associated with th emodch_dep_t[4]. modch_dep_t[5] is a terminator
 *     for dependency list.
 *
 *    +--------------------+   <--- Start of dependencies for modch_ent_t[0]
 *    |   modch_dep_t[0]   |
 *    |(mcd_module != NONE)|
 *    +--------------------+
 *    |   modch_dep_t[1]   |
 *    |(mcd_module != NONE)|
 *    +--------------------+
 *    |   modch_dep_t[2]   |
 *    |(mcd_module == NONE)|
 *    +--------------------+   <--- Start of dependencies for modch_ent_t[1]
 *    |   modch_dep_t[3]   |
 *    |(mcd_module == NONE)|
 *    +--------------------+   <--- Start of dependencies for modch_ent_t[2]
 *    |   modch_dep_t[4]   |
 *    |(mcd_module != NONE)|
 *    +--------------------+
 *    |   modch_dep_t[5]   |
 *    |(mcd_module == NONE)|
 *    +--------------------+
 *
 * - Reverse dependency section.
 *   This section has the same structure with the dependency section, but
 *   the this section keeps reverse dependency link. That is, modch_dep_t
 *   elements associated with the module section represents all modules which
 *   depend on the module.
 *
 *   Unlike the dependency section, all modch_dep_t entries in the reverse
 *   dependency section has zero as module version field.
 *
 * - String Table
 *   String table, which has the same structure as ELF string table.
 *   The first entry, pointed by offset zero, is an empty string.
 */

/*
 * Alignment of section offset.
 * The value must be power of 2.
 */
#define	MODCH_SECT_ALIGN	PFC_CONST_U(8)

/*
 * Number of identifier bytes in the cache header.
 */
#define	MODCH_NIDENT		PFC_CONST_U(8)

/*
 * The cache header.
 */
typedef struct {
	uint8_t		mch_ident[MODCH_NIDENT];	/* identifier */
	uint32_t	mch_modoff;	/* module section offset */
	uint32_t	mch_modsize;	/* module section size */
	uint32_t	mch_depoff;	/* dependency section offset */
	uint32_t	mch_depsize;	/* dependency section size */
	uint32_t	mch_rdepoff;	/* reverse dependency section offset */
	uint32_t	mch_rdepsize;	/* reverse dependency section size */
	uint32_t	mch_stroff;	/* string table offset */
	uint32_t	mch_strsize;	/* string table size */
} modch_head_t;

typedef const modch_head_t	modch_chead_t;

/*
 * Identifier[0..3]: Magic number
 */
#define	MODCH_IDT_MAGIC0	0
#define	MODCH_IDT_MAGIC1	1
#define	MODCH_IDT_MAGIC2	2
#define	MODCH_IDT_MAGIC3	3

#define	MODCH_MAGIC0		0x7f
#define	MODCH_MAGIC1		'P'
#define	MODCH_MAGIC2		'M'
#define	MODCH_MAGIC3		'D'

/*
 * Identifier[4]: Byte order
 */
#define	MODCH_IDT_ORDER		4

#define	MODCH_ORDER_LITTLE	1		/* little endian */
#define	MODCH_ORDER_BIG		2		/* big endian */

#ifdef	PFC_LITTLE_ENDIAN
#define	MODCH_ORDER_NATIVE	MODCH_ORDER_LITTLE
#else	/* !PFC_LITTLE_ENDIAN */
#define	MODCH_ORDER_NATIVE	MODCH_ORDER_BIG
#endif	/* PFC_LITTLE_ENDIAN */

/*
 * Identifier[5]: ELF class.
 */
#define	MODCH_IDT_CLASS		5

#ifdef	PFC_LP64
#define	MODCH_CLASS_NATIVE	ELFCLASS64
#else	/* !PFC_LP64 */
#define	MODCH_CLASS_NATIVE	ELFCLASS32
#endif	/* PFC_LP64 */

/*
 * Identifier[6]: PFC module system version.
 */
#define	MODCH_IDT_SYSVER	6

/*
 * Identifier[7]: Module cache format version.
 */
#define	MODCH_IDT_FMTVER	7

/*
 * Current module cache file format version.
 */
#define	MODCH_FMTVER_CURRENT	0

/*
 * Module section entry.
 */
typedef struct {
	uint32_t	mce_name;	/* string offset of module name */
} modch_ent_t;

typedef const modch_ent_t	modch_cent_t;

/*
 * Dependency section entry.
 */
typedef struct {
	uint16_t	mcd_module;	/* module index in module section */
	uint8_t		mcd_version;	/* required module version */
	uint8_t		mcd_pad;	/* currently not used */
} modch_dep_t;

typedef const modch_dep_t	modch_cdep_t;

/*
 * Helper struct to read module cache data.
 */
typedef struct {
	modch_chead_t	*mx_head;	/* cache header */
	modch_cent_t	*mx_mod_first;	/* first module entry */
	modch_cent_t	*mx_module;	/* current module entry */
	modch_cdep_t	*mx_dep_first;	/* first dependency entry */
	modch_cdep_t	*mx_depend;	/* current dependency entry */
	modch_cdep_t	*mx_rdep_first;	/* first reverse dependency entry */
	modch_cdep_t	*mx_rdepend;	/* current reverse dependency entry */
	uint32_t	mx_nmodules;	/* number of modules */
	uint32_t	mx_ndepends;	/* number of dependency entries */
	uint32_t	mx_nrdepends;	/* number of rev. dependency entries */
} modch_ctx_t;

/*
 * Module index which means terminator of module dependency list.
 */
#define	MODCH_DEP_NONE		PFC_CONST_U(0xffff)

/*
 * Permission of module directory lock file.
 */
#define	MODCH_LOCK_PERM		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Mapping associated with the module cache file.
 */
typedef struct {
	const uint8_t	*mm_addr;	/* base address of the mapping */
	uint32_t	mm_size;	/* size of the mapping */
} modch_map_t;

/*
 * Prototypes.
 */
extern int	pfc_modcache_attach(const char *PFC_RESTRICT dir,
				    modch_map_t *PFC_RESTRICT map);
extern int	pfc_modcache_detach(modch_map_t *map);
extern int	pfc_modcache_exists(const char *dir);

/*
 * static inline void
 * modch_head_init(modch_head_t *headp)
 *	Initialize identifier of module cache file header.
 */
static inline void
modch_head_init(modch_head_t *headp)
{
	headp->mch_ident[MODCH_IDT_MAGIC0] = MODCH_MAGIC0;
	headp->mch_ident[MODCH_IDT_MAGIC1] = MODCH_MAGIC1;
	headp->mch_ident[MODCH_IDT_MAGIC2] = MODCH_MAGIC2;
	headp->mch_ident[MODCH_IDT_MAGIC3] = MODCH_MAGIC3;

	headp->mch_ident[MODCH_IDT_ORDER] = MODCH_ORDER_NATIVE;
	headp->mch_ident[MODCH_IDT_CLASS] = MODCH_CLASS_NATIVE;
	headp->mch_ident[MODCH_IDT_SYSVER] = PFC_MODULE_SYSTEM_VERSION;
	headp->mch_ident[MODCH_IDT_FMTVER] = MODCH_FMTVER_CURRENT;
}

/*
 * static inline pfc_bool_t
 * modch_head_is_valid(modch_chead_t *headp, size_t size)
 *	Return PFC_TRUE if the specified module cache header is valid.
 */
static inline pfc_bool_t
modch_head_is_valid(modch_chead_t *headp, size_t size)
{
	if (headp->mch_ident[MODCH_IDT_MAGIC0] != MODCH_MAGIC0 ||
	    headp->mch_ident[MODCH_IDT_MAGIC1] != MODCH_MAGIC1 ||
	    headp->mch_ident[MODCH_IDT_MAGIC2] != MODCH_MAGIC2 ||
	    headp->mch_ident[MODCH_IDT_MAGIC3] != MODCH_MAGIC3 ||
	    headp->mch_ident[MODCH_IDT_ORDER] != MODCH_ORDER_NATIVE ||
	    headp->mch_ident[MODCH_IDT_CLASS] != MODCH_CLASS_NATIVE ||
	    headp->mch_ident[MODCH_IDT_SYSVER] != PFC_MODULE_SYSTEM_VERSION ||
	    headp->mch_ident[MODCH_IDT_FMTVER] != MODCH_FMTVER_CURRENT) {
		return PFC_FALSE;
	}

	if ((headp->mch_modsize % sizeof(modch_ent_t)) != 0 ||
	    (headp->mch_depsize % sizeof(modch_dep_t)) != 0) {
		return PFC_FALSE;
	}

	if (headp->mch_modoff + headp->mch_modsize > headp->mch_depoff ||
	    headp->mch_depoff + headp->mch_depsize > headp->mch_stroff ||
	    headp->mch_stroff + headp->mch_strsize > size) {
		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static inline modch_cent_t *
 * modch_first_module(modch_chead_t *headp)
 *	Return a pointer to the first modch_ent_t entry.
 *	`headp' must be an address returned by pfc_modcache_attach().
 */
static inline modch_cent_t *
modch_first_module(modch_chead_t *headp)
{
	const uint8_t	*addr = (const uint8_t *)headp;

	return (modch_cent_t *)(addr + headp->mch_modoff);
}

/*
 * static inline modch_cdep_t *
 * modch_first_moddep(modch_chead_t *headp)
 *	Return a pointer to the first modch_dep_t entry in the dependency
 *	section.
 *	`headp' must be an address returned by pfc_modcache_attach().
 */
static inline modch_cdep_t *
modch_first_moddep(modch_chead_t *headp)
{
	const uint8_t	*addr = (const uint8_t *)headp;

	return (modch_cdep_t *)(addr + headp->mch_depoff);
}

/*
 * static inline modch_cdep_t *
 * modch_first_rev_moddep(modch_chead_t *headp)
 *	Return a pointer to the first modch_dep_t entry in the reverse
 *	dependency section.
 *	`headp' must be an address returned by pfc_modcache_attach().
 */
static inline modch_cdep_t *
modch_first_rev_moddep(modch_chead_t *headp)
{
	const uint8_t	*addr = (const uint8_t *)headp;

	return (modch_cdep_t *)(addr + headp->mch_rdepoff);
}

/*
 * static inline const char *
 * modch_string(modch_chead_t *headp, uint32_t index)
 *	Return string associated with the specified index in the string table.
 */
static inline const char *
modch_string(modch_chead_t *headp, uint32_t index)
{
	const uint8_t	*addr = (const uint8_t *)headp;

	return (const char *)(addr + headp->mch_stroff + index);
}

/*
 * static inline void
 * modch_ctx_init(modch_ctx_t *ctx, const uint8_t *addr)
 *	Initialize module cache reader context.
 *	`addr' must be a pointer to module cache contents obtained by the call
 *	of pfc_modcache_attach().
 */
static inline void
modch_ctx_init(modch_ctx_t *ctx, const uint8_t *addr)
{
	modch_chead_t	*headp = (modch_chead_t *)addr;

	ctx->mx_head = headp;
	ctx->mx_mod_first = ctx->mx_module = modch_first_module(headp);
	ctx->mx_dep_first = ctx->mx_depend = modch_first_moddep(headp);
	ctx->mx_rdep_first = ctx->mx_rdepend = modch_first_rev_moddep(headp);
	ctx->mx_nmodules = headp->mch_modsize / sizeof(modch_ent_t);
	ctx->mx_ndepends = headp->mch_depsize / sizeof(modch_dep_t);
	ctx->mx_nrdepends = headp->mch_rdepsize / sizeof(modch_dep_t);
}

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_CMD_MODCACHE_H */
