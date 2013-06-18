/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_PWD_IMPL_H
#define	_PFC_LIBPFC_UTIL_PWD_IMPL_H

/*
 * Internal definitions for password and group file utilitiies.
 */

#include <pwd.h>
#include <pfc/base.h>

/*
 * Minimal user information.
 */
typedef struct {
	const char	*pu_name;	/* user name */
	uid_t		pu_uid;		/* user ID */
	gid_t		pu_gid;		/* group ID */
} pwd_user_t;

/*
 * Prototypes.
 */
extern int	pfc_pwd_getuser(pwd_user_t *usp, uid_t uid);
extern int	pfc_pwd_getuserbyname(pwd_user_t *PFC_RESTRICT usp,
				      const char *PFC_RESTRICT name);
extern int	pfc_pwd_initgroups(pwd_user_t *usp, gid_t gid);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_pwd_inituser(pwd_user_t *usp, uid_t uid)
 *	Initialize pwd_user_t buffer pointed by `usp' with the given user ID.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_pwd_inituser(pwd_user_t *usp, uid_t uid)
{
	usp->pu_name = NULL;
	usp->pu_uid = uid;
	usp->pu_gid = (gid_t)-1;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_pwd_destroyuser(pwd_user_t *usp)
 *	Free up resources held by the given user information.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_pwd_destroyuser(pwd_user_t *usp)
{
	void	*name = (void *)usp->pu_name;

	if (name != NULL) {
		free(name);
	}
}

#endif	/* !_PFC_LIBPFC_UTIL_PWD_IMPL_H */
