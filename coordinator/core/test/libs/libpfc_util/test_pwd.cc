/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_pwd.cc - Test for password and group file utilities.
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <map>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <pfc/util.h>
#include "misc.hh"

#define	TEST_PWD_LOOP		1000U

/*
 * Test for pfc_pwd_ismember().
 */
TEST(pwd, ismember)
{
    uid_t	uid(getuid());
    gid_t	gid(getgid());

    ASSERT_EQ(0, pfc_pwd_ismember(uid, gid, PFC_FALSE));
    ASSERT_EQ(0, pfc_pwd_ismember(uid, gid, PFC_TRUE));

    for (gid_t g(1); g <= 10; g++) {
        ASSERT_EQ(-1, pfc_pwd_ismember(uid, gid + g, PFC_FALSE));
    }

    int	size(getgroups(0, NULL));
    ASSERT_GE(size, 0);

    // Supplementary group test.
    if (size != 0) {
        TmpBuffer	buf(size * sizeof(gid_t));
        gid_t	*glist;
        TMPBUF_ASSERT(buf, glist, gid_t *);

        ASSERT_EQ(size, getgroups(size, glist));
        for (int i(0); i < size; i++) {
            gid_t	g(*(glist + i));

            ASSERT_EQ(0, pfc_pwd_ismember(uid, g, PFC_TRUE));
            if (g != gid) {
                ASSERT_EQ(-1, pfc_pwd_ismember(uid, g, PFC_FALSE));
            }
        }

        for (gid_t g(0); g <= TEST_PWD_LOOP; g++) {
            bool	found(false);

            for (int i(0); i < size; i++) {
                if (g == *(glist + i)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                ASSERT_EQ(-1, pfc_pwd_ismember(uid, g, PFC_TRUE));
            }
        }
    }

    // Pass invalid user ID.
    gid = getgid();
    for (uid = 0; uid <= TEST_PWD_LOOP; uid++) {
        struct passwd	*pwd = getpwuid(uid);

        if (pwd == NULL) {
            ASSERT_EQ(-1, pfc_pwd_ismember(uid, gid, PFC_TRUE));
            ASSERT_EQ(-1, pfc_pwd_ismember(uid, gid, PFC_FALSE));
        }
        else {
            ASSERT_EQ(0, pfc_pwd_ismember(uid, pwd->pw_gid, PFC_TRUE));
        }
    }

    // Pass invalid group ID.
    uid = getuid();
    for (gid = 0; gid <= TEST_PWD_LOOP; gid++) {
        struct group	*grp = getgrgid(gid);

        if (grp == NULL) {
            ASSERT_EQ(-1, pfc_pwd_ismember(uid, gid, PFC_TRUE));
            ASSERT_EQ(-1, pfc_pwd_ismember(uid, gid, PFC_FALSE));
        }
    }
}

/*
 * Test for pfc_pwd_strtouid().
 */
TEST(pwd, strtouid)
{
    uid_t  uid;

    // Invalid argument.
    ASSERT_EQ(EINVAL, pfc_pwd_strtouid(NULL, &uid));
    ASSERT_EQ(EINVAL, pfc_pwd_strtouid("", &uid));
    ASSERT_EQ(EINVAL, pfc_pwd_strtouid("user", NULL));

    // Obtain valid UIDs.
    uidmap_t       uidmap;
    struct passwd  *pwd;

    setpwent();
    while ((pwd = getpwent()) != NULL) {
        uidmap.insert(uidmap_t::value_type(pwd->pw_name, pwd->pw_uid));
    }
    endpwent();

    // Test with valid user name.
    for (uidmap_t::iterator it(uidmap.begin()); it != uidmap.end(); it++) {
        const std::string  *name(&(it->first));

        ASSERT_EQ(0, pfc_pwd_strtouid(name->c_str(), &uid))
            << "name=" << name;
        ASSERT_EQ(it->second, uid);
    }

    char  buf[64];

    // Test with valid integer.
    for (uid_t required(0); required < TEST_PWD_LOOP; required++) {
        snprintf(buf, sizeof(buf), "%u", required);
        ASSERT_EQ(0, pfc_pwd_strtouid(buf, &uid))
            << "name=" << buf;
        ASSERT_EQ(required, uid);

        snprintf(buf, sizeof(buf), "0%o", required);
        ASSERT_EQ(0, pfc_pwd_strtouid(buf, &uid))
            << "name=" << buf;
        ASSERT_EQ(required, uid);

        snprintf(buf, sizeof(buf), "0x%x", required);
        ASSERT_EQ(0, pfc_pwd_strtouid(buf, &uid))
            << "name=" << buf;
        ASSERT_EQ(required, uid);
    }

    // Test with invalid name.
    uint32_t  tested(0);
    for (uint32_t idx(0); tested < TEST_PWD_LOOP; idx++) {
        snprintf(buf, sizeof(buf), "invalid_%u", idx);
        uidmap_t::iterator	it(uidmap.find(buf));
        if (it != uidmap.end()) {
            continue;
        }

        tested++;
        ASSERT_EQ(ENOENT, pfc_pwd_strtouid(buf, &uid))
            << "name=" << buf;
    }

    // Test with too large integer.
    uint64_t  base(UINT32_MAX);
    for (uint32_t idx(0); idx < TEST_PWD_LOOP; idx++) {
        snprintf(buf, sizeof(buf), "%" PFC_PFMT_u64, base + idx);
        ASSERT_EQ(ENOENT, pfc_pwd_strtouid(buf, &uid))
            << "name=" << buf;
    }
}

/*
 * Test for pfc_pwd_strtogid().
 */
TEST(pwd, strtogid)
{
    gid_t	gid;

    // Invalid argument.
    ASSERT_EQ(EINVAL, pfc_pwd_strtogid(NULL, &gid));
    ASSERT_EQ(EINVAL, pfc_pwd_strtogid("", &gid));
    ASSERT_EQ(EINVAL, pfc_pwd_strtogid("group", NULL));

    // Obtain valid GIDs.
    gidmap_t		gidmap;
    struct group	*grp;

    setgrent();
    while ((grp = getgrent()) != NULL) {
        gidmap.insert(gidmap_t::value_type(grp->gr_name, grp->gr_gid));
    }
    endgrent();

    // Test with valid group name.
    for (gidmap_t::iterator it(gidmap.begin()); it != gidmap.end(); it++) {
        const std::string  *name(&(it->first));

        ASSERT_EQ(0, pfc_pwd_strtogid(name->c_str(), &gid))
            << "name=" << name;
        ASSERT_EQ((*it).second, gid);
    }

    char	buf[64];

    // Test with valid integer.
    for (gid_t required(0); required < TEST_PWD_LOOP; required++) {
        snprintf(buf, sizeof(buf), "%u", required);
        ASSERT_EQ(0, pfc_pwd_strtogid(buf, &gid))
            << "name=" << buf;
        ASSERT_EQ(required, gid);

        snprintf(buf, sizeof(buf), "0%o", required);
        ASSERT_EQ(0, pfc_pwd_strtogid(buf, &gid))
            << "name=" << buf;
        ASSERT_EQ(required, gid);

        snprintf(buf, sizeof(buf), "0x%x", required);
        ASSERT_EQ(0, pfc_pwd_strtogid(buf, &gid))
            << "name=" << buf;
        ASSERT_EQ(required, gid);
    }

    // Test with invalid name.
    uint32_t	tested(0);
    for (uint32_t idx(0); tested < TEST_PWD_LOOP; idx++) {
        snprintf(buf, sizeof(buf), "invalid_%u", idx);
        gidmap_t::iterator	it(gidmap.find(buf));
        if (it != gidmap.end()) {
            continue;
        }

        tested++;
        ASSERT_EQ(ENOENT, pfc_pwd_strtogid(buf, &gid))
            << "name=" << buf;
    }

    // Test with too large integer.
    uint64_t	base(UINT32_MAX);
    for (uint32_t idx(0); idx < TEST_PWD_LOOP; idx++) {
        snprintf(buf, sizeof(buf), "%" PFC_PFMT_u64, base + idx);
        ASSERT_EQ(ENOENT, pfc_pwd_strtogid(buf, &gid))
            << "name=" << buf;
    }
}

/*
 * Test for pfc_pwd_uidtoname().
 */
TEST(pwd, uidtoname)
{
    char    name[256];
    size_t  namelen(0);

    // Invalid argument.
    ASSERT_EQ(EINVAL, pfc_pwd_uidtoname(0, NULL, &namelen));
    ASSERT_EQ(EINVAL, pfc_pwd_uidtoname(0, name, NULL));

    // Obtain valid user names.
    usermap_t      usermap;
    struct passwd  *pwd;

    setpwent();
    while ((pwd = getpwent()) != NULL) {
        usermap.insert(usermap_t::value_type(pwd->pw_uid, pwd->pw_name));
    }
    endpwent();

    // Test with valid UID.
    for (usermap_t::iterator it(usermap.begin()); it != usermap.end(); it++) {
        uid_t  uid(it->first);

        namelen = sizeof(name);
        ASSERT_EQ(0, pfc_pwd_uidtoname(uid, name, &namelen))
            << "uid=" << uid;

        const std::string  *required(&(it->second));
        const size_t        rlen(required->size());
        ASSERT_STREQ(required->c_str(), name);
        ASSERT_EQ(rlen, namelen);

        // ENOSPC test.
        for (size_t len(0); len <= rlen; len++) {
            namelen = len;
            ASSERT_EQ(ENOSPC, pfc_pwd_uidtoname(uid, name, &namelen));
            ASSERT_EQ(rlen + 1, namelen);
        }
    }

    // Test with invalid UID.
    uint32_t  tested(0);
    for (uid_t uid(0); tested < TEST_PWD_LOOP; uid++) {
        usermap_t::iterator  it(usermap.find(uid));
        if (it != usermap.end()) {
            continue;
        }

        tested++;

        namelen = sizeof(name);
        ASSERT_EQ(ENOENT, pfc_pwd_uidtoname(uid, name, &namelen))
            << "uid=" << uid;
    }
}

/*
 * Test for pfc_pwd_gidtoname().
 */
TEST(pwd, gidtoname)
{
    char    name[256];
    size_t  namelen(0);

    // Invalid argument.
    ASSERT_EQ(EINVAL, pfc_pwd_gidtoname(0, NULL, &namelen));
    ASSERT_EQ(EINVAL, pfc_pwd_gidtoname(0, name, NULL));

    // Obtain valid group names.
    groupmap_t    groupmap;
    struct group  *grp;

    setgrent();
    while ((grp = getgrent()) != NULL) {
        groupmap.insert(groupmap_t::value_type(grp->gr_gid, grp->gr_name));
    }
    endgrent();

    // Test with valid GID.
    for (groupmap_t::iterator it(groupmap.begin()); it != groupmap.end();
         it++) {
        gid_t  gid(it->first);

        namelen = sizeof(name);
        ASSERT_EQ(0, pfc_pwd_gidtoname(gid, name, &namelen))
            << "gid=" << gid;

        const std::string  *required(&(it->second));
        const size_t        rlen(required->size());
        ASSERT_STREQ(required->c_str(), name);
        ASSERT_EQ(rlen, namelen);

        // ENOSPC test.
        for (size_t len(0); len <= rlen; len++) {
            namelen = len;
            ASSERT_EQ(ENOSPC, pfc_pwd_gidtoname(gid, name, &namelen));
            ASSERT_EQ(rlen + 1, namelen);
        }
    }

    // Test with invalid GID.
    uint32_t  tested(0);
    for (gid_t gid(0); tested < TEST_PWD_LOOP; gid++) {
        groupmap_t::iterator  it(groupmap.find(gid));
        if (it != groupmap.end()) {
            continue;
        }

        tested++;

        namelen = sizeof(name);
        ASSERT_EQ(ENOENT, pfc_pwd_gidtoname(gid, name, &namelen))
            << "gid=" << gid;
    }
}
