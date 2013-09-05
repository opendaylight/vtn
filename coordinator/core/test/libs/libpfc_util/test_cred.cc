/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_cred.cc - Test for credentials handling.
 *
 * Remarks:
 *	- This test may installs the system configuration file (pfcd.conf)
 *	  to PFC_PFCD_CONF_PATH.
 *
 *	- Some tests requires root privilege.
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <sstream>
#include <boost/bind.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <pfc/util.h>
#include <pfc/path.h>
#include <pfc/conf.h>
#include <pfc/strtoint.h>
#include <conf_impl.h>
#include "test.h"
#include "child.hh"
#include "misc.hh"
#include "tmpfile.hh"
#include "random.hh"

#ifdef	PFC_HAVE_PRCTL_DUMPABLE
#include <sys/prctl.h>
#include <fcntl.h>

/*
 * Verify suid dumpable flag for the calling process.
 */
#define ASSERT_DUMPABLE(expected)                                       \
    {                                                                   \
        int  __value(prctl(PR_GET_DUMPABLE));                           \
                                                                        \
        ASSERT_NE(-1, __value) << "*** ERROR: " << strerror(errno);     \
        ASSERT_EQ(expected, __value);                                   \
    }

#endif	/* PFC_HAVE_PRCTL_DUMPABLE */

/*
 * This test uses getres[ug]id() and setres[ug]id().
 */
#if     defined(PFC_HAVE_GETRESUID) && defined(PFC_HAVE_GETRESGID) &&   \
        defined(PFC_HAVE_SETRESUID) && defined(PFC_HAVE_SETRESGID)
#define	DO_CRED_TEST    1
#endif	/* defined(PFC_HAVE_GETRESUID) && defined(PFC_HAVE_GETRESGID) &&
           defined(PFC_HAVE_SETRESUID) && defined(PFC_HAVE_SETRESGID) */

/*
 * The number of credential IDs.
 * Index 0 is for real, 1 is for effective, and 2 is saved ID.
 */
#define NUM_CREDID      3

/*
 * Undefined value for user and group ID.
 */
#define UID_UNDEF       static_cast<uid_t>(-1)
#define GID_UNDEF       static_cast<gid_t>(-1)

/*
 * Get all user IDs.
 */
#define	GETRESUID(uids)                                         \
    ASSERT_EQ(0, getresuid((uids), (uids) + 1, (uids) + 2))

/*
 * Get all group IDs.
 */
#define	GETRESGID(gids)                                         \
    ASSERT_EQ(0, getresgid((gids), (gids) + 1, (gids) + 2))

/*
 * Verify user ID and group ID.
 */
#define CHECK_RESUGID(uids, gids)                                       \
    do {                                                                \
        uid_t  __uids[NUM_CREDID];                                      \
        gid_t  __gids[NUM_CREDID];                                      \
                                                                        \
        GETRESUID(__uids);                                              \
        GETRESGID(__gids);                                              \
                                                                        \
        for (uint32_t __i(0); __i < PFC_ARRAY_CAPACITY(__uids); __i++) { \
            ASSERT_EQ(*((uids) + __i), __uids[__i]);                    \
        }                                                               \
        for (uint32_t __i(0); __i < PFC_ARRAY_CAPACITY(__gids); __i++) { \
            ASSERT_EQ(*((gids) + __i), __gids[__i]);                    \
        }                                                               \
    } while (0)

#define CHECK_UGID(uid, gid)                    \
    do {                                        \
        uid_t  __u[] = {(uid), (uid), (uid)};   \
        gid_t  __g[] = {(gid), (gid), (gid)};   \
                                                \
        CHECK_RESUGID(__u, __g);                \
    } while (0)

/*
 * User/Group maps.
 */
struct pwdmap {
    usermap_t   pm_users;      // UID to user name conversion
    uidmap_t    pm_uids;       // User name to UID conversion
    groupmap_t  pm_groups;     // GID to group name conversion
    gidmap_t    pm_gids;       // Group name to GID conversion
    const char  *pm_uname;     // User name associated with real UID
    const char  *pm_gname;     // Group name associated with real GID
    const char  *pm_euname;    // User name associated with effective UID
    const char  *pm_egname;    // Group name associated with effective GID

    pwdmap();

    const char *getUser(uid_t uid);
    const char *getGroup(gid_t gid);
    uid_t      getUID(const char *user);
    uid_t      getGID(const char *group);
};

/*
 * Initialize user and group map.
 */
pwdmap::pwdmap()
{
    setpwent();
    struct passwd  *pwd;
    while ((pwd = getpwent()) != NULL) {
        pm_users.insert(usermap_t::value_type(pwd->pw_uid, pwd->pw_name));
        pm_uids.insert(uidmap_t::value_type(pwd->pw_name, pwd->pw_uid));
    }
    endpwent();

    setgrent();
    struct group  *grp;
    while ((grp = getgrent()) != NULL) {
        pm_groups.insert(groupmap_t::value_type(grp->gr_gid, grp->gr_name));
        pm_gids.insert(gidmap_t::value_type(grp->gr_name, grp->gr_gid));
    }
    endgrent();

    pm_uname = getUser(getuid());
    pm_gname = getGroup(getgid());
    pm_euname = getUser(geteuid());
    pm_egname = getGroup(getegid());
}

/*
 * Convert user ID into user name.
 */
const char *
pwdmap::getUser(uid_t uid)
{
    usermap_t::iterator it(pm_users.find(uid));

    return (it == pm_users.end()) ? NULL : it->second.c_str();
}

/*
 * Convert group ID into group name.
 */
const char *
pwdmap::getGroup(gid_t gid)
{
    groupmap_t::iterator it(pm_groups.find(gid));

    return (it == pm_groups.end()) ? NULL : it->second.c_str();
}

/*
 * Convert user name into user ID.
 */
uid_t
pwdmap::getUID(const char *user)
{
    uidmap_t::iterator it(pm_uids.find(user));

    return (it == pm_uids.end()) ? UID_UNDEF : it->second;
}

/*
 * Convert group name into group ID.
 */
gid_t
pwdmap::getGID(const char *group)
{
    gidmap_t::iterator it(pm_gids.find(group));

    return (it == pm_gids.end()) ? GID_UNDEF : it->second;
}

#ifdef  DO_CRED_TEST

/*
 * Load current supplementary access group list.
 */
static void
load_group_list(gidset_t &set)
{
    int  ngroups(getgroups(0, NULL));

    ASSERT_GE(ngroups, 0) << "errno=" << errno;

    void *buf(malloc(ngroups * sizeof(gid_t)));
    ASSERT_TRUE(buf != NULL);
    MallocRef mref(buf);
    gid_t *groups(reinterpret_cast<gid_t *>(buf));

    int ng(getgroups(ngroups, groups));
    ASSERT_EQ(ngroups, ng) << "errno=" << errno;

    for (int i(0); i < ng; i++) {
        set.insert(*(groups + i));
    }
}

#endif  /* DO_CRED_TEST */

/*
 * Load supplementary access group list for the user specified by password
 * entry.
 */
static void
load_group_list(gidset_t &set, struct passwd &pwd)
{
    set.insert(pwd.pw_gid);

    setgrent();
    struct group  *grp;
    while ((grp = getgrent()) != NULL) {
        char  **mempp(grp->gr_mem);

	for (char *memp(*mempp); memp != NULL; mempp++, memp = *mempp) {
            if (strcmp(pwd.pw_name, memp) == 0) {
                set.insert(grp->gr_gid);
            }
        }
    }
    endgrent();
}

/*
 * Load supplementary access group list for the user specified by `uid'.
 */
static void
load_group_list(gidset_t &set, uid_t uid)
{
    errno = 0;
    struct passwd *pwd(getpwuid(uid));
    ASSERT_EQ(0, errno);
    if (pwd != NULL) {
        load_group_list(set, *pwd);
    }
}

/*
 * Ensure that pfc_cred_set() can change user and group ID.
 */
static void
check_set(ChildContext *ctx, uid_t uid, gid_t gid)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // Change user and group ID.
    ASSERT_EQ(0, pfc_cred_set(uid, gid, PFC_FALSE));

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);

    // Ensure that user and group ID have been changed.
    if (uid != UID_UNDEF) {
        for (uid_t *u(uids); u < PFC_ARRAY_LIMIT(uids); u++) {
            *u = uid;
        }
    }
    if (gid != GID_UNDEF) {
        for (gid_t *u(gids); u < PFC_ARRAY_LIMIT(gids); u++) {
            *u = gid;
        }
    }

    CHECK_RESUGID(uids, gids);

    if (uids[1] != 0) {
        // Ensure that we can not get root privilege any more.
        ASSERT_EQ(EPERM, pfc_cred_set(0, GID_UNDEF, PFC_FALSE));
        CHECK_RESUGID(uids, gids);
        ASSERT_EQ(EPERM, pfc_cred_set(0, 0, PFC_FALSE));
        CHECK_RESUGID(uids, gids);

        if (gids[0] != 0 && gids[1] != 0 && gids[2] != 0) {
            ASSERT_EQ(EPERM, pfc_cred_set(UID_UNDEF, 0, PFC_FALSE));
            CHECK_RESUGID(uids, gids);
        }
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_set() can change user and group ID and group list.
 */
static void
check_set_initgrp(ChildContext *ctx, uid_t uid, gid_t gid)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // Change user and group ID and group list.
    ASSERT_EQ(0, pfc_cred_set(uid, gid, PFC_TRUE))
        << "uid=" << uid << " gid=" << gid;

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);

    // Ensure that user and group ID have been changed.
    uid_t curuids[NUM_CREDID];
    gid_t curgids[NUM_CREDID];
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(uids); i++) {
        curuids[i] = (uid == UID_UNDEF) ? uids[i] : uid;
    }

    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(gids); i++) {
        curgids[i] = (gid == GID_UNDEF) ? gids[i] : gid;
    }

    CHECK_RESUGID(curuids, curgids);

    if (curuids[1] != 0) {
        // Ensure that we can not get root privilege any more.
        ASSERT_EQ(EPERM, pfc_cred_set(0, GID_UNDEF, PFC_FALSE));
        CHECK_RESUGID(curuids, curgids);
        ASSERT_EQ(EPERM, pfc_cred_set(0, 0, PFC_FALSE));
        CHECK_RESUGID(curuids, curgids);

        if (curgids[0] != 0 && curgids[1] != 0 && curgids[2] != 0) {
            ASSERT_EQ(EPERM, pfc_cred_set(UID_UNDEF, 0, PFC_FALSE));
            CHECK_RESUGID(curuids, curgids);
        }
    }

    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();

    if (uid != UID_UNDEF && uids[0] != uid) {
        // Ensure that group list has been initialized.
        groups.clear();
        load_group_list(groups, uid);
        RETURN_ON_ERROR();
        if (gid != GID_UNDEF) {
            groups.insert(gid);
        }

        ASSERT_GIDSET_EQ(groups, curgroups);
    }
    else {
        // Ensure that group list is not changed.
        ASSERT_GIDSET_EQ(groups, curgroups);
    }
#endif  /* DO_CRED_TEST */
}

/*
 * pfc_cred_set() test for unprivileged user.
 */
static void
check_set_unpriv(ChildContext *ctx)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // This should do nothing.
    ASSERT_EQ(0, pfc_cred_set(UID_UNDEF, GID_UNDEF, PFC_FALSE));
    CHECK_RESUGID(uids, gids);

    ASSERT_DUMPABLE(1);

    // Unprivileged process can switch user and group ID to effective user and
    // group ID respectively.
    gid_t egid(gids[1]);
    ASSERT_EQ(0, pfc_cred_set(UID_UNDEF, egid, PFC_FALSE));
    gid_t g[] = {egid, egid, egid};
    CHECK_RESUGID(uids, g);
    ASSERT_DUMPABLE(1);

    uid_t euid(uids[1]);
    ASSERT_EQ(0, pfc_cred_set(euid, GID_UNDEF, PFC_FALSE));
    CHECK_UGID(euid, egid);
    ASSERT_DUMPABLE(1);
    ASSERT_EQ(0, pfc_cred_set(uids[1], gids[1], PFC_FALSE));
    CHECK_UGID(euid, egid);
    ASSERT_DUMPABLE(1);

    // Ensure that unprivileged process can not change user and group ID to
    // others.
    for (int i(1); i <= 100; i++) {
        ASSERT_EQ(EPERM, pfc_cred_set(UID_UNDEF, egid + i, PFC_FALSE));
        CHECK_UGID(euid, egid);
        ASSERT_EQ(EPERM, pfc_cred_set(euid + i, GID_UNDEF, PFC_FALSE));
        CHECK_UGID(euid, egid);
        ASSERT_EQ(EPERM, pfc_cred_set(euid + i, egid + i, PFC_FALSE));
        CHECK_UGID(euid, egid);
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);
#endif  /* DO_CRED_TEST */
}

/*
 * pfc_cred_set() test for unprivileged user.
 * This test checks whether unprivileged user can't initialize supplementary
 * groups.
 */
static void
check_set_initgrp_unpriv(ChildContext *ctx)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // This should do nothing.
    ASSERT_EQ(0, pfc_cred_set(UID_UNDEF, GID_UNDEF, PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Group list is not changed if the real user ID is not changed.
    uid_t uid(uids[0]);
    ASSERT_EQ(0, pfc_cred_set(uid, GID_UNDEF, PFC_TRUE));
    uid_t u[] = {uid, uid, uid};
    CHECK_RESUGID(u, gids);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Ensure that unprivileged user can not change the group list.
    ASSERT_EQ(EPERM, pfc_cred_set(uid + 1, GID_UNDEF, PFC_TRUE));
    CHECK_RESUGID(u, gids);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_set() can set user and group ID to real,
 * effective, and saved user and group ID.
 * This test requires root privilege.
 */
static void
check_setres(ChildContext *ctx, uid_t uids[NUM_CREDID], gid_t gids[NUM_CREDID])
{
#ifdef  DO_CRED_TEST
    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // Change user and group ID to the specified IDs.
    // This will drop SETUID capability.
    ASSERT_EQ(0, setresgid(gids[0], gids[1], gids[2]));
    ASSERT_EQ(0, setresuid(uids[0], uids[1], uids[2]));
    CHECK_RESUGID(uids, gids);

    // Ensure that user and group ID can be set to real, effective, and
    // saved ID.
    for (uint32_t i(0); i < NUM_CREDID; i++) {
        for (uint32_t j(0); j < NUM_CREDID; j++) {
            uid_t  uid(uids[i]);
            gid_t  gid(gids[j]);

            {
                ChildContext child;
                child_func_t func(boost::bind(check_set, _1, uid, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_set, _1, UID_UNDEF, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_set, _1, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
        }
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that unprivileged user can not change real user ID and
 * supplementary group access list.
 * This test requires root privilege.
 */
static void
check_setres_initgrp(ChildContext *ctx, uid_t uids[NUM_CREDID],
                     gid_t gids[NUM_CREDID])
{
#ifdef  DO_CRED_TEST
    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // Change user and group ID to the specified IDs.
    // This will drop SETUID capability.
    ASSERT_EQ(0, setresgid(gids[0], gids[1], gids[2]));
    ASSERT_EQ(0, setresuid(uids[0], uids[1], uids[2]));
    CHECK_RESUGID(uids, gids);

    // Ensure that real user ID can not be changed.
    for (uint32_t i(1); i < NUM_CREDID; i++) {
        uid_t  uid(uids[i]);

        ASSERT_EQ(EPERM, pfc_cred_set(uid, GID_UNDEF, PFC_TRUE));

        // Ensure that group list is not changed.
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Ensure that real group ID can be set as effective group ID.
    ASSERT_EQ(0, pfc_cred_set(uids[0], gids[1], PFC_TRUE));
    CHECK_UGID(uids[0], gids[1]);
    ASSERT_DUMPABLE(1);

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_setbyname() can change user and group ID.
 */
static void
check_setbyname(ChildContext *ctx, pwdmap *map, const char *user,
                const char *group, uid_t requid, gid_t reqgid)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // Change user and group ID.
    ASSERT_EQ(0, pfc_cred_setbyname(user, group, PFC_FALSE));

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);

    // Ensure that user and group ID have been changed.
    if (user != NULL) {
        uid_t uid((requid == UID_UNDEF) ? map->getUID(user) : requid);
        ASSERT_NE(UID_UNDEF, uid);
        for (uid_t *u(uids); u < PFC_ARRAY_LIMIT(uids); u++) {
            *u = uid;
        }
    }
    if (group != NULL) {
        gid_t gid((reqgid == UID_UNDEF) ? map->getGID(group) : reqgid);
        ASSERT_NE(GID_UNDEF, gid);
        for (gid_t *u(gids); u < PFC_ARRAY_LIMIT(gids); u++) {
            *u = gid;
        }
    }

    CHECK_RESUGID(uids, gids);

    const char *uroot(map->getUser(0));
    const char *groot(map->getGroup(0));
    if (uids[1] != 0 && uroot != NULL && groot != NULL) {
        // Ensure that we can not get root privilege any more.
        ASSERT_EQ(EPERM, pfc_cred_setbyname(uroot, NULL, PFC_FALSE));
        CHECK_RESUGID(uids, gids);
        ASSERT_EQ(EPERM, pfc_cred_setbyname(uroot, groot, PFC_FALSE));
        CHECK_RESUGID(uids, gids);

        if (gids[0] != 0 && gids[1] != 0 && gids[2] != 0) {
            ASSERT_EQ(EPERM, pfc_cred_setbyname(NULL, groot, PFC_FALSE));
            CHECK_RESUGID(uids, gids);
        }
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_setbyname() can change user and group ID and group
 * list.
 */
static void
check_setbyname_initgrp(ChildContext *ctx, pwdmap *map, const char *user,
                        const char *group, uid_t uid, gid_t gid)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // Change user and group ID.
    ASSERT_EQ(0, pfc_cred_setbyname(user, group, PFC_TRUE));

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);

    // Ensure that user and group ID have been changed.
    uid_t curuids[NUM_CREDID];
    gid_t curgids[NUM_CREDID];

    if (user != NULL) {
        if (uid == UID_UNDEF) {
            uid = map->getUID(user);
            ASSERT_NE(UID_UNDEF, uid);
        }

        for (uid_t *u(curuids); u < PFC_ARRAY_LIMIT(curuids); u++) {
            *u = uid;
        }
    }
    else {
        uid = UID_UNDEF;
        memcpy(curuids, uids, sizeof(uids));
    }

    if (group != NULL) {
        if (gid == GID_UNDEF) {
            gid = map->getGID(group);
            ASSERT_NE(GID_UNDEF, gid);
        }

        for (gid_t *g(curgids); g < PFC_ARRAY_LIMIT(curgids); g++) {
            *g = gid;
        }
    }
    else {
        gid = GID_UNDEF;
        memcpy(curgids, gids, sizeof(gids));
    }

    CHECK_RESUGID(curuids, curgids);

    const char *uroot(map->getUser(0));
    const char *groot(map->getGroup(0));
    if (curuids[1] != 0 && uroot != NULL && groot != NULL) {
        // Ensure that we can not get root privilege any more.
        ASSERT_EQ(EPERM, pfc_cred_setbyname(uroot, NULL, PFC_FALSE));
        CHECK_RESUGID(curuids, curgids);
        ASSERT_EQ(EPERM, pfc_cred_setbyname(uroot, groot, PFC_FALSE));
        CHECK_RESUGID(curuids, curgids);

        if (gids[0] != 0 && gids[1] != 0 && gids[2] != 0) {
            ASSERT_EQ(EPERM, pfc_cred_setbyname(NULL, groot, PFC_FALSE));
            CHECK_RESUGID(curuids, curgids);
        }
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();

    if (user != NULL && uids[0] != uid) {
        // Ensure that group list has been initialized.
        groups.clear();
        load_group_list(groups, uid);
        RETURN_ON_ERROR();
        if (gid != GID_UNDEF) {
            groups.insert(gid);
        }

        ASSERT_GIDSET_EQ(groups, curgroups);
    }
    else {
        // Ensure that group list is not changed.
        ASSERT_GIDSET_EQ(groups, curgroups);
    }
#endif  /* DO_CRED_TEST */
}

/*
 * pfc_cred_setbyname() test for unprivileged user.
 */
static void
check_setbyname_unpriv(ChildContext *ctx, pwdmap *map)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // This should do nothing.
    ASSERT_EQ(0, pfc_cred_setbyname(NULL, NULL, PFC_FALSE));
    CHECK_RESUGID(uids, gids);

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);

    // EINVAL test.
    ASSERT_EQ(EINVAL, pfc_cred_setbyname("", NULL, PFC_FALSE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(EINVAL, pfc_cred_setbyname(NULL, "", PFC_FALSE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(EINVAL, pfc_cred_setbyname("", "", PFC_FALSE));
    CHECK_RESUGID(uids, gids);

    // ENOENT test.
    char  invuser[32];
    for (uint32_t idx(0); true; idx++) {
        snprintf(invuser, sizeof(invuser), "invalid-user-%u", idx);
        uidmap_t::iterator  it(map->pm_uids.find(invuser));
        if (it == map->pm_uids.end()) {
            break;
        }
    }

    char  invgroup[32];
    for (uint32_t idx(0); true; idx++) {
        snprintf(invgroup, sizeof(invgroup), "invalid-group-%u", idx);
        uidmap_t::iterator  it(map->pm_gids.find(invgroup));
        if (it == map->pm_gids.end()) {
            break;
        }
    }

    ASSERT_EQ(ENOENT, pfc_cred_setbyname(invuser, NULL, PFC_FALSE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(ENOENT, pfc_cred_setbyname(NULL, invgroup, PFC_FALSE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(ENOENT, pfc_cred_setbyname(invuser, invgroup, PFC_FALSE));
    CHECK_RESUGID(uids, gids);

    // Unprivileged process can switch user and group ID to effective user and
    // group ID respectively.
    ASSERT_EQ(0, pfc_cred_setbyname(NULL, map->pm_egname, PFC_FALSE));
    gid_t egid(gids[1]);
    gid_t g[] = {egid, egid, egid};
    CHECK_RESUGID(uids, g);
    ASSERT_DUMPABLE(1);

    ASSERT_EQ(0, pfc_cred_setbyname(map->pm_euname, NULL, PFC_FALSE));
    uid_t euid(uids[1]);
    CHECK_UGID(euid, egid);
    ASSERT_EQ(0, pfc_cred_setbyname(map->pm_euname, map->pm_egname,
                                    PFC_FALSE));
    CHECK_UGID(euid, egid);
    ASSERT_DUMPABLE(1);

    // Ensure that unprivileged process can not change user and group ID to
    // others.
    usermap_t  &umap(map->pm_users);
    groupmap_t &gmap(map->pm_groups);
    const uint32_t max_loop(200);
    uint32_t       loop(0);

    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        uid_t uid(uit->first);
        if (uid == euid) {
            continue;
        }

        const char *user(uit->second.c_str());
        for (groupmap_t::iterator git(gmap.begin()); git != gmap.end();
             git++) {
            gid_t gid(git->first);
            if (gid == egid) {
                continue;
            }

            const char *group(git->second.c_str());
            ASSERT_EQ(EPERM, pfc_cred_setbyname(NULL, group, PFC_FALSE));
            CHECK_UGID(euid, egid);
            ASSERT_EQ(EPERM, pfc_cred_setbyname(user, NULL, PFC_FALSE));
            CHECK_UGID(euid, egid);
            ASSERT_EQ(EPERM, pfc_cred_setbyname(user, group, PFC_FALSE));
            CHECK_UGID(euid, egid);

            loop++;
            if (loop > max_loop) {
                break;
            }
        }
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);
#endif  /* DO_CRED_TEST */
}

/*
 * pfc_cred_setbyname() test for unprivileged user.
 * This test checks whether unprivileged user can't initialize supplementary
 * groups.
 */
static void
check_setbyname_initgrp_unpriv(ChildContext *ctx, pwdmap *map)
{
#ifdef  DO_CRED_TEST
    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t groups;
    load_group_list(groups);
    RETURN_ON_ERROR();

    // This should do nothing.
    ASSERT_EQ(0, pfc_cred_setbyname(NULL, NULL, PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // EINVAL test.
    ASSERT_EQ(EINVAL, pfc_cred_setbyname("", NULL, PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(EINVAL, pfc_cred_setbyname(NULL, "", PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(EINVAL, pfc_cred_setbyname("", "", PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // ENOENT test.
    char  invuser[32];
    for (uint32_t idx(0); true; idx++) {
        snprintf(invuser, sizeof(invuser), "invalid-user-%u", idx);
        uidmap_t::iterator  it(map->pm_uids.find(invuser));
        if (it == map->pm_uids.end()) {
            break;
        }
    }

    char  invgroup[32];
    for (uint32_t idx(0); true; idx++) {
        snprintf(invgroup, sizeof(invgroup), "invalid-group-%u", idx);
        uidmap_t::iterator  it(map->pm_gids.find(invgroup));
        if (it == map->pm_gids.end()) {
            break;
        }
    }

    ASSERT_EQ(ENOENT, pfc_cred_setbyname(invuser, NULL, PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(ENOENT, pfc_cred_setbyname(NULL, invgroup, PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    ASSERT_EQ(ENOENT, pfc_cred_setbyname(invuser, invgroup, PFC_TRUE));
    CHECK_RESUGID(uids, gids);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Unprivileged process can switch group ID to effective group ID.
    ASSERT_EQ(0, pfc_cred_setbyname(NULL, map->pm_egname, PFC_FALSE));
    gid_t egid(gids[1]);
    gid_t g[] = {egid, egid, egid};
    CHECK_RESUGID(uids, g);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Group list should not be changed if the real user ID is not changed.
    ASSERT_EQ(0, pfc_cred_setbyname(map->pm_uname, NULL, PFC_TRUE));
    uid_t ruid(uids[0]);
    CHECK_UGID(ruid, egid);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    ASSERT_EQ(0, pfc_cred_setbyname(map->pm_uname, map->pm_egname, PFC_TRUE));
    CHECK_UGID(ruid, egid);
    ASSERT_DUMPABLE(1);
    {
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(groups, curgroups);
    }

    // Ensure that unprivileged process can not change user and group ID to
    // others.
    usermap_t  &umap(map->pm_users);
    groupmap_t &gmap(map->pm_groups);
    const uint32_t max_loop(200);
    uint32_t       loop(0);

    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        uid_t uid(uit->first);
        if (uid == ruid) {
            continue;
        }

        const char *user(uit->second.c_str());
        for (groupmap_t::iterator git(gmap.begin()); git != gmap.end();
             git++) {
            gid_t gid(git->first);
            if (gid == egid) {
                continue;
            }

            const char *group(git->second.c_str());
            ASSERT_EQ(EPERM, pfc_cred_setbyname(NULL, group, PFC_TRUE));
            CHECK_UGID(ruid, egid);
            ASSERT_EQ(EPERM, pfc_cred_setbyname(user, NULL, PFC_TRUE));
            CHECK_UGID(ruid, egid);
            ASSERT_EQ(EPERM, pfc_cred_setbyname(user, group, PFC_TRUE));
            CHECK_UGID(ruid, egid);

            loop++;
            if (loop > max_loop) {
                break;
            }
        }
    }

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(groups, curgroups);

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_setbyname() can set user and group ID to real,
 * effective, and saved user and group ID.
 * This test requires root privilege.
 */
static void
check_setres_byname(ChildContext *ctx, pwdmap *map,
                    const char *users[NUM_CREDID],
                    const char *groups[NUM_CREDID])
{
#ifdef  DO_CRED_TEST
    // Change user and group ID to the specified IDs.
    // This will drop SETUID capability.
    uid_t uids[] = {
        map->getUID(users[0]),
        map->getUID(users[1]),
        map->getUID(users[2]),
    };
    gid_t gids[] = {
        map->getGID(groups[0]),
        map->getGID(groups[1]),
        map->getGID(groups[2]),
    };

    for (uint32_t i(0); i < NUM_CREDID; i++) {
        ASSERT_NE(UID_UNDEF, uids[i]);
        ASSERT_NE(GID_UNDEF, gids[i]);
    }

    ASSERT_EQ(0, setresgid(gids[0], gids[1], gids[2]));
    ASSERT_EQ(0, setresuid(uids[0], uids[1], uids[2]));
    CHECK_RESUGID(uids, gids);

    const char  *nil(reinterpret_cast<const char *>(NULL));

    // Ensure that user and group ID can be set to real, effective, and
    // saved ID.
    for (uint32_t i(0); i < NUM_CREDID; i++) {
        for (uint32_t j(0); j < NUM_CREDID; j++) {
            const char  *user(users[i]);
            const char  *group(groups[j]);

            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, map,
                                              user, nil,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, map,
                                              nil, group,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, map,
                                              user, group,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
        }
    }
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_setbyname() can not change real user ID and
 * supplementary group access list.
 * This test requires root privilege.
 */
static void
check_setres_byname_initgrp(ChildContext *ctx, pwdmap *map,
                            const char *users[NUM_CREDID],
                            const char *groups[NUM_CREDID])
{
#ifdef  DO_CRED_TEST
    // Preserve current group list.
    gidset_t glist;
    load_group_list(glist);
    RETURN_ON_ERROR();

    // Change user and group ID to the specified IDs.
    // This will drop SETUID capability.
    uid_t uids[] = {
        map->getUID(users[0]),
        map->getUID(users[1]),
        map->getUID(users[2]),
    };
    gid_t gids[] = {
        map->getGID(groups[0]),
        map->getGID(groups[1]),
        map->getGID(groups[2]),
    };

    for (uint32_t i(0); i < NUM_CREDID; i++) {
        ASSERT_NE(UID_UNDEF, uids[i]);
        ASSERT_NE(GID_UNDEF, gids[i]);
    }

    // This will drop SETUID capability.
    ASSERT_EQ(0, setresgid(gids[0], gids[1], gids[2]));
    ASSERT_EQ(0, setresuid(uids[0], uids[1], uids[2]));
    CHECK_RESUGID(uids, gids);

    // Ensure that real user ID can not be changed.
    for (uint32_t i(1); i < NUM_CREDID; i++) {
        const char  *user(users[i]);

        ASSERT_EQ(EPERM, pfc_cred_setbyname(user, NULL, PFC_TRUE));

        // Ensure that group list is not changed.
        gidset_t curgroups;
        load_group_list(curgroups);
        RETURN_ON_ERROR();
        ASSERT_GIDSET_EQ(glist, curgroups);
    }

    // Ensure that real group ID can be set as effective group ID.
    ASSERT_EQ(0, pfc_cred_setbyname(users[0], groups[1], PFC_TRUE));
    CHECK_UGID(uids[0], gids[1]);
    ASSERT_DUMPABLE(1);

    // Ensure that group list is not changed.
    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();
    ASSERT_GIDSET_EQ(glist, curgroups);
#endif  /* DO_CRED_TEST */
}

/*
 * Ensure that pfc_cred_setup() works.
 */
static void
check_setup(ChildContext *ctx, pwdmap *map, const char *cfpath, uid_t requid,
            gid_t reqgid, int reqerr)
{
#ifdef  DO_CRED_TEST
    if (cfpath != NULL) {
        pfc_refptr_t *rpath(pfc_refptr_string_create(cfpath));
        ASSERT_TRUE(rpath != NULL);
        int  err(pfc_sysconf_init(rpath));
        pfc_refptr_put(rpath);
        ASSERT_EQ(0, err);
    }

    // Preserve current UIDs and GIDs.
    uid_t uids[NUM_CREDID];
    gid_t gids[NUM_CREDID];
    GETRESUID(uids);
    GETRESGID(gids);

    // Preserve current group list.
    gidset_t glist;
    load_group_list(glist);
    RETURN_ON_ERROR();

    // Call pfc_cred_setup().
    ASSERT_EQ(reqerr, pfc_cred_setup());
    if (reqerr != 0) {
        return;
    }

    // Ensure that user and group ID have been changed.
    uid_t curuids[NUM_CREDID];
    gid_t curgids[NUM_CREDID];

    if (requid != UID_UNDEF) {
        for (uid_t *u(curuids); u < PFC_ARRAY_LIMIT(curuids); u++) {
            *u = requid;
        }
    }
    else {
        memcpy(curuids, uids, sizeof(uids));
    }
    if (reqgid != GID_UNDEF) {
        for (gid_t *u(curgids); u < PFC_ARRAY_LIMIT(curgids); u++) {
            *u = reqgid;
        }
    }
    else {
        memcpy(curgids, gids, sizeof(gids));
    }

    CHECK_RESUGID(curuids, curgids);

    gidset_t curgroups;
    load_group_list(curgroups);
    RETURN_ON_ERROR();

    if (requid != UID_UNDEF && uids[0] != requid) {
        // Ensure that group list has been initialized.
        glist.clear();
        load_group_list(glist, requid);
        RETURN_ON_ERROR();
        if (reqgid != GID_UNDEF) {
            glist.insert(reqgid);
        }

        ASSERT_GIDSET_EQ(glist, curgroups);
    }
    else {
        // Ensure that group list is not changed.
        ASSERT_GIDSET_EQ(glist, curgroups);
    }

    // Ensure that suid dumpable flag is set.
    ASSERT_DUMPABLE(1);
#endif  /* DO_CRED_TEST */
}

/*
 * Determine whether this program can install official configuration file.
 */
static bool
can_install_sysconf(void)
{
    if (access(PFC_SYSCONFDIR, R_OK | W_OK | X_OK) != 0) {
        return false;
    }

    struct stat sbuf;
    if (stat(PFC_SYSCONFDIR, &sbuf) != 0) {
        return false;
    }

    return (S_ISDIR(sbuf.st_mode)) ? true : false;
}

/*
 * Write user/group definition to the configuration file, and run
 * test for pfc_cred_setup().
 */
static void
run_setup_test(pwdmap &map, const char *path, const char *cfpath,
               const char *user, const char *group, uid_t uid, gid_t gid,
               int reqerr, strlist_t *errlist = NULL)
{
    {
        StdioRef fref(path, "w");
        FILE *fp(*fref);
        ASSERT_TRUE(fp != NULL);

        ASSERT_NE(EOF, fputs("options {\n", fp));

        if (user != NULL) {
            ASSERT_LT(0, fprintf(fp, "\tadmin_user = \"%s\";\n", user));
        }
        if (group != NULL) {
            ASSERT_LT(0, fprintf(fp, "\tadmin_group = \"%s\";\n", group));
        }

        // pfc_cred_setup() should ignore "user" and "group".
        ASSERT_LT(0, fprintf(fp, "\tuser = \"invalid-user\";\n"));
        ASSERT_LT(0, fprintf(fp, "\tgroup = \"invalid-group\";\n"));

        ASSERT_NE(EOF, fputs("}\n", fp));
        ASSERT_EQ(0, fflush(fp));
    }

    ChildContext child;
    if (errlist != NULL) {
        child.setErrorList(*errlist);
    }

    child_func_t func(boost::bind(check_setup, _1, &map, cfpath, uid, gid,
                                  reqerr));
    child.run(func);
    RETURN_ON_ERROR();
    child.verify();
}

/*
 * Construct expected parser error message that indicates the length of
 * parameter value is invalid.
 */
static void
parser_error_length(std::string &msg, const char *cfpath, uint32_t line,
                    const char *param, const char *value)
{
    size_t vlen(strlen(value));
    const char  *errmsg;
    if (vlen < 1) {
        errmsg = "String length must be more than or equal 1";
    }
    else {
        errmsg = "String length must be less than or equal 31";
    }

    std::ostringstream os;
    os << cfpath << ":" << line << ": " << param << ": " << vlen
       << ": " << errmsg << std::endl;

    msg.assign(os.str());
}

/*
 * Run test for pfc_cred_setup().
 */
static void
test_setup(pwdmap &map, RandomGenerator &rand, const char *cfpath = NULL)
{
    const char *path;
    char       userbuf[32], groupbuf[32];

    uid_t    ruid(getuid());
    uid_t    euid(geteuid());
    gid_t    rgid(getgid());
    gid_t    egid(getegid());
    bool     is_root(euid == 0);

    if (cfpath == NULL) {
        strlist_t   errlist;

        path = PFC_PFCD_CONF_PATH;

        // Minimum length of user and group.
        uid_t uid(3);
        gid_t gid(4);
        snprintf(userbuf, sizeof(userbuf), "%d", uid);
        snprintf(groupbuf, sizeof(groupbuf), "%d", gid);

        int   reqerr((is_root || (uid == ruid && (gid == rgid || gid == egid)))
                     ? 0 : EPERM);
        run_setup_test(map, path, cfpath, userbuf, groupbuf, uid, gid,
                       reqerr, &errlist);
        RETURN_ON_ERROR();
        ASSERT_EQ(0U, errlist.size());

        // Maximum length of user and group.
        const char  *longname("long-user-or-group-name-0123456");
        uid = map.getUID(longname);
        gid = map.getGID(longname);
        if (uid == UID_UNDEF || gid == GID_UNDEF) {
            reqerr = ENOENT;
        }
        else {
            reqerr = (is_root || (uid == ruid && (gid == rgid || gid == egid)))
                ? 0 : EPERM;
        }

        run_setup_test(map, path, cfpath, longname, longname, uid, gid,
                       reqerr, &errlist);
        RETURN_ON_ERROR();
        ASSERT_EQ(0U, errlist.size());

        //
        // Broken configuration file test.
        //
        std::string errmsg;
        const char  *param, *value;
        const char  *toolong("too-long-user-or-group-name-0123");

        // "admin_user" is empty.
        param = "admin_user";
        value = "";
        parser_error_length(errmsg, path, 2, param, value);
        run_setup_test(map, path, cfpath, value, NULL, UID_UNDEF, GID_UNDEF,
                       EINVAL, &errlist);
        RETURN_ON_ERROR();
        ASSERT_EQ(1U, errlist.size());
        ASSERT_EQ(errmsg, errlist.front());

        // "admin_user" is too long.
        errlist.clear();
        value = toolong;
        parser_error_length(errmsg, path, 2, param, value);
        run_setup_test(map, path, cfpath, value, NULL, UID_UNDEF, GID_UNDEF,
                       EINVAL, &errlist);
        RETURN_ON_ERROR();

        // "admin_group" is empty.
        errlist.clear();
        param = "admin_group";
        value = "";
        parser_error_length(errmsg, path, 2, param, value);
        run_setup_test(map, path, cfpath, NULL, value, UID_UNDEF, GID_UNDEF,
                       EINVAL, &errlist);
        RETURN_ON_ERROR();

        // "admin_group" is too long.
        errlist.clear();
        value = toolong;
        parser_error_length(errmsg, path, 2, param, value);
        run_setup_test(map, path, cfpath, NULL, value, UID_UNDEF, GID_UNDEF,
                       EINVAL, &errlist);
        RETURN_ON_ERROR();
    }
    else {
        path = cfpath;
    }

    // Neither user nor group is specified.
    {
        run_setup_test(map, path, cfpath, NULL, NULL, UID_UNDEF, GID_UNDEF, 0);
        RETURN_ON_ERROR();
    }

    // Both UID and GID are not changed.
    snprintf(userbuf, sizeof(userbuf), "%u", getuid());
    snprintf(groupbuf, sizeof(groupbuf), "%u", getgid());
    {
        const char *users[] = {
            NULL, map.pm_uname, userbuf,
        };
        const char *groups[] = {
            NULL, map.pm_gname, groupbuf,
        };

        for (const char **u(users); u < PFC_ARRAY_LIMIT(users); u++) {
            for (const char **g(groups); g < PFC_ARRAY_LIMIT(groups); g++) {
                run_setup_test(map, path, cfpath, *u, *g, UID_UNDEF,
                               GID_UNDEF, 0);
                RETURN_ON_ERROR();
            }
        }
    }

    // ENOENT test.
    for (uint32_t idx(0); true; idx++) {
        snprintf(userbuf, sizeof(userbuf), "invalid-user-%u", idx);
        uidmap_t::iterator  it(map.pm_uids.find(userbuf));
        if (it == map.pm_uids.end()) {
            break;
        }
    }

    for (uint32_t idx(0); true; idx++) {
        snprintf(groupbuf, sizeof(groupbuf), "invalid-group-%u", idx);
        uidmap_t::iterator  it(map.pm_gids.find(groupbuf));
        if (it == map.pm_gids.end()) {
            break;
        }
    }

    run_setup_test(map, path, cfpath, userbuf, NULL, UID_UNDEF, GID_UNDEF,
                   ENOENT);
    RETURN_ON_ERROR();

    run_setup_test(map, path, cfpath, NULL, groupbuf, UID_UNDEF, GID_UNDEF,
                   ENOENT);

    // Valid user and group name.
    const uint32_t max_loop(200);

    uint32_t  loop(0);
    usermap_t  &umap(map.pm_users);
    groupmap_t &gmap(map.pm_groups);

    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        uid_t uid(uit->first);
        const char *user(uit->second.c_str());

        for (groupmap_t::iterator git(gmap.begin()); git != gmap.end();
             git++) {
            gid_t gid(git->first);
            const char *group(git->second.c_str());

            int reqerr((is_root ||
                        (uid == ruid && (gid == rgid || gid == egid)))
                       ? 0 : EPERM);

            run_setup_test(map, path, cfpath, user, group, uid, gid, reqerr);
            RETURN_ON_ERROR();

            loop++;
            if (loop > max_loop) {
                break;
            }
        }
    }

    // Numeric user and group.
    for (loop = 0; loop < max_loop; loop++) {
        uint32_t u32;

        RANDOM_INTEGER_MAX(rand, u32, 30000U);
        uid_t uid(static_cast<uid_t>(u32));
        snprintf(userbuf, sizeof(userbuf), "%u", uid);

        RANDOM_INTEGER_MAX(rand, u32, 30000U);
        gid_t gid(static_cast<gid_t>(u32));
        snprintf(groupbuf, sizeof(groupbuf), "%u", gid);

        int reqerr((is_root ||
                    (uid == ruid && (gid == rgid || gid == egid)))
                   ? 0 : EPERM);

        run_setup_test(map, path, cfpath, userbuf, groupbuf, uid, gid, reqerr);
        RETURN_ON_ERROR();
    }
}

/*
 * Test for pfc_cred_set().
 */
TEST(cred, set)
{
    if (geteuid() != 0) {
        // Run test for unprivileged user.
        ChildContext ctx;

        ctx.run(check_set_unpriv);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();

        return;
    }

    // Ensure that pfc_cred_set() can set user and group ID to real,
    // effective, and saved user and group ID.
    {
        ChildContext ctx;
        uid_t  uids[] = {100, 200, 300};
        gid_t  gids[] = {1000, 2000, 3000};

        child_func_t  func(boost::bind(check_setres, _1, uids, gids));
        ctx.run(func);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();
    }

    // Ensure that pfc_cred_set() can set user and group ID to arbitrary user
    // and group ID if the calling process has root privilege.
    uid_t uids[] = {0, 1, 2, 100, 200, 300};
    gid_t gids[] = {0, 10, 20, 1000, 2000, 3000};

    for (uid_t *u(uids); u < PFC_ARRAY_LIMIT(uids); u++) {
        for (gid_t *g(gids); g < PFC_ARRAY_LIMIT(gids); g++) {
            uid_t uid(*u);
            gid_t gid(*g);

            {
                ChildContext child;
                child_func_t func(boost::bind(check_set, _1, uid, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_set, _1, UID_UNDEF, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_set, _1, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
        }
    }
}

/*
 * Test for pfc_cred_set() with specifying PFC_TRUE to initgrp.
 */
TEST(cred, set_initgrp)
{
    if (geteuid() != 0) {
        // Run test for unprivileged user.
        ChildContext ctx;

        ctx.run(check_set_initgrp_unpriv);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();

        return;
    }

    // Ensure that unprivileged user can not change real user ID and
    // supplementary group access list.
    {
        ChildContext ctx;
        uid_t  uids[] = {100, 200, 300};
        gid_t  gids[] = {1000, 2000, 3000};

        child_func_t  func(boost::bind(check_setres_initgrp, _1, uids, gids));
        ctx.run(func);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();
    }

    // Determine user ID which belongs to more than one groups.
    pwdmap     map;
    usermap_t  &umap(map.pm_users);
    uid_t      uid_multi(UID_UNDEF);
    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        uid_t u(uit->first);
        if (u == 0) {
            continue;
        }

        gidset_t groups;
        load_group_list(groups, u);
        RETURN_ON_ERROR();
        if (groups.size() > 1) {
            uid_multi = u;
            break;
        }
    }

    if (PFC_EXPECT_FALSE(uid_multi == UID_UNDEF)) {
        fprintf(stderr, "*** WARNING: No user belongs to more than one "
                "groups.\n");
        uid_multi = 100;
    }

    // Determine undefined user ID.
    uid_t uid_undef(1);
    for (; uid_undef < 0x10000; uid_undef++) {
        if (umap.find(uid_undef) != umap.end()) {
            break;
        }
    }

    if (PFC_EXPECT_FALSE(uid_undef == 0x10000)) {
        fprintf(stderr, "*** WARNING: Unable to determine undefined UID.\n");
        uid_undef = 20000;
    }

    // Ensure that privileged user can change real user ID and supplementary
    // group access list.
    uid_t uids[] = {uid_multi, uid_undef};
    gid_t gids[] = {0, 10, 20, 1000, 2000, 3000};

    for (uid_t *u(uids); u < PFC_ARRAY_LIMIT(uids); u++) {
        for (gid_t *g(gids); g < PFC_ARRAY_LIMIT(gids); g++) {
            uid_t uid(*u);
            gid_t gid(*g);

            {
                ChildContext child;
                child_func_t func(boost::bind(check_set_initgrp, _1,
                                              uid, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_set_initgrp, _1,
                                              UID_UNDEF, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_set_initgrp, _1,
                                              uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
        }
    }
}

/*
 * Test for pfc_cred_setbyname().
 */
TEST(cred, setbyname)
{
    pwdmap  map;

    if (PFC_EXPECT_FALSE(map.pm_euname == NULL)) {
        fprintf(stderr, "*** WARNING: No user name is associated with "
                "EUID(%d).\n", geteuid());
        return;
    }
    if (PFC_EXPECT_FALSE(map.pm_egname == NULL)) {
        fprintf(stderr, "*** WARNING: No group name associated with "
                "EGID(%d).\n", getegid());
        return;
    }

    if (geteuid() != 0) {
        // Run test for unprivileged user.
        ChildContext ctx;
        child_func_t func(boost::bind(check_setbyname_unpriv, _1, &map));
        ctx.run(func);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();

        return;
    }

    // Ensure that pfc_cred_setbyname() can set user and group ID to real,
    // effective, and saved user and group ID.
    const char *users[NUM_CREDID], *groups[NUM_CREDID];
    uint32_t uidx(0);
    usermap_t  &umap(map.pm_users);
    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        uid_t uid(uit->first);
        if (uid == 0) {
            continue;
        }

        users[uidx] = uit->second.c_str();
        uidx++;
        if (uidx == NUM_CREDID) {
            break;
        }
    }

    uint32_t gidx(0);
    groupmap_t &gmap(map.pm_groups);
    for (groupmap_t::iterator git(gmap.begin()); git != gmap.end(); git++) {
        gid_t gid(git->first);
        if (gid == 0) {
            continue;
        }

        groups[gidx] = git->second.c_str();
        gidx++;
        if (gidx == NUM_CREDID) {
            break;
        }
    }

    if (uidx == NUM_CREDID && gidx == NUM_CREDID) {
        ChildContext ctx;
        child_func_t  func(boost::bind(check_setres_byname, _1, &map,
                                       users, groups));
        ctx.run(func);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();
    }

    const char  *nil(reinterpret_cast<const char *>(NULL));

    // Ensure that pfc_cred_setbyname() can set user and group ID to arbitrary
    // user and group ID if the calling process has root privilege.
    const uint32_t max_loop(200);
    uint32_t       loop(0);
    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        const char *user(uit->second.c_str());

        for (groupmap_t::iterator git(gmap.begin()); git != gmap.end();
             git++) {
            const char *group(git->second.c_str());

            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, &map,
                                              user, nil,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, &map,
                                              nil, group,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, &map,
                                              user, group,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }

            loop++;
            if (loop > max_loop) {
                break;
            }
        }
    }

    // Specify user and group by numeric string.
    uid_t uids[] = {0, 1, 2, 1000, 2000, 3000};
    gid_t gids[] = {0, 10, 20, 10000, 20000, 30000};

    for (uid_t *u(uids); u < PFC_ARRAY_LIMIT(uids); u++) {
        for (gid_t *g(gids); g < PFC_ARRAY_LIMIT(gids); g++) {
            uid_t uid(*u);
            gid_t gid(*g);
            char  user[32], group[32];

            snprintf(user, sizeof(user), "%u", uid);
            snprintf(group, sizeof(group), "%u", gid);

            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, &map,
                                              user, nil, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, &map,
                                              nil, group, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname, _1, &map,
                                              user, group, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
        }
    }
}

/*
 * Test for pfc_cred_setbyname() with specifying PFC_TRUE to initgrp.
 */
TEST(cred, setbyname_initgrp)
{
    pwdmap  map;

    if (PFC_EXPECT_FALSE(map.pm_uname == NULL)) {
        fprintf(stderr, "*** WARNING: No user name is associated with "
                "UID(%d).\n", getuid());
        return;
    }
    if (PFC_EXPECT_FALSE(map.pm_gname == NULL)) {
        fprintf(stderr, "*** WARNING: No group name associated with "
                "GID(%d).\n", getgid());
        return;
    }
    if (PFC_EXPECT_FALSE(map.pm_euname == NULL)) {
        fprintf(stderr, "*** WARNING: No user name is associated with "
                "EUID(%d).\n", geteuid());
        return;
    }
    if (PFC_EXPECT_FALSE(map.pm_egname == NULL)) {
        fprintf(stderr, "*** WARNING: No group name associated with "
                "EGID(%d).\n", getegid());
        return;
    }

    if (geteuid() != 0) {
        // Run test for unprivileged user.
        ChildContext ctx;
        child_func_t func(boost::bind(check_setbyname_initgrp_unpriv,
                                      _1, &map));
        ctx.run(func);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();

        return;
    }

    // Ensure that unprivileged user can not change real user ID and
    // supplementary group access list.
    const char *users[NUM_CREDID], *groups[NUM_CREDID];
    uint32_t uidx(0);
    usermap_t  &umap(map.pm_users);
    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        uid_t uid(uit->first);
        if (uid == 0) {
            continue;
        }

        users[uidx] = uit->second.c_str();
        uidx++;
        if (uidx == NUM_CREDID) {
            break;
        }
    }

    uint32_t gidx(0);
    groupmap_t &gmap(map.pm_groups);
    for (groupmap_t::iterator git(gmap.begin()); git != gmap.end(); git++) {
        gid_t gid(git->first);
        if (gid == 0) {
            continue;
        }

        groups[gidx] = git->second.c_str();
        gidx++;
        if (gidx == NUM_CREDID) {
            break;
        }
    }

    if (uidx == NUM_CREDID && gidx == NUM_CREDID) {
        ChildContext ctx;
        child_func_t  func(boost::bind(check_setres_byname_initgrp, _1,
                                       &map, users, groups));
        ctx.run(func);
        RETURN_ON_ERROR();
        ctx.verify();
        RETURN_ON_ERROR();
    }

    const char  *nil(reinterpret_cast<const char *>(NULL));

    // Ensure that privileged user can change real user ID and supplementary
    // group access list.
    const uint32_t max_loop(200);
    uint32_t       loop(0);
    for (usermap_t::iterator uit(umap.begin()); uit != umap.end(); uit++) {
        const char *user(uit->second.c_str());

        for (groupmap_t::iterator git(gmap.begin()); git != gmap.end();
             git++) {
            const char *group(git->second.c_str());

            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname_initgrp, _1,
                                              &map, user, nil,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname_initgrp, _1,
                                              &map, nil, group,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname_initgrp, _1,
                                              &map, user, group,
                                              UID_UNDEF, GID_UNDEF));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }

            loop++;
            if (loop > max_loop) {
                break;
            }
        }
    }

    // Specify user and group by numeric string.
    uid_t uids[] = {0, 1, 2, 1000, 2000, 3000};
    gid_t gids[] = {0, 10, 20, 10000, 20000, 30000};

    for (uid_t *u(uids); u < PFC_ARRAY_LIMIT(uids); u++) {
        for (gid_t *g(gids); g < PFC_ARRAY_LIMIT(gids); g++) {
            uid_t uid(*u);
            gid_t gid(*g);
            char  user[32], group[32];

            snprintf(user, sizeof(user), "%u", uid);
            snprintf(group, sizeof(group), "%u", gid);

            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname_initgrp, _1,
                                              &map, user, nil, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname_initgrp, _1,
                                              &map, nil, group, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
            {
                ChildContext child;
                child_func_t func(boost::bind(check_setbyname_initgrp, _1,
                                              &map, user, group, uid, gid));
                child.run(func);
                RETURN_ON_ERROR();
                child.verify();
                RETURN_ON_ERROR();
            }
        }
    }
}

/*
 * Test for pfc_cred_setup().
 */
TEST(cred, setup)
{
    RandomGenerator rand;
    pwdmap          map;

    if (can_install_sysconf()) {
        // Install test configuration file to official place.
        RenameFile r;
        r.rename(PFC_PFCD_CONF_PATH);
        RETURN_ON_ERROR();

        test_setup(map, rand);
        RETURN_ON_ERROR();
    }

    // Write configuration to the temporary file.
    TmpFile tmpf("cred_test.conf");
    ASSERT_EQ(0, tmpf.createFile());
    const char *path(tmpf.getPath());
    test_setup(map, rand, path);
}
