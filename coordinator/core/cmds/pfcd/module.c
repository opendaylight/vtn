/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * module.c - PFC module management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <pfc/path.h>
#include <pfc/debug.h>
#include <pfc/conf.h>
#include <pfc/thread.h>
#include <pfc/clock.h>
#include <pfc/synch.h>
#include <pfc/listmodel.h>
#include <pfc/exstatus.h>
#include <pfc/atomic.h>
#include <pfc/extcmd.h>
#include <modcache.h>
#include <module_impl.h>
#include "pfcd.h"

#ifndef	PFC_HAVE_PPOLL
#error	Current implementation requires ppoll(2).
#endif	/* !PFC_HAVE_PPOLL */

struct mod_loader;
typedef struct mod_loader	mod_loader_t;

/*
 * Structure which defines work on module loader threads.
 */
typedef struct {
	/*
	 * Parameter name which determines number of loader threads.
	 * If NULL, only one thread is used.
	 */
	const char	*mlw_param_nthreads;

	/*
	 * Parameter name which determines timeout per module.
	 */
	const char	*mlw_param_timeout;

	/*
	 * Default module operation timeout in seconds.
	 */
	uint32_t	mlw_default_timeout;
	pmod_state_t	mlw_state;	/* target module state */
	pfc_bool_t	mlw_busy_wait;	/* module busy wait is needed */

	/*
	 * Select modules to be handled, and push them to the loader queue.
	 * Argument specified to module_loader_exec() is passed to `arg'.
	 */
	int		(*mlw_select)(pfc_listm_t queue, pfc_ptr_t arg);

	/*
	 * Constructor of module loader context, which will be called just
	 * after creating module loader context.
	 * It must return zero on success, and error number on failure.
	 * NULL can be set if there is nothing to do.
	 */
	int		(*mlw_ctor)(mod_loader_t *loader);

	/*
	 * Destructor of module loader context, which will be called just
	 * before releasing module loader lock.
	 * NULL can be set if there is nothing to do.
	 */
	void		(*mlw_dtor)(mod_loader_t *loader);

	/*
	 * Print error message.
	 */
	void		(*mlw_error)(const char *fmt, ...)
		PFC_FATTR_PRINTFLIKE(1, 2);
} mod_loader_spec_t;

/*
 * Module loader/unloader context.
 */
struct mod_loader {
	pfc_listm_t		ml_queue;	/* module descriptor queue */
	pfc_mutex_t		ml_lock;	/* loader context lock */
	pfc_cond_t		ml_cond;	/* condition variable */
	pmod_state_t		ml_state;	/* target module state */
	pfc_ptr_t		ml_arg;		/* argument for mlw_select */
	pfc_ptr_t		ml_data;	/* private data */
	volatile int		ml_result;	/* result */
	uint8_t			ml_nthreads;	/* number of threads */
	volatile uint8_t	ml_done;	/* number of exited threads */
	volatile uint16_t	ml_flags;	/* flags */
};

/*
 * Flags for ml_flags.
 */
#define	MODLDF_START		0x1U	/* job start */
#define	MODLDF_PREPARE		0x2U	/* loader thread exists */
#define	MODLDF_CANCEL		0x4U	/* loader has been canceled */

#define	MOD_LOADER_LOCK(loader)		pfc_mutex_lock(&(loader)->ml_lock)
#define	MOD_LOADER_UNLOCK(loader)	pfc_mutex_unlock(&(loader)->ml_lock)

#define	MOD_LOADER_SIGNAL(loader)	pfc_cond_signal(&(loader)->ml_cond)
#define	MOD_LOADER_BROADCAST(loader)	pfc_cond_broadcast(&(loader)->ml_cond)
#define	MOD_LOADER_WAIT(loader)					\
	pfc_cond_wait(&(loader)->ml_cond, &(loader)->ml_lock)
#define	MOD_LOADER_TIMEDWAIT_ABS(loader, abstime)			\
	pfc_cond_timedwait_abs(&(loader)->ml_cond, &(loader)->ml_lock,	\
			       (abstime))

/*
 * Module boot context.
 */
typedef struct {
	int		mb_pipe[2];	/* control pipe FDs */
	pfc_thread_t	mb_thread;	/* module boot cancel thread */
} modboot_t;

#define	MODBOOT_PIPE_FOR_READ(mbp)	((mbp)->mb_pipe[0])
#define	MODBOOT_PIPE_FOR_WRITE(mbp)	((mbp)->mb_pipe[1])

/*
 * Default module load/unload timeout in seconds.
 */
#define	MOD_LOAD_TIMEOUT_DEFAULT	10U
#define	MOD_UNLOAD_TIMEOUT_DEFAULT	10U

/*
 * How long, in seconds, we should wait for completion of ongoing module
 * operation in module_operation_set().
 */
#define	MOD_OP_SET_TIMEOUT		1U		/* 1 second */

/*
 * Maximum length of the name of PFC module cache management command.
 */
#define	MOD_CACHECMD_MAXLEN		64U

/*
 * "module" block handle in pfcd.conf.
 */
static pfc_cfblk_t	conf_module = PFC_CFBLK_INVALID;
static const char	conf_module_name[] = "module";

/*
 * Parameter names in "module" block.
 */
static const char	param_load_modules[] = "load_modules";
static const char	param_load_nthreads[] = "load_nthreads";
static const char	param_load_timeout[] = "load_timeout";
static const char	param_unload_nthreads[] = "unload_nthreads";
static const char	param_unload_timeout[] = "unload_timeout";
static const char	param_cache_update[] = "cache_update";

/*
 * Maximum and minimum value for default value of load_nthreads and
 * unload_nthreads.
 */
#define	MODULE_MIN_LOADER_NTHREADS	2U
#define	MODULE_MAX_LOADER_NTHREADS	16U

/*
 * Internal prototypes.
 */
static void		module_cache_attach(pfc_flock_t lock,
					    modch_map_t *map);
static void		module_cache_update(void);
static pfc_flock_t	module_dir_lock_open(void);
static void		module_dir_lock_close(pfc_flock_t lock);
static void		module_dir_lock(pfc_flock_t lock);
static void		module_dir_unlock(pfc_flock_t lock);
static int		module_loader_exec(const mod_loader_spec_t
					   *PFC_RESTRICT spec,
					   pfc_ptr_t PFC_RESTRICT arg);
static int		module_loader_init(mod_loader_t *PFC_RESTRICT loader,
					   pfc_ptr_t arg, pmod_state_t state,
					   const char *PFC_RESTRICT param);
static void		module_loader_destroy(mod_loader_t *PFC_RESTRICT loader,
					      const mod_loader_spec_t
					      *PFC_RESTRICT spec);
static int		module_loader_prepare(mod_loader_t *loader);
static int		module_loader_select_all(pfc_listm_t queue,
						 pfc_ptr_t unused);

static int		module_loader_select_boot(pfc_listm_t queue,
						  pfc_ptr_t unused);
static int		module_loader_ctor_boot(mod_loader_t *loader);
static void		module_loader_dtor_boot(mod_loader_t *loader);
static int		module_loader_select_shutdown(pfc_listm_t queue,
						      pfc_ptr_t unused);
static void		*boot_cancel_thread(void *arg);

static void		module_loader_start(mod_loader_t *loader);
static uint32_t		module_loader_nthreads(const char *param);
static void 		*module_loader_main(void *arg);
static int		module_busy_wait(pmodule_t *PFC_RESTRICT mod,
					 const pfc_timespec_t *PFC_RESTRICT
					 abstime);
static void		module_loader_seterror(mod_loader_t *loader, int err)
	PFC_FATTR_NOINLINE;
static size_t		module_set_mandatory(int argc, char **argv,
					     uint32_t flags);

/*
 * Module loader specifications.
 */

/* Module boot */
static const mod_loader_spec_t	mod_loader_spec_boot = {
	.mlw_param_nthreads	= param_load_nthreads,
	.mlw_param_timeout	= param_load_timeout,
	.mlw_default_timeout	= MOD_LOAD_TIMEOUT_DEFAULT,
	.mlw_state		= PMOD_STATE_RUNNING,
	.mlw_busy_wait		= PFC_FALSE,
	.mlw_select		= module_loader_select_boot,
	.mlw_ctor		= module_loader_ctor_boot,
	.mlw_dtor		= module_loader_dtor_boot,
	.mlw_error		= die,
};

/* Module load check */
static const mod_loader_spec_t	mod_loader_spec_check = {
	.mlw_param_timeout	= param_load_timeout,
	.mlw_default_timeout	= MOD_LOAD_TIMEOUT_DEFAULT,
	.mlw_state		= PMOD_STATE_LOADED,
	.mlw_busy_wait		= PFC_FALSE,
	.mlw_select		= module_loader_select_all,
	.mlw_error		= die,
};

/* Module shutdown */
static const mod_loader_spec_t	mod_loader_spec_shutdown = {
	.mlw_param_nthreads	= param_unload_nthreads,
	.mlw_param_timeout	= param_unload_timeout,
	.mlw_default_timeout	= MOD_UNLOAD_TIMEOUT_DEFAULT,
	.mlw_state		= PMOD_STATE_UNLOADED,
	.mlw_busy_wait		= PFC_TRUE,
	.mlw_select		= module_loader_select_shutdown,
	.mlw_error		= die,
};

/*
 * void
 * module_init(int argc, char **argv, uint32_t flags)
 *	Initialize PFC modules.
 *
 *	module_init() takes arguments specified by command line.
 *	Each argument is considered as name of module to be loaded.
 *
 *	`flags' is a bitmask which determines the behavior of this function.
 *	Supported bits are:
 *
 *	- PFCD_MODF_ALL:	Load all modules registered in the module
 *				cache.
 *	- PFCD_MODF_CHECKONLY:	PFC module system is initialized only for
 *				for module configuration file check.
 *
 *	- PFCD_MODF_NOCONF:	Ignore "modules.load_modules" option in the
 *				configuration file.
 */
void
module_init(int argc, char **argv, uint32_t flags)
{
	modch_map_t	map;
	pfc_flock_t	lock;
	const char	*confdir;
	size_t		count;

	/* Get "module" configuration block handle. */
	conf_module = pfc_sysconf_get_block(conf_module_name);

	/* Acquire module cache directory lock in reader mode. */
	lock = module_dir_lock_open();
	module_dir_lock(lock);

	/* Attach contents of the module cache file. */
	module_cache_attach(lock, &map);

	/* Register all modules in the cache. */
	confdir = pfc_conf_get_string(pfcd_options, "modconf_dir",
				      PFC_MODCONFDIR);
	pfc_module_init(pfcd_module_dir, confdir, map.mm_addr, map.mm_size);

	/* Detach the cache, and release the lock. */
	pfc_modcache_detach(&map);
	module_dir_lock_close(lock);

	if (flags & PFCD_MODF_ALL) {
		/* Load all modules. */
		count = pfc_module_count();
		if (PFC_EXPECT_TRUE(count != 0)) {
			pfc_module_set_mandatory_all(PFC_TRUE);
		}
	}
	else if ((flags & PFCD_MODF_CHECKONLY) == 0) {
		/* Set mandatory flag to modules to be loaded. */
		count = module_set_mandatory(argc, argv, flags);
	}
	else {
		count = 0;
	}

	if (PFC_EXPECT_FALSE((flags & PFCD_MODF_CHECKONLY) == 0 &&
			     count == 0)) {
		warning("No module is specified to be loaded.");
	}
}

/*
 * int
 * module_boot(void)
 *	Start all mandatory modules, specified by pfcd.conf and command line
 *	arguments.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *	Note that ECANCELED is returned if shutdown signal has been received
 *	during module boot.
 */
int
module_boot(void)
{
	int	err;

	/* Verify signal mask of the current thread. */
	signal_mask_assert();

	/* Load all mandatory modules. */
	err = module_loader_exec(&mod_loader_spec_boot, NULL);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_module_set_started();
	}

	return err;
}

/*
 * void
 * module_shutdown(void)
 *	Shutdown the PFC module system.
 */
void
module_shutdown(void)
{
	int	err;

	/* Call module unload hooks. */
	pfc_module_runhooks();

	/* Unload all modules. */
	err = module_loader_exec(&mod_loader_spec_shutdown, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		die("Module shutdown failed: %s", strerror(err));
		/* NOTREACHED */
	}
}

/*
 * void
 * module_check_conf(void)
 *	Verify module configuration files.
 *
 * Calling/Exit State:
 *	Program exits if an error is detected.
 *	Otherwise error number which indicates the cause of error is returned.
 */
void
module_check_conf(void)
{
	int	err;

	/* Initialize PFC module. */
	module_init(0, NULL, PFCD_MODF_CHECKONLY);

	/* Load all modules to verify all module configuration files. */
	err = module_loader_exec(&mod_loader_spec_check, NULL);
	if (PFC_EXPECT_FALSE(err == EINVAL)) {
		fatal("Module configuration file check failed.");
		/* NOTREACHED */
	}
	else if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Module configuration file check failed: %s",
		      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * static void
 * module_cache_attach(pfc_flock_t lock, modch_map_t *map)
 *	Attach the contents of the module cache file.
 *
 * Remarks:
 *	The caller must call this function with holding module directory
 *	lock in reader mode. Although module_cache_attach() may release
 *	the lock, the lock is held again on return.
 */
static void
module_cache_attach(pfc_flock_t lock, modch_map_t *map)
{
	pfc_bool_t	do_update;
	const char	*dir = pfcd_modcache_dir;
	int		err;

	/*
	 * Check to see whether the cache is requested to be updated on
	 * every boot. Default is "false".
	 */
	do_update = pfc_conf_get_bool(conf_module, param_cache_update,
				      PFC_FALSE);
	verbose("module.cache_update = %s", (do_update) ? "true" : "false");

	if (!do_update) {
		/* Try to attach existing module cache file. */
		err = pfc_modcache_attach(dir, map);
		if (PFC_EXPECT_TRUE(err == 0)) {
			return;
		}

		if (err == ENOENT) {
			verbose("The module cache must be newly created.");
		}
		else {
			warning("Failed to attach module cache file: %s",
				strerror(err));
		}
		/* FALLTHROUGH */
	}

	/* The module cache file must be updated. */
	module_dir_unlock(lock);
	module_cache_update();
	module_dir_lock(lock);

	/* Attach the module cache file. */
	err = pfc_modcache_attach(dir, map);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to attach module cache file: %s", strerror(err));
		/* NOTREACHED */
	}
}

/*
 * static void
 * module_cache_update(void)
 *	Update module cache file.
 *	The module directory lock must not be held on call.
 */
static void
module_cache_update(void)
{
	pfc_extcmd_t	cmd;
	const char	*prog, *lastp;
	char		pbuf[MOD_CACHECMD_MAXLEN];
	size_t		len = strlen(pfcd_progname);
	int		err, sig, status;

	/* Construct modcache command name by program name. */
	PFC_ASSERT(len < sizeof(pbuf));
	lastp = pfcd_progname + len - 1;
	if (*lastp == 'd') {
		const char	*pp;
		char		*dst, buf[MOD_CACHECMD_MAXLEN];

		for (dst = buf, pp = pfcd_progname; pp < lastp; dst++, pp++) {
			*dst = *pp;
		}
		*dst = '\0';
		snprintf(pbuf, sizeof(pbuf), "%s_modcache", buf);
		prog = pbuf;
	}
	else {
		prog = "pfc_modcache";
	}

	/* Create command context to invoke pfc_modcache. */
	err = pfc_extcmd_create(&cmd, prog, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to create %s command context: %s",
		      prog, strerror(err));
		/* NOTREACHED */
	}

	err = pfc_extcmd_add_arguments(cmd, "-uwM", pfcd_module_dir,
				       "-c", pfcd_modcache_dir, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to add arguments for %s command: %s",
		      prog, strerror(err));
		/* NOTREACHED */
	}

	/* Invoke pfc_modcache command. */
	verbose("Invoke %s.", prog);
	err = pfc_extcmd_execute(cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to invoke %s command: %s", prog, strerror(err));
		/* NOTREACHED */
	}

	/* Check the results. */
	sig = pfc_extcmd_get_signal(cmd);
	if (PFC_EXPECT_FALSE(!PFC_EXTCMD_ISERR(sig))) {
		fatal("%s command was killed by signal %d.", prog, sig);
		/* NOTREACHED */
	}

	status = pfc_extcmd_get_status(cmd);
	if (PFC_EXPECT_FALSE(status != PFC_EX_OK)) {
		fatal("%s command exited with status %d.", prog, status);
		/* NOTREACHED */
	}

	pfc_extcmd_destroy(cmd);
}

/*
 * static pfc_flock_t
 * module_dir_lock_open(void)
 *	Return the module directory lock handle.
 */
static pfc_flock_t
module_dir_lock_open(void)
{
	pfc_flock_t	lock;
	int	err;

	err = pfc_flock_opendir(&lock, pfcd_modcache_dir, O_RDWR | O_CREAT,
				MODCH_LOCK_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to open module directory lock handle: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	return lock;
}

/*
 * static void
 * module_dir_lock_close(pfc_flock_t lock)
 *	Close the module directory lock handle.
 */
static void
module_dir_lock_close(pfc_flock_t lock)
{
	PFC_ASSERT_INT(pfc_flock_closedir(lock), 0);
}

/*
 * static void
 * module_dir_lock(pfc_flock_t lock)
 *	Acquire module directory lock in reader mode.
 */
static void
module_dir_lock(pfc_flock_t lock)
{
	int	err;

	/*
	 * Acquire directory lock.
	 * If the lock is busy, this thread will be blocked until the lock
	 * is released.
	 */
	err = pfc_flock_rdlock(lock, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to acquire module directory lock: %s",
		      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * static void
 * module_dir_unlock(pfc_flock_t lock)
 *	Release the module directory lock.
 */
static void
module_dir_unlock(pfc_flock_t lock)
{
	PFC_ASSERT_INT(pfc_flock_unlock(lock), 0);
}

/*
 * static int
 * module_loader_exec(const mod_loader_spec_t *PFC_RESTRICT spec,
 *		      pfc_ptr_t PFC_RESTRICT arg)
 *	Execute module operation.
 *	Operation is defined by mod_loader_spec.
 *
 *	`arg' will be passed to `mlw_select' method.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
module_loader_exec(const mod_loader_spec_t *PFC_RESTRICT spec,
		   pfc_ptr_t PFC_RESTRICT arg)
{
	mod_loader_t	loader;
	pfc_listm_t	queue;
	pmodule_t	*mod;
	uint32_t	timeout, nmodules;
	pfc_timespec_t	tsbuf, *abstime;
	int	err;

	/* Initialize module loader. */
	err = module_loader_init(&loader, arg, spec->mlw_state,
				 spec->mlw_param_nthreads);
	if (PFC_EXPECT_FALSE(err != 0)) {
		spec->mlw_error("Failed to initialize module loader: %s",
				strerror(err));

		return err;
	}

	/* Call constructor of module loader context. */
	if (spec->mlw_ctor != NULL) {
		err = spec->mlw_ctor(&loader);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}
	}

	/* Select modules to be handled. */
	queue = loader.ml_queue;
	err = spec->mlw_select(queue, arg);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}
	nmodules = pfc_listm_get_size(queue);

	if (PFC_EXPECT_FALSE(nmodules == 0)) {
		/* There is no module to be handled. */
		goto out;
	}

	/* Determine module operation timeout. */
	timeout = pfc_conf_get_uint32(conf_module, spec->mlw_param_timeout,
				      spec->mlw_default_timeout);
	if (timeout == 0) {
		pfc_log_verbose("Module operation timeout: none");
		abstime = NULL;
	}
	else {
		pfc_timespec_t	ts;

		pfc_log_verbose("Module operation timeout: %u seconds per "
				"module (%u seconds)", timeout,
				timeout * nmodules);
		abstime = &tsbuf;
		ts.tv_sec = timeout * nmodules;
		ts.tv_nsec = 0;
		PFC_ASSERT_INT(pfc_clock_abstime(abstime, &ts), 0);
	}

	MOD_LOADER_LOCK(&loader);
	err = loader.ml_result;
	MOD_LOADER_UNLOCK(&loader);
	if (err != 0) {
		/* This can happen if the module loader has been canceled. */
		goto out;
	}

	/* Prepare module loader threads. */
	err = module_loader_prepare(&loader);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	if (spec->mlw_busy_wait) {
		pfc_listiter_t	it;

		/* Ensure that all modules to be handled are not busy. */
		it = pfc_listiter_create(queue);
		if (PFC_EXPECT_FALSE(it == NULL)) {
			spec->mlw_error("Failed to create module queue "
					"iterator.");
			err = ENOMEM;
			goto out;
		}
		while ((err = pfc_listiter_next(it, (pfc_cptr_t *)&mod)) == 0) {
			err = module_busy_wait(mod, abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				spec->mlw_error("%s: Module is busy.",
						PMODULE_NAME(mod));
				pfc_listiter_destroy(it);
				goto out;
			}
		}
		PFC_ASSERT(err == ENOENT);
		pfc_listiter_destroy(it);
	}

	/* Start module loader threads. */
	module_loader_start(&loader);

	/* Wait for all threads to finish their work. */
	MOD_LOADER_LOCK(&loader);
	while ((err = loader.ml_result) == 0 &&
	       loader.ml_done != loader.ml_nthreads) {
		err = MOD_LOADER_TIMEDWAIT_ABS(&loader, abstime);
		if (PFC_EXPECT_FALSE(err == EINTR)) {
			continue;
		}

		if (PFC_EXPECT_FALSE(err == ETIMEDOUT)) {
			module_loader_seterror(&loader, err);
			MOD_LOADER_UNLOCK(&loader);
			spec->mlw_error("Module operation did not complete "
					"within %u seconds.",
					timeout * nmodules);
			goto out;
		}
		PFC_ASSERT(err == 0);
	}
	MOD_LOADER_UNLOCK(&loader);

out:
	module_loader_destroy(&loader, spec);

	return err;
}

/*
 * static int
 * module_loader_init(mod_loader_t *PFC_RESTRICT loader, pfc_ptr_t arg,
 *		      pmod_state_t state, const char *PFC_RESTRICT param)
 *	Initialize module loader context.
 *	`arg' is an argument to be passed to mlw_select.
 *	`state' specifies the target module state.
 *	`param' is a parameter name which determines number of loader threads.
 */
static int
module_loader_init(mod_loader_t *PFC_RESTRICT loader, pfc_ptr_t arg,
		   pmod_state_t state, const char *PFC_RESTRICT param)
{
	uint32_t	nthreads;
	int		err;

	/* Determine number of module loader threads. */
	nthreads = (param == NULL) ? 1 : module_loader_nthreads(param);
	PFC_ASSERT(nthreads != 0 && nthreads < UINT8_MAX);

	err = PFC_MUTEX_INIT(&loader->ml_lock);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should not happen. */

		return err;
	}

	err = pfc_cond_init(&loader->ml_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should not happen. */

		return err;
	}

	/* Create linked list to keep module descriptors. */
	err = pfc_llist_create(&loader->ml_queue);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create module descriptor list: %s",
			      strerror(err));

		return err;
	}

	loader->ml_arg = arg;
	loader->ml_data = NULL;
	loader->ml_state = state;
	loader->ml_result = 0;
	loader->ml_done = 0;
	loader->ml_flags = 0;
	loader->ml_nthreads = nthreads;

	return 0;
}

/*
 * static void
 * module_loader_destroy(mod_loader_t *PFC_RESTRICT loader,
 *			 const mod_loader_spec_t *PFC_RESTRICT spec)
 *	Destroy module loader context.
 */
static void
module_loader_destroy(mod_loader_t *PFC_RESTRICT loader,
		      const mod_loader_spec_t *PFC_RESTRICT spec)
{
	pfc_listm_t	queue = loader->ml_queue;
	uint16_t	flags;

	/* Call destructor of module loader context. */
	if (spec->mlw_dtor != NULL) {
		spec->mlw_dtor(loader);
	}

	/* Clear module queue. */
	pfc_llist_shutdown(queue, PFC_TRUE);

	MOD_LOADER_LOCK(loader);

	flags = loader->ml_flags;
	if ((flags & MODLDF_PREPARE) &&
	    loader->ml_done != loader->ml_nthreads) {
		if ((flags & MODLDF_START) == 0) {
			/* Wake up all threads. */
			loader->ml_flags = (flags | MODLDF_START);
		}
		else if (PFC_EXPECT_FALSE(loader->ml_result != 0)) {
			/*
			 * We must cancel all module operations currently
			 * running, or active module operation may not return.
			 */
			pfc_module_cancel();
		}

		MOD_LOADER_BROADCAST(loader);
		do {
			MOD_LOADER_WAIT(loader);
		} while (loader->ml_done != loader->ml_nthreads);
	}

	MOD_LOADER_UNLOCK(loader);

	pfc_listm_destroy(queue);

#ifdef	PFC_VERBOSE_DEBUG
	PFC_ASSERT_INT(pfc_cond_destroy(&loader->ml_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&loader->ml_lock), 0);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * static int
 * module_loader_prepare(mod_loader_t *loader)
 *	Prepare module loader threads.
 */
static int
module_loader_prepare(mod_loader_t *loader)
{
	uint32_t	i, nthreads, nmodules;
	uint16_t	flags = 0;
	int		err;

	nthreads = loader->ml_nthreads;
	nmodules = pfc_listm_get_size(loader->ml_queue);
	if (PFC_EXPECT_FALSE(nmodules == 0)) {
		/* The module loader has been canceled. */
		return ECANCELED;
	}

	/*
	 * No need to create loader threads more than the number of modules
	 * to be handled.
	 */
	if (nthreads > nmodules) {
		nthreads = nmodules;
	}

	err = 0;
	for (i = 0; i < nthreads; i++) {
		pfc_thread_t	t;

		/* Create loader thread. */
		err = pfc_thread_create(&t, module_loader_main, (void *)loader,
					PFC_THREAD_DETACHED);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
	}

	if (PFC_EXPECT_FALSE(i == 0)) {
		pfc_log_error("Failed to create module loader thread: %s",
			      strerror(err));
		PFC_ASSERT(err != 0);
		goto out;
	}

	/* At least one thread has been created. */
	flags |= MODLDF_PREPARE;

	if (PFC_EXPECT_TRUE(i == nthreads)) {
		pfc_log_verbose("Number of module loader threads: %u", i);
	}
	else {
		pfc_log_warn("Required %u loader threads, but created %u: %s",
			     nthreads, i, strerror(err));
	}
	err = 0;

out:
	MOD_LOADER_LOCK(loader);
	loader->ml_flags |= flags;
	loader->ml_nthreads = (uint8_t)i;
	MOD_LOADER_UNLOCK(loader);

	return err;
}

/*
 * static int
 * module_loader_select_all(pfc_listm_t queue, pfc_ptr_t unused)
 *	Select all modules.
 *	This function is used to load all modules to check module configuration
 *	file.
 */
static int
module_loader_select_all(pfc_listm_t queue, pfc_ptr_t unused)
{
	int	err;

	/* Select all modules. */
	err = pfc_module_select(queue, pfc_module_select_all, PFC_FALSE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		die("Failed to select all modules: %s", strerror(err));
		/* NOTREACHED */
	}

	return 0;
}

/*
 * static int
 * module_loader_select_boot(pfc_listm_t queue, pfc_ptr_t unused)
 *	Select modules to be loaded at bootstrap.
 */
static int
module_loader_select_boot(pfc_listm_t queue, pfc_ptr_t unused)
{
	int	err;

	/* Select mandatory modules. */
	err = pfc_module_select(queue, pfc_module_select_mandatory, PFC_FALSE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		die("Failed to select mandatory modules: %s", strerror(err));
		/* NOTREACHED */
	}

	/* Change daemon state. */
	pfcd_setstate(PFCD_STATE_MODLOAD);

	return 0;
}

/*
 * static int
 * module_loader_ctor_boot(mod_loader_t *loader)
 *	Constructor of loader context for module boot.
 *
 *	This function creates a module boot cancel thread which receives
 *	shutdown signal. If received, module boot will be canceled.
 */
static int
module_loader_ctor_boot(mod_loader_t *loader)
{
	modboot_t	*mbp;
	int		err;

	/* Create module boot context. */
	mbp = (modboot_t *)malloc(sizeof(*mbp));
	if (PFC_EXPECT_FALSE(mbp == NULL)) {
		pfc_log_error("Failed to allocate module boot context.");

		return ENOMEM;
	}

	/* Create pipe which is used to cancel module boot cancel thread. */
	if (PFC_EXPECT_FALSE(pfc_pipe_open(mbp->mb_pipe,
					   PFC_PIPE_CLOEXEC_NB) != 0)) {
		err = errno;
		pfc_log_error("Failed to create pipe for module boot: %s",
			      strerror(err));
		goto error;
	}

	loader->ml_data = mbp;

	/* Create module boot cancel thread. */
	err = pfc_thread_create(&mbp->mb_thread, boot_cancel_thread,
				loader, 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create module boot cancel thread: %s",
			      strerror(err));
		close(MODBOOT_PIPE_FOR_READ(mbp));
		close(MODBOOT_PIPE_FOR_WRITE(mbp));
		loader->ml_data = NULL;
		goto error;
	}

	return 0;

error:
	free(mbp);

	return err;
}

/*
 * static void
 * module_loader_dtor_boot(mod_loader_t *loader)
 *	Destructor of module loader context associated with module boot.
 *
 *	This function destroys a module boot cancel thread created by
 *	module_loader_ctor_boot().
 */
static void
module_loader_dtor_boot(mod_loader_t *loader)
{
	modboot_t	*mbp = (modboot_t *)loader->ml_data;
	int		err;

	if (PFC_EXPECT_FALSE(mbp == NULL)) {
		/* Nothing to do. */
		return;
	}

	/* Close write side of the control pipe. */
	close(MODBOOT_PIPE_FOR_WRITE(mbp));

	/* Wait for a boot cancel thread to quit. */
	err = pfc_thread_join(mbp->mb_thread, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should not happen. */
		pfc_log_warn("Failed to join module boot loader thread: %s",
			     strerror(err));
	}

	free(mbp);
	loader->ml_data = NULL;
}

/*
 * static void *
 * boot_cancel_thread(void *arg)
 *	Start routine of module boot cancel thread.
 *
 *	The purpose of module boot cancel thread is to receive shutdown signal.
 *	If received, this thread cancels module boot, and starts shutdown
 *	sequence.
 */
static void *
boot_cancel_thread(void *arg)
{
	mod_loader_t	*loader = (mod_loader_t *)arg;
	modboot_t	*mbp = (modboot_t *)loader->ml_data;
	int		fd = MODBOOT_PIPE_FOR_READ(mbp);
	sigset_t	mask;
	pfc_bool_t	down = PFC_FALSE;

	pfc_log_verbose("Start module boot cancel thread.");

	/* Obtain current signal mask. */
	PFC_ASSERT_INT(pthread_sigmask(SIG_SETMASK, NULL, &mask), 0);

	/* Create signal mask applied during the call of ppoll(). */
	signal_delete_shutdown(&mask);

	for (;;) {
		struct pollfd	pfd;
		int		nfds;

		pfd.fd = fd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		/* Wait for an input event on the control pipe. */
		nfds = ppoll(&pfd, 1, NULL, (const sigset_t *)&mask);
		if (nfds < 0) {
			int	err = errno;

			if (PFC_EXPECT_TRUE(err == EINTR)) {
				/* Dispatch signals. */
				down = signal_dispatch();
				if (PFC_EXPECT_TRUE(down)) {
					pfc_log_warn("Module boot has been "
						     "canceled by a signal.");
					break;
				}
			}
			else {
				pfc_log_error("ppoll() failed on module boot "
					      "cancel thread: %s",
					      strerror(err));
				down = PFC_TRUE;
				break;
			}
		}
		else if (PFC_EXPECT_TRUE(nfds > 0)) {
			/*
			 * The module loader does not write any data to the
			 * control pipe, so this means the pipe has been
			 * closed.
			 */
			break;
		}
	}
	close(fd);

	if (down) {
		/* Cancel all loader threads. */
		MOD_LOADER_LOCK(loader);
		module_loader_seterror(loader, ECANCELED);
		MOD_LOADER_UNLOCK(loader);
	}

	pfc_log_verbose("Module boot cancel thread is going to quit.");

	return NULL;
}

/*
 * static int
 * module_loader_select_shutdown(pfc_listm_t queue, pfc_ptr_t unused)
 *	Select modules to be unloaded on system shutdown.
 */
static int
module_loader_select_shutdown(pfc_listm_t queue, pfc_ptr_t unused)
{
	int	err;

	err = pfc_module_shutdown(queue);
	if (PFC_EXPECT_FALSE(err != 0)) {
		die("Failed to select modules to be unloaded on shutdown: %s",
		    strerror(err));
		/* NOTREACHED */
	}

	return 0;
}

/*
 * static void
 * module_loader_start(mod_loader_t *loader)
 *	Start module loader threads.
 */
static void
module_loader_start(mod_loader_t *loader)
{
	/* Shutdown module queue with retaining elements. */
	pfc_llist_shutdown(loader->ml_queue, PFC_FALSE);

	MOD_LOADER_LOCK(loader);

	loader->ml_flags |= MODLDF_START;
	MOD_LOADER_BROADCAST(loader);

	MOD_LOADER_UNLOCK(loader);
}

/*
 * static uint32_t
 * module_loader_nthreads(const char *param)
 *	Return number of loader or unloader threads.
 *	`param' must be a parameter name, "load_nthreads" or "unload_nthreads".
 */
static uint32_t
module_loader_nthreads(const char *param)
{
	uint32_t	nthreads;

	nthreads = pfc_conf_get_uint32(conf_module, param, 0);
	if (nthreads == 0) {
		/* Determine default value using number of online CPUs. */
		nthreads = pfc_get_online_cpus();
		if (nthreads <= (MODULE_MIN_LOADER_NTHREADS + 1)) {
			nthreads = MODULE_MIN_LOADER_NTHREADS;
		}
		else if (nthreads > MODULE_MAX_LOADER_NTHREADS) {
			nthreads = MODULE_MAX_LOADER_NTHREADS;
		}
		else {
			nthreads--;
		}
	}

	return nthreads;
}

/*
 * static void *
 * module_loader_main(void *arg)
 *	Start routine of module loader/unloader thread.
 */
static void *
module_loader_main(void *arg)
{
	mod_loader_t	*loader = (mod_loader_t *)arg;
	pfc_listm_t	queue = loader->ml_queue;
	pmodule_t	*mod;
	pmod_state_t	state = loader->ml_state;
	uint8_t	done;
	int	err;

	/* Wait for the loader context to be ready. */
	MOD_LOADER_LOCK(loader);
	while ((loader->ml_flags & MODLDF_START) == 0) {
		MOD_LOADER_WAIT(loader);
	}
	MOD_LOADER_UNLOCK(loader);

	while (1) {
		/* Pop one module at the head of queue. */
		err = pfc_llist_pop_wait(queue, (pfc_cptr_t *)&mod);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ESHUTDOWN);
			err = 0;
			break;
		}

		/* Change module state. */
		pfc_log_verbose("%s: module_set_state(%d): 0x%x",
				PMODULE_NAME(mod), state, mod->pm_flags);
		err = pfc_module_set_state(mod, state);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/* Succeeded. */
			continue;
		}

		if (err == EPROTO &&
		    loader->ml_state == PMOD_STATE_UNLOADED) {
			/*
			 * This means module's fini() returned PFC_FALSE.
			 * We can ignore this error because that module is
			 * treated as unloaded.
			 */
			pfc_log_verbose("%s: Ignore fini() error on "
					"unloading.", PMODULE_NAME(mod));
			continue;
		}

		if (err == ESHUTDOWN || err == ECANCELED) {
			/*
			 * Don't report error due to module system shutdown or
			 * cancellation.
			 */
			pfc_log_debug("%s: Module operation was canceled:"
				      " %d", PMODULE_NAME(mod), err);
			err = 0;
		}
		else {
			pfc_log_error("%s: Failed to change module state: %s",
				      PMODULE_NAME(mod), strerror(err));
		}
		break;
	}

	MOD_LOADER_LOCK(loader);

	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Install error number as result. */
		module_loader_seterror(loader, err);
	}

	done = loader->ml_done;
	done++;
	loader->ml_done = done;

	if (done == loader->ml_nthreads) {
		/* All loader threads have finished their work. */
		MOD_LOADER_SIGNAL(loader);
	}

	MOD_LOADER_UNLOCK(loader);

	return NULL;
}

/*
 * static int
 * module_busy_wait(pmodule_t *PFC_RESTRICT mod,
 *		    const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Wait for the busy flag in the specified module descriptor to be
 *	cleared.
 */
static int
module_busy_wait(pmodule_t *PFC_RESTRICT mod,
		 const pfc_timespec_t *PFC_RESTRICT abstime)
{
	int	err = 0, werr = 0;

	PMODULE_LOCK(mod);

	while (mod->pm_flags & PMODF_BUSY) {
		if (werr != 0) {
			err = werr;
			break;
		}

		mod->pm_nwaiting++;
		werr = PMODULE_TIMEDWAIT_ABS(mod, abstime);
		mod->pm_nwaiting--;
		if (PFC_EXPECT_TRUE(werr == 0)) {
			continue;
		}
		if (PFC_EXPECT_FALSE(werr == EINTR)) {
			werr = 0;
			continue;
		}

		/* One more check should be done. */
		PFC_ASSERT(werr == ETIMEDOUT);
	}

	PMODULE_UNLOCK(mod);

	return err;
}

/*
 * static void
 * module_loader_seterror(mod_loader_t *loader, int err)
 *	Set error number to the module loader context as result code.
 *
 * Remarks:
 *	This function must be called with holding the module loader context
 *	lock.
 */
static void
module_loader_seterror(mod_loader_t *loader, int err)
{
	if (loader->ml_result == 0) {
		loader->ml_result = err;
		MOD_LOADER_BROADCAST(loader);
	}
}

/*
 * static size_t
 * module_set_mandatory(int argc, char **argv, uint32_t flags)
 *	Select mandatory modules according to the configuration.
 *
 * Calling/Exit State:
 *	The number of selected mandatory modules is returned.
 */
static size_t
module_set_mandatory(int argc, char **argv, uint32_t flags)
{
	size_t	count = 0;
	int	err;

	if ((flags & PFCD_MODF_NOCONF) == 0) {
		int	index, nloads;

		/* Set mandatory flag to modules specified by pfcd.conf. */
		nloads = pfc_conf_array_size(conf_module, param_load_modules);
		for (index = 0; index < nloads; index++) {
			const char	*name;

			name = pfc_conf_array_stringat(conf_module,
						       param_load_modules,
						       index, NULL);
			if (PFC_EXPECT_FALSE(name == NULL)) {
				continue;
			}

			err = pfc_module_set_mandatory(name, PFC_TRUE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				PFC_ASSERT(err == ENOENT);
				fatal("%s: Unknown module name: %s",
				      param_load_modules, name);
				/* NOTREACHED */
			}
			count++;
		}
	}

	/*Set mandatory flag to modules specified by command line argument. */
	for (; argc > 0; argc--, argv++) {
		const char	*name = (const char *)*argv;

		err = pfc_module_set_mandatory(name, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ENOENT);
			fatal("Unknown module name: %s", name);
			/* NOTREACHED */
		}
		count++;
	}

	return count;
}
