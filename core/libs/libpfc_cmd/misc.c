/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.c - Miscellaneous utilities.
 */

#include <string.h>
#include <pfc/path.h>
#include <pfc/util.h>
#include "cmdutil.h"

/*
 * Default daemon name.
 */
static const char	daemon_name_default[] = "pfcd";

/*
 * pfc_refptr_t *
 * progname_init(const char *PFC_RESTRICT arg0,
 *		 const char **PFC_RESTRICT namepp)
 *	Determine the daemon name associated with the given command.
 *
 *	`arg0' must be a value of argv[0] passed to main().
 *	A command name in `arg0' is set to the buffer pointed by `namepp'.
 *
 *	Command name should have a prefix joined with an underscore character.
 *	If `name' is "foo_bar", the daemon name will be "food".
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to pfc_refptr_t which
 *	contains a daemon name is returned. Note that it must be released
 *	by the caller.
 *
 *	Otherwise NULL is returned.
 */
pfc_refptr_t *
progname_init(const char *PFC_RESTRICT arg0, const char **PFC_RESTRICT namepp)
{
	pfc_refptr_t	*rpath;
	const char	*p, *name;
	char		*prefix;
	size_t		len;

	/* Determine command name. */
	name = strrchr(arg0, '/');
	if (name == NULL) {
		name = arg0;
	}
	else {
		name++;
	}

	if (PFC_EXPECT_TRUE(*name != '\0')) {
		*namepp = name;
	}

	/* Search for a prefix in the given command name. */
	p = strchr(name, '_');
	if (PFC_EXPECT_FALSE(p == NULL)) {
		/* Unexpected command name. Use default daemon name. */
		return pfc_refptr_string_create(daemon_name_default);
	}

	/* Determine command name prefix. */
	len = p - name;
	prefix = (char *)malloc(len + 1);
	if (PFC_EXPECT_FALSE(prefix == NULL)) {
		return NULL;
	}

	memcpy(prefix, name, len);
	*(prefix + len) = '\0';

	/* Construct a daemon name. */
	rpath = pfc_refptr_sprintf("%sd", prefix);
	free(prefix);

	return rpath;
}

/*
 * int
 * pfccmd_switchuser(pfc_cfblk_t options, pfc_bool_t daemon,
 *		     pfccmd_err_t errfunc)
 *	Change user and group ID of the calling process as per options block
 *	in pfcd.conf.
 *
 *	`options' must be a PFC configuration block handle associated with
 *	options block in pfcd.conf.
 *
 *	If `daemon' is PFC_TRUE, this function uses the configuration for the
 *	daemon. Otherwise the configuration for administrative command is
 *	used.
 *
 *	If `errfunc' is not NULL, it is called when pfccmd_switchuser() fails.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfccmd_switchuser(pfc_cfblk_t options, pfc_bool_t daemon, pfccmd_err_t errfunc)
{
	const char	*user, *group, *ukey, *gkey;
	int		err;

	/* Determine user and group name. */
	if (daemon) {
		/* Use configuration for the daemon. */
		ukey = "user";
		gkey = "group";
	}
	else {
		/* Use configuration for administrative command. */
		ukey = "admin_user";
		gkey = "admin_group";
	}

	user = pfc_conf_get_string(options, ukey, NULL);
	group = pfc_conf_get_string(options, gkey, NULL);

	/*
	 * Set user and group ID, and initialize the supplementary access
	 * group list.
	 */
	err = pfc_cred_setbyname(user, group, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0) && errfunc != NULL) {
		if (err == EPERM) {
			(*errfunc)("You are not allowed to use this program.");
			
		}
		else {
			(*errfunc)("Failed to switch user: %s,%s: %s",
				   user, group, strerror(err));
		}
	}

	return err;
}
