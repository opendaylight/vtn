/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dump.c - Dump the contents of PFC module cache.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pfc/path.h>
#include <cmdutil.h>
#include "modcache_impl.h"

/*
 * Context to dump module cache.
 */
typedef struct {
	modch_ctx_t	dc_ctx;			/* cache reader context */
	modch_map_t	dc_map;			/* cache file mapping */
} dump_ctx_t;

#define	dc_head		dc_ctx.mx_head
#define	dc_mod_first	dc_ctx.mx_mod_first
#define	dc_module	dc_ctx.mx_module
#define	dc_dep_first	dc_ctx.mx_dep_first
#define	dc_depend	dc_ctx.mx_depend
#define	dc_rdep_first	dc_ctx.mx_rdep_first
#define	dc_rdepend	dc_ctx.mx_rdepend
#define	dc_nmodules	dc_ctx.mx_nmodules
#define	dc_ndepends	dc_ctx.mx_ndepends
#define	dc_nrdepends	dc_ctx.mx_nrdepends

/*
 * Internal prototypes.
 */
static pfc_bool_t	check_cache_dir(const char *path);

static void	cache_module_dump(dump_ctx_t *ctx);
static void	dump_ctx_init(dump_ctx_t *ctx);
static void	dump_ctx_fini(dump_ctx_t *ctx);
static void	no_cache(void);

/*
 * static inline void
 * module_ent_verify(dump_ctx_t *ctx, modch_cent_t *mod)
 *	Ensure that the specified module entry is valid.
 */
static inline void
module_ent_verify(dump_ctx_t *ctx, modch_cent_t *mod)
{
#ifdef	PFC_VERBOSE_DEBUG
	modch_cent_t	*last = ctx->dc_mod_first + ctx->dc_nmodules;

	PFC_ASSERT(mod < last);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * static inline void
 * module_dep_verify(dump_ctx_t *ctx, modch_cdep_t *dep)
 *	Ensure that the specified dependency entry is valid.
 */
static inline void
module_dep_verify(dump_ctx_t *ctx, modch_cdep_t *dep)
{
#ifdef	PFC_VERBOSE_DEBUG
	modch_cdep_t	*last = ctx->dc_dep_first + ctx->dc_ndepends;

	PFC_ASSERT(dep < last);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * static inline void
 * module_rdep_verify(dump_ctx_t *ctx, modch_cdep_t *dep)
 *	Ensure that the specified reverse dependency entry is valid.
 */
static inline void
module_rdep_verify(dump_ctx_t *ctx, modch_cdep_t *dep)
{
#ifdef	PFC_VERBOSE_DEBUG
	modch_cdep_t	*last = ctx->dc_rdep_first + ctx->dc_nrdepends;

	PFC_ASSERT(dep < last);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * static inline modch_cent_t *
 * module_at(dump_ctx_t *ctx, uint16_t index)
 *	Return module entry at the specified module index.
 */
static inline modch_cent_t *
module_at(dump_ctx_t *ctx, uint16_t index)
{
	PFC_ASSERT(index < ctx->dc_nmodules);

	return ctx->dc_mod_first + index;
}

/*
 * void
 * cache_dump(void)
 *	Dump the contents of PFC module cache.
 */
void
cache_dump(void)
{
	pfc_flock_t	lock;
	dump_ctx_t	ctx;
	uint32_t	i;

	if (strcmp(module_dir, cache_dir) != 0) {
		/* Validate the module cache directory. */
		if (!check_cache_dir(cache_dir)) {
			no_cache();

			return;
		}
	}

	/* Acquire module directory lock. */
	lock = module_dir_lock(PFC_FALSE);

	/* Initialize dump context. */
	dump_ctx_init(&ctx);

	if (ctx.dc_nmodules == 0) {
		no_cache();
	}
	else {
		printf("Number of modules: %u\n\n", ctx.dc_nmodules);
		for (i = 0; i < ctx.dc_nmodules; i++) {
			cache_module_dump(&ctx);
		}
	}

	/* Finalize dump context.*/
	dump_ctx_fini(&ctx);

	/* Release module directory lock. */
	module_dir_unlock(lock);
}

/*
 * static pfc_bool_t
 * check_cache_dir(const char *path)
 *	Ensure that the given module cache directory is valid.
 *
 * Calling/Exit State:
 *	PFC_TRUE is return if the given directory path is valid.
 *	PFC_FALSE is returned if the given directory does not exist.
 *
 *	The program exit on error.
 */
static pfc_bool_t
check_cache_dir(const char *path)
{
	struct stat	sbuf;

	if (PFC_EXPECT_FALSE(lstat(path, &sbuf) != 0)) {
		int	err = errno;

		if (err == ENOENT) {
			return PFC_FALSE;
		}

		if (err == EACCES) {
			not_allowed();
		}
		else {
			fatal("Unable to get attributes of the cache "
			      "directory: %s", strerror(err));
		}
		/* NOTREACHED */
	}

	if (PFC_EXPECT_FALSE(!S_ISDIR(sbuf.st_mode))) {
		fatal("%s: Non-directory path is specified as the cache "
		      "directory.", path);
		/* NOTREACHED */
	}

	/* Ensure that the cache directory is safe. */
	path_verify(path);

	return PFC_TRUE;
}

/*
 * static void
 * cache_module_dump(dump_ctx_t *ctx)
 *	Dump the module pointed by the ctx->dc_module.
 */
static void
cache_module_dump(dump_ctx_t *ctx)
{
	modch_chead_t	*head = ctx->dc_head;
	modch_cent_t	*mod = ctx->dc_module;
	modch_cdep_t	*dep = ctx->dc_depend;
	modch_cdep_t	*rdep = ctx->dc_rdepend;
	const char	*name = modch_string(head, mod->mce_name);

	module_ent_verify(ctx, mod);
	printf("- Module name: %s\n", name);
	if (dep->mcd_module != MODCH_DEP_NONE) {
		const char	*separator = "";

		printf("  + depends on: ");
		do {
			uint16_t	midx = dep->mcd_module;
			modch_cent_t	*dmod = module_at(ctx, midx);
			const char	*dname =
				modch_string(head, dmod->mce_name);

			module_dep_verify(ctx, dep);
			printf("%s%s@%u", separator, dname, dep->mcd_version);
			separator = ", ";
			dep++;
		} while (dep->mcd_module != MODCH_DEP_NONE);
		fputc('\n', stdout);
	}
	if (rdep->mcd_module != MODCH_DEP_NONE) {
		const char	*separator = "";

		printf("  + depended by: ");
		do {
			uint16_t	midx = rdep->mcd_module;
			modch_cent_t	*dmod = module_at(ctx, midx);
			const char	*dname =
				modch_string(head, dmod->mce_name);

			module_rdep_verify(ctx, rdep);
			printf("%s%s", separator, dname);
			separator = ", ";
			rdep++;
		} while (rdep->mcd_module != MODCH_DEP_NONE);
		fputc('\n', stdout);
	}

	ctx->dc_depend = dep + 1;
	ctx->dc_rdepend = rdep + 1;
	ctx->dc_module = mod + 1;
}

/*
 * static void
 * dump_ctx_init(dump_ctx_t *ctx)
 *	Initialize context to dump module cache.
 */
static void
dump_ctx_init(dump_ctx_t *ctx)
{
	modch_chead_t	*headp;
	modch_map_t	*map = &ctx->dc_map;
	int		err;

	/* Attach the module cache file. */
	err = pfc_modcache_attach(cache_dir, map);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != ENOENT) {
			fatal("Failed to attach the module cache file: %s",
			      strerror(err));
			/* NOTREACHED */
		}

		map->mm_addr = NULL;
		map->mm_size = 0;
		memset(&ctx->dc_ctx, 0, sizeof(ctx->dc_ctx));

		return;
	}

	debug_printf(2, "cache size: %u", map->mm_size);

	modch_ctx_init(&ctx->dc_ctx, map->mm_addr);

	headp = ctx->dc_head;
	debug_printf(3, "module section          : off=0x%x, size=0x%x",
		     headp->mch_modoff, headp->mch_modsize);
	debug_printf(3, "dependency section      : off=0x%x, size=0x%x",
		     headp->mch_depoff, headp->mch_depsize);
	debug_printf(3, "rev. dependency section : off=0x%x, size=0x%x",
		     headp->mch_rdepoff, headp->mch_rdepsize);
	debug_printf(3, "string table            : off=0x%x, size=0x%x",
		     headp->mch_stroff, headp->mch_strsize);
}

/*
 * static void
 * dump_ctx_fini(dump_ctx_t *ctx)
 *	Finalize the dump context.
 */
static void
dump_ctx_fini(dump_ctx_t *ctx)
{
	modch_map_t	*map = &ctx->dc_map;

	if (map->mm_addr != NULL) {
		PFC_ASSERT_INT(pfc_modcache_detach(map), 0);
	}
}

/*
 * static void
 * no_cache(void)
 *	Print message to inform that no module is cached.
 */
static void
no_cache(void)
{
	puts("No module is cached.");
}
