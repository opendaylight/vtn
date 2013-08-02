/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * update.c - Update PFC module cache.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pfc/hash.h>
#include <pfc/list.h>
#include <pfc/listmodel.h>
#include <pfc/path.h>
#include <pfc/ctype.h>
#include "modcache_impl.h"

#ifndef	PFC_HAVE_ATFILE_SYSCALL
#error	Current implementation requires ATFILE system calls.
#endif	/* !PFC_HAVE_ATFILE_SYSCALL */

/*
 * Number of module hash buckets.
 */
#define	MODULE_HASH_NBUCKETS	31U

/*
 * Base name of temporary file.
 * Process ID of pfc_modcache will be appended to this.
 */
#define	MODULE_CACHE_TMP	".modcache.%d"

/*
 * File permission of module cache file.
 */
#define	MODULE_CACHE_PERM	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Permission of module cache directory.
 */
#define	MODULE_CACHE_DIR_PERM						\
	(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)	/* 0755 */

/*
 * open(2) flags to create a temporary file under module cache directory.
 */
#define	MODULE_CACHE_OFLAGS	(O_WRONLY | O_CREAT | O_TRUNC | O_EXCL)

struct mch_module;
typedef struct mch_module	mch_module_t;

/*
 * Module dependency data.
 */
typedef struct {
	pfc_refptr_t	*md_refptr;	/* refptr for this instance */
	mch_module_t	*md_module;	/* module data  */
	uint8_t		md_version;	/* required module version */
} mch_moddep_t;

#define	REF_MODDEP(ref)		PFC_REFPTR_VALUE((ref), mch_moddep_t *)

/*
 * Module data to be cached.
 */
struct mch_module {
	pfc_refptr_t	*mc_refptr;	/* refptr for this instance */
	pfc_refptr_t	*mc_name;	/* module name */
	pfc_listm_t	mc_moddeps;	/* mch_moddep_t list */
	pfc_listm_t	mc_depended;	/* modules which depend on this */
	pfc_list_t	mc_deplist;	/* link for deplist */
	uint32_t	mc_depth;	/* dependency graph depth */
	uint32_t	mc_score;	/* load order score */
	uint16_t	mc_index;	/* module index in module section */
};

#define	REF_MODULE(ref)		PFC_REFPTR_VALUE((ref), mch_module_t *)

#define	MCH_MODULE_NAME(mod)	pfc_refptr_string_value((mod)->mc_name)

#define	LIST_TO_MODULE(elem)					\
	PFC_CAST_CONTAINER(elem, mch_module_t, mc_list)

#define	DEPLIST_TO_MODULE(elem)					\
	PFC_CAST_CONTAINER(elem, mch_module_t, mc_deplist)

#define	MCH_MODULE_SCORE_UNDEF		((uint32_t)-1)

/*
 * ELF header for native system.
 */
#ifdef	PFC_LP64
#define	ElfXX_Ehdr	Elf64_Ehdr
#else	/* !PFC_LP64 */
#define	ElfXX_Ehdr	Elf32_Ehdr
#endif	/* PFC_LP64 */

/*
 * ELF data encoding for native system.
 */
#ifdef	PFC_LITTLE_ENDIAN
#define	ELFDATA_NATIVE		ELFDATA2LSB
#else	/* !PFC_LITTLE_ENDIAN */
#define	ELFDATA_NATIVE		ELFDATA2MSB
#endif	/* PFC_LITTLE_ENDIAN */

/*
 * Calculate load order score, which determines the order of module loading.
 * A module which has larger score is loaded earlier than lower.
 *
 * - Modules depended by many modules are preceded.
 * - Modules which has no dependency are preceded.
 */
static inline uint32_t
module_score(mch_module_t *mod)
{
	uint32_t	score = mod->mc_score;

	if (score == MCH_MODULE_SCORE_UNDEF) {
		size_t	ndeps = pfc_listm_get_size(mod->mc_moddeps);

		PFC_ASSERT(ndeps < PFC_MODULE_MAX);
		score = (mod->mc_depth << 16) - ndeps;

		debug_printf(3, "%s: score = %u (depth = %u, ndeps = %u)",
			     MCH_MODULE_NAME(mod), score, mod->mc_depth,
			     (uint32_t)ndeps);
		mod->mc_score = score;
	}

	return score;
}

/*
 * Context for module cache update.
 */
typedef struct {
	pfc_hash_t	uc_modules;	/* all module data */
	pfc_list_t	uc_deplist;	/* modules which have module.dep */

	/* Temporary file name under module cache directory. */
	char		uc_tmpfile[PFC_MODULE_FNAME_MAX];

	uint32_t	uc_ndeps;	/* number of dependency data */
	int		uc_moddirfd;	/* FD associated with moduel_dir */
	int		uc_chdirfd;	/* FD associated with cache_dir */
} update_ctx_t;

static update_ctx_t	update_context;

/*
 * Context for module.dep parser.
 */
typedef struct {
	FILE		*mdc_file;	/* FILE pointer */
	uint32_t	mdc_line;	/* current line number */
	uint8_t		mdc_version;	/* parsed required version */

	/* Name of module.dep file. */
	char		mdc_fname[PFC_MODULE_FNAME_MAX];
} moddep_ctx_t;

/*
 * Internal prototypes.
 */
static void		collect_modules(update_ctx_t *ctx);
static void		load_dependencies(update_ctx_t *PFC_RESTRICT ctx,
					  mch_module_t *PFC_RESTRICT mod);
static void		check_loop(mch_module_t *parent, mch_module_t *mod,
				   const char *PFC_RESTRICT depfile);
static void		update_graph_depth(mch_module_t *mod, uint32_t depth);
static int		module_sort_comp(const void *o1, const void *o2);
static void		cache_opendir(update_ctx_t *ctx);
static void		cache_write(update_ctx_t *PFC_RESTRICT ctx,
				    mch_module_t **PFC_RESTRICT modules,
				    size_t nmodules);
static uint32_t		cache_write_modules(int fd, uint32_t off,
					    strtable_t *PFC_RESTRICT stable,
					    mch_module_t **PFC_RESTRICT modules,
					    size_t nmodules);
static uint32_t		cache_write_depends(update_ctx_t *PFC_RESTRICT ctx,
					    int fd, uint32_t off,
					    mch_module_t **PFC_RESTRICT modules,
					    size_t nmodules);
static uint32_t		cache_write_rev_depends(update_ctx_t *PFC_RESTRICT ctx,
						int fd, uint32_t off,
						mch_module_t **PFC_RESTRICT
						modules, size_t nmodules);
static void		read_data(int fd, uint8_t *buf, size_t size);
static void		write_data(int fd, const uint8_t *buf, size_t size);
static void		update_cleanup(void);

static pfc_bool_t	modfile_validate(int dfd, const char *name,
					 struct stat *sbuf);
static pfc_bool_t	module_validate(int dfd, const char *name);
static pfc_bool_t	moddep_exists(int dfd, const char *name);

static mch_module_t	*module_create(const char *name);
static void		module_dtor(pfc_ptr_t object);
static int		module_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static pfc_bool_t	module_equals(pfc_cptr_t o1, pfc_cptr_t o2);
static uint32_t		module_hashfunc(pfc_cptr_t object);

static mch_moddep_t	*moddep_create(update_ctx_t *PFC_RESTRICT ctx,
				       const char *PFC_RESTRICT depfile,
				       pfc_refptr_t *PFC_RESTRICT name,
				       uint8_t version);
static void		moddep_dtor(pfc_ptr_t object);
static int		moddep_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static pfc_bool_t	moddep_equals(pfc_cptr_t o1, pfc_cptr_t o2);
static uint32_t		moddep_hashfunc(pfc_cptr_t object);

static void		moddep_parser_init(moddep_ctx_t *PFC_RESTRICT mdctx,
					   int dfd,
					   const char *PFC_RESTRICT name);
static void		moddep_parser_destroy(moddep_ctx_t *mdctx);
static pfc_refptr_t	*moddep_parser_next(moddep_ctx_t *mdctx);
static int		moddep_parser_getchar(moddep_ctx_t *mdctx);

#ifdef	PFC_VERBOSE_DEBUG
static void	module_sort_verify(mch_module_t **modules, size_t nmodules);
#else	/* !PFC_VERBOSE_DEBUG */
#define	module_sort_verify(modules, nmodules)
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * Reference pointer operations for module data.
 */
static const pfc_refptr_ops_t	module_refops = {
	.dtor		= module_dtor,
	.compare	= module_compare,
	.equals		= module_equals,
	.hashfunc	= module_hashfunc,
};

/*
 * Reference pointer operations for module dependency.
 */
static const pfc_refptr_ops_t	moddep_refops = {
	.dtor		= moddep_dtor,
	.compare	= moddep_compare,
	.equals		= moddep_equals,
	.hashfunc	= moddep_hashfunc,
};

/*
 * void
 * cache_update(void)
 *	Update module cache.
 */
void
cache_update(void)
{
	struct rlimit	rlim;
	pfc_flock_t	lock;
	update_ctx_t	*ctx = &update_context;
	pfc_list_t	*elem;
	pfc_hashiter_t	it;
	pfc_refptr_t	*rmod;
	uint16_t	index;
	mch_module_t	**table, **modpp;
	size_t		nmodules;
	int		err;

	/*
	 * Initialize cache update context.
	 */
	err = pfc_strhash_create(&ctx->uc_modules, NULL,
				 MODULE_HASH_NBUCKETS, PFC_HASH_NOLOCK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to create module hash table: %s", strerror(err));
		/* NOTREACHED */
	}

	pfc_list_init(&ctx->uc_deplist);

	/* Open module directory. */
	ctx->uc_moddirfd = open(module_dir, O_RDONLY);
	if (PFC_EXPECT_FALSE(ctx->uc_moddirfd == -1)) {
		fatal("Failed to open module directory: %s: %s", module_dir,
		      strerror(errno));
		/* NOTREACHED */
	}

	ctx->uc_ndeps = 0;
	cache_opendir(ctx);

	/* Ensure that the module cache directory is safe. */
	path_verify(cache_dir);

	/* Set stack limit to the hard limit because we use recursive call. */
	if (PFC_EXPECT_FALSE(getrlimit(RLIMIT_STACK, &rlim) != 0)) {
		warning("Failed to get stack limit: %s", strerror(errno));
	}
	else if (rlim.rlim_cur != rlim.rlim_max) {
		rlim.rlim_cur = rlim.rlim_max;
		if (PFC_EXPECT_FALSE(setrlimit(RLIMIT_STACK, &rlim) != 0)) {
			warning("Failed to set stack limit: %s",
				strerror(errno));
		}
	}

	/* Acquire module directory lock. */
	lock = module_dir_lock(PFC_TRUE);

	/* Collect module information. */
	collect_modules(ctx);

	/* Load dependencies. */
	PFC_LIST_FOREACH(&ctx->uc_deplist, elem) {
		mch_module_t	*mod = DEPLIST_TO_MODULE(elem);

		load_dependencies(ctx, mod);
	}

	/* Create module table to sort modules. */
	nmodules = pfc_hash_get_size(ctx->uc_modules);
	table = (mch_module_t **)malloc(sizeof(mch_module_t *) * nmodules);
	if (PFC_EXPECT_FALSE(table == NULL)) {
		fatal("Failed to allocate sort buffer.");
		/* NOTREACHED */
	}

	/* Update dependency graph depth. */
	it = pfc_hashiter_get(ctx->uc_modules);
	if (PFC_EXPECT_FALSE(it == NULL)) {
		fatal("Failed to create module hash table iterator.");
		/* NOTREACHED */
	}

	modpp = table;
	while ((err = pfc_hashiter_next(it, NULL, (pfc_cptr_t *)&rmod)) == 0) {
		mch_module_t	*mod = REF_MODULE(rmod);

		if (pfc_listm_get_size(mod->mc_depended) == 0) {
			/*
			 * No module depends on this module.
			 * Update module dependency depth from this module.
			 */
			debug_printf(3, "Graph update: %s",
				     MCH_MODULE_NAME(mod));
			update_graph_depth(mod, 1);
		}
		*modpp = mod;
		modpp++;
	}
	PFC_ASSERT(err == ENOENT);
	PFC_ASSERT(modpp == table + nmodules);

	/* Sort modules in descending order of module load score. */
	qsort(table, nmodules, sizeof(mch_module_t *), module_sort_comp);
	module_sort_verify(table, nmodules);

	/* Reorder module indices. */
	for (index = 0, modpp = table; modpp < table + nmodules;
	     index++, modpp++) {
		(*modpp)->mc_index = index;
	}

	/* Write module data into the cache file. */
	cache_write(ctx, table, nmodules);

	free(table);

	/* Release module directory lock. */
	module_dir_unlock(lock);

	/* Destroy update context. */
	pfc_hash_destroy(ctx->uc_modules);
	PFC_ASSERT_INT(close(ctx->uc_moddirfd), 0);
	if (ctx->uc_moddirfd != ctx->uc_chdirfd) {
		PFC_ASSERT_INT(close(ctx->uc_chdirfd), 0);
	}
}

/*
 * static void
 * collect_modules(update_ctx_t *ctx)
 *	Collect module information.
 *	The caller must change working directory to the module directory
 *	before the call of this function.
 */
static void
collect_modules(update_ctx_t *ctx)
{
	DIR		*dirp;
	struct dirent	*dp;
	pfc_hash_t	hash = ctx->uc_modules;
	int		dfd = ctx->uc_moddirfd;

	dirp = opendir(module_dir);
	if (PFC_EXPECT_FALSE(dirp == NULL)) {
		fatal("Failed to open module directory: %s: %s",
		      module_dir, strerror(errno));
		/* NOTREACHED */
	}

	/* We can use readdir() because pfc_modcache is single-threaded. */
	while ((dp = readdir(dirp)) != NULL) {
		const char	*name = dp->d_name, *modname;
		mch_module_t	*mod;
		int		err;

		if (*name == '.') {
			continue;
		}

		debug_printf(2, "filename: %s", name);
		mod = module_create(name);
		if (mod == NULL) {
			continue;
		}
		modname = MCH_MODULE_NAME(mod);

		/* Ensure this module file is valid or not. */
		if (PFC_EXPECT_FALSE(!module_validate(dfd, name))) {
			/* This file is not module object. */
			pfc_refptr_put(mod->mc_refptr);
			continue;
		}

		debug_printf(1, "module found: %s (%s/%s)", modname,
			     module_dir, name);

		/* Check to see whether module dependency file exists. */
		if (moddep_exists(dfd, modname)) {
			pfc_list_push_tail(&ctx->uc_deplist, &mod->mc_deplist);
		}

		if (pfc_hash_get_size(hash) >= PFC_MODULE_MAX) {
			fatal("Too many modules.");
			/* NOTREACHED */
		}

		err = pfc_hash_put_kvref(hash, mod->mc_name, mod->mc_refptr);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fatal("Failed to register module data for %s: %s",
			      modname, strerror(err));
			/* NOTREACHED */
		}
		pfc_refptr_put(mod->mc_refptr);
	}

	closedir(dirp);
}

/*
 * static void
 * load_dependencies(update_ctx_t *PFC_RESTRICT ctx,
 *		     mch_module_t *PFC_RESTRICT mod)
 *	Load module dependencies for the specified module.
 */
static void
load_dependencies(update_ctx_t *PFC_RESTRICT ctx,
		  mch_module_t *PFC_RESTRICT mod)
{
	const char	*modname = MCH_MODULE_NAME(mod);
	moddep_ctx_t	mdctx;
	pfc_refptr_t	*dname;

	/* Initialize module.dep file parser. */
	moddep_parser_init(&mdctx, ctx->uc_moddirfd, modname);
	while ((dname = moddep_parser_next(&mdctx)) != NULL) {
		mch_moddep_t	*dep;
		mch_module_t	*dmod;
		pfc_refptr_t	*rdep;
		int	err;

		/* Reject self reference. */
		if (PFC_EXPECT_FALSE(pfc_refptr_equals(mod->mc_name, dname))) {
			fatal("%s: Circular dependency: %s", mdctx.mdc_fname,
			      pfc_refptr_string_value(dname));
			/* NOTREACHED */
		}

		/* Create a module dependency data. */
		dep = moddep_create(ctx, mdctx.mdc_fname, dname,
				    mdctx.mdc_version);
		dmod = dep->md_module;
		rdep = dep->md_refptr;

		/* Detect dependency loop. */
		check_loop(mod, dmod, mdctx.mdc_fname);

		/* Append this module to moddep list. */
		err = pfc_listm_push_tail(mod->mc_moddeps, (pfc_cptr_t)rdep);
		if (PFC_EXPECT_FALSE(err == EEXIST)) {
			fatal("%s: Duplicated module dependency: %s",
			      mdctx.mdc_fname, pfc_refptr_string_value(dname));
			/* NOTREACHED */
		}
		else if (PFC_EXPECT_FALSE(err != 0)) {
			fatal("%s: Failed to append dependency: %s: %s",
			      mdctx.mdc_fname, pfc_refptr_string_value(dname),
			      strerror(err));
			/* NOTREACHED */
		}

		/* Update list of modules which depends on this module. */
		err = pfc_listm_push_tail(dmod->mc_depended, (pfc_cptr_t)mod);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fatal("%s: Failed to append depended list: %s: %s",
			      mdctx.mdc_fname, pfc_refptr_string_value(dname),
			      strerror(err));
			/* NOTREACHED */
		}

		debug_printf(1, "%s: depends on %s@%u", mdctx.mdc_fname,
			     pfc_refptr_string_value(dname),
			     dep->md_version);
		ctx->uc_ndeps++;
		pfc_refptr_put(dname);
	}

	moddep_parser_destroy(&mdctx);
}

/*
 * static void
 * check_loop(mch_module_t *parent, mch_module_t *mod,
 *	      const char *PFC_RESTRICT depfile)
 *	Detect dependency loop.
 *
 *	This function ensures that the module specified by `mod' never appears
 *	in `parent' modules.
 */
static void
check_loop(mch_module_t *parent, mch_module_t *mod,
	   const char *PFC_RESTRICT depfile)
{
	pfc_listiter_t	it;
	mch_module_t	*m;
	int	err;

	if (mod == parent) {
		fatal("%s: Circular dependency: %s", depfile,
		      MCH_MODULE_NAME(mod));
		/* NOTREACHED */
	}

	it = pfc_listiter_create(parent->mc_depended);
	if (PFC_EXPECT_FALSE(it == NULL)) {
		fatal(" Failed to create list iterator.");
		/* NOTREACHED */
	}

	while ((err = pfc_listiter_next(it, (pfc_cptr_t *)&m)) == 0) {
		check_loop(m, mod, depfile);
	}
	PFC_ASSERT(err == ENOENT);
	pfc_listiter_destroy(it);
}

/*
 * static void
 * update_graph_depth(mch_module_t *mod, uint32_t depth)
 *	Update depth of module dependency from modules on which no module
 *	depends.
 */
static void
update_graph_depth(mch_module_t *mod, uint32_t depth)
{
	pfc_listiter_t	it;
	pfc_refptr_t	*rdep;
	uint32_t	curdepth = mod->mc_depth;
	int	err;

	if (depth > curdepth) {
		mod->mc_depth = depth;
		debug_printf(2, "%s: depth = %u",
			     MCH_MODULE_NAME(mod), mod->mc_depth);
		curdepth = depth;
	}

	it = pfc_listiter_create(mod->mc_moddeps);
	if (PFC_EXPECT_FALSE(it == NULL)) {
		fatal("Failed to create dependency list iterator.");
		/* NOTREACHED */
	}

	depth = curdepth + 1;
	while ((err = pfc_listiter_next(it, (pfc_cptr_t *)&rdep)) == 0) {
		mch_moddep_t	*dep = REF_MODDEP(rdep);
		mch_module_t	*dmod = dep->md_module;

		update_graph_depth(dmod, depth);
	}
	PFC_ASSERT(err == ENOENT);
	pfc_listiter_destroy(it);
}

/*
 * static int
 * module_sort_comp(const void *o1, const void *o2)
 *	Compare modules using module load score.
 *	This function is used as comparator for qsort(3).
 */
static int
module_sort_comp(const void *o1, const void *o2)
{
	mch_module_t	*m1 = *((mch_module_t **)o1);
	mch_module_t	*m2 = *((mch_module_t **)o2);
	uint32_t	s1 = module_score(m1);
	uint32_t	s2 = module_score(m2);

	if (s1 == s2) {
		return 0;
	}

	return (s1 < s2) ? 1 : -1;
}

/*
 * static void
 * cache_opendir(update_ctx_t *ctx)
 *	Open the module cache directory.
 *	The module cache directory is created if not exist.
 *
 * Calling/Exit State:
 *	Program exits on error.
 */
static void
cache_opendir(update_ctx_t *ctx)
{
	int	err;

	if (strcmp(module_dir, cache_dir) == 0) {
		ctx->uc_chdirfd = ctx->uc_moddirfd;

		return;
	}

	/* Open module cache directory. */
	ctx->uc_chdirfd = open(cache_dir, O_RDONLY);
	if (PFC_EXPECT_TRUE(ctx->uc_chdirfd != -1)) {
		return;
	}

	err = errno;
	if (PFC_EXPECT_TRUE(err == ENOENT)) {
		/* Create module cache directory. */
		err = pfc_mkdir(cache_dir, MODULE_CACHE_DIR_PERM);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fatal("Failed to create cache directory: %s: %s",
			      cache_dir, strerror(err));
			/* NOTREACHED */
		}

		/* Open the cache directory again. */
		ctx->uc_chdirfd = open(cache_dir, O_RDONLY);
		if (PFC_EXPECT_TRUE(ctx->uc_chdirfd != -1)) {
			return;
		}

		err = errno;
	}

	fatal("Failed to open cache directory: %s: %s", cache_dir,
	      strerror(err));
	/* NOTREACHED */
}

/*
 * static void
 * cache_write(update_ctx_t *PFC_RESTRICT ctx,
 *	       mch_module_t **PFC_RESTRICT modules, size_t nmodules)
 *	Write module information to the cache file.
 */
static void
cache_write(update_ctx_t *PFC_RESTRICT ctx,
	    mch_module_t **PFC_RESTRICT modules, size_t nmodules)
{
	int		fd, chdfd = ctx->uc_chdirfd;
	modch_head_t	head;
	uint32_t	modoff, modend, modsize, depoff, depend, depsize;
	uint32_t	rdepoff, rdepend, rdepsize, stroff, strsize;
	const char	*string;
	strtable_t	*stable;

	debug_printf(1, "Writing module cache.");

	/* Create cache file header. */
	modch_head_init(&head);

	/* Create string table instance. */
	strtable_create(&stable);

	/* Create a temporary file. */
	snprintf(ctx->uc_tmpfile, sizeof(ctx->uc_tmpfile), MODULE_CACHE_TMP,
		 getpid());
	debug_printf(2, "tmpfile = %s/%s", cache_dir, ctx->uc_tmpfile);
	fd = openat(chdfd, ctx->uc_tmpfile, MODULE_CACHE_OFLAGS,
		    MODULE_CACHE_PERM);
	if (PFC_EXPECT_FALSE(fd == -1 && errno == EEXIST)) {
		/* Remove existing temporary file, and try again. */
		unlinkat(chdfd, ctx->uc_tmpfile, 0);
		fd = openat(chdfd, ctx->uc_tmpfile,
			    MODULE_CACHE_OFLAGS, MODULE_CACHE_PERM);
	}
	if (PFC_EXPECT_FALSE(fd == -1)) {
		fatal("open(%s/%s) failed: %s", cache_dir, ctx->uc_tmpfile,
		      strerror(errno));
		/* NOTREACHED */
	}

	if (atexit(update_cleanup) != 0) {
		unlinkat(chdfd, ctx->uc_tmpfile, 0);
		fatal("Failed to register exit handler.");
		/* NOTREACHED */
	}

	/* Write module section. */
	modoff = PFC_POW2_ROUNDUP(sizeof(head), MODCH_SECT_ALIGN);
	modend = cache_write_modules(fd, modoff, stable, modules, nmodules);
	modsize = modend - modoff;
	head.mch_modoff = modoff;
	head.mch_modsize = modsize;
	debug_printf(3, "module section          : off=0x%x, size=0x%x",
		     modoff, modsize);

	/* Write dependency section. */
	depoff = PFC_POW2_ROUNDUP(modend, MODCH_SECT_ALIGN);
	depend = cache_write_depends(ctx, fd, depoff, modules, nmodules);
	depsize = depend - depoff;
	head.mch_depoff = depoff;
	head.mch_depsize = depsize;
	debug_printf(3, "dependency section      : off=0x%x, size=0x%x",
		     depoff, depsize);

	/* Write reverse dependency section. */
	rdepoff = PFC_POW2_ROUNDUP(depend, MODCH_SECT_ALIGN);
	rdepend = cache_write_rev_depends(ctx, fd, rdepoff, modules, nmodules);
	rdepsize = rdepend - rdepoff;
	head.mch_rdepoff = rdepoff;
	head.mch_rdepsize = rdepsize;
	debug_printf(3, "rev. dependency section : off=0x%x, size=0x%x",
		     rdepoff, rdepsize);

	/* Write string table. */
	stroff = PFC_POW2_ROUNDUP(rdepend, MODCH_SECT_ALIGN);
	string = strtable_finalize(stable, &strsize);
	if (lseek(fd, stroff, SEEK_SET) == -1) {
		fatal("lseek(0x%x) failed: %s", stroff, strerror(errno));
		/* NOTREACHED */
	}
	write_data(fd, (const uint8_t *)string, strsize);
	free((void *)string);

	head.mch_stroff = stroff;
	head.mch_strsize = strsize;
	debug_printf(3, "string table            : off=0x%x, size=0x%x",
		     stroff, strsize);

	/* Write cache header. */
	if (lseek(fd, 0, SEEK_SET) == -1) {
		fatal("lseek(0x%x) failed: %s", 0, strerror(errno));
		/* NOTREACHED */
	}
	write_data(fd, (const uint8_t *)&head, sizeof(head));
	close(fd);

	/* Rename temporary file to the module cache file. */
	debug_printf(1, "Install the module cache file: %s: %s -> %s",
		     cache_dir, ctx->uc_tmpfile, PFC_MODCACHE_NAME);
	if (PFC_EXPECT_FALSE(renameat(chdfd, ctx->uc_tmpfile, chdfd,
				      PFC_MODCACHE_NAME) == -1)) {
		fatal("Failed to install the module cache file: %s",
		      strerror(errno));
		/* NOTREACHED */
	}

	ctx->uc_tmpfile[0] = '\0';
}

/*
 * static uint32_t
 * cache_write_modules(int fd, uint32_t off, strtable_t *PFC_RESTRICT stable,
 *		       mch_module_t **PFC_RESTRICT modules, size_t nmodules)
 *	Write module section to the cache file.
 *
 * Calling/Exit State:
 *	End offset of module section is returned.
 */
static uint32_t
cache_write_modules(int fd, uint32_t off, strtable_t *PFC_RESTRICT stable,
		    mch_module_t **PFC_RESTRICT modules, size_t nmodules)
{
	mch_module_t	**modpp;

	if (lseek(fd, off, SEEK_SET) == -1) {
		fatal("lseek(0x%x) failed: %s", off, strerror(errno));
		/* NOTREACHED */
	}

	for (modpp = modules; modpp < modules + nmodules; modpp++) {
		modch_ent_t	modent;
		mch_module_t	*mod = *modpp;

		modent.mce_name = strtable_add(stable, mod->mc_name);
		write_data(fd, (const uint8_t *)&modent, sizeof(modent));
	}

	off += sizeof(modch_ent_t) * nmodules;
	PFC_ASSERT(lseek(fd, 0, SEEK_CUR) == (off_t)off);

	return off;
}

/*
 * static uint32_t
 * cache_write_depends(update_ctx_t *PFC_RESTRICT ctx, int fd, uint32_t off,
 *		       mch_module_t **PFC_RESTRICT modules, size_t nmodules)
 *	Write dependency section to the cache file.
 *
 * Calling/Exit State:
 *	End offset of dependency section is returned.
 */
static uint32_t
cache_write_depends(update_ctx_t *PFC_RESTRICT ctx, int fd, uint32_t off,
		    mch_module_t **PFC_RESTRICT modules, size_t nmodules)
{
	mch_module_t	**modpp;
	modch_dep_t	depend;

	if (lseek(fd, off, SEEK_SET) == -1) {
		fatal("lseek(0x%x) failed: %s", off, strerror(errno));
		/* NOTREACHED */
	}

	depend.mcd_pad = 0;

	for (modpp = modules; modpp < modules + nmodules; modpp++) {
		mch_module_t	*mod = *modpp;
		pfc_listiter_t	it;
		pfc_cptr_t	value;
		int	err;

		it = pfc_listiter_create(mod->mc_moddeps);
		if (PFC_EXPECT_FALSE(it == NULL)) {
			fatal("Failed to create dependency list iterator.");
			/* NOTREACHED */
		}

		while ((err = pfc_listiter_next(it, &value)) == 0) {
			pfc_refptr_t	*rdep = (pfc_refptr_t *)value;
			mch_moddep_t	*dep = REF_MODDEP(rdep);
			mch_module_t	*dmod = dep->md_module;

			depend.mcd_module = dmod->mc_index;
			depend.mcd_version = dep->md_version;
			write_data(fd, (const uint8_t *)&depend,
				   sizeof(depend));
		}
		PFC_ASSERT(err == ENOENT);
		pfc_listiter_destroy(it);

		/* Write terminator. */
		depend.mcd_module = MODCH_DEP_NONE;
		depend.mcd_version = 0;
		write_data(fd, (const uint8_t *)&depend, sizeof(depend));
	}
	off += sizeof(modch_dep_t) * (nmodules + ctx->uc_ndeps);
	PFC_ASSERT(lseek(fd, 0, SEEK_CUR) == (off_t)off);

	return off;
}

/*
 * static uint32_t
 * cache_write_rev_depends(update_ctx_t *PFC_RESTRICT ctx, int fd, uint32_t off,
 *			   mch_module_t **PFC_RESTRICT modules, size_t nmodules)
 *	Write reverse dependency section to the cache file.
 *
 * Calling/Exit State:
 *	End offset of reverse dependency section is returned.
 */
static uint32_t
cache_write_rev_depends(update_ctx_t *PFC_RESTRICT ctx, int fd, uint32_t off,
			mch_module_t **PFC_RESTRICT modules, size_t nmodules)
{
	mch_module_t	**modpp;
	modch_dep_t	depend;

	if (lseek(fd, off, SEEK_SET) == -1) {
		fatal("lseek(0x%x) failed: %s", off, strerror(errno));
		/* NOTREACHED */
	}

	depend.mcd_version = 0;
	depend.mcd_pad = 0;

	for (modpp = modules; modpp < modules + nmodules; modpp++) {
		mch_module_t	*mod = *modpp;
		pfc_listiter_t	it;
		pfc_cptr_t	value;
		int	err, size;

		size = pfc_listm_get_size(mod->mc_depended);
		PFC_ASSERT(size >= 0);

		it = pfc_listiter_create(mod->mc_depended);
		if (PFC_EXPECT_FALSE(it == NULL)) {
			fatal("Failed to create reverse dependency list "
			      "iterator.");
			/* NOTREACHED */
		}

		while ((err = pfc_listiter_next(it, &value)) == 0) {
			mch_module_t	*dmod = (mch_module_t *)value;

			depend.mcd_module = dmod->mc_index;
			write_data(fd, (const uint8_t *)&depend,
				   sizeof(depend));
		}
		PFC_ASSERT(err == ENOENT);
		pfc_listiter_destroy(it);

		/* Write terminator. */
		depend.mcd_module = MODCH_DEP_NONE;
		depend.mcd_version = 0;
		write_data(fd, (const uint8_t *)&depend, sizeof(depend));

		off += sizeof(modch_dep_t) * (size + 1);
	}
	PFC_ASSERT(lseek(fd, 0, SEEK_CUR) == (off_t)off);

	return off;
}

/*
 * static void
 * read_data(int fd, uint8_t *buf, size_t size)
 *	Read `size' bytes from the file specified by the file descriptor `fd'
 *	into the buffer pointed by `buf'.
 */
static void
read_data(int fd, uint8_t *buf, size_t size)
{
	uint8_t	*ptr = buf;

	while (size > 0) {
		ssize_t	nbytes = read(fd, ptr, size);

		if (PFC_EXPECT_FALSE(nbytes == -1)) {
			fatal("Read I/O error: %s", strerror(errno));
			/* NOTREACHED */
		}
		PFC_ASSERT(size >= (size_t)nbytes);
		ptr += nbytes;
		size -= nbytes;
	}
}

/*
 * static void
 * write_data(int fd, const uint8_t *buf, size_t size)
 *	Write `size' bytes from the buffer pointed by `buf' to the file
 *	specified by the file descriptor `fd'.
 */
static void
write_data(int fd, const uint8_t *buf, size_t size)
{
	const uint8_t	*ptr = buf;

	while (size > 0) {
		ssize_t	nbytes = write(fd, ptr, size);

		if (PFC_EXPECT_FALSE(nbytes == -1)) {
			fatal("Write I/O error: %s", strerror(errno));
			/* NOTREACHED */
		}
		PFC_ASSERT(size >= (size_t)nbytes);
		ptr += nbytes;
		size -= nbytes;
	}
}

/*
 * static void
 * update_cleanup(void)
 *	Clean up handler for module cache update.
 */
static void
update_cleanup(void)
{
	update_ctx_t	*ctx = &update_context;

	if (ctx->uc_tmpfile[0] != '\0') {
		/* Remove temporary file. */
		PFC_ASSERT(ctx->uc_chdirfd != -1);
		unlinkat(ctx->uc_chdirfd, ctx->uc_tmpfile, 0);
	}
}

/*
 * static pfc_bool_t
 * modfile_validate(int dfd, const char *name, struct stat *sbuf)
 *	Ensure that the specified module file is valid.
 *	PFC_FALSE is returned if not.
 *
 * Remarks:
 *	This function doesn't ensure the parent directory is safe.
 *	The caller must do in advance.
 */
static pfc_bool_t
modfile_validate(int dfd, const char *name, struct stat *sbuf)
{
	mode_t	mode;

	mode = sbuf->st_mode;
	if (PFC_EXPECT_FALSE(!S_ISREG(mode))) {
		warning("\"%s\" is unsafe: non-regular file", name);

		return PFC_FALSE;
	}

	if (PFC_EXPECT_FALSE(!(mode & S_IRUSR))) {
		warning("\"%s\" is not readable.", name);

		return PFC_FALSE;
	}

	if (PFC_EXPECT_FALSE(mode & S_IWOTH)) {
		warning("\"%s\" is unsafe: world writable", name);

		return PFC_FALSE;
	}

	if (PFC_EXPECT_FALSE(mode & S_IWGRP)) {
		warning("\"%s\" is unsafe: group writable", name);

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static pfc_bool_t
 * module_validate(int dfd, const char *name)
 *	Ensure that the file specified by `name' is a module object file.
 *	PFC_FALSE is returned if not.
 */
static pfc_bool_t
module_validate(int dfd, const char *name)
{
	ElfXX_Ehdr	ehdr;
	struct stat	sbuf;
	int		fd;

	if (PFC_EXPECT_FALSE(fstatat(dfd, name, &sbuf,
				     AT_SYMLINK_NOFOLLOW) != 0)) {
		/* This should never happen. */
		fatal("fstatat(%s) failed: %s", name, strerror(errno));
		/* NOTREACHED */
	}

	if (PFC_EXPECT_FALSE(!modfile_validate(dfd, name, &sbuf))) {
		return PFC_FALSE;
	}

	if ((size_t)sbuf.st_size <= sizeof(ehdr)) {
		warning("\"%s\" is ignored: File size is too small.", name);

		return PFC_FALSE;
	}

	/* Read ELF header. */
	fd = openat(dfd, name, O_RDONLY);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		warning("\"%s\" is ignored: open() failed: %s", name,
			strerror(errno));
		PFC_ASSERT_INT(close(fd), 0);

		return PFC_FALSE;
	}
	read_data(fd, (uint8_t *)&ehdr, sizeof(ehdr));
	PFC_ASSERT_INT(close(fd), 0);

	if (PFC_EXPECT_FALSE(ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
			     ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
			     ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			     ehdr.e_ident[EI_MAG3] != ELFMAG3)) {
		warning("\"%s\" is ignored: Not an ELF object.", name);

		return PFC_FALSE;
	}

	if (PFC_EXPECT_FALSE(ehdr.e_type != ET_DYN)) {
		warning("\"%s\" is ignored: Not an ELF shared object.", name);

		return PFC_FALSE;
	}

	if (PFC_EXPECT_FALSE(ehdr.e_ident[EI_DATA] != ELFDATA_NATIVE)) {
		uint8_t	d = ehdr.e_ident[EI_DATA];

		warning("\"%s\" is ignored: ELF data encoding doesn't match: "
			"<%s>", name,
			(d == ELFDATA2MSB) ? "big endian" :
			(d == ELFDATA2LSB) ? "little endian" : "unknown");

		return PFC_FALSE;
	}

	if (PFC_EXPECT_FALSE(ehdr.e_ident[EI_CLASS] != MODCH_CLASS_NATIVE)) {
		uint8_t	c = ehdr.e_ident[EI_CLASS];

		warning("\"%s\" is ignored: ELF class doesn't match: <%s>",
			name,
			(c == ELFCLASS32) ? "ELF32" :
			(c == ELFCLASS64) ? "ELF64" : "unknown");

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static pfc_bool_t
 * moddep_exists(int dfd, const char *name)
 *	Determine whether the module dependency file for the given module
 *	exists or not.
 *
 *	PFC_TRUE is returned only if exists.
 */
static pfc_bool_t
moddep_exists(int dfd, const char *name)
{
	struct stat	sbuf;
	char		fname[PFC_MODULE_FNAME_MAX];

	snprintf(fname, sizeof(fname), "%s.dep", name);
	if (PFC_EXPECT_FALSE(fstatat(dfd, fname, &sbuf,
				     AT_SYMLINK_NOFOLLOW) != 0)) {
		int	err = errno;

		if (PFC_EXPECT_TRUE(err == ENOENT)) {
			return PFC_FALSE;
		}

		/* This should never happen. */
		fatal("fstatat(%s) failed: %s", fname, strerror(err));
		/* NOTREACHED */
	}

	if (PFC_EXPECT_FALSE(!modfile_validate(dfd, fname, &sbuf))) {
		return PFC_FALSE;
	}

	/* Ensure that the file can be read. */
	if (PFC_EXPECT_FALSE(faccessat(dfd, fname, R_OK, AT_EACCESS) != 0)) {
		warning("\"%s\" is ignored: unable to open: %s", fname,
			strerror(errno));

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static mch_module_t *
 * module_create(const char *name)
 *	Create module data instance for a module specified by the name.
 *	NULL is returned if the specified file is not module object file.
 */
static mch_module_t *
module_create(const char *name)
{
	mch_module_t	*mod = NULL;
	pfc_refptr_t	*rname;
	const char	*p;
	char	*dot, *buf;
	int	err;

	/* First character must be an alphabet. */
	if (PFC_EXPECT_FALSE(!pfc_isalpha_u(*name))) {
		return NULL;
	}

	/* Duplicate filename. */
	buf = strdup(name);
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		fatal("Failed to duplicate filename.");
		/* NOTREACHED */
	}

	/* Search for a dot in the name. */
	dot = strrchr(buf, '.');
	if (PFC_EXPECT_FALSE(dot == NULL)) {
		goto out;
	}

	if (PFC_EXPECT_FALSE(strcmp(dot + 1, PFC_MODULE_LIB_SUFFIX) != 0)) {
		/* Bad filename suffix. */
		goto out;
	}

	for (p = buf + 1; p < dot; p++) {
		char	c = *p;

		if (PFC_EXPECT_FALSE(!pfc_isalnum_u(c) && c != '_')) {
			goto out;
		}
	}

	*dot = '\0';
	rname = pfc_refptr_string_create(buf);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		fatal("Failed to allocate memory for module name.");
		/* NOTREACHED */
	}

	if (PFC_EXPECT_FALSE(pfc_refptr_string_length(rname) >
			     PFC_MODULE_NAME_MAX)) {
		/* Too long module name. */
		warning("%s/%s is ignored: Module name is too long.",
			module_dir, pfc_refptr_string_value(rname));
		pfc_refptr_put(rname);
		goto out;
	}

	mod = (mch_module_t *)malloc(sizeof(*mod));
	if (PFC_EXPECT_FALSE(mod == NULL)) {
		fatal("Failed to allocate module data.");
		/* NOTREACHED */
	}

	mod->mc_refptr = pfc_refptr_create(&module_refops, mod);
	if (PFC_EXPECT_FALSE(mod->mc_refptr == NULL)) {
		fatal("Failed to create reference to module data.");
		/* NOTREACHED */
	}

	/*
	 * Create hash list which keeps modules on which this module
	 * depends.
	 */
	err = pfc_hashlist_create_ref(&mod->mc_moddeps, &moddep_refops,
				      MODULE_HASH_NBUCKETS, 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to create module dependency list: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	/*
	 * Create hash list which keeps modules which depends on this module.
	 * This list doesn't handle reference counter of modules in order to
	 * avoid cross reference.
	 */
	err = pfc_hashlist_create(&mod->mc_depended, NULL,
				  MODULE_HASH_NBUCKETS, 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to create module depended list: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	/* Initialize deplist link for so that destructor can work. */
	pfc_list_init(&mod->mc_deplist);

	mod->mc_name = rname;
	mod->mc_depth = 0;
	mod->mc_score = MCH_MODULE_SCORE_UNDEF;
	mod->mc_index = MODCH_DEP_NONE;

out:
	free(buf);

	return mod;
}

/*
 * static void
 * module_dtor(pfc_ptr_t object)
 *	Destructor of module data.
 */
static void
module_dtor(pfc_ptr_t object)
{
	mch_module_t	*mod = (mch_module_t *)object;
	pfc_cptr_t	value;
	int	err;

	while ((err = pfc_listm_pop(mod->mc_moddeps, &value)) == 0) {
		pfc_refptr_t	*rdep = (pfc_refptr_t *)value;
		mch_moddep_t	*dep = REF_MODDEP(rdep);
		mch_module_t	*dmod = dep->md_module;

		PFC_ASSERT_INT(pfc_listm_remove(dmod->mc_depended,
						(pfc_cptr_t)mod), 0);
		pfc_refptr_put(rdep);
	}
	PFC_ASSERT(err == ENOENT);
	PFC_ASSERT(pfc_listm_get_size(mod->mc_depended) == 0);

	pfc_listm_destroy(mod->mc_moddeps);
	pfc_listm_destroy(mod->mc_depended);

	pfc_list_remove(&mod->mc_deplist);
	pfc_refptr_put(mod->mc_name);
	free(mod);
}

/*
 * static int
 * module_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Comparator for module data.
 *	This function compares module name.
 */
static int
module_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	mch_module_t	*m1 = (mch_module_t *)o1;
	mch_module_t	*m2 = (mch_module_t *)o2;
	pfc_refptr_t	*n1 = m1->mc_name;
	pfc_refptr_t	*n2 = m2->mc_name;
	const pfc_refptr_ops_t	*rops = pfc_refptr_operation(n1);

	return rops->compare((pfc_cptr_t)pfc_refptr_value(n1),
			     (pfc_cptr_t)pfc_refptr_value(n2));
}

/*
 * static pfc_bool_t
 * module_equals(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Determine whether the two module data is identical or not.
 */
static pfc_bool_t
module_equals(pfc_cptr_t o1, pfc_cptr_t o2)
{
	mch_module_t	*m1 = (mch_module_t *)o1;
	mch_module_t	*m2 = (mch_module_t *)o2;
	pfc_refptr_t	*n1 = m1->mc_name;
	pfc_refptr_t	*n2 = m2->mc_name;

	return pfc_refptr_equals(n1, n2);
}

/*
 * static uint32_t
 * module_hashfunc(pfc_cptr_t object)
 *	Return hash value for the specified module data.
 */
static uint32_t
module_hashfunc(pfc_cptr_t object)
{
	mch_module_t	*mod = (mch_module_t *)object;
	pfc_refptr_t	*name = mod->mc_name;
	const pfc_refptr_ops_t	*rops = pfc_refptr_operation(name);

	return rops->hashfunc((pfc_cptr_t)pfc_refptr_value(name));
}

/*
 * static mch_moddep_t *
 * moddep_create(update_ctx_t *PFC_RESTRICT ctx,
 *		 const char *PFC_RESTRICT depfile,
 *		 pfc_refptr_t *PFC_RESTRICT name, uint8_t version)
 *	Create a module dependency instance.
 */
static mch_moddep_t *
moddep_create(update_ctx_t *PFC_RESTRICT ctx, const char *PFC_RESTRICT depfile,
	      pfc_refptr_t *PFC_RESTRICT name, uint8_t version)
{
	pfc_hash_t	modules = ctx->uc_modules;
	pfc_refptr_t	*rmod;
	mch_module_t	*dmod;
	mch_moddep_t	*dep;
	int	err;

	/* Ensure that the specified module exists. */
	err = pfc_hash_get_kref(modules, name, (pfc_cptr_t *)&rmod);
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT(err == ENOENT);
		fatal("%s: Unknown module name: %s", depfile,
		      pfc_refptr_string_value(name));
		/* NOTREACHED */
	}
	dmod = REF_MODULE(rmod);

	dep = (mch_moddep_t *)malloc(sizeof(*dep));
	if (PFC_EXPECT_FALSE(dep == NULL)) {
		fatal("Failed to allocate dependency entry.");
		/* NOTREACHED */
	}

	dep->md_refptr = pfc_refptr_create(&moddep_refops, dep);
	if (PFC_EXPECT_FALSE(dep->md_refptr == NULL)) {
		fatal("Failed to create reference to module dependency.");
		/* NOTREACHED */
	}

	pfc_refptr_get(rmod);
	dep->md_module = dmod;
	dep->md_version = version;

	return dep;
}

/*
 * static void
 * moddep_dtor(pfc_ptr_t object)
 *	Destructor of module dependency.
 */
static void
moddep_dtor(pfc_ptr_t object)
{
	mch_moddep_t	*dep = (mch_moddep_t *)object;

	pfc_refptr_put(dep->md_module->mc_refptr);
	free(dep);
}

/*
 * static int
 * moddep_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Comparator for module dependency.
 *	This function compares module name in md_module.
 */
static int
moddep_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	mch_moddep_t	*d1 = (mch_moddep_t *)o1;
	mch_moddep_t	*d2 = (mch_moddep_t *)o2;
	pfc_refptr_t	*n1 = d1->md_module->mc_name;
	pfc_refptr_t	*n2 = d2->md_module->mc_name;
	const pfc_refptr_ops_t	*rops = pfc_refptr_operation(n1);

	return rops->compare((pfc_cptr_t)pfc_refptr_value(n1),
			     (pfc_cptr_t)pfc_refptr_value(n2));
}

/*
 * static pfc_bool_t
 * moddep_equals(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Determine whether the two module dependency is identical or not.
 */
static pfc_bool_t
moddep_equals(pfc_cptr_t o1, pfc_cptr_t o2)
{
	mch_moddep_t	*d1 = (mch_moddep_t *)o1;
	mch_moddep_t	*d2 = (mch_moddep_t *)o2;
	pfc_refptr_t	*n1 = d1->md_module->mc_name;
	pfc_refptr_t	*n2 = d2->md_module->mc_name;

	return pfc_refptr_equals(n1, n2);
}

/*
 * static uint32_t
 * moddep_hashfunc(pfc_cptr_t object)
 *	Return hash value for the specified module dependency.
 */
static uint32_t
moddep_hashfunc(pfc_cptr_t object)
{
	mch_moddep_t	*dep = (mch_moddep_t *)object;
	pfc_refptr_t	*name = dep->md_module->mc_name;
	const pfc_refptr_ops_t	*rops = pfc_refptr_operation(name);

	return rops->hashfunc((pfc_cptr_t)pfc_refptr_value(name));
}

/*
 * static void
 * moddep_parser_init(moddep_ctx_t *PFC_RESTRICT mdctx, int dfd,
 *		      const char *PFC_RESTRICT name)
 *	Initialize module.dep file parser.
 */
static void
moddep_parser_init(moddep_ctx_t *PFC_RESTRICT mdctx, int dfd,
		   const char *PFC_RESTRICT name)
{
	int	fd;

	snprintf(mdctx->mdc_fname, sizeof(mdctx->mdc_fname), "%s.dep", name);
	debug_printf(1, "Loading %s", mdctx->mdc_fname);

	fd = openat(dfd, mdctx->mdc_fname, O_RDONLY);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		fatal("Failed to open %s: %s", mdctx->mdc_fname,
		      strerror(errno));
		/* NOTREACHED */
	}

	mdctx->mdc_file = fdopen(fd, "r");
	if (PFC_EXPECT_FALSE(mdctx->mdc_file == NULL)) {
		fatal("Failed to open %s: %s", mdctx->mdc_fname,
		      strerror(errno));
		/* NOTREACHED */
	}

	mdctx->mdc_line = 1;
}

/*
 * static void
 * moddep_parser_destroy(moddep_ctx_t *mdctx)
 *	Destroy module.dep file parser.
 */
static void
moddep_parser_destroy(moddep_ctx_t *mdctx)
{
	PFC_ASSERT_INT(fclose(mdctx->mdc_file), 0);
}

/*
 * static pfc_refptr_t *
 * moddep_parser_next(moddep_ctx_t *mdctx)
 *	Return next module name in module.dep file.
 *	Required version is set to mdctx->mdc_version.
 *
 *	NULL is returned if no more dependency is defined.
 */
static pfc_refptr_t *
moddep_parser_next(moddep_ctx_t *mdctx)
{
	uint32_t	count, version = 0;
	pfc_refptr_t	*name;
	int	ic, at = 0, comment = 0;
	char	namebuf[PFC_MODULE_NAME_MAX + 1];

	/* Search for a next module dependency line. */
	while (1) {
		ic = moddep_parser_getchar(mdctx);
		if (ic == EOF) {
			return NULL;
		}

		/* Skip newline. */
		if (ic == '\n') {
			comment = 0;
			continue;
		}
		if (comment != 0) {
			continue;
		}

		/* Skip whitespace. */
		if (pfc_isspace_u(ic)) {
			continue;
		}

		/* Skip comment line. */
		if (ic == '#') {
			comment = 1;
		}
		else {
			break;
		}
	}

	/* This must be the first character of module name. */
	if (PFC_EXPECT_FALSE(!pfc_isalpha_u(ic))) {
		fatal("%s:%u: Unexpected character: 0x%02x",
		      mdctx->mdc_fname, mdctx->mdc_line, (uint8_t)ic);
		/* NOTREACHED */
	}

	/* Parse module name. */
	count = 1;
	namebuf[0] = (char)ic;
	while ((ic = moddep_parser_getchar(mdctx)) != EOF) {
		if (pfc_isalnum_u(ic) || ic == '_') {
			if (count >= PFC_MODULE_NAME_MAX) {
				fatal("%s:%u: Too long module name.",
				      mdctx->mdc_fname, mdctx->mdc_line);
				/* NOTREACHED */
			}
			namebuf[count] = (char)ic;
			count++;
		}
		else {
			if (ic == '@') {
				at = 1;
			}
			else if (PFC_EXPECT_FALSE(!pfc_isspace_u(ic))) {
				fatal("%s:%u: Unexpected character: 0x%02x",
				      mdctx->mdc_fname, mdctx->mdc_line,
				      (uint8_t)ic);
				/* NOTREACHED */
			}

			PFC_ASSERT(count <= PFC_MODULE_NAME_MAX);
			namebuf[count] = '\0';
			break;
		}
	}

	if (at) {
		pfc_bool_t	parsed = PFC_FALSE;

		/* Parse required version. */
		while ((ic = moddep_parser_getchar(mdctx)) != EOF) {
			if (pfc_isdigit_u(ic)) {
				uint32_t	nv;

				nv = version * 10 + (ic - '0');
				if (PFC_EXPECT_FALSE(nv < version ||
						     nv > UINT8_MAX)) {
					fatal("%s:%u: Invalid module version "
					      "for \"%s\"", mdctx->mdc_fname,
					      mdctx->mdc_line, namebuf);
					/* NOTREACHED */
				}
				version = nv;
				parsed = PFC_TRUE;
			}
			else if (PFC_EXPECT_TRUE(pfc_isspace_u(ic))) {
				break;
			}
			else {
				fatal("%s:%u: Unexpected character: 0x%02x",
				      mdctx->mdc_fname, mdctx->mdc_line,
				      (uint8_t)ic);
				/* NOTREACHED */
			}
		}

		if (PFC_EXPECT_FALSE(!parsed)) {
			fatal("%s:%u: Module version for \"%s\" is not "
			      "specified.", mdctx->mdc_fname, mdctx->mdc_line,
			      namebuf);
			/* NOTREACHED */
		}
	}

	mdctx->mdc_version = version;

	/* Skip cursor to the end of line. */
	if (ic != EOF && ic != '\n') {
		while ((ic = moddep_parser_getchar(mdctx)) != EOF) {
			if (ic == '\n') {
				break;
			}
			if (PFC_EXPECT_FALSE(pfc_isspace_u(ic))) {
				fatal("%s:%u: Unexpected character: 0x%02x",
				      mdctx->mdc_fname, mdctx->mdc_line,
				      (uint8_t)ic);
				/* NOTREACHED */
			}
		}
	}

	name = pfc_refptr_string_create(namebuf);
	if (PFC_EXPECT_FALSE(name == NULL)) {
		fatal("Failed to allocate dependency name.");
		/* NOTREACHED */
	}

	return name;
}

/*
 * static int
 * moddep_parser_getchar(moddep_ctx_t *mdctx)
 *	Read a character from the module.dep file.
 */
static int
moddep_parser_getchar(moddep_ctx_t *mdctx)
{
	int	ic;
	FILE	*fp = mdctx->mdc_file;

	ic = getc(fp);
	if (ic == EOF) {
		if (PFC_EXPECT_FALSE(ferror(fp))) {
			fatal("%s: I/O error: %s", mdctx->mdc_fname,
			      strerror(errno));
			/* NOTREACHED */
		}
	}

	if (ic == '\n') {
		/* Bump up the line number. */
		mdctx->mdc_line++;
	}

	return ic;
}

#ifdef	PFC_VERBOSE_DEBUG

/*
 * static void
 * module_sort_verify(mch_module_t **modules, size_t nmodules)
 *	Ensure that module load order is valid.
 */
static void
module_sort_verify(mch_module_t **modules, size_t nmodules)
{
	mch_module_t	**modpp, *mod;
	pfc_hash_t	hash;
	int	err;

	if (PFC_EXPECT_FALSE(nmodules <= 1)) {
		return;
	}

	/* Create hash table that keeps module name. */
	err = pfc_strhash_create(&hash, NULL, MODULE_HASH_NBUCKETS,
				 PFC_HASH_NOLOCK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to create module name hash table: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	/* Enter name of module which must be loaded first. */
	mod = *modules;
	err = pfc_hash_put_kref(hash, mod->mc_name, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to put module name to the hash table: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	for (modpp = modules + 1; modpp < modules + nmodules; modpp++) {
		pfc_listiter_t	it;
		pfc_cptr_t	value;

		/*
		 * All modules on which this module depends must be loaded
		 * before this module.
		 */
		mod = *modpp;
		it = pfc_listiter_create(mod->mc_moddeps);
		if (PFC_EXPECT_FALSE(it == NULL)) {
			fatal("Failed to create dependency list iterator.");
			/* NOTREACHED */
		}

		while ((err = pfc_listiter_next(it, &value)) == 0) {
			pfc_refptr_t	*rdep = (pfc_refptr_t *)value;
			mch_moddep_t	*dep = REF_MODDEP(rdep);
			mch_module_t	*dmod = dep->md_module;

			err = pfc_hash_get_kref(hash, dmod->mc_name, NULL);
			if (PFC_EXPECT_FALSE(err != 0)) {
				fatal("Module \"%s\" must be loaded before "
				      "\"%s\".",
				      MCH_MODULE_NAME(dmod),
				      MCH_MODULE_NAME(mod));
				/* NOTREACHED */
			}
		}
		PFC_ASSERT(err == ENOENT);
		pfc_listiter_destroy(it);

		err = pfc_hash_put_kref(hash, mod->mc_name, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fatal("Failed to put module name to the hash table: "
			      "%s", strerror(err));
			/* NOTREACHED */
		}
	}

	pfc_hash_destroy(hash);
}

#endif	/* PFC_VERBOSE_DEBUG */
