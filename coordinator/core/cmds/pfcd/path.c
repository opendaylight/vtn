/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * path.c - PFC system file path management.
 */

#include <string.h>
#include <pfc/extcmd.h>
#include <property_impl.h>
#include "pfcd.h"

/*
 * Path to PFC module directory.
 */
const char	*pfcd_module_dir;

/*
 * Path to PFC module cache directory.
 */
const char	*pfcd_modcache_dir;

/*
 * Path to PFC working directory.
 */
const char	*pfcd_work_dir;

/*
 * Length of PFC working directory path.
 */
size_t		pfcd_work_dir_len;

/*
 * Path to stderr logging directory.
 */
const char	*pfcd_stderr_logdir;

/*
 * Path to PID file.
 */
const char	*pfcd_pid_file_path;

/*
 * Name of parameters which determine PFC system file path.
 * All of them must belong to "options" block in the PFC system configuration
 * file.
 */
typedef struct {
	const char	*c_name;		/* parameter name */
	const char	*c_default;		/* default value */
	const char	**c_variable;		/* pointer to variable */
	uint32_t	c_flags;		/* flags */
} cfpath_t;

#define	PATHF_CREATE	PFC_CONST_U(0x1)	/* create in path_init() */

static const cfpath_t	pfcd_paths[] = {
	{"work_dir", PFC_PFCD_WORKDIR_PATH, &pfcd_work_dir, PATHF_CREATE},
	{"module_dir", PFC_MODULEDIR, &pfcd_module_dir, PATHF_CREATE},
	{"modcache_dir", PFC_MODULEDIR, &pfcd_modcache_dir, PATHF_CREATE},
	{"stderr_logdir", PFC_PFCD_ERRLOG_PATH, &pfcd_stderr_logdir, 0},
};

/*
 * Name of parameter which determines external command search path.
 */
static const char	param_cmd_path[] = "cmd_path";

/*
 * Command path list which is always prepended to the command search path.
 */
static const char	*cmd_prepend_path[] = {
	PFC_BINDIR,
#ifdef	PFC_EXTCMD_BINDIR
	PFC_EXTCMD_BINDIR,
#endif	/* PFC_EXTCMD_BINDIR */
};

/*
 * Internal prototypes.
 */
static void	path_extcmd_init(void);

/*
 * void
 * path_init(void)
 *	Initialize PFC system file path.
 */
void
path_init(void)
{
	const cfpath_t	*cfp;
	pfc_cfblk_t	options = pfcd_options;
	int		err;

	for (cfp = pfcd_paths; cfp < PFC_ARRAY_LIMIT(pfcd_paths); cfp++) {
		const char	*path;

		/* Derive path from configuration file. */
		path = pfc_conf_get_string(options, cfp->c_name,
					   cfp->c_default);

		if (cfp->c_flags & PATHF_CREATE) {
			/* Create directory if it does not exist. */
			err = pfc_mkdir(path, PFCD_DIR_PERM);
			if (PFC_EXPECT_FALSE(err != 0)) {
				fatal("%s: Unable to create directory: %s",
				      path, strerror(err));
				/* NOTREACHED */
			}

			/* Ensure that this file is safe. */
			path_verify(path);
		}

		*(cfp->c_variable) = path;
	}

	/* Derive PID file path from configuration file. */
	pfcd_pid_file_path = pidfile_default_path();

	/* Initialize command search path. */
	path_extcmd_init();

	/* Set working directory path to the system property. */
	err = pfc_prop_set(PFC_PROP_WORKDIR, pfcd_work_dir);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to set work_dir to the system property.: %s",
		      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * void
 * path_verify(const char *path)
 *	Ensure that the specified file path can be used safely.
 *
 * Calling/Exit State:
 *	Program exits on error.
 */
void
path_verify(const char *path)
{
	char	*cpath;
	int	err;

	/*
	 * We need to copy the specified path because pfc_is_safepath()
	 * always breaks the string.
	 */
	cpath = strdup(path);
	if (PFC_EXPECT_FALSE(cpath == NULL)) {
		pfc_log_fatal("Failed to duplicate file path.");
		/* NOTREACHED */
	}

	err = pfc_is_safepath(cpath);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			pfc_log_fatal("%s: File does not exist.", path);
		}
		else {
			pfc_log_fatal("%s: File is unsafe.: %s: %s",
				      path, cpath, strerror(err));
		}
		/* NOTREACHED */
	}
	free(cpath);
}

/*
 * static void
 * path_extcmd_init(void)
 *	Initialize external command search path.
 *
 *	PFC daemon uses pfcd.conf to determine command search path.
 *	PATH environment variable is ignored.
 */
static void
path_extcmd_init(void)
{
	pfc_cfblk_t	options = pfcd_options;
	uint32_t	index;
	int	size, npaths;
	char	**path, **pp;

	/*
	 * Determine size of cmd_path array.
	 * Use system default path if not defined.
	 */
	size = pfc_conf_array_size(options, param_cmd_path);
	if (size < 0) {
		path = NULL;
	}
	else {
		const char	**prp;

		/* Prepend paths in cmd_prepend_path, and append NULL. */
		npaths = size + PFC_ARRAY_CAPACITY(cmd_prepend_path) + 1;
		path = (char **)pfcd_malloc(sizeof(char *) * npaths);
		for (prp = cmd_prepend_path, pp = path;
		     prp < PFC_ARRAY_LIMIT(cmd_prepend_path); prp++, pp++) {
			*pp = (char *)*prp;
			verbose("extcmd_path[%u] = \"%s\"",
				(uint32_t)(pp - path), *prp);
		}

		for (index = 0; index < (uint32_t)size; index++) {
			const char	*p;
			char	*buf;
			int	err;

			p = pfc_conf_array_stringat(options, param_cmd_path,
						    index, NULL);
			if (PFC_EXPECT_FALSE(p == NULL)) {
				continue;
			}

			buf = strdup(p);
			if (PFC_EXPECT_FALSE(buf == NULL)) {
				fatal("Failed to copy command search path.");
				/* NOTREACHED */
			}

			err = pfc_is_safepath(buf);
			if (PFC_EXPECT_TRUE(err == 0)) {
				verbose("extcmd_path[%u] = \"%s\"",
					(uint32_t)(pp - path), p);
				*pp = (char *)p;
				pp++;
			}
			else if (err == ENOENT) {
				warning("cmd_path: \"%s\" is ignored: "
					"File does not exist", p);
			}
			else {
				warning("cmd_path: \"%s\" is ignored: "
					"%s is unsafe: %s", p, buf,
					strerror(err));
			}
			free(buf);
		}

		PFC_ASSERT(pp < path + npaths);
		*pp = NULL;
	}

	__pfc_extcmd_path_init((const char **)path);
}

/*
 * char *
 * path_create(const char *name, size_t namelen)
 *	Create file path related to PFC working directory.
 *
 *	`name' must be a pointer to file name, and `namelen' must be its
 *	length.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to file path is
 *	returned.
 *
 *	NULL is returned on failure.
 */
char *
path_create(const char *name, size_t namelen)
{
	size_t	len = pfcd_work_dir_len + namelen + 2;
	char	*path;

	path = (char *)malloc(len);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		return NULL;
	}

	PFC_ASSERT_INT(snprintf(path, len, "%s/%s", pfcd_work_dir, name),
		       (int)len - 1);

	return path;
}
