/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * pwd.c - Utilities to handle password and group file entry.
 */

#include <pfc/util.h>
#include <pfc/strtoint.h>
#include <pfc/debug.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include "pwd_impl.h"
#include "util_impl.h"

/*
 * Default buffer size to obtain password or group file entry.
 */
#define	PWD_BUFSIZE		PFC_CONST_U(64)

/*
 * Temporary buffer to access password or group file.
 */
typedef struct {
	char		*pb_buffer;		/* temporary buffer */
	size_t		pb_size;		/* size of buffer */
} pwd_buffer_t;

/*
 * Context to obtain password file entry.
 */
typedef struct {
	struct passwd		pp_passwd;	/* password entry */
	pwd_buffer_t		pp_buffer;	/* temporary buffer */
} pwd_passwd_t;

/*
 * Context to obtain group file entry.
 */
typedef struct {
	struct group		pg_group;	/* group entry */
	pwd_buffer_t		pg_buffer;	/* temporary buffer */
} pwd_group_t;

/*
 * Internal prototypes.
 */
static int	pwd_getpwuid(pwd_passwd_t *pctx, uid_t uid);
static int	pwd_getgrgid(pwd_group_t *gctx, gid_t gid);
static int	pwd_getpwnam(pwd_passwd_t *pctx, const char *name);
static int	pwd_getgrnam(pwd_group_t *gctx, const char *name);

/*
 * Define body of database accessor function.
 */
#define	PWD_ACCESSOR_DECL(pbp, enttype, entbuf, err, rfunc, key)	\
	do {								\
		pwd_buffer_init(pbp);					\
									\
		for (;;) {						\
			enttype	*__entp;				\
									\
			(err) = pwd_buffer_alloc(pbp);			\
			if (PFC_EXPECT_FALSE((err) != 0)) {		\
				break;					\
			}						\
			(err) = rfunc((key), (entbuf), (pbp)->pb_buffer, \
				      (pbp)->pb_size, &__entp);		\
			if (PFC_EXPECT_TRUE((err) == 0)) {		\
				if (__entp == NULL) {			\
					/* No entry was found. */	\
					(err) = -1;			\
				}					\
				else {					\
					PFC_ASSERT(__entp == (entbuf));	\
				}					\
				break;					\
			}						\
									\
			if (PFC_EXPECT_FALSE((err) != ERANGE)) {	\
				/* Fatal error. */			\
				break;					\
			}						\
									\
			/* Expand the buffer, and try again. */		\
			pwd_buffer_expand(pbp);				\
		}							\
	} while (0)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pwd_buffer_init(pwd_buffer_t *pbp)
 *	Initialize the given temporary buffer.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pwd_buffer_init(pwd_buffer_t *pbp)
{
	pbp->pb_buffer = NULL;
	pbp->pb_size = PWD_BUFSIZE;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pwd_buffer_alloc(pwd_buffer_t *pbp)
 *	Allocate temporary buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to the buffer is stored
 *	to pbp->pb_buffer, and zero is returned.
 *
 *	On failure, NULL is stored to pbp->pb_buffer, and ENOMEM is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pwd_buffer_alloc(pwd_buffer_t *pbp)
{
	pbp->pb_buffer = (char *)malloc(pbp->pb_size);
	if (PFC_EXPECT_FALSE(pbp->pb_buffer == NULL)) {
		return ENOMEM;
	}

	return 0;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pwd_buffer_free(pwd_buffer_t *pbp)
 *	Free temporary buffer.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pwd_buffer_free(pwd_buffer_t *pbp)
{
	free(pbp->pb_buffer);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pwd_buffer_expand(pwd_buffer_t *pbp)
 *	Expand the size of the temporary buffer for succeeding call of
 *	pwd_buffer_alloc().
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pwd_buffer_expand(pwd_buffer_t *pbp)
{
	pwd_buffer_free(pbp);
	pbp->pb_size += PWD_BUFSIZE;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pwd_grouplist_contains(gid_t *glist, int ngroups, gid_t gid)
 *	Return PFC_TRUE only if the group list specified by `glist' contains
 *	the group ID `gid'.
 *
 *	`ngroups' must be the number of groups in the group list.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pwd_grouplist_contains(gid_t *glist, int ngroups, gid_t gid)
{
	gid_t	*gp;

	for (gp = glist; ngroups > 0; gp++, ngroups--) {
		if (*gp == gid) {
			return PFC_TRUE;
		}
	}

	return PFC_FALSE;
}

/*
 * int
 * pfc_pwd_ismember(uid_t uid, gid_t gid, pfc_bool_t supp)
 *	Determine whether the user specified by user ID `uid' is a member of
 *	the group specified by group ID `gid'.
 *
 *	If PFC_TRUE is specified to `supp', this function returns zero if the
 *	group specified by `gid' is one of supplementary groups for the
 *	specified user. If PFC_FALSE is specified, this function does not
 *	check supplementary groups.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified user is a member of the specified
 *	group.
 *
 *	-1 is returned if the specified user is not a member.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_pwd_ismember(uid_t uid, gid_t gid, pfc_bool_t supp)
{
	pwd_passwd_t	pctx;
	pwd_group_t	gctx;
	struct passwd	*pwd;
	struct group	*grp;
	char		*uname, *memp, **mempp;
	int		err;

	/* Get password entry for the given user ID. */
	err = pwd_getpwuid(&pctx, uid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	pwd = &pctx.pp_passwd;
	if (pwd->pw_gid == gid) {
		/* The given user is a member of the given group. */
		PFC_ASSERT(err == 0);
		goto out;
	}

	if (!supp) {
		/* Don't check supplementary groups. */
		err = -1;
		goto out;
	}

	uname = pwd->pw_name;
	if (PFC_EXPECT_FALSE(uname == NULL)) {
		/* This should never happen. */
		err = EFAULT;
		goto out;
	}

	/* Get group entry for the given group ID. */
	err = pwd_getgrgid(&gctx, gid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out_group;
	}

	grp = &gctx.pg_group;
	mempp = grp->gr_mem;
	err = -1;
	if (PFC_EXPECT_FALSE(mempp == NULL)) {
		goto out_group;
	}

	for (memp = *mempp; memp != NULL; mempp++, memp = *mempp) {
		if (strcmp(memp, uname) == 0) {
			/* Found. */
			err = 0;
			break;
		}
	}

out_group:
	pwd_buffer_free(&gctx.pg_buffer);

out:
	pwd_buffer_free(&pctx.pp_buffer);

	return err;
}

/*
 * int
 * pfc_pwd_strtouid(const char *name, uid_t *uidp)
 *	Convert string to user ID.
 *
 *	If the string specified by `name' is a string representation of
 *	integer, and it is not a valid user name, it will be converted to
 *	uid_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted user ID is stored to `*uidp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_pwd_strtouid(const char *name, uid_t *uidp)
{
	pwd_passwd_t	pctx;
	uint32_t	value;
	int		err;

	PFC_ASSERT(sizeof(uint32_t) == sizeof(uid_t));

	if (PFC_EXPECT_FALSE(name == NULL || *name == '\0' || uidp == NULL)) {
		return EINVAL;
	}

	/* Look for a name in password database file. */
	err = pwd_getpwnam(&pctx, name);
	if (PFC_EXPECT_TRUE(err == 0)) {
		*uidp = pctx.pp_passwd.pw_uid;
		goto out;
	}
	if (PFC_EXPECT_FALSE(err > 0)) {
		goto out;
	}

	/* Try to convert the given string into uid_t. */
	err = pfc_strtou32(name, &value);
	if (PFC_EXPECT_TRUE(err == 0 && (uid_t)value != (uid_t)-1)) {
		*uidp = value;
	}
	else {
		err = ENOENT;
	}

out:
	pwd_buffer_free(&pctx.pp_buffer);

	return err;
}

/*
 * int
 * pfc_pwd_strtogid(const char *name, gid_t *gidp)
 *	Convert string to group ID.
 *
 *	If the string specified by `name' is a string representation of
 *	integer, and it is not a valid group name, it will be converted to
 *	gid_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted group ID is stored to `*gidp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_pwd_strtogid(const char *name, gid_t *gidp)
{
	pwd_group_t	gctx;
	uint32_t	value;
	int		err;

	PFC_ASSERT(sizeof(uint32_t) == sizeof(gid_t));

	if (PFC_EXPECT_FALSE(name == NULL || *name == '\0' || gidp == NULL)) {
		return EINVAL;
	}

	/* Look for a name in group database file. */
	err = pwd_getgrnam(&gctx, name);
	if (PFC_EXPECT_TRUE(err == 0)) {
		*gidp = gctx.pg_group.gr_gid;
		goto out;
	}
	if (PFC_EXPECT_FALSE(err > 0)) {
		goto out;
	}

	/* Try to convert the given string into gid_t. */
	err = pfc_strtou32(name, &value);
	if (PFC_EXPECT_TRUE(err == 0 && (gid_t)value != (gid_t)-1)) {
		*gidp = value;
	}
	else {
		err = ENOENT;
	}

out:
	pwd_buffer_free(&gctx.pg_buffer);

	return err;
}

/*
 * int
 * pfc_pwd_uidtoname(uid_t uid, char *PFC_RESTRICT name,
 *		     size_t *PFC_RESTRICT namelenp)
 *	Search for a user name associated with the user ID specified by `uid'.
 *
 *	`namelenp' must be initialized to the number of available bytes in
 *	the buffer pointed by `name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, user name associated with `uid' is set
 *	to the buffer pointed by `name', and zero is returned.
 *	In this case, the length of user name (strlen(name)) is set to the
 *	buffer pointed by `namelenp'.
 *
 *	ENOSPC is returned if the length of user name exceeds the size
 *	specified by `namelenp'. In this case, required size of the destination
 *	buffer (strlen(name) + 1) is stored to the buffer pointed by
 *	`namelenp'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_pwd_uidtoname(uid_t uid, char *PFC_RESTRICT name,
		  size_t *PFC_RESTRICT namelenp)
{
	pwd_passwd_t	pctx;
	struct passwd	*pwd;
	size_t		sz, len;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL || namelenp == NULL)) {
		return EINVAL;
	}

	err = pwd_getpwuid(&pctx, uid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err < 0) {
			PFC_ASSERT(err == -1);
			err = ENOENT;
		}
		goto out;
	}

	pwd = &pctx.pp_passwd;
	sz = *namelenp;
	len = pfc_strlcpy(name, pwd->pw_name, sz);
	if (PFC_EXPECT_FALSE(len >= sz)) {
		err = ENOSPC;
		len++;
	}

	*namelenp = len;

out:
	pwd_buffer_free(&pctx.pp_buffer);

	return err;
}

/*
 * int
 * pfc_pwd_gidtoname(uid_t gid, char *PFC_RESTRICT name,
 *		     size_t *PFC_RESTRICT namelenp)
 *	Search for a group name associated with the group ID specified by
 *	`gid'.
 *
 *	`namelenp' must be initialized to the number of available bytes in
 *	the buffer pointed by `name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to group name is stored to the
 *	buffer pointed by `namep', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Upon successful completion, group name associated with `gid' is set
 *	to the buffer pointed by `name', and zero is returned.
 *	In this case, the length of group name (strlen(name) is set to the
 *	buffer pointed by `namelenp'.
 *
 *	ENOSPC is returned if the length of group name exceeds the size
 *	specified by `namelenp'. In this case, required size of the destination
 *	buffer (strlen(name) + 1) is stored to the buffer pointed by
 *	`namelenp'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_pwd_gidtoname(uid_t gid, char *PFC_RESTRICT name,
		  size_t *PFC_RESTRICT namelenp)
{
	pwd_group_t	gctx;
	struct group	*grp;
	size_t		sz, len;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL || namelenp == NULL)) {
		return EINVAL;
	}

	err = pwd_getgrgid(&gctx, gid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err < 0) {
			PFC_ASSERT(err == -1);
			err = ENOENT;
		}
		goto out;
	}

	grp = &gctx.pg_group;
	sz = *namelenp;
	len = pfc_strlcpy(name, grp->gr_name, sz);
	if (PFC_EXPECT_FALSE(len >= sz)) {
		err = ENOSPC;
		len++;
	}

	*namelenp = len;

out:
	pwd_buffer_free(&gctx.pg_buffer);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_pwd_getuser(pwd_user_t *usp, uid_t uid)
 *	Get user information specified by the user ID `uid'.
 *
 * Calling/Exit State:
 *	Upon successful completion, user information associated with `uid' is
 *	set to the buffer pointed by `usp', and zero is returned.
 *	Note that this function returns zero if the given user does not exist
 *	in the password database. In that case, NULL and -1 is set to
 *	pu_name and pu_gid field respectively.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remaks:
 *	If this function returns zero, the caller must call
 *	pfc_pwd_destroyuser() to destroy user information.
 */
int PFC_ATTR_HIDDEN
pfc_pwd_getuser(pwd_user_t *usp, uid_t uid)
{
	pwd_passwd_t	pctx;
	struct passwd	*pwd;
	int		err;

	err = pwd_getpwuid(&pctx, uid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (PFC_EXPECT_TRUE(err < 0)) {
			/* The given user ID was not found. */
			PFC_ASSERT(err == -1);
			pfc_pwd_inituser(usp, uid);
			err = 0;
		}
		goto out;
	}

	pwd = &pctx.pp_passwd;
	usp->pu_name = strdup(pwd->pw_name);
	if (PFC_EXPECT_FALSE(usp->pu_name == NULL)) {
		err = ENOMEM;
	}
	else {
		usp->pu_uid = uid;
		usp->pu_gid = pwd->pw_gid;
	}

out:
	pwd_buffer_free(&pctx.pp_buffer);

	return err;
}

/*
 * int
 * pfc_pwd_getuserbyname(pwd_user_t *PFC_RESTRICT usp,
 *			 const char *PFC_RESTRICT name)
 *	Get user information specified by the user name `name'.
 *
 *	If `name' is a string representation of integer, and it is no a valid
 *	user name, it will be converted to uid_t and try to find user
 *	user information by calling pfc_pwd_getuser().
 *
 * Calling/Exit State:
 *	Upon successful completion, user information associated with `name'
 *	is set to the buffer pointed by `usp' and zero is returned.
 *	Note that this function returns zero if the given name is a string
 *	representation and it does not exist in the password database.
 *	In that case, NULL and -1 is set to pu_name and pu_gid field
 *	respectively.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remaks:
 *	If this function returns zero, the caller must call
 *	pfc_pwd_destroyuser() to destroy user information.
 */
int
pfc_pwd_getuserbyname(pwd_user_t *PFC_RESTRICT usp,
		      const char *PFC_RESTRICT name)
{
	pwd_passwd_t	pctx;
	int		err;

	PFC_ASSERT(name != NULL);

	if (PFC_EXPECT_FALSE(*name == '\0')) {
		return EINVAL;
	}

	err = pwd_getpwnam(&pctx, name);
	if (PFC_EXPECT_TRUE(err == 0)) {
		struct passwd	*pwd = &pctx.pp_passwd;

		usp->pu_name = strdup(pwd->pw_name);
		if (PFC_EXPECT_FALSE(usp->pu_name == NULL)) {
			err = ENOMEM;
		}
		else {
			usp->pu_uid = pwd->pw_uid;
			usp->pu_gid = pwd->pw_gid;
		}
	}
	else {
		uint32_t	value;

		/* Try to convert the given string into uid_t. */
		err = pfc_strtou32(name, &value);
		if (PFC_EXPECT_TRUE(err == 0 && value != (uint32_t)-1)) {
			/* Look for a user information by user ID. */
			pwd_buffer_free(&pctx.pp_buffer);

			return pfc_pwd_getuser(usp, (uid_t)value);
		}

		err = ENOENT;
	}

	pwd_buffer_free(&pctx.pp_buffer);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_pwd_initgroups(pwd_user_t *usp, gid_t gid)
 *	Initialize supplementary access group list.
 *
 *	`usp' must be a pointer to pwd_user_t which keeps user information.
 *	If usp->pu_name is not NULL, this function adds all groups to which
 *	the user specified by usp->pu_name belongs.
 *
 *	If `gid' is not -1, `gid' is also added to the access list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- Note that this function requires appropriate privilege.
 *
 *	- This function must be called while single-threaded.
 */
int PFC_ATTR_HIDDEN
pfc_pwd_initgroups(pwd_user_t *usp, gid_t gid)
{
	const char	*name = usp->pu_name;
	gid_t		*glist;
	int		err, ngroups;

	if (name == NULL) {
		/*
		 * The given group is the only group to be added to the
		 * access list.
		 */
		ngroups = (gid == (gid_t)-1) ? 0 : 1;
		if (PFC_EXPECT_FALSE(setgroups(ngroups, &gid) != 0)) {
			err = errno;
			goto error;
		}

		return 0;
	}

	/* Initialize the access group list. */
	if (PFC_EXPECT_FALSE(initgroups(name, usp->pu_gid) != 0)) {
		err = errno;
		goto error;
	}

	if (gid == (gid_t)-1) {
		/* No need to append more group. */
		return 0;
	}

	/* Get current access group list. */
	ngroups = getgroups(0, NULL);
	if (PFC_EXPECT_FALSE(ngroups < 0)) {
		err = errno;
		goto error;
	}

	glist = (gid_t *)malloc(sizeof(*glist) * (ngroups + 1));
	if (PFC_EXPECT_FALSE(glist == NULL)) {
		err = ENOMEM;
		goto error;
	}

	if (PFC_EXPECT_FALSE(getgroups(ngroups, glist) < 0)) {
		err = errno;
		goto error_glist;
	}

	if (!pwd_grouplist_contains(glist, ngroups, gid) &&
	    (uint32_t)ngroups < pfc_ngroups_max) {
		/* Add the given group to the access list. */
		*(glist + ngroups) = gid;
		if (PFC_EXPECT_FALSE(setgroups(ngroups + 1, glist) != 0)) {
			err = errno;
			goto error_glist;
		}
	}

	free(glist);

	return 0;

error_glist:
	free(glist);

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		/* This should never happen. */
		err = EIO;
	}

	return err;
}

/*
 * static int
 * pwd_getpwuid(pwd_passwd_t *pctx, uid_t uid)
 *	Get password file entry associated with the user specified by the user
 *	ID `uid'.
 *
 * Calling/Exit State:
 *	Upon successful completion, password file entry is stored to
 *	pctx->pp_passwd, and zero is returned.
 *
 *	-1 is returned if no password file entry was found.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function allocates temporary buffer, and retains it on return.
 *	So the caller must release the buffer in pctx->pp_buffer by calling
 *	pwd_buffer_free(), irrespective of result.
 */
static int
pwd_getpwuid(pwd_passwd_t *pctx, uid_t uid)
{
	struct passwd	*pwbuf = &pctx->pp_passwd;
	pwd_buffer_t	*pbp = &pctx->pp_buffer;
	int		err;

	PWD_ACCESSOR_DECL(pbp, struct passwd, pwbuf, err, getpwuid_r, uid);

	return err;
}

/*
 * static int
 * pwd_getgrgid(pwd_group_t *gctx, gid_t gid)
 *	Get group file entry associated with the group specified by the group
 *	ID `gid'.
 *
 * Calling/Exit State:
 *	Upon successful completion, group file entry is stored to
 *	gctx->pg_group, and zero is returned.
 *
 *	-1 is returned if no group file entry was found.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function allocates temporary buffer, and retains it on return.
 *	So the caller must release the buffer in gctx->pg_buffer by calling
 *	pwd_buffer_free(), irrespective of result.
 */
static int
pwd_getgrgid(pwd_group_t *gctx, gid_t gid)
{
	struct group	*grbuf = &gctx->pg_group;
	pwd_buffer_t	*pbp = &gctx->pg_buffer;
	int		err;

	PWD_ACCESSOR_DECL(pbp, struct group, grbuf, err, getgrgid_r, gid);

	return err;
}

/*
 * static int
 * pwd_getpwnam(pwd_group_t *gctx, const char *name)
 *	Get password file entry associated with the user name specified by
 *	`name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, password file entry is stored to
 *	pctx->pp_passwd, and zero is returned.
 *
 *	-1 is returned if no password file entry was found.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function allocates temporary buffer, and retains it on return.
 *	So the caller must release the buffer in gctx->pp_buffer by calling
 *	pwd_buffer_free(), irrespective of result.
 */
static int
pwd_getpwnam(pwd_passwd_t *pctx, const char *name)
{
	struct passwd	*pwbuf = &pctx->pp_passwd;
	pwd_buffer_t	*pbp = &pctx->pp_buffer;
	int		err;

	PWD_ACCESSOR_DECL(pbp, struct passwd, pwbuf, err, getpwnam_r, name);

	return err;
}

/*
 * static int
 * pwd_getgrnam(pwd_group_t *gctx, const char *name)
 *	Get group file entry associated with the group name specified by
 *	`name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, group file entry is stored to
 *	gctx->pg_group, and zero is returned.
 *
 *	-1 is returned if no group file entry was found.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function allocates temporary buffer, and retains it on return.
 *	So the caller must release the buffer in gctx->pg_buffer by calling
 *	pwd_buffer_free(), irrespective of result.
 */
static int
pwd_getgrnam(pwd_group_t *gctx, const char *name)
{
	struct group	*grbuf = &gctx->pg_group;
	pwd_buffer_t	*pbp = &gctx->pg_buffer;
	int		err;

	PWD_ACCESSOR_DECL(pbp, struct group, grbuf, err, getgrnam_r, name);

	return err;
}
