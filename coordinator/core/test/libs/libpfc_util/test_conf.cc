/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for conf
 */
#include <string>
#include <map>
#include <conf_impl.h>
#include <pfc/conf.h>
#include <pfc/refptr.h>
#include <pfcxx/conf.hh>
#include <pfcxx/refptr.hh>
#include <errno.h>
#include <boost/bind.hpp>
#include <gtest/gtest.h>
#include "test_conf.hh"
#include "tmpfile.hh"
#include "child.hh"
#include "test.h"

using namespace  pfc::core;

/* Configuration file Handle */
extern pfc_cfdef_t	test_conf_cfdef;

/* defvalue */
const char *defvalue_string = "invalid";
#define	TEST_CONF_DEFVALUE_STR_LEN	8

/* Configuration file name. */
#ifdef	PFC_LP64
#define	TEST_CONF_NAME		"test_conf.conf.64"
#else	/* !PFC_LP64 */
#define	TEST_CONF_NAME		"test_conf.conf.32"
#endif	/* PFC_LP64 */

/*
 * Map keys defined in test_conf.conf.
 */
static const char  *test_keys_test_conf_map[] = {
    "test_conf_map_key1",
    "test_conf_map_key2",
};

static const char  *test_keys_test_conf_map_byte[] = {
    "test_conf_map_byte_k1",
};

static const char  *test_keys_test_conf_map_str[] = {
    "test_conf_map_str_k1",
};

static const char  *test_keys_test_conf_map_bool[] = {
    "test_conf_map_bool_k1",
};

static const char  *test_keys_test_conf_map_i32[] = {
    "test_conf_map_i32_k1",
    "test_conf_map_i32_k2",
};

static const char  *test_keys_test_conf_map_ui32[] = {
    "test_conf_map_ui32_k1",
};

static const char  *test_keys_test_conf_map_i64[] = {
    "test_conf_map_i64_k1",
    "test_conf_map_i64_k2",
};

static const char  *test_keys_test_conf_map_ui64[] = {
    "test_conf_map_ui64_k1",
};

static const char  *test_keys_test_conf_map_lng[] = {
    "test_conf_map_lng_k1",
    "test_conf_map_lng_k2",
};

static const char  *test_keys_test_conf_map_ulng[] = {
    "test_conf_map_ulng_k1",
};

static const char  *test_keys_test_conf_map_array_size[] = {
    "test_conf_map_array_size_k1",
};

static const char  *test_keys_test_conf_invalid_map_name[] = {
    "test_conf_invalid_map_name_key",
};

static const char  *test_keys__test_conf_invalid_map_name[] = {
    "_test_conf_invalid_map_name_key",
};

typedef const struct {
    const char     *m_name;
    const char     **m_keys;
    uint32_t       m_nkeys;
} mapkeys_t;

#define	TEST_MAPKEYS_DECL(name)                                         \
    { #name, test_keys_##name, PFC_ARRAY_CAPACITY(test_keys_##name), }

static mapkeys_t  test_map_keys[] = {
    TEST_MAPKEYS_DECL(test_conf_map),
    TEST_MAPKEYS_DECL(test_conf_map_byte),
    TEST_MAPKEYS_DECL(test_conf_map_str),
    TEST_MAPKEYS_DECL(test_conf_map_bool),
    TEST_MAPKEYS_DECL(test_conf_map_i32),
    TEST_MAPKEYS_DECL(test_conf_map_ui32),
    TEST_MAPKEYS_DECL(test_conf_map_ui32),
    TEST_MAPKEYS_DECL(test_conf_map_i64),
    TEST_MAPKEYS_DECL(test_conf_map_ui64),
    TEST_MAPKEYS_DECL(test_conf_map_lng),
    TEST_MAPKEYS_DECL(test_conf_map_ulng),
    TEST_MAPKEYS_DECL(test_conf_map_array_size),
    TEST_MAPKEYS_DECL(test_conf_invalid_map_name),
    TEST_MAPKEYS_DECL(_test_conf_invalid_map_name),
};

typedef struct {
    uint32_t  ce_nthreads;
    uint32_t  ce_timeout;
} conf_evsrc_t;

typedef struct {
    std::string  co_log_level;
    uint32_t     co_stderr_rotate;
    uint32_t     co_message_size;
} conf_options_t;

typedef struct {
    uint32_t     ct_stack_size;
    uint32_t     ct_max_threads;
    uint32_t     ct_max_free;
    uint32_t     ct_min_free;
    uint32_t     ct_reap_threads;
} conf_tpool_t;

typedef std::map<std::string, conf_evsrc_t>  map_evsrc_t;
typedef std::map<std::string, conf_tpool_t>  map_tpool_t;

typedef struct {
    pfc_refptr_t     *c_path;
    conf_options_t   c_options;
    map_evsrc_t      c_evsrc;
    map_tpool_t      c_tpool;
} conf_sysconf_t;

/*
 * Set up test configuration file.
 */
static void
setup_conffile(TmpFile &tmpf)
{
    ASSERT_EQ(0, tmpf.createFile());
    FILE *fp(fopen(TEST_CONF_NAME, "r"));
    ASSERT_TRUE(fp != NULL) << "*** ERROR: " << strerror(errno);
    StdioRef fpref(fp);

    char buf[1024];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        tmpf.print("%s", buf);
    }

    tmpf.flush();
}

/*
 * Run test for pfc_sysconf_get_path().
 */
static void
check_sysconf_get_path(ChildContext *ctx, pfc_refptr_t *rconf)
{
    ASSERT_EQ(0, pfc_sysconf_init(rconf));
    pfc_refptr_put(rconf);

    ASSERT_STREQ(pfc_refptr_string_value(rconf), pfc_sysconf_get_path());
}

/*
 * Ensure that we can access to the system configuration file.
 */
static void
check_sysconf_get(ChildContext *ctx, conf_sysconf_t *sysconf)
{
    // pfc_sysconf_get_mapkeys() should fail if the system configuration file
    // is not initialized.
    pfc_listm_t  keys(PFC_LISTM_INVALID);
    {
        const char *names[] = {
            "thread_pool",
            "event_source",
            "mcache",
        };

        for (const char **namep(names); namep < PFC_ARRAY_LIMIT(names);
             namep++) {
            const char *name(*namep);

            ASSERT_EQ(EINVAL, pfc_sysconf_get_mapkeys(name, &keys));
            ASSERT_TRUE(keys == PFC_LISTM_INVALID);
        }
    }

    pfc_conf_t conf(pfc_sysconf_open());
    ASSERT_EQ(PFC_CONF_INVALID, conf);

    pfc_refptr_t  *rconf(sysconf->c_path);
    ASSERT_EQ(0, pfc_sysconf_init(rconf));
    pfc_refptr_put(rconf);

    conf = pfc_sysconf_open();
    ASSERT_NE(PFC_CONF_INVALID, conf);

    // Valid block name.
    conf_options_t *opts(&sysconf->c_options);
    pfc_cfblk_t    block(pfc_sysconf_get_block("options"));
    ASSERT_NE(PFC_CFBLK_INVALID, block);

    ASSERT_STREQ(opts->co_log_level.c_str(),
                 pfc_conf_get_string(block, "log_level", "<undef>"));
    ASSERT_EQ(opts->co_stderr_rotate,
              pfc_conf_get_uint32(block, "stderr_rotate", 0));
    ASSERT_EQ(opts->co_message_size,
              pfc_conf_get_uint32(block, "message_size", 0));

    ASSERT_FALSE(pfc_conf_get_bool(block, "log_syslog", PFC_FALSE));

    const char *facility("local3");
    ASSERT_STREQ(facility,
                 pfc_conf_get_string(block, "log_facility", facility));

    // Valid block name, but not defined.
    block = pfc_sysconf_get_block("rlimit");
    ASSERT_EQ(PFC_CFBLK_INVALID, block);
    {
        const char  *names[] = {
            "as_size",
            "core_size",
            "data_size",
            "open_files",
            "stack_size",
        };

        for (const char **namep(names); namep < PFC_ARRAY_LIMIT(names);
             namep++) {
            const char *name(*namep);
            const pfc_long_t  lvalue(12345);

            ASSERT_EQ(lvalue, pfc_conf_get_long(block, name, lvalue));
        }
    }

    // Invalid block name.
    block = pfc_sysconf_get_block("invalid_block");
    ASSERT_EQ(PFC_CFBLK_INVALID, block);

    // Below are pfc_sysconf_get_mapkeys() tests.

    // Invalid parameter.
    ASSERT_EQ(EINVAL, pfc_sysconf_get_mapkeys(NULL, &keys));
    ASSERT_TRUE(keys == PFC_LISTM_INVALID);
    ASSERT_EQ(EINVAL, pfc_sysconf_get_mapkeys("test", NULL));

    // Unknown map name.
    for (int i(0); i < 10; i++) {
        char  name[64];

        snprintf(name, sizeof(name), "unknown_map_name_%u", i);
        ASSERT_EQ(ENOENT, pfc_sysconf_get_mapkeys(name, &keys));
        ASSERT_TRUE(keys == PFC_LISTM_INVALID);
    }

    // Valid map name, but not defined.
    ASSERT_EQ(ENOENT, pfc_sysconf_get_mapkeys("mcache", &keys));
    ASSERT_TRUE(keys == PFC_LISTM_INVALID);

    // Valid map name. (event_source)
    ASSERT_EQ(0, pfc_sysconf_get_mapkeys("event_source", &keys));
    ASSERT_TRUE(keys != PFC_LISTM_INVALID);

    map_evsrc_t  *evmap(&sysconf->c_evsrc);
    int nkeys(pfc_listm_get_size(keys));
    ASSERT_EQ(static_cast<size_t>(nkeys), evmap->size());
    for (int i(0); i < nkeys; i++) {
        pfc_cptr_t    value;
        ASSERT_EQ(0, pfc_listm_getat(keys, i, &value));
        pfc_refptr_t  *rkey(reinterpret_cast<pfc_refptr_t *>
                            (const_cast<pfc_ptr_t>(value)));
        const char *key(pfc_refptr_string_value(rkey));
        map_evsrc_t::iterator it(evmap->find(key));
        ASSERT_TRUE(it != evmap->end());

        conf_evsrc_t *evsrc(&it->second);
        block = pfc_sysconf_get_map("event_source", key);
        ASSERT_NE(PFC_CFBLK_INVALID, block);

        ASSERT_EQ(evsrc->ce_nthreads,
                  pfc_conf_get_uint32(block, "nthreads", 0));
        ASSERT_EQ(evsrc->ce_timeout,
                  pfc_conf_get_uint32(block, "timeout", 0));

        std::string invalid(key);
        invalid.append("-<undef>");
        block = pfc_sysconf_get_map("event_source", invalid.c_str());
        ASSERT_EQ(PFC_CFBLK_INVALID, block);

        evmap->erase(it);
    }
    pfc_listm_destroy(keys);
    ASSERT_EQ(0U, evmap->size());

    // Valid map name. (thread_pool)
    keys = PFC_LISTM_INVALID;
    ASSERT_EQ(0, pfc_sysconf_get_mapkeys("thread_pool", &keys));
    ASSERT_TRUE(keys != PFC_LISTM_INVALID);

    map_tpool_t  *tmap(&sysconf->c_tpool);
    nkeys = pfc_listm_get_size(keys);
    ASSERT_EQ(static_cast<size_t>(nkeys), tmap->size());
    for (int i(0); i < nkeys; i++) {
        pfc_cptr_t    value;
        ASSERT_EQ(0, pfc_listm_getat(keys, i, &value));
        pfc_refptr_t  *rkey(reinterpret_cast<pfc_refptr_t *>
                            (const_cast<pfc_ptr_t>(value)));
        const char *key(pfc_refptr_string_value(rkey));
        map_tpool_t::iterator it(tmap->find(key));
        ASSERT_TRUE(it != tmap->end());

        conf_tpool_t *tpool(&it->second);
        block = pfc_sysconf_get_map("thread_pool", key);
        ASSERT_NE(PFC_CFBLK_INVALID, block);

        ASSERT_EQ(tpool->ct_stack_size,
                  pfc_conf_get_uint32(block, "stack_size", 0));
        ASSERT_EQ(tpool->ct_max_threads,
                  pfc_conf_get_uint32(block, "max_threads", 0));
        ASSERT_EQ(tpool->ct_max_free,
                  pfc_conf_get_uint32(block, "max_free", 0));
        ASSERT_EQ(tpool->ct_min_free,
                  pfc_conf_get_uint32(block, "min_free", 0));
        ASSERT_EQ(tpool->ct_reap_threads,
                  pfc_conf_get_uint32(block, "reap_threads", 0));

        std::string invalid(key);
        invalid.append("-<undef>");
        block = pfc_sysconf_get_map("thread_pool", invalid.c_str());
        ASSERT_EQ(PFC_CFBLK_INVALID, block);

        tmap->erase(it);
    }
    pfc_listm_destroy(keys);
    ASSERT_EQ(0U, tmap->size());
}

/*
 * Test Case
 *      pfc_conf_open()
 */
TEST(conf, pfc_conf_open)
{
	int ret;
	pfc_conf_t confp;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	pfc_conf_close(confp);

	/* invalid path */
	ret = pfc_conf_open(&confp, NULL, &test_conf_cfdef);
	ASSERT_EQ(ret, EINVAL);
}

/*
 * Test Case
 *      pfc_conf_refopen()
 */
TEST(conf, pfc_conf_refopen)
{
	int ret;
	pfc_conf_t confp;
	pfc_refptr_t *rpath;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	rpath = pfc_refptr_string_create(tmpf.getPath());
	ASSERT_TRUE(rpath != NULL);

	ret = pfc_conf_refopen(&confp, rpath, &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	pfc_conf_close(confp);
	pfc_refptr_put(rpath);

	/* invalid path */
	ret = pfc_conf_refopen(&confp, NULL, &test_conf_cfdef);
	ASSERT_EQ(ret, EINVAL);
}

/*
 * Test Case
 *      pfc_conf_reload()
 */
TEST(conf, pfc_conf_reload)
{
	int ret;
	pfc_conf_t confp;
	pfc_conf_t confp_inv = PFC_CONF_INVALID;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	ret = pfc_conf_reload(confp);
	ASSERT_EQ(ret, 0);

	/* invalid handle */
	ret = pfc_conf_reload(confp_inv);
	ASSERT_EQ(ret, EINVAL);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_reload() before and after
 */
TEST(conf, pfc_conf_reload_before_and_after)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	ret = pfc_conf_get_int32(cfblk, "test_conf_blk_p1", -1);
	ASSERT_EQ(ret, 15);

	/* Reload */
	ret = pfc_conf_reload(confp);
	ASSERT_EQ(0, ret);

	ret = pfc_conf_get_int32(cfblk, "test_conf_blk_p1", -1);
	ASSERT_EQ(ret, -1);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_close()
 */
TEST(conf, pfc_conf_close)
{
	int ret;
	uint32_t ui32_ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk, cfmap;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	cfmap = pfc_conf_get_map(confp, "test_conf_map", "test_conf_map_key1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ret = pfc_conf_get_int32(cfblk, "test_conf_blk_p1", -1);
	ASSERT_EQ(ret, 15);

	ui32_ret = pfc_conf_get_uint32(cfmap, "test_conf_map_p1", 555);
	ASSERT_EQ(ui32_ret, (uint32_t)1);
	pfc_conf_close(confp);

	/* after closing */
	ret = pfc_conf_get_int32(cfblk, "test_conf_blk_p1", -1);
	ASSERT_EQ(ret, -1);
	ret = pfc_conf_get_uint32(cfmap, "test_conf_map_p1", 555);
	ASSERT_EQ(ret, 555);
}

/*
 * Test Case
 *      pfc_conf_get_block()
 */
TEST(conf, pfc_conf_get_block)
{
	int ret;
	pfc_cfblk_t cfblk;
	pfc_conf_t confp;
	pfc_conf_t confp_inv = PFC_CONF_INVALID;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	ret = pfc_conf_get_int32(cfblk, "test_conf_blk_p1", -1);
	ASSERT_EQ(ret, 15);


	cfblk = pfc_conf_get_block(confp_inv, "test_conf_blk");
	ASSERT_EQ(cfblk, PFC_CFBLK_INVALID);

	cfblk = pfc_conf_get_block(confp, "invalid_block");
	ASSERT_EQ(cfblk, PFC_CFBLK_INVALID);

	ASSERT_EQ(PFC_CFBLK_INVALID,
		  pfc_conf_get_block(PFC_CONF_INVALID, "key"));
	ASSERT_EQ(PFC_CFBLK_INVALID,
		  pfc_conf_get_block(confp, NULL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_map()
 */
TEST(conf, pfc_conf_get_map)
{
	int ret;
	uint32_t ui32_ret;
	pfc_cfblk_t cfmap;
	pfc_conf_t confp;
	pfc_conf_t confp_inv = PFC_CONF_INVALID;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfmap = pfc_conf_get_map(confp, "test_conf_map", "test_conf_map_key1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ui32_ret = pfc_conf_get_uint32(cfmap, "test_conf_map_p1", 555);
	ASSERT_EQ(ui32_ret, (uint32_t)1);

	cfmap = pfc_conf_get_map(confp_inv, "test_conf_map",
				 "test_conf_map_key1");
	ASSERT_EQ(cfmap, PFC_CFBLK_INVALID);

	cfmap = pfc_conf_get_map(confp, "invalid_map", "test_conf_map_key1");
	ASSERT_EQ(cfmap, PFC_CFBLK_INVALID);

	cfmap = pfc_conf_get_map(confp, "invalid_map", "invalid_key");
	ASSERT_EQ(cfmap, PFC_CFBLK_INVALID);

	ASSERT_EQ(PFC_CFBLK_INVALID,
		  pfc_conf_get_map(PFC_CONF_INVALID, "name", "key"));
	ASSERT_EQ(PFC_CFBLK_INVALID,
		  pfc_conf_get_map(confp, NULL, "key"));
	ASSERT_EQ(PFC_CFBLK_INVALID,
		  pfc_conf_get_map(confp, "name", NULL));
	ASSERT_EQ(PFC_CFBLK_INVALID,
		  pfc_conf_get_map(confp, NULL, NULL));

	pfc_conf_close(confp);
}

/*
 * Test case for pfc_conf_get_mapkeys().
 */
TEST(conf, pfc_conf_get_mapkeys)
{
    TmpFile tmpf("conf_test");
    setup_conffile(tmpf);
    RETURN_ON_ERROR();

    pfc_conf_t  conf;
    ASSERT_EQ(0, pfc_conf_open(&conf, tmpf.getPath(), &test_conf_cfdef));

    // Invalid parameter.
    pfc_listm_t keys(PFC_LISTM_INVALID);
    ASSERT_EQ(EINVAL, pfc_conf_get_mapkeys(PFC_CONF_INVALID, "test_conf_map",
                                           &keys));
    ASSERT_TRUE(keys == PFC_LISTM_INVALID);
    ASSERT_EQ(EINVAL, pfc_conf_get_mapkeys(conf, NULL, &keys));
    ASSERT_TRUE(keys == PFC_LISTM_INVALID);
    ASSERT_EQ(EINVAL, pfc_conf_get_mapkeys(conf, "test_conf_map", NULL));

    // Unknown map name.
    for (int i(0); i < 10; i++) {
        char  name[64];

        snprintf(name, sizeof(name), "unknown_map_name_%u", i);
        ASSERT_EQ(ENOENT, pfc_conf_get_mapkeys(conf, name, &keys));
        ASSERT_TRUE(keys == PFC_LISTM_INVALID);
    }

    for (mapkeys_t *mkp(test_map_keys); mkp < PFC_ARRAY_LIMIT(test_map_keys);
         mkp++) {
        ASSERT_EQ(0, pfc_conf_get_mapkeys(conf, mkp->m_name, &keys))
            << "name=" << mkp->m_name;

        uint32_t nkeys(pfc_listm_get_size(keys));
        ASSERT_EQ(mkp->m_nkeys, nkeys)
            << "name=" << mkp->m_name;

        const char  **mkeys(mkp->m_keys);
        for (uint32_t i(0); i < mkp->m_nkeys; i++) {
            const char   *required(*(mkeys + i));
            pfc_refptr_t *rkey;

            ASSERT_EQ(0, pfc_listm_getat(keys, i, (pfc_cptr_t *)&rkey));
            ASSERT_STREQ(required, pfc_refptr_string_value(rkey))
                << "name=" << mkp->m_name << ", i=" << i;
        }

        pfc_listm_destroy(keys);
    }

    pfc_conf_close(conf);
}

/*
 * Test case for ConfHandle::getMapKeys().
 */
TEST(conf, cxx_getMapKeys)
{
    TmpFile tmpf("conf_test");
    setup_conffile(tmpf);
    RETURN_ON_ERROR();

    ConfHandle cf(tmpf.getPath(), &test_conf_cfdef);
    ASSERT_EQ(0, cf.getError());

    // Invalid parameter.
    pfc_listm_t keys(PFC_LISTM_INVALID);
    ASSERT_EQ(EINVAL, cf.getMapKeys(NULL, keys));
    ASSERT_TRUE(keys == PFC_LISTM_INVALID);

    // Unknown map name.
    for (int i(0); i < 10; i++) {
        char  name[64];

        snprintf(name, sizeof(name), "unknown_map_name_%u", i);
        ASSERT_EQ(ENOENT, cf.getMapKeys(name, keys));
        ASSERT_TRUE(keys == PFC_LISTM_INVALID);
    }

    for (mapkeys_t *mkp(test_map_keys); mkp < PFC_ARRAY_LIMIT(test_map_keys);
         mkp++) {
        ASSERT_EQ(0, cf.getMapKeys(mkp->m_name, keys))
            << "name=" << mkp->m_name;

        uint32_t nkeys(pfc_listm_get_size(keys));
        ASSERT_EQ(mkp->m_nkeys, nkeys)
            << "name=" << mkp->m_name;

        const char  **mkeys(mkp->m_keys);
        for (uint32_t i(0); i < mkp->m_nkeys; i++) {
            const char   *required(*(mkeys + i));
            pfc_refptr_t *rkey;

            ASSERT_EQ(0, pfc_listm_getat(keys, i, (pfc_cptr_t *)&rkey));
            ASSERT_STREQ(required, pfc_refptr_string_value(rkey))
                << "name=" << mkp->m_name << ", i=" << i;
        }

        pfc_listm_destroy(keys);

        std::string sname(mkp->m_name);
        ASSERT_EQ(0, cf.getMapKeys(sname, keys)) << "name=" << sname;

        nkeys = pfc_listm_get_size(keys);
        ASSERT_EQ(mkp->m_nkeys, nkeys) << "name=" << sname;

        for (uint32_t i(0); i < mkp->m_nkeys; i++) {
            const char   *required(*(mkeys + i));
            pfc_refptr_t *rkey;

            ASSERT_EQ(0, pfc_listm_getat(keys, i, (pfc_cptr_t *)&rkey));
            ASSERT_STREQ(required, pfc_refptr_string_value(rkey))
                << "name=" << sname << ", i=" << i;
        }

        pfc_listm_destroy(keys);
    }
}

/*
 * Test Case
 *      pfc_conf_is_defined()
 */
TEST(conf, pfc_conf_is_defined)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_bool_t bool_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	bool_ret = pfc_conf_is_defined(cfblk, "test_conf_blk_p1");
	ASSERT_EQ(bool_ret, PFC_TRUE);

	cfmap = pfc_conf_get_map(confp, "test_conf_map", "test_conf_map_key1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);
	bool_ret = pfc_conf_is_defined(cfmap, "test_conf_map_p1");
	ASSERT_EQ(bool_ret, PFC_TRUE);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	bool_ret = pfc_conf_is_defined(cfblk, "invalid_param");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	cfmap = pfc_conf_get_map(confp, "test_conf_map", "test_conf_map_key1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);
	bool_ret = pfc_conf_is_defined(cfmap, "invalid_param");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_is_defined(cfb_inv, "test_conf_blk_p1");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_is_array()
 */
TEST(conf, pfc_conf_is_array)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_bool_t bool_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	bool_ret = pfc_conf_is_array(cfblk, "test_conf_blk_p2");
	ASSERT_EQ(bool_ret, PFC_TRUE);

	cfmap = pfc_conf_get_map(confp, "test_conf_map", "test_conf_map_key1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);
	bool_ret = pfc_conf_is_defined(cfmap, "test_conf_map_p2");
	ASSERT_EQ(bool_ret, PFC_TRUE);

	bool_ret = pfc_conf_is_array(cfblk, "test_conf_blk_p1");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_is_array(cfmap, "test_conf_map_p1");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_is_array(cfblk, "invalid_param");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_is_array(cfmap, "invalid_param");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_is_array(cfb_inv, "test_conf_blk_p1");
	ASSERT_EQ(bool_ret, PFC_FALSE);

	ASSERT_EQ(PFC_FALSE, pfc_conf_is_array(cfblk, NULL));
	ASSERT_EQ(PFC_FALSE, pfc_conf_is_array(cfmap, NULL));

	pfc_conf_close(confp);
}

/*
 * Test case for pfc_conf_get_path().
 */
TEST(conf, pfc_conf_get_path)
{
	ASSERT_EQ((const char *)NULL, pfc_conf_get_path(PFC_CONF_INVALID));

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	pfc_conf_t	conf;
	ASSERT_EQ(0, pfc_conf_open(&conf, tmpf.getPath(), &test_conf_cfdef));
	ASSERT_STREQ(tmpf.getPath(), pfc_conf_get_path(conf));

	pfc_conf_close(conf);
}

/*
 * Test case for pfc_sysconf_get_path().
 */
TEST(conf, pfc_sysconf_get_path)
{
    ASSERT_EQ((const char *)NULL, pfc_sysconf_get_path());

    TmpFile tmp("pfcd_test.conf");
    ASSERT_EQ(0, tmp.createFile());

    pfc_refptr_t *rpath(pfc_refptr_string_create(tmp.getPath()));
    ASSERT_TRUE(rpath != NULL);
    RefPointer ref(rpath);

    ChildContext ctx;
    child_func_t func(boost::bind(check_sysconf_get_path, _1, rpath));

    ctx.run(func);
    RETURN_ON_ERROR();

    int status;
    ctx.wait(status);
    RETURN_ON_ERROR();
    ASSERT_EQ(0, status);
}

/*
 * Test Case
 *      pfc_sysconf_open()
 *      pfc_sysconf_get_block()
 *      pfc_sysconf_get_map()
 *      pfc_sysconf_get_mapkeys()
 */
TEST(conf, pfc_sysconf_open_get_block_map)
{
    // Create configuration file for test.
    TmpFile tmp("pfcd_test.conf");
    ASSERT_EQ(0, tmp.createFile());

    pfc_refptr_t *rpath(pfc_refptr_string_create(tmp.getPath()));
    ASSERT_TRUE(rpath != NULL);
    RefPointer ref(rpath);

    conf_sysconf_t  sysconf;
    conf_options_t *opts(&sysconf.c_options);
    map_evsrc_t    *evmap(&sysconf.c_evsrc);
    map_tpool_t    *tmap(&sysconf.c_tpool);

    sysconf.c_path = rpath;
    opts->co_log_level.assign("notice");
    opts->co_stderr_rotate = 500;
    opts->co_message_size = 12345678;

    tmp.print("options { log_level = \"%s\"; stderr_rotate = %u; "
              "message_size = %u; }\n",
              opts->co_log_level.c_str(), opts->co_stderr_rotate,
              opts->co_message_size);

    {
        conf_evsrc_t  evsrc;
        const char    *key("test_evsrc");

        evsrc.ce_nthreads = 25;
        evsrc.ce_timeout = 2800;
        (*evmap)[key] = evsrc;
        ASSERT_EQ(static_cast<size_t>(1), evmap->size());

        tmp.print("event_source \"%s\" { nthreads = %u; timeout = %u; }\n",
                  key, evsrc.ce_nthreads, evsrc.ce_timeout);
    }

    conf_tpool_t  tpool;
    tpool.ct_stack_size = 0x4000;
    tpool.ct_max_threads = 5;
    tpool.ct_max_free = 4;
    tpool.ct_min_free = 1;
    tpool.ct_reap_threads = 0;

    {
        const char    *key("default");

        (*tmap)[key] = tpool;

        tmp.print("thread_pool %s {\n"
                  "    stack_size   = %u;\n"
                  "    max_threads  = %u;\n"
                  "    max_free     = %u;\n"
                  "    min_free     = %u;\n"
                  "    reap_threads = %u;\n"
                  "}\n", key, tpool.ct_stack_size, tpool.ct_max_threads,
                  tpool.ct_max_free, tpool.ct_min_free, tpool.ct_reap_threads);
    }

    const int npools(10);
    for (int i(1); i <= npools; i++) {
        tpool.ct_stack_size += 0x2000;
        tpool.ct_max_threads += 7;
        tpool.ct_max_free += 5;
        tpool.ct_min_free += 3;
        tpool.ct_reap_threads += 11;

        char key[64];
        snprintf(key, sizeof(key), "thread_pool_%u", i);
        (*tmap)[key] = tpool;

        tmp.print("thread_pool \"%s\" {\n"
                  "    stack_size   = %u;\n"
                  "    max_threads  = %u;\n"
                  "    max_free     = %u;\n"
                  "    min_free     = %u;\n"
                  "    reap_threads = %u;\n"
                  "}\n", key, tpool.ct_stack_size, tpool.ct_max_threads,
                  tpool.ct_max_free, tpool.ct_min_free, tpool.ct_reap_threads);
    }
    ASSERT_EQ(static_cast<size_t>(npools + 1), tmap->size());
    tmp.flush();

    ChildContext ctx;
    child_func_t func(boost::bind(check_sysconf_get, _1, &sysconf));

    ctx.run(func);
    RETURN_ON_ERROR();

    int status;
    ctx.wait(status);
    RETURN_ON_ERROR();
    ASSERT_EQ(0, status);
}

/*
 * Test Case
 *      pfc_conf_get_byte()
 */
TEST(conf, pfc_conf_get_byte)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	uint8_t ui8_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_byte");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_byte",
				 "test_conf_map_byte_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ui8_ret = pfc_conf_get_byte(cfblk, "test_conf_blk_byte_oct", -1);
	ASSERT_EQ(ui8_ret, 00);
	ui8_ret = pfc_conf_get_byte(cfblk, "test_conf_blk_byte_dec", -1);
	ASSERT_EQ(ui8_ret, 0);
	ui8_ret = pfc_conf_get_byte(cfblk, "test_conf_blk_byte_hex", -1);
	ASSERT_EQ(ui8_ret, 0x00);

	ui8_ret = pfc_conf_get_byte(cfmap, "test_conf_map_byte_oct", -1);
	ASSERT_EQ(ui8_ret, UINT8_MAX);
	ui8_ret = pfc_conf_get_byte(cfmap, "test_conf_map_byte_dec", -1);
	ASSERT_EQ(ui8_ret, UINT8_MAX);
	ui8_ret = pfc_conf_get_byte(cfmap, "test_conf_map_byte_hex", -1);
	ASSERT_EQ(ui8_ret, UINT8_MAX);

	ui8_ret = pfc_conf_get_byte(cfblk, "test_conf_blk_byte_inv1", 1);
	ASSERT_EQ(ui8_ret, 1);
	ui8_ret = pfc_conf_get_byte(cfmap, "test_conf_map_byte_inv1", 1);
	ASSERT_EQ(ui8_ret, 1);

	ui8_ret = pfc_conf_get_byte(cfblk, "test_conf_blk_byte_undef", 1);
	ASSERT_EQ(ui8_ret, 1);
	ui8_ret = pfc_conf_get_byte(cfmap, "test_conf_map_byte_undef", 1);
	ASSERT_EQ(ui8_ret, 1);

	ui8_ret = pfc_conf_get_byte(cfb_inv, "test_conf_blk_byte_oct", 2);
	ASSERT_EQ(ui8_ret, 2);

	ui8_ret = pfc_conf_get_byte(cfblk, "test_conf_blk_byte_inv2", 3);
	ASSERT_EQ(ui8_ret, 3);
	ui8_ret = pfc_conf_get_byte(cfmap, "test_conf_map_byte_inv2", 3);
	ASSERT_EQ(ui8_ret, 3);

	ASSERT_EQ(11U, pfc_conf_get_byte(cfblk, NULL, 11U));
	ASSERT_EQ(12U, pfc_conf_get_byte(cfmap, NULL, 12U));
	ASSERT_EQ(13U, pfc_conf_get_byte(cfb_inv, NULL, 13U));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_uint32()
 */
TEST(conf, pfc_conf_get_uint32)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	uint32_t ui32_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ui32");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_ui32",
				 "test_conf_map_ui32_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ui32_ret = pfc_conf_get_uint32(cfblk, 
				       "test_conf_blk_ui32_oct", -1);
	ASSERT_EQ(ui32_ret, (uint32_t)00);
	ui32_ret = pfc_conf_get_uint32(cfblk,
				       "test_conf_blk_ui32_dec", -1);
	ASSERT_EQ(ui32_ret, (uint32_t)0);
	ui32_ret = pfc_conf_get_uint32(cfblk,
				       "test_conf_blk_ui32_hex", -1);
	ASSERT_EQ(ui32_ret, (uint32_t)0x00);
	ui32_ret = pfc_conf_get_uint32(cfmap,
				        "test_conf_map_ui32_oct", -1);
	ASSERT_EQ(ui32_ret, UINT32_MAX);
	ui32_ret = pfc_conf_get_uint32(cfmap,
				       "test_conf_map_ui32_dec", -1);
	ASSERT_EQ(ui32_ret, UINT32_MAX);
	ui32_ret = pfc_conf_get_uint32(cfmap,
				       "test_conf_map_ui32_hex", -1);
	ASSERT_EQ(ui32_ret, UINT32_MAX);

	ui32_ret = pfc_conf_get_uint32(cfblk,
				       "test_conf_blk_ui32_inv1", UINT32_MAX);
	ASSERT_EQ(ui32_ret, UINT32_MAX);
	ui32_ret = pfc_conf_get_uint32(cfmap,
				       "test_conf_map_ui32_inv1", 0);
	ASSERT_EQ(ui32_ret, (uint32_t)0);

	ui32_ret = pfc_conf_get_uint32(cfblk,
				       "test_conf_blk_ui32_undef", UINT32_MAX);
	ASSERT_EQ(ui32_ret, UINT32_MAX);
	ui32_ret = pfc_conf_get_uint32(cfmap,
				       "test_conf_map_ui32_undef", 0);
	ASSERT_EQ(ui32_ret, (uint32_t)0);

	ui32_ret = pfc_conf_get_uint32(cfb_inv,
				       "test_conf_blk_ui32_oct", UINT32_MAX);
	ASSERT_EQ(ui32_ret, UINT32_MAX);

	ui32_ret = pfc_conf_get_uint32(cfblk,
				       "test_conf_blk_ui32_inv2", UINT32_MAX);
	ASSERT_EQ(ui32_ret, UINT32_MAX);
	ui32_ret = pfc_conf_get_uint32(cfmap,
				       "test_conf_map_ui32_inv2", 0);
	ASSERT_EQ(ui32_ret, (uint32_t)0);

	ASSERT_EQ(11U, pfc_conf_get_uint32(cfblk, NULL, 11U));
	ASSERT_EQ(12U, pfc_conf_get_uint32(cfmap, NULL, 12U));
	ASSERT_EQ(13U, pfc_conf_get_uint32(cfb_inv, NULL, 13U));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_uint64()
 */
TEST(conf, pfc_conf_get_uint64)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	uint64_t ui64_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ui64");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_ui64",
				 "test_conf_map_ui64_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ui64_ret = pfc_conf_get_uint64(cfblk,
				       "test_conf_blk_ui64_oct", -1);
	ASSERT_EQ(ui64_ret, (uint64_t)00);
	ui64_ret = pfc_conf_get_uint64(cfblk,
				       "test_conf_blk_ui64_dec", -1);
	ASSERT_EQ(ui64_ret, (uint64_t)0);
	ui64_ret = pfc_conf_get_uint64(cfblk,
				       "test_conf_blk_ui64_hex", -1);
	ASSERT_EQ(ui64_ret, (uint64_t)0x00);
	ui64_ret = pfc_conf_get_uint64(cfmap,
				       "test_conf_map_ui64_oct", -1);
	ASSERT_EQ(ui64_ret, UINT64_MAX);
	ui64_ret = pfc_conf_get_uint64(cfmap,
				       "test_conf_map_ui64_dec", -1);
	ASSERT_EQ(ui64_ret, UINT64_MAX);
	ui64_ret = pfc_conf_get_uint64(cfmap,
				       "test_conf_map_ui64_hex", -1);
	ASSERT_EQ(ui64_ret, UINT64_MAX);

	ui64_ret = pfc_conf_get_uint64(cfblk,
				       "test_conf_blk_ui64_inv1", UINT64_MAX);
	ASSERT_EQ(ui64_ret, UINT64_MAX);
	ui64_ret = pfc_conf_get_uint64(cfmap,
				       "test_conf_map_ui64_inv1", 0);
	ASSERT_EQ(ui64_ret, (uint64_t)0);

	ui64_ret = pfc_conf_get_uint64(cfblk,
				       "test_conf_blk_ui64_undef", UINT64_MAX);
	ASSERT_EQ(ui64_ret, UINT64_MAX);
	ui64_ret = pfc_conf_get_uint64(cfmap,
				       "test_conf_map_ui64_undef", 0);
	ASSERT_EQ(ui64_ret, (uint64_t)0);

	ui64_ret = pfc_conf_get_uint64(cfb_inv,
				       "test_conf_blk_ui64_oct", UINT64_MAX);
	ASSERT_EQ(ui64_ret, UINT64_MAX);

	ui64_ret = pfc_conf_get_uint64(cfblk,
				       "test_conf_blk_ui64_inv2", UINT64_MAX);
	ASSERT_EQ(ui64_ret, UINT64_MAX);
	ui64_ret = pfc_conf_get_uint64(cfmap,
				       "test_conf_map_ui64_inv2", 0);
	ASSERT_EQ(ui64_ret, (uint64_t)0);

	ASSERT_EQ(11ULL, pfc_conf_get_uint64(cfblk, NULL, 11ULL));
	ASSERT_EQ(12ULL, pfc_conf_get_uint64(cfmap, NULL, 12ULL));
	ASSERT_EQ(13ULL, pfc_conf_get_uint64(cfb_inv, NULL, 13ULL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_ulong()
 */
TEST(conf, pfc_conf_get_ulong)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_ulong_t ulng_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ulng");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_ulng",
				 "test_conf_map_ulng_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ulng_ret = pfc_conf_get_ulong(cfblk, "test_conf_blk_ulng_oct", -1);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)00);
	ulng_ret = pfc_conf_get_ulong(cfblk, "test_conf_blk_ulng_dec", -1);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)0);
	ulng_ret = pfc_conf_get_ulong(cfblk, "test_conf_blk_ulng_hex", -1);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)0x00);
	ulng_ret = pfc_conf_get_ulong(cfmap, "test_conf_map_ulng_oct", -1);
	ASSERT_EQ(ulng_ret, PFC_ULONG_MAX);
	ulng_ret = pfc_conf_get_ulong(cfmap, "test_conf_map_ulng_dec", -1);
	ASSERT_EQ(ulng_ret, PFC_ULONG_MAX);
	ulng_ret = pfc_conf_get_ulong(cfmap, "test_conf_map_ulng_hex", -1);
	ASSERT_EQ(ulng_ret, PFC_ULONG_MAX);

	ulng_ret = pfc_conf_get_ulong(cfblk, "test_conf_blk_ulng_inv1", 0);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)0);
	ulng_ret = pfc_conf_get_ulong(cfmap, "test_conf_map_ulng_inv1", 0);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)0);

	ulng_ret = pfc_conf_get_ulong(cfblk, "test_conf_blk_ulng_undef", 0);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)0);
	ulng_ret = pfc_conf_get_ulong(cfmap, "test_conf_map_ulng_undef", 0);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)0);

	ulng_ret = pfc_conf_get_ulong(cfb_inv, "test_conf_blk_ulng_oct", 1);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)1);

	ulng_ret = pfc_conf_get_ulong(cfblk, "test_conf_blk_ulng_inv2", 2);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)2);
	ulng_ret = pfc_conf_get_ulong(cfmap, "test_conf_map_ulng_inv2", 2);
	ASSERT_EQ(ulng_ret, (pfc_ulong_t)2);

	ASSERT_EQ(11UL, pfc_conf_get_ulong(cfblk, NULL, 11UL));
	ASSERT_EQ(12UL, pfc_conf_get_ulong(cfmap, NULL, 12UL));
	ASSERT_EQ(13UL, pfc_conf_get_ulong(cfb_inv, NULL, 13UL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_int32()
 */
TEST(conf, pfc_conf_get_int32)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	int32_t i32_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i32_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_i32",
				 "test_conf_map_i32_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_oct", 0);
	ASSERT_EQ(i32_ret, INT32_MIN);
	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_dec", 0);
	ASSERT_EQ(i32_ret, INT32_MIN);
	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_hex", 0);
	ASSERT_EQ(i32_ret, INT32_MIN);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_oct", 0);
	ASSERT_EQ(i32_ret, INT32_MIN);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_dec", 0);
	ASSERT_EQ(i32_ret, INT32_MIN);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_hex", 0);
	ASSERT_EQ(i32_ret, INT32_MIN);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i32_max");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_i32",
				 "test_conf_map_i32_k2");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_oct", 0);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_dec", 0);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_hex", 0);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_oct", 0);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_dec", 0);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_hex", 0);
	ASSERT_EQ(i32_ret, INT32_MAX);

	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_inv1",
				     INT32_MAX);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_inv1",
				     INT32_MIN);
	ASSERT_EQ(i32_ret, INT32_MIN);

	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_undef",
				     INT32_MAX);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_undef",
				     INT32_MIN);
	ASSERT_EQ(i32_ret, INT32_MIN);

	i32_ret = pfc_conf_get_int32(cfb_inv, "test_conf_blk_i32_oct",
				     INT32_MAX);
	ASSERT_EQ(i32_ret, INT32_MAX);

	i32_ret = pfc_conf_get_int32(cfblk, "test_conf_blk_i32_inv2",
				     INT32_MAX);
	ASSERT_EQ(i32_ret, INT32_MAX);
	i32_ret = pfc_conf_get_int32(cfmap, "test_conf_map_i32_inv2",
				     INT32_MIN);
	ASSERT_EQ(i32_ret, INT32_MIN);

	ASSERT_EQ(11, pfc_conf_get_int32(cfblk, NULL, 11));
	ASSERT_EQ(12, pfc_conf_get_int32(cfmap, NULL, 12));
	ASSERT_EQ(13, pfc_conf_get_int32(cfb_inv, NULL, 13));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_int64()
 */
TEST(conf, pfc_conf_get_int64)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	int64_t i64_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i64_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_i64",
				 "test_conf_map_i64_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_oct", 0);
	ASSERT_EQ(i64_ret, INT64_MIN);
	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_dec", 0);
	ASSERT_EQ(i64_ret, INT64_MIN);
	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_hex", 0);
	ASSERT_EQ(i64_ret, INT64_MIN);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_oct", 0);
	ASSERT_EQ(i64_ret, INT64_MIN);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_dec", 0);
	ASSERT_EQ(i64_ret, INT64_MIN);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_hex", 0);
	ASSERT_EQ(i64_ret, INT64_MIN);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i64_max");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_i64",
				 "test_conf_map_i64_k2");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_oct", 0);
	ASSERT_EQ(i64_ret, INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_dec", 0);
	ASSERT_EQ(i64_ret, INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_hex", 0);
	ASSERT_EQ(i64_ret, INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_oct", 0);
	ASSERT_EQ(i64_ret, INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_dec", 0);
	ASSERT_EQ(i64_ret, INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_hex", 0);
	ASSERT_EQ(i64_ret, INT64_MAX);

	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_inv1",
				     INT64_MAX);
	ASSERT_EQ(i64_ret,INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_inv1",
				     INT64_MIN);
	ASSERT_EQ(i64_ret, INT64_MIN);

	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_undef",
				     INT64_MAX);
	ASSERT_EQ(i64_ret,INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_undef",
				     INT64_MIN);
	ASSERT_EQ(i64_ret, INT64_MIN);

	i64_ret = pfc_conf_get_int64(cfb_inv, "test_conf_blk_i64_oct",
				      INT64_MAX);
	ASSERT_EQ(i64_ret, INT64_MAX);

	i64_ret = pfc_conf_get_int64(cfblk, "test_conf_blk_i64_inv2",
				       INT64_MAX);
	ASSERT_EQ(i64_ret, INT64_MAX);
	i64_ret = pfc_conf_get_int64(cfmap, "test_conf_map_i64_inv2",
				        INT64_MIN);
	ASSERT_EQ(i64_ret, INT64_MIN);

	ASSERT_EQ(11LL, pfc_conf_get_int64(cfblk, NULL, 11LL));
	ASSERT_EQ(12LL, pfc_conf_get_int64(cfmap, NULL, 12LL));
	ASSERT_EQ(13LL, pfc_conf_get_int64(cfb_inv, NULL, 13LL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_long()
 */
TEST(conf, pfc_conf_get_long)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_long_t lng_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk_lng_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_lng",
				 "test_conf_map_lng_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_oct", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);
	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_dec", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);
	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_hex", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_oct", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_dec", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_hex", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);

	cfblk = pfc_conf_get_block(confp, "test_conf_blk_lng_max");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_lng",
				 "test_conf_map_lng_k2");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_oct", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_dec", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_hex", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_oct", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_dec", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_hex", 0);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);

	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_inv1",
				    PFC_LONG_MAX);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_inv1",
				    PFC_LONG_MIN);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);

	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_undef",
				    PFC_LONG_MAX);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_undef",
				    PFC_LONG_MIN);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);

	lng_ret = pfc_conf_get_long(cfb_inv, "test_conf_blk_lng_oct",
				    PFC_LONG_MAX);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);

	lng_ret = pfc_conf_get_long(cfblk, "test_conf_blk_lng_inv2",
				    PFC_LONG_MAX);
	ASSERT_EQ(lng_ret, PFC_LONG_MAX);
	lng_ret = pfc_conf_get_long(cfmap, "test_conf_map_lng_inv2",
				    PFC_LONG_MIN);
	ASSERT_EQ(lng_ret, PFC_LONG_MIN);

	ASSERT_EQ(11L, pfc_conf_get_long(cfblk, NULL, 11L));
	ASSERT_EQ(12L, pfc_conf_get_long(cfmap, NULL, 12L));
	ASSERT_EQ(13L, pfc_conf_get_long(cfb_inv, NULL, 13L));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_string()
 */
test_case_get_str_t tc_get_str[TEST_CONF_HDL_CNT][GET_STRING_TEST_COUNT] = {
	/*
	 * from block
	 */
	{{"test_conf_blk_str", "test_string_blk"},
	/* different type1 */
	 {"test_conf_blk_str_inv1", defvalue_string},
	/* different type2 */
	 {"test_conf_blk_str_inv2", defvalue_string},
	/* undefined Parameter */
	 {TEST_CONF_UNDEF_PARAM, defvalue_string}},

	/*
	 * from map
	 */
	{{"test_conf_map_str", "test_string_map"},
	/* different type1 */
	 {"test_conf_map_str_inv1", defvalue_string},
	/* different type1 */
	 {"test_conf_map_str_inv2", defvalue_string},
	/* undefined Parameter */
	 {TEST_CONF_UNDEF_PARAM, defvalue_string}}
};


TEST(conf, pfc_conf_get_string)
{
	int ret, i, j;
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];
	pfc_conf_t confp;
	const char *str_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cf[0] = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);
	cf[1] = pfc_conf_get_map(confp, "test_conf_map_str",
				 "test_conf_map_str_k1");
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {
		for (j = 0; j < GET_STRING_TEST_COUNT; j++) {
			str_ret = pfc_conf_get_string(cf[i],
						      tc_get_str[i][j].param,
						      defvalue_string);
			ASSERT_STREQ(str_ret, tc_get_str[i][j].expect);

			ASSERT_EQ(defvalue_string,
				  pfc_conf_get_string(cf[i], NULL,
						      defvalue_string));
		}
	}

	/*
	 * Invalid handle
	 *     Handle and Parameter value are no object.
	 */
	str_ret = pfc_conf_get_string(PFC_CFBLK_INVALID,
				      tc_get_str[0][0].param,
				      tc_get_str[0][0].expect);
	ASSERT_STREQ(str_ret, tc_get_str[0][0].expect);

	ASSERT_EQ(defvalue_string, pfc_conf_get_string(PFC_CFBLK_INVALID,
						       NULL, defvalue_string));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_stringat()
 */
test_case_ary_str_t tc_ary_str[TEST_CONF_HDL_CNT][ARY_STRING_TEST_COUNT]=
	{
	/*
	 * from block
	 */
	{{"test_conf_blk_str_array1", "test_string-1", TEST_CONF_MIN_INDEX},
	 {"test_conf_blk_str_array2", "test_string-1", TEST_CONF_MIN_INDEX},
	/* different type1 */
	 {"test_conf_blk_str_array_inv1", defvalue_string,
	  TEST_CONF_MIN_INDEX},
	/* different type2 */
	 {"test_conf_blk_str_array_inv2", defvalue_string,
	  TEST_CONF_MIN_INDEX},
	/* invalid array index */
	 {"test_conf_blk_str_array1", defvalue_string,
	  TEST_CONF_MAX_ARRAY_SIZE}},

	/*
	 * from map
	 */
	{{"test_conf_map_str_array1", "test_string-6", TEST_CONF_MAX_INDEX},
	 {"test_conf_map_str_array2", "test_string-6", TEST_CONF_MAX_INDEX},
	/* different type1 */
	 {"test_conf_map_str_array_inv1", defvalue_string,
	  TEST_CONF_MIN_INDEX},
	/* different type2 */
	 {"test_conf_map_str_array_inv2", defvalue_string,
	  TEST_CONF_MIN_INDEX},
	/* invalid array index */
	 {"test_conf_blk_str_array1", defvalue_string, UINT32_MAX}}
};

TEST(conf, pfc_conf_array_stringat)
{
	int ret, i, j;
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];
	pfc_conf_t confp;
	const char *str_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cf[0] = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);
	cf[1] = pfc_conf_get_map(confp, "test_conf_map_str",
				 "test_conf_map_str_k1");
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {
		for (j = 0; j < GET_STRING_TEST_COUNT; j++) {
			str_ret 
			 = pfc_conf_array_stringat(cf[i],
						   tc_ary_str[i][j].param,
						   tc_ary_str[i][j].index, 
						   defvalue_string);
			ASSERT_STREQ(str_ret, tc_ary_str[i][j].expect);

			str_ret = pfc_conf_array_stringat
				(cf[i], NULL, tc_ary_str[i][j].index,
				 defvalue_string);
			ASSERT_EQ(defvalue_string, str_ret);
		}
	}

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	str_ret = pfc_conf_array_stringat(PFC_CFBLK_INVALID,
					  tc_ary_str[0][0].param,
					  tc_ary_str[0][0].index,
					  tc_ary_str[0][0].expect);
	ASSERT_STREQ(str_ret, tc_ary_str[0][0].expect);

	str_ret = pfc_conf_array_stringat(PFC_CFBLK_INVALID, NULL,
					  tc_ary_str[0][0].index,
					  defvalue_string);
	ASSERT_EQ(defvalue_string, str_ret);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_get_bool()
 */
TEST(conf, pfc_conf_get_bool)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_bool_t bool_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_bool");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_bool",
				 "test_conf_map_bool_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	bool_ret = pfc_conf_get_bool(cfblk, "test_conf_blk_bool",
				     PFC_FALSE);
	ASSERT_EQ(bool_ret, PFC_TRUE);
	bool_ret = pfc_conf_get_bool(cfmap, "test_conf_map_bool",
				     PFC_TRUE);
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_get_bool(cfblk, "test_conf_blk_bool_inv1",
				     PFC_FALSE);
	ASSERT_EQ(bool_ret, PFC_FALSE);
	bool_ret = pfc_conf_get_bool(cfmap, "test_conf_map_bool_inv1",
				     PFC_TRUE);
	ASSERT_EQ(bool_ret, PFC_TRUE);

	bool_ret = pfc_conf_get_bool(cfblk, "test_conf_blk_bool_undef",
				     PFC_FALSE);
	ASSERT_EQ(bool_ret, PFC_FALSE);
	bool_ret = pfc_conf_get_bool(cfmap, "test_conf_map_bool_undef",
				     PFC_TRUE);
	ASSERT_EQ(bool_ret, PFC_TRUE);

	bool_ret = pfc_conf_get_bool(cfb_inv, "test_conf_blk_bool", PFC_FALSE);
	ASSERT_EQ(bool_ret, PFC_FALSE);

	bool_ret = pfc_conf_get_bool(cfblk, "test_conf_blk_bool_inv2",
				     PFC_FALSE);
	ASSERT_EQ(bool_ret, PFC_FALSE);
	bool_ret = pfc_conf_get_bool(cfmap, "test_conf_map_bool_inv2",
				     PFC_TRUE);
	ASSERT_EQ(bool_ret, PFC_TRUE);

	ASSERT_EQ(PFC_FALSE, pfc_conf_get_bool(cfblk, NULL, PFC_FALSE));
	ASSERT_EQ(PFC_FALSE, pfc_conf_get_bool(cfmap, NULL, PFC_FALSE));
	ASSERT_EQ(PFC_FALSE, pfc_conf_get_bool(cfb_inv, NULL, PFC_FALSE));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_size()
 */
TEST(conf, pfc_conf_array_size)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	int size_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_array_size");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_array_size",
				 "test_conf_map_array_size_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	size_ret = pfc_conf_array_size(cfblk, "test_conf_blk_array_size_p1");
	ASSERT_EQ(size_ret, TEST_CONF_MAX_ARRAY_SIZE);
	size_ret = pfc_conf_array_size(cfmap, "test_conf_map_array_size_p1");
	ASSERT_EQ(size_ret, TEST_CONF_MAX_ARRAY_SIZE);

	size_ret = pfc_conf_array_size(cfblk, "test_conf_blk_array_size_p2");
	ASSERT_EQ(size_ret, TEST_CONF_MAX_ARRAY_SIZE);
	size_ret = pfc_conf_array_size(cfmap, "test_conf_map_array_size_p2");
	ASSERT_EQ(size_ret, TEST_CONF_MAX_ARRAY_SIZE);

	size_ret = pfc_conf_array_size(cfblk, "test_conf_blk_array_size_inv1");
	ASSERT_EQ(size_ret, -1);
	size_ret = pfc_conf_array_size(cfmap, "test_conf_map_array_size_inv1");
	ASSERT_EQ(size_ret, -1);

	size_ret = pfc_conf_array_size(cfblk, "test_conf_blk_array_size_inv2");
	ASSERT_EQ(size_ret, -1);
	size_ret = pfc_conf_array_size(cfmap, "test_conf_map_array_size_inv2");
	ASSERT_EQ(size_ret, -1);

	size_ret = pfc_conf_array_size(cfb_inv, "test_conf_blk_array_size_p1");
	ASSERT_EQ(size_ret, -1);

	ASSERT_EQ(-1, pfc_conf_array_size(cfblk, NULL));
	ASSERT_EQ(-1, pfc_conf_array_size(cfmap, NULL));
	ASSERT_EQ(-1, pfc_conf_array_size(cfb_inv, NULL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_byteat()
 */
TEST(conf, pfc_conf_array_byteat)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	uint8_t byteat_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_byte");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_byte",
				 "test_conf_map_byte_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	byteat_ret = pfc_conf_array_byteat(cfblk, "test_conf_blk_byte_array1",
					   TEST_CONF_MIN_INDEX, UINT8_MAX);
	ASSERT_EQ(byteat_ret, 0);
	byteat_ret = pfc_conf_array_byteat(cfmap, "test_conf_map_byte_array1",
					   TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(byteat_ret, UINT8_MAX);

	byteat_ret = pfc_conf_array_byteat(cfblk, "test_conf_blk_byte_array2",
					   TEST_CONF_MIN_INDEX, UINT8_MAX);
	ASSERT_EQ(byteat_ret, 0);
	byteat_ret = pfc_conf_array_byteat(cfmap, "test_conf_map_byte_array2",
					   TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(byteat_ret, UINT8_MAX);

	byteat_ret = pfc_conf_array_byteat(cfblk,
					   "test_conf_blk_byte_array_inv1",
					   TEST_CONF_MIN_INDEX, UINT8_MAX);
	ASSERT_EQ(byteat_ret, UINT8_MAX);
	byteat_ret = pfc_conf_array_byteat(cfmap,
					   "test_conf_map_byte_array_inv1",
					   TEST_CONF_MIN_INDEX, 0);
	ASSERT_EQ(byteat_ret, 0);

	byteat_ret = pfc_conf_array_byteat(cfblk,
					   "test_conf_blk_byte_array_inv2",
					   TEST_CONF_MIN_INDEX, UINT8_MAX);
	ASSERT_EQ(byteat_ret, UINT8_MAX);
	byteat_ret = pfc_conf_array_byteat(cfmap,
					   "test_conf_map_byte_array_inv2",
					   TEST_CONF_MIN_INDEX, 0);
	ASSERT_EQ(byteat_ret, 0);

	/* invalid index */
	byteat_ret = pfc_conf_array_byteat(cfblk, "test_conf_blk_byte_array1",
					   TEST_CONF_MAX_ARRAY_SIZE,
					   UINT8_MAX);
	ASSERT_EQ(byteat_ret, UINT8_MAX);
	byteat_ret = pfc_conf_array_byteat(cfmap, "test_conf_map_byte_array1",
					   UINT32_MAX, 0);
	ASSERT_EQ(byteat_ret, 0);

	byteat_ret = pfc_conf_array_byteat(cfb_inv,
					   "test_conf_blk_byte_array1",
					   TEST_CONF_MIN_INDEX,
					   UINT8_MAX);
	ASSERT_EQ(byteat_ret, UINT8_MAX);

	ASSERT_EQ(0U, pfc_conf_array_byteat(cfblk, NULL,
					    TEST_CONF_MIN_INDEX, 0U));
	ASSERT_EQ(1U, pfc_conf_array_byteat(cfmap, NULL,
					    TEST_CONF_MIN_INDEX, 1U));
	ASSERT_EQ(2U, pfc_conf_array_byteat(cfb_inv, NULL,
					    TEST_CONF_MIN_INDEX, 2U));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_boolat()
 */
TEST(conf, pfc_conf_array_boolat)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_bool_t boolat_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_bool");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_bool",
				 "test_conf_map_bool_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	boolat_ret = pfc_conf_array_boolat(cfblk,
					   "test_conf_blk_bool_array1", 
					   TEST_CONF_MIN_INDEX, PFC_FALSE);
	ASSERT_EQ(boolat_ret, PFC_TRUE);
	boolat_ret = pfc_conf_array_boolat(cfmap,
					   "test_conf_map_bool_array1",
					   TEST_CONF_MAX_INDEX, PFC_TRUE);
	ASSERT_EQ(boolat_ret, PFC_FALSE);

	boolat_ret = pfc_conf_array_boolat(cfblk,
					   "test_conf_blk_bool_array2",
					   TEST_CONF_MIN_INDEX, PFC_FALSE);
	ASSERT_EQ(boolat_ret, PFC_TRUE);
	boolat_ret = pfc_conf_array_boolat(cfmap,
					   "test_conf_map_bool_array2",
					   TEST_CONF_MAX_INDEX, PFC_TRUE);
	ASSERT_EQ(boolat_ret, PFC_FALSE);

	boolat_ret = pfc_conf_array_boolat(cfblk,
					   "test_conf_blk_bool_array_inv1",
					   TEST_CONF_MIN_INDEX, PFC_TRUE);
	ASSERT_EQ(boolat_ret, PFC_TRUE);
	boolat_ret = pfc_conf_array_boolat(cfmap,
					   "test_conf_map_bool_array_inv1",
					   TEST_CONF_MIN_INDEX, PFC_FALSE);
	ASSERT_EQ(boolat_ret, PFC_FALSE);

	boolat_ret = pfc_conf_array_boolat(cfblk,
					   "test_conf_blk_bool_array_inv2",
					   TEST_CONF_MIN_INDEX, PFC_FALSE);
	ASSERT_EQ(boolat_ret, PFC_FALSE);
	boolat_ret = pfc_conf_array_boolat(cfmap,
					   "test_conf_map_bool_array_inv2",
					   TEST_CONF_MIN_INDEX, PFC_TRUE);
	ASSERT_EQ(boolat_ret, PFC_TRUE);

	/* invalid index */
	boolat_ret = pfc_conf_array_boolat(cfblk, "test_conf_blk_bool_array1",
					   TEST_CONF_MAX_ARRAY_SIZE,
					   PFC_FALSE);
	ASSERT_EQ(boolat_ret, PFC_FALSE);
	boolat_ret = pfc_conf_array_boolat(cfmap, "test_conf_map_bool_array1",
					   UINT32_MAX, PFC_TRUE);
	ASSERT_EQ(boolat_ret, PFC_TRUE);

	boolat_ret = pfc_conf_array_boolat(cfb_inv, 
					   "test_conf_blk_bool_array1",
					   TEST_CONF_MIN_INDEX, PFC_TRUE);
	ASSERT_EQ(boolat_ret, PFC_TRUE);

	ASSERT_EQ(PFC_FALSE, pfc_conf_array_boolat(cfblk, NULL,
						   TEST_CONF_MIN_INDEX,
						   PFC_FALSE));
	ASSERT_EQ(PFC_FALSE, pfc_conf_array_boolat(cfmap, NULL,
						   TEST_CONF_MIN_INDEX,
						   PFC_FALSE));
	ASSERT_EQ(PFC_FALSE, pfc_conf_array_boolat(cfb_inv, NULL,
						   TEST_CONF_MIN_INDEX,
						   PFC_FALSE));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_int32at()
 */
TEST(conf, pfc_conf_array_int32at)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	int32_t i32at_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i32_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_i32", 
				 "test_conf_map_i32_k2");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	i32at_ret = pfc_conf_array_int32at(cfblk, "test_conf_blk_i32_array1",
					   TEST_CONF_MIN_INDEX, INT32_MAX);
	ASSERT_EQ(i32at_ret, INT32_MIN);
	i32at_ret = pfc_conf_array_int32at(cfmap, "test_conf_map_i32_array1",
					   TEST_CONF_MAX_INDEX, INT32_MIN);
	ASSERT_EQ(i32at_ret, INT32_MAX);

	i32at_ret = pfc_conf_array_int32at(cfblk, "test_conf_blk_i32_array2",
					   TEST_CONF_MIN_INDEX, INT32_MAX);
	ASSERT_EQ(i32at_ret, INT32_MIN);
	i32at_ret = pfc_conf_array_int32at(cfmap, "test_conf_map_i32_array2",
					   TEST_CONF_MAX_INDEX, INT32_MIN);
	ASSERT_EQ(i32at_ret, INT32_MAX);

	i32at_ret = pfc_conf_array_int32at(cfblk,
					   "test_conf_blk_i32_array_inv1", 
					   TEST_CONF_MIN_INDEX, INT32_MAX);
	ASSERT_EQ(i32at_ret, INT32_MAX);
	i32at_ret = pfc_conf_array_int32at(cfmap,
					   "test_conf_map_i32_array_inv1",
					   TEST_CONF_MAX_INDEX, INT32_MIN);
	ASSERT_EQ(i32at_ret, INT32_MIN);

	i32at_ret = pfc_conf_array_int32at(cfblk,
					   "test_conf_blk_i32_array_inv2",
					   TEST_CONF_MIN_INDEX, INT32_MAX);
	ASSERT_EQ(i32at_ret, INT32_MAX);
	i32at_ret = pfc_conf_array_int32at(cfmap,
					   "test_conf_map_i32_array_inv2",
					   TEST_CONF_MAX_INDEX, INT32_MIN);
	ASSERT_EQ(i32at_ret, INT32_MIN);

	/* invalid index */
	i32at_ret = pfc_conf_array_int32at(cfblk, "test_conf_blk_i32_array1",
					   TEST_CONF_MAX_ARRAY_SIZE,
					   INT32_MAX);
	ASSERT_EQ(i32at_ret, INT32_MAX);
	i32at_ret = pfc_conf_array_int32at(cfmap, "test_conf_map_i32_array1",
					   UINT32_MAX, INT32_MIN);
	ASSERT_EQ(i32at_ret, INT32_MIN);

	i32at_ret = pfc_conf_array_int32at(cfb_inv, "test_conf_blk_i32_array1",
					   TEST_CONF_MIN_INDEX, INT32_MAX);
	ASSERT_EQ(i32at_ret, INT32_MAX);

	ASSERT_EQ(0, pfc_conf_array_int32at(cfblk, NULL,
					    TEST_CONF_MIN_INDEX, 0));
	ASSERT_EQ(1, pfc_conf_array_int32at(cfmap, NULL,
					    TEST_CONF_MIN_INDEX, 1));
	ASSERT_EQ(2, pfc_conf_array_int32at(cfb_inv, NULL,
					    TEST_CONF_MIN_INDEX, 2));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_uint32at()
 */
TEST(conf, pfc_conf_array_uint32at)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	uint32_t ui32at_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ui32");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_ui32",
				 "test_conf_map_ui32_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ui32at_ret = pfc_conf_array_uint32at(cfblk,
					     "test_conf_blk_ui32_array1",
					     TEST_CONF_MIN_INDEX, UINT32_MAX);
	ASSERT_EQ(ui32at_ret, (uint32_t)0);
	ui32at_ret = pfc_conf_array_uint32at(cfmap,
					     "test_conf_map_ui32_array1",
					     TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ui32at_ret, UINT32_MAX);

	ui32at_ret = pfc_conf_array_uint32at(cfblk,
					     "test_conf_blk_ui32_array2",
					     TEST_CONF_MIN_INDEX, UINT32_MAX);
	ASSERT_EQ(ui32at_ret, (uint32_t)0);
	ui32at_ret = pfc_conf_array_uint32at(cfmap,
					     "test_conf_map_ui32_array2",
					     TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ui32at_ret, UINT32_MAX);

	ui32at_ret = pfc_conf_array_uint32at(cfblk,
					     "test_conf_blk_ui32_array_inv1",
					     TEST_CONF_MIN_INDEX, UINT32_MAX);
	ASSERT_EQ(ui32at_ret, UINT32_MAX);
	ui32at_ret = pfc_conf_array_uint32at(cfmap,
					     "test_conf_map_ui32_array_inv1",
					     TEST_CONF_MIN_INDEX, 0);
	ASSERT_EQ(ui32at_ret, (uint32_t)0);

	ui32at_ret = pfc_conf_array_uint32at(cfblk,
					     "test_conf_blk_ui32_array_inv2",
					     TEST_CONF_MIN_INDEX, UINT32_MAX);
	ASSERT_EQ(ui32at_ret, UINT32_MAX);
	ui32at_ret = pfc_conf_array_uint32at(cfmap,
					     "test_conf_map_ui32_array_inv2",
					     TEST_CONF_MIN_INDEX, 0);
	ASSERT_EQ(ui32at_ret, (uint32_t)0);

	/* invalid index */
	ui32at_ret = pfc_conf_array_uint32at(cfblk,
					     "test_conf_blk_ui32_array1",
					     TEST_CONF_MAX_ARRAY_SIZE,
					     UINT32_MAX);
	ASSERT_EQ(ui32at_ret, UINT32_MAX);
	ui32at_ret = pfc_conf_array_uint32at(cfmap,
					     "test_conf_map_ui32_array1",
					     UINT32_MAX, 0);
	ASSERT_EQ(ui32at_ret, (uint32_t)0);

	ui32at_ret = pfc_conf_array_uint32at(cfb_inv,
					     "test_conf_blk_ui32_array1",
					     TEST_CONF_MIN_INDEX, UINT32_MAX);
	ASSERT_EQ(ui32at_ret, UINT32_MAX);

	ASSERT_EQ(0U, pfc_conf_array_uint32at(cfblk, NULL,
					      TEST_CONF_MIN_INDEX, 0U));
	ASSERT_EQ(1U, pfc_conf_array_uint32at(cfmap, NULL,
					      TEST_CONF_MIN_INDEX, 1U));
	ASSERT_EQ(2U, pfc_conf_array_uint32at(cfb_inv, NULL,
					      TEST_CONF_MIN_INDEX, 2U));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_int64at()
 */
TEST(conf, pfc_conf_array_int64at)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	int64_t i64at_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i64_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_i64", 
				 "test_conf_map_i64_k2");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	i64at_ret = pfc_conf_array_int64at(cfblk, "test_conf_blk_i64_array1",
					   TEST_CONF_MIN_INDEX, INT64_MAX);
	ASSERT_EQ(i64at_ret, INT64_MIN);
	i64at_ret = pfc_conf_array_int64at(cfmap, "test_conf_map_i64_array1",
					   TEST_CONF_MAX_INDEX, INT64_MIN);
	ASSERT_EQ(i64at_ret, INT64_MAX);

	i64at_ret = pfc_conf_array_int64at(cfblk, "test_conf_blk_i64_array2",
					   TEST_CONF_MIN_INDEX, INT64_MAX);
	ASSERT_EQ(i64at_ret, INT64_MIN);
	i64at_ret = pfc_conf_array_int64at(cfmap, "test_conf_map_i64_array2",
					   TEST_CONF_MAX_INDEX, INT64_MIN);
	ASSERT_EQ(i64at_ret, INT64_MAX);

	i64at_ret = pfc_conf_array_int64at(cfblk,
					   "test_conf_blk_i64_array_inv1", 0,
					   INT64_MAX);
	ASSERT_EQ(i64at_ret, INT64_MAX);
	i64at_ret = pfc_conf_array_int64at(cfmap,
					   "test_conf_map_i64_array_inv1",
					   TEST_CONF_MAX_INDEX, INT64_MIN);
	ASSERT_EQ(i64at_ret, INT64_MIN);

	i64at_ret = pfc_conf_array_int64at(cfblk,
					   "test_conf_blk_i64_array_inv2",
					   TEST_CONF_MIN_INDEX, INT64_MAX);
	ASSERT_EQ(i64at_ret, INT64_MAX);
	i64at_ret = pfc_conf_array_int64at(cfmap,
					   "test_conf_map_i64_array_inv2",
					   TEST_CONF_MAX_INDEX, INT64_MIN);
	ASSERT_EQ(i64at_ret, INT64_MIN);

	/* invalid index */
	i64at_ret = pfc_conf_array_int64at(cfblk, "test_conf_blk_i64_array1",
					   TEST_CONF_MAX_ARRAY_SIZE,
					   INT64_MAX);
	ASSERT_EQ(i64at_ret, INT64_MAX);
	i64at_ret = pfc_conf_array_int64at(cfmap, "test_conf_map_i64_array1",
					   UINT32_MAX, INT64_MIN);
	ASSERT_EQ(i64at_ret, INT64_MIN);

	i64at_ret = pfc_conf_array_int64at(cfb_inv, "test_conf_blk_i64_array1",
					   TEST_CONF_MIN_INDEX, INT64_MAX);
	ASSERT_EQ(i64at_ret, INT64_MAX);

	ASSERT_EQ(0LL, pfc_conf_array_int64at(cfblk, NULL,
					      TEST_CONF_MIN_INDEX, 0LL));
	ASSERT_EQ(1LL, pfc_conf_array_int64at(cfmap, NULL,
					      TEST_CONF_MIN_INDEX, 1LL));
	ASSERT_EQ(2LL, pfc_conf_array_int64at(cfb_inv, NULL,
					      TEST_CONF_MIN_INDEX, 2LL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_uint64at()
 */
TEST(conf, pfc_conf_array_uint64at)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	uint64_t ui64at_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ui64");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_ui64",
				 "test_conf_map_ui64_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ui64at_ret = pfc_conf_array_uint64at(cfblk,
					     "test_conf_blk_ui64_array1",
					     TEST_CONF_MIN_INDEX, UINT64_MAX);
	ASSERT_EQ(ui64at_ret, (uint64_t)0);
	ui64at_ret = pfc_conf_array_uint64at(cfmap,
					     "test_conf_map_ui64_array1",
					     TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ui64at_ret, UINT64_MAX);

	ui64at_ret = pfc_conf_array_uint64at(cfblk,
					     "test_conf_blk_ui64_array2",
					     TEST_CONF_MIN_INDEX, UINT64_MAX);
	ASSERT_EQ(ui64at_ret, (uint64_t)0);
	ui64at_ret = pfc_conf_array_uint64at(cfmap,
					     "test_conf_map_ui64_array2",
					     TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ui64at_ret, UINT64_MAX);

	ui64at_ret = pfc_conf_array_uint64at(cfblk,
					     "test_conf_blk_ui64_array_inv1",
					     TEST_CONF_MIN_INDEX, UINT64_MAX);
	ASSERT_EQ(ui64at_ret, UINT64_MAX);
	ui64at_ret = pfc_conf_array_uint64at(cfmap,
					     "test_conf_map_ui64_array_inv1",
					     TEST_CONF_MIN_INDEX, 0);
	ASSERT_EQ(ui64at_ret, (uint64_t)0);

	ui64at_ret = pfc_conf_array_uint64at(cfblk,
					     "test_conf_blk_ui64_array_inv2",
					     TEST_CONF_MIN_INDEX, UINT64_MAX);
	ASSERT_EQ(ui64at_ret, UINT64_MAX);
	ui64at_ret = pfc_conf_array_uint64at(cfmap,
					     "test_conf_map_ui64_array_inv2",
					     TEST_CONF_MIN_INDEX, 0);
	ASSERT_EQ(ui64at_ret, (uint64_t)0);

	/* invalid index */
	ui64at_ret = pfc_conf_array_uint64at(cfblk,
					     "test_conf_blk_ui64_array1",
					     TEST_CONF_MAX_ARRAY_SIZE,
					     UINT64_MAX);
	ASSERT_EQ(ui64at_ret, UINT64_MAX);
	ui64at_ret = pfc_conf_array_uint64at(cfmap,
					     "test_conf_map_ui64_array1",
					     UINT32_MAX, 0);
	ASSERT_EQ(ui64at_ret, (uint64_t)0);

	ui64at_ret = pfc_conf_array_uint64at(cfb_inv,
					     "test_conf_blk_ui64_array1",
					     TEST_CONF_MIN_INDEX, UINT64_MAX);
	ASSERT_EQ(ui64at_ret, UINT64_MAX);

	ASSERT_EQ(0ULL, pfc_conf_array_uint64at(cfblk, NULL,
						TEST_CONF_MIN_INDEX, 0ULL));
	ASSERT_EQ(1ULL, pfc_conf_array_uint64at(cfmap, NULL,
						TEST_CONF_MIN_INDEX, 1ULL));
	ASSERT_EQ(2ULL, pfc_conf_array_uint64at(cfb_inv, NULL,
						TEST_CONF_MIN_INDEX, 2ULL));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_longat()
 */
TEST(conf, pfc_conf_array_longat)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_long_t lngat_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_lng_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_lng", 
				 "test_conf_map_lng_k2");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	lngat_ret = pfc_conf_array_longat(cfblk, "test_conf_blk_lng_array1",
					  TEST_CONF_MIN_INDEX, PFC_LONG_MAX);
	ASSERT_EQ(lngat_ret, PFC_LONG_MIN);
	lngat_ret = pfc_conf_array_longat(cfmap, "test_conf_map_lng_array1",
					  TEST_CONF_MAX_INDEX, PFC_LONG_MIN);
	ASSERT_EQ(lngat_ret, PFC_LONG_MAX);

	lngat_ret = pfc_conf_array_longat(cfblk, "test_conf_blk_lng_array2",
					  TEST_CONF_MIN_INDEX, PFC_LONG_MAX);
	ASSERT_EQ(lngat_ret, PFC_LONG_MIN);
	lngat_ret = pfc_conf_array_longat(cfmap, "test_conf_map_lng_array2",
					  TEST_CONF_MAX_INDEX, PFC_LONG_MIN);
	ASSERT_EQ(lngat_ret, PFC_LONG_MAX);

	lngat_ret = pfc_conf_array_longat(cfblk,
					  "test_conf_blk_lng_array_inv1", 0,
					  PFC_LONG_MAX);
	ASSERT_EQ(lngat_ret, PFC_LONG_MAX);
	lngat_ret = pfc_conf_array_longat(cfmap,
					  "test_conf_map_lng_array_inv1",
					  TEST_CONF_MAX_INDEX, PFC_LONG_MIN);
	ASSERT_EQ(lngat_ret, PFC_LONG_MIN);

	lngat_ret = pfc_conf_array_longat(cfblk,
					  "test_conf_blk_lng_array_inv2",
					  TEST_CONF_MIN_INDEX, PFC_LONG_MAX);
	ASSERT_EQ(lngat_ret, PFC_LONG_MAX);
	lngat_ret = pfc_conf_array_longat(cfmap,
					  "test_conf_map_lng_array_inv2",
					  TEST_CONF_MAX_INDEX, PFC_LONG_MIN);
	ASSERT_EQ(lngat_ret, PFC_LONG_MIN);

	/* invalid index */
	lngat_ret = pfc_conf_array_longat(cfblk, "test_conf_blk_lng_array1",
					  TEST_CONF_MAX_ARRAY_SIZE,
					  PFC_LONG_MAX);
	ASSERT_EQ(lngat_ret, PFC_LONG_MAX);
	lngat_ret = pfc_conf_array_longat(cfmap, "test_conf_map_lng_array1",
					  UINT32_MAX, PFC_LONG_MIN);
	ASSERT_EQ(lngat_ret, PFC_LONG_MIN);

	lngat_ret = pfc_conf_array_longat(cfb_inv, "test_conf_blk_lng_array1",
					  TEST_CONF_MIN_INDEX, PFC_LONG_MAX);
	ASSERT_EQ(lngat_ret, PFC_LONG_MAX);

	ASSERT_EQ(0L, pfc_conf_array_longat(cfblk, NULL,
					    TEST_CONF_MIN_INDEX, 0L));
	ASSERT_EQ(1L, pfc_conf_array_longat(cfmap, NULL,
					    TEST_CONF_MIN_INDEX, 1L));
	ASSERT_EQ(2L, pfc_conf_array_longat(cfb_inv, NULL,
					    TEST_CONF_MIN_INDEX, 2L));

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_ulongat()
 */
TEST(conf, pfc_conf_array_ulongat)
{
	int ret;
	pfc_cfblk_t cfblk, cfmap;
	pfc_conf_t confp;
	pfc_cfblk_t cfb_inv = PFC_CFBLK_INVALID;
	pfc_ulong_t ulngat_ret;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ulng");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);
	cfmap = pfc_conf_get_map(confp, "test_conf_map_ulng", 
				 "test_conf_map_ulng_k1");
	ASSERT_NE(cfmap, PFC_CFBLK_INVALID);

	ulngat_ret = pfc_conf_array_ulongat(cfblk, "test_conf_blk_ulng_array1",
					    TEST_CONF_MIN_INDEX,
					    PFC_ULONG_MAX);
	ASSERT_EQ(ulngat_ret, (pfc_ulong_t)0);
	ulngat_ret = pfc_conf_array_ulongat(cfmap, "test_conf_map_ulng_array1",
					    TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ulngat_ret, PFC_ULONG_MAX);

	ulngat_ret = pfc_conf_array_ulongat(cfblk, "test_conf_blk_ulng_array2",
					    TEST_CONF_MIN_INDEX,
					    PFC_ULONG_MAX);
	ASSERT_EQ(ulngat_ret, (pfc_ulong_t)0);
	ulngat_ret = pfc_conf_array_ulongat(cfmap, "test_conf_map_ulng_array2",
					    TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ulngat_ret, PFC_ULONG_MAX);

	ulngat_ret = pfc_conf_array_ulongat(cfblk,
					    "test_conf_blk_ulng_array_inv1",
					    TEST_CONF_MIN_INDEX,
					     PFC_ULONG_MAX);
	ASSERT_EQ(ulngat_ret, PFC_ULONG_MAX);
	ulngat_ret = pfc_conf_array_ulongat(cfmap,
					    "test_conf_map_ulng_array_inv1",
					    TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ulngat_ret, (pfc_ulong_t)0);

	ulngat_ret = pfc_conf_array_ulongat(cfblk,
					    "test_conf_blk_ulng_array_inv2",
					    TEST_CONF_MIN_INDEX,
					    PFC_ULONG_MAX);
	ASSERT_EQ(ulngat_ret, PFC_ULONG_MAX);
	ulngat_ret = pfc_conf_array_ulongat(cfmap,
					    "test_conf_map_ulng_array_inv2",
					    TEST_CONF_MAX_INDEX, 0);
	ASSERT_EQ(ulngat_ret, (pfc_ulong_t)0);

	/* invalid index */
	ulngat_ret = pfc_conf_array_ulongat(cfblk, "test_conf_blk_ulng_array1",
					    TEST_CONF_MAX_ARRAY_SIZE,
					    PFC_ULONG_MAX);
	ASSERT_EQ(ulngat_ret, PFC_ULONG_MAX);
	ulngat_ret = pfc_conf_array_ulongat(cfmap, "test_conf_map_ulng_array1",
					    UINT32_MAX, 0);
	ASSERT_EQ(ulngat_ret, (pfc_ulong_t)0);

	ulngat_ret = pfc_conf_array_ulongat(cfb_inv,
					    "test_conf_blk_ulng_array1",
					    TEST_CONF_MIN_INDEX,
					    PFC_ULONG_MAX);
	ASSERT_EQ(ulngat_ret, PFC_ULONG_MAX);

	ASSERT_EQ(0ULL, pfc_conf_array_ulongat(cfblk, NULL,
					       TEST_CONF_MIN_INDEX, 0ULL));
	ASSERT_EQ(1ULL, pfc_conf_array_ulongat(cfmap, NULL,
					       TEST_CONF_MIN_INDEX, 1ULL));
	ASSERT_EQ(2ULL, pfc_conf_array_ulongat(cfb_inv, NULL,
					       TEST_CONF_MIN_INDEX, 2ULL));

	pfc_conf_close(confp);
}

/*
 * Test Data
 *      pfc_conf_copy_string()
 */
test_case_cpy_str_t tc_cpy_str[TEST_CONF_HDL_CNT][CPY_STRING_TEST_COUNT] 
	= {
	/*
	 * from block
	 */
	{{"test_conf_blk_str", "test_string_blk", sizeof("test_string_blk")},
	 {"test_conf_blk_str", "test_", sizeof("test_")},
	/* different type1 */
	 {"test_conf_blk_str_inv1", defvalue_string,
	  TEST_CONF_DEFVALUE_STR_LEN},
	/* different type2 */
	 {"test_conf_blk_str_inv2", defvalue_string,
	  TEST_CONF_DEFVALUE_STR_LEN},
	/* undefined Parameter */
	 {TEST_CONF_UNDEF_PARAM, defvalue_string, TEST_CONF_DEFVALUE_STR_LEN}},

	/*
	 * from map
	 */
	{{"test_conf_map_str", "test_string_map", sizeof("test_string_map")},
	 {"test_conf_map_str", "test_", sizeof("test_")},
	/* different type1 */
	 {"test_conf_map_str_inv1", defvalue_string,
	  TEST_CONF_DEFVALUE_STR_LEN},
	/* different type2 */
	 {"test_conf_map_str_inv2", defvalue_string,
	  TEST_CONF_DEFVALUE_STR_LEN},
	/* undefined Parameter */
	 {TEST_CONF_UNDEF_PARAM, defvalue_string, TEST_CONF_DEFVALUE_STR_LEN}}
};

/*
 * Test Case
 *      pfc_conf_copy_string()
 */
TEST(conf, pfc_conf_copy_string)
{
	int ret, i, j;
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];
	pfc_conf_t confp;
	char *buf, tmpbuf[32];

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cf[0] = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);
	cf[1] = pfc_conf_get_map(confp, "test_conf_map_str",
				 "test_conf_map_str_k1");
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {
		for (j = 0; j < CPY_STRING_TEST_COUNT; j++) {
			buf = (char *)malloc(tc_cpy_str[i][j].bufsz);
			ASSERT_TRUE(buf != NULL);
			pfc_conf_copy_string(cf[i], tc_cpy_str[i][j].param,
					     defvalue_string,
					     buf, tc_cpy_str[i][j].bufsz);
			ASSERT_STREQ(tc_cpy_str[i][j].expect, buf);
			free(buf);
		}

		tmpbuf[0] = '\0';
		pfc_conf_copy_string(cf[i], NULL, defvalue_string, tmpbuf,
				     sizeof(tmpbuf));
		ASSERT_STREQ(defvalue_string, tmpbuf);
	}

	pfc_conf_copy_string(PFC_CFBLK_INVALID, NULL, defvalue_string,
			     tmpbuf, sizeof(tmpbuf));
	ASSERT_STREQ(defvalue_string, tmpbuf);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	buf = (char *)malloc(tc_cpy_str[0][0].bufsz);
	pfc_conf_copy_string(PFC_CFBLK_INVALID, tc_cpy_str[0][0].param,
			     defvalue_string, buf, tc_cpy_str[0][0].bufsz);
	ASSERT_STREQ(defvalue_string, buf);
	free(buf);

	buf = (char *)malloc(tc_cpy_str[0][0].bufsz);
	ASSERT_TRUE(buf != NULL);
	pfc_conf_copy_string(cf[0], tc_cpy_str[0][0].param, defvalue_string,
			     buf, tc_cpy_str[0][0].bufsz);

	pfc_conf_close(confp);

	/*
	 * After Closing
	 */
	ASSERT_STREQ(tc_cpy_str[0][0].expect, buf);
	free(buf);
}

/*
 * Test Data
 *      pfc_conf_array_copy_string()
 */
tc_ary_cpy_str_t tc_ary_cpy_str[TEST_CONF_HDL_CNT][ARY_CPY_STR_TEST_COUNT] 
	= {
	/*
	 * from block
	 */
	{{"test_conf_blk_str_array1", "test_string-1", TEST_CONF_MIN_INDEX,
	  sizeof("test_string-1")},
	 {"test_conf_blk_str_array1", "test_", TEST_CONF_MIN_INDEX,
	  sizeof("test_")},
	 {"test_conf_blk_str_array2", "test_string-1", TEST_CONF_MIN_INDEX,
	  sizeof("test_string-1")},
	 {"test_conf_blk_str_array2", "test_s", TEST_CONF_MIN_INDEX,
	  sizeof("test_s")},
	/* different type1 */
	 {"test_conf_blk_str_array_inv1", defvalue_string,
	  TEST_CONF_MIN_INDEX, TEST_CONF_DEFVALUE_STR_LEN},
	/* different type2 */
	 {"test_conf_blk_str_array_inv2", defvalue_string,
	  TEST_CONF_MIN_INDEX, TEST_CONF_DEFVALUE_STR_LEN},
	/* invalid array index */
	 {"test_conf_blk_str_array1", defvalue_string,
	  TEST_CONF_MAX_ARRAY_SIZE, TEST_CONF_DEFVALUE_STR_LEN}},

	/*
	 * from map
	 */
	{{"test_conf_map_str_array1", "test_string-6", TEST_CONF_MAX_INDEX,
	  sizeof("test_string-6")},
	 {"test_conf_map_str_array1", "test_", TEST_CONF_MAX_INDEX,
	  sizeof("test_")},
	 {"test_conf_map_str_array2", "test_string-6", TEST_CONF_MAX_INDEX,
	  sizeof("test_string-6")},
	 {"test_conf_map_str_array2", "test_s", TEST_CONF_MAX_INDEX,
	  sizeof("test_s")},
	/* different type1 */
	 {"test_conf_map_str_array_inv1", defvalue_string,
	  TEST_CONF_MIN_INDEX, TEST_CONF_DEFVALUE_STR_LEN},
	/* different type2 */
	 {"test_conf_map_str_array_inv2", defvalue_string,
	  TEST_CONF_MIN_INDEX, TEST_CONF_DEFVALUE_STR_LEN},
	/* invalid array index */
	 {"test_conf_blk_str_array1", defvalue_string, UINT32_MAX,
	   TEST_CONF_DEFVALUE_STR_LEN}}
};

/*
 * Test Case
 *      pfc_conf_array_copy_string()
 */
TEST(conf, pfc_conf_array_copy_string)
{
	int ret, i, j;
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];
	pfc_conf_t confp;
	char *buf;

	TmpFile tmpf("conf_test");
	setup_conffile(tmpf);
	RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cf[0] = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);
	cf[1] = pfc_conf_get_map(confp, "test_conf_map_str",
				 "test_conf_map_str_k1");
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {
		for (j = 0; j < ARY_CPY_STR_TEST_COUNT; j++) {
			buf = (char *)malloc(tc_ary_cpy_str[i][j].bufsz);
			ASSERT_TRUE(buf != NULL);
			pfc_conf_array_copy_string(cf[i],
						   tc_ary_cpy_str[i][j].param,
						   tc_ary_cpy_str[i][j].index,
						   defvalue_string, buf,
						   tc_ary_cpy_str[i][j].bufsz);
			ASSERT_STREQ(tc_ary_cpy_str[i][j].expect, buf);
			free(buf);

			char	tmpbuf[32];
			tmpbuf[0] = '\0';
			pfc_conf_array_copy_string(cf[i], NULL,
						   tc_ary_cpy_str[i][j].index,
						   defvalue_string, tmpbuf,
						   sizeof(tmpbuf));
			ASSERT_STREQ(defvalue_string, tmpbuf);

			tmpbuf[0] = '\0';
			pfc_conf_array_copy_string(PFC_CFBLK_INVALID, NULL,
						   tc_ary_cpy_str[i][j].index,
						   defvalue_string, tmpbuf,
						   sizeof(tmpbuf));
			ASSERT_STREQ(defvalue_string, tmpbuf);
		}
	}

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	buf = (char *)malloc(tc_ary_cpy_str[0][0].bufsz);
	pfc_conf_array_copy_string(PFC_CFBLK_INVALID,
				    tc_ary_cpy_str[0][0].param, 
				    tc_ary_cpy_str[0][0].index,
				    defvalue_string, buf,
				    tc_ary_cpy_str[0][0].bufsz);
	ASSERT_STREQ(defvalue_string, buf);
	free(buf);

	buf = (char *)malloc(tc_ary_cpy_str[0][0].bufsz);
	ASSERT_TRUE(buf != NULL);
	pfc_conf_array_copy_string(cf[0], tc_ary_cpy_str[0][0].param, 
				   tc_ary_cpy_str[0][0].index, defvalue_string,
				   buf, tc_ary_cpy_str[0][0].bufsz);
	pfc_conf_close(confp);

	/*
	 * After Closing
	 */
	ASSERT_STREQ(tc_ary_cpy_str[0][0].expect, buf);
	free(buf);
}

/*
 * Test framework
 *    pfc_conf_array_xxx_range()
 */
#define RANGE_PREPARE(BLK, MAP, KEY)					\
	int ret, i, j;							\
	uint32_t k;							\
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];				\
	pfc_conf_t confp;						\
									\
	TmpFile tmpf("conf_test");					\
	setup_conffile(tmpf);						\
	RETURN_ON_ERROR();						\
									\
	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);	\
	ASSERT_EQ(ret, 0);						\
	cf[0] = pfc_conf_get_block(confp, (BLK));			\
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);				\
	cf[1] = pfc_conf_get_map(confp, (MAP), (KEY));			\
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

#define RANGE_ROUTINE(TYPE, STRUCT, FUNC, AST)				\
	TYPE *buf;							\
	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {			\
		for (j = 0; j < ARY_RNG_TEST_COUNT; j++) {		\
			buf = (TYPE*)malloc(STRUCT[i][j].		\
					    ary_rng_cmn.bufsz);		\
			ASSERT_TRUE(buf != NULL);			\
									\
			ret = FUNC(cf[i],STRUCT[i][j].ary_rng_cmn.param,\
				   STRUCT[i][j]. ary_rng_cmn.st_idx,	\
				   STRUCT[i][j]. ary_rng_cmn.el_cnt,	\
				   buf);				\
			ASSERT_EQ(STRUCT[i][j].ary_rng_cmn.ext_ret,	\
				  ret);					\
			if (ret == 0) {					\
				for (k = 0;  k < STRUCT[i][j].		\
					ary_rng_cmn.el_cnt; k++) {	\
					AST(*(buf + k),			\
					    STRUCT[i][j].ext_ary[k]);	\
				}					\
			}						\
			ASSERT_EQ(ENOENT, FUNC				\
				  (cf[i], NULL,				\
				   STRUCT[i][j].ary_rng_cmn.st_idx,	\
				   STRUCT[i][j]. ary_rng_cmn.el_cnt,	\
				   buf));				\
			ASSERT_EQ(STRUCT[i][j].ary_rng_cmn.ext_ret,	\
				  ret);					\
			free(buf);					\
		}							\
	}

#define RANGE_INVALID_HND(TYPE, STRUCT, FUNC)				\
	buf = (TYPE *)malloc(STRUCT[0][0].ary_rng_cmn.bufsz);		\
	ret = FUNC(PFC_CFBLK_INVALID, STRUCT[0][0].ary_rng_cmn.param,	\
		   STRUCT[0][0].ary_rng_cmn.st_idx,			\
		   STRUCT[0][0].ary_rng_cmn.el_cnt,			\
		   buf);						\
	ASSERT_EQ(ENOENT, ret);						\
	ASSERT_EQ(ENOENT, FUNC(PFC_CFBLK_INVALID, NULL,			\
			       STRUCT[0][0].ary_rng_cmn.st_idx,		\
			       STRUCT[0][0].ary_rng_cmn.el_cnt, buf));	\
	free(buf);							\
	pfc_conf_close(confp);


/*
 * Test Data
 *      pfc_conf_array_byte_range()
 */
tc_ary_rng_byt_t ary_rng_byt[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_byte_array1", 0, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_byte_array2", 0, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_byte_array_inv1", EINVAL, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_byte_array_inv2", EINVAL, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_byte_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}}
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_byte_array1", 0, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT8_MAX - 1), (UINT8_MAX)}},
	 {{"test_conf_map_byte_array2", 0, 1, 2,
	   (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT8_MAX - 1), (UINT8_MAX)}},
	 {{"test_conf_map_byte_array_inv1", EINVAL, 1, 2,
	  (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_map_byte_array_inv2", EINVAL, 1, 2,
	  (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_map_byte_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	  (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	  (sizeof(uint8_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {1, 2}}
	}};

/*
 * Test Case
 *      pfc_conf_array_byte_range()
 */
TEST(conf, pfc_conf_array_byte_range)
{
	RANGE_PREPARE("test_conf_blk_byte", "test_conf_map_byte",
		      "test_conf_map_byte_k1");

	RANGE_ROUTINE(uint8_t, ary_rng_byt, pfc_conf_array_byte_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(uint8_t, ary_rng_byt, pfc_conf_array_byte_range);
}

/*
 * Test Data
 *      pfc_conf_array_string_range()
 */
tc_ary_rng_str_t ary_rng_str[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_str_array1", 0, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"test_string-2", "test_string-3"}},
	 {{"test_conf_blk_str_array2", 0, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"test_string-2", "test_string-3"}},
	 {{"test_conf_blk_str_array_inv1", EINVAL, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"", ""}},
	 {{"test_conf_blk_str_array_inv2", EINVAL, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"", ""}},
	 {{"test_conf_blk_str_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   ARY_RNG_STR_BUF_SIZE}, {"", ""}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"", ""}}
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_str_array1", 0, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"test_string-5", "test_string-6"}},
	 {{"test_conf_map_str_array2", 0, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"test_string-5", "test_string-6"}},
	 {{"test_conf_map_str_array_inv1", EINVAL, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"", ""}},
	 {{"test_conf_map_str_array_inv2", EINVAL, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"", ""}},
	 {{"test_conf_map_str_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   ARY_RNG_STR_BUF_SIZE}, {"", ""}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2, ARY_RNG_STR_BUF_SIZE},
	  {"", ""}}
	}};


/*
 * Test Case
 *      pfc_conf_array_string_range()
 */
TEST(conf, pfc_conf_array_string_range)
{
	RANGE_PREPARE("test_conf_blk_str", "test_conf_map_str",
		      "test_conf_map_str_k1");

	RANGE_ROUTINE(const char *, ary_rng_str, pfc_conf_array_string_range,
		      ASSERT_STREQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(const char *, ary_rng_str, 
			  pfc_conf_array_string_range);
}

/*
 * Test Data
 *      pfc_conf_array_bool_range()
 */
tc_ary_rng_bol_t ary_rng_bol[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_bool_array1", 0, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_TRUE, PFC_TRUE}},
	 {{"test_conf_blk_bool_array2", 0, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_TRUE, PFC_TRUE}},
	 {{"test_conf_blk_bool_array_inv1", EINVAL, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_TRUE, PFC_TRUE}},
	 {{"test_conf_blk_bool_array_inv2", EINVAL, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_TRUE, PFC_TRUE}},
	 {{"test_conf_blk_bool_array1", ERANGE,  TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_TRUE, PFC_TRUE}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_TRUE, PFC_TRUE}}
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_bool_array1", 0, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_FALSE, PFC_FALSE}},
	 {{"test_conf_map_bool_array2", 0, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_FALSE, PFC_FALSE}},
	 {{"test_conf_map_bool_array_inv1", EINVAL, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_FALSE, PFC_FALSE}},
	 {{"test_conf_map_bool_array_inv2", EINVAL, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_FALSE, PFC_FALSE}},
	 {{"test_conf_map_bool_array1", ERANGE,  TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_FALSE, PFC_FALSE}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(pfc_bool_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {PFC_FALSE, PFC_FALSE}}
	}};

/*
 * Test Case
 *      pfc_conf_array_bool_range()
 */
TEST(conf, pfc_conf_array_bool_range)
{
	RANGE_PREPARE("test_conf_blk_bool", "test_conf_map_bool",
		      "test_conf_map_bool_k1");

	RANGE_ROUTINE(pfc_bool_t, ary_rng_bol, pfc_conf_array_bool_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(pfc_bool_t, ary_rng_bol, pfc_conf_array_bool_range);
}


/*
 * Test Data
 *      pfc_conf_array_int32_range()
 */
tc_ary_rng_i32_t ary_rng_i32[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_i32_array1", 0, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT32_MIN + 1), (INT32_MIN + 2)}},
	 {{"test_conf_blk_i32_array2", 0, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT32_MIN + 1), (INT32_MIN + 2)}},
	 {{"test_conf_blk_i32_array_inv1", EINVAL, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_i32_array_inv2", EINVAL, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_i32_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}}
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_i32_array1", 0, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT32_MAX - 1), INT32_MAX}},
	 {{"test_conf_map_i32_array2", 0, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT32_MAX - 1), INT32_MAX}},
	 {{"test_conf_map_i32_array_inv1", EINVAL, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_map_i32_array_inv2", EINVAL, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_map_i32_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(int32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}}
	}};

/*
 * Test Case
 *      pfc_conf_array_int32_range()
 */
TEST(conf, pfc_conf_array_int32_range)
{
	RANGE_PREPARE("test_conf_blk_i32_min", "test_conf_map_i32",
		      "test_conf_map_i32_k2");

	RANGE_ROUTINE(int32_t, ary_rng_i32, pfc_conf_array_int32_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(int32_t, ary_rng_i32, pfc_conf_array_int32_range);
}


/*
 * Test Data
 *      pfc_conf_array_uint32_range()
 */
tc_ary_rng_ui32_t ary_rng_ui32[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_ui32_array1", 0, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui32_array2", 0, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui32_array_inv1", EINVAL, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui32_array_inv2", EINVAL, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui32_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},  {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_ui32_array1", 0, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT32_MAX - 1), UINT32_MAX}},
	 {{"test_conf_map_ui32_array2", 0, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT32_MAX - 1), UINT32_MAX}},
	 {{"test_conf_map_ui32_array_inv1", EINVAL, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT32_MAX - 1), UINT32_MAX}},
	 {{"test_conf_map_ui32_array_inv2", EINVAL, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT32_MAX - 1), UINT32_MAX}},
	 {{"test_conf_map_ui32_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT32_MAX - 1), UINT32_MAX}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(uint32_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT32_MAX - 1), UINT32_MAX}},
	}};

/*
 * Test Case
 *      pfc_conf_array_uint32_range()
 */
TEST(conf, pfc_conf_array_uint32_range)
{
	RANGE_PREPARE("test_conf_blk_ui32", "test_conf_map_ui32",
		      "test_conf_map_ui32_k1");

	RANGE_ROUTINE(uint32_t, ary_rng_ui32, pfc_conf_array_uint32_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(uint32_t, ary_rng_ui32, pfc_conf_array_uint32_range);
}

/*
 * Test Data
 *      pfc_conf_array_int64_range()
 */
tc_ary_rng_i64_t ary_rng_i64[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_i64_array1", 0, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MIN + 1), (INT64_MIN + 2)}},
	 {{"test_conf_blk_i64_array2", 0, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MIN + 1), (INT64_MIN + 2)}},
	 {{"test_conf_blk_i64_array_inv1", EINVAL, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MIN + 1), (INT64_MIN + 2)}},
	 {{"test_conf_blk_i64_array_inv2", EINVAL, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MIN + 1), (INT64_MIN + 2)}},
	 {{"test_conf_blk_i64_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MIN + 1), (INT64_MIN + 2)}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MIN + 1), (INT64_MIN + 2)}},
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_i64_array1", 0, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MAX - 1), INT64_MAX}},
	 {{"test_conf_map_i64_array2", 0, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MAX - 1), INT64_MAX}},
	 {{"test_conf_map_i64_array_inv1", EINVAL, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MAX - 1), INT64_MAX}},
	 {{"test_conf_map_i64_array_inv2", EINVAL, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MAX - 1), INT64_MAX}},
	 {{"test_conf_map_i64_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MAX - 1), INT64_MAX}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(int64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(INT64_MAX - 1), INT64_MAX}},
	}};

/*
 * Test Case
 *      pfc_conf_array_int64_range()
 */
TEST(conf, pfc_conf_array_int64_range)
{
	RANGE_PREPARE("test_conf_blk_i64_min", "test_conf_map_i64",
		      "test_conf_map_i64_k2");

	RANGE_ROUTINE(int64_t, ary_rng_i64, pfc_conf_array_int64_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(int64_t, ary_rng_i64, pfc_conf_array_int64_range);
}

/*
 * Test Data
 *      pfc_conf_array_uint64_range()
 */
tc_ary_rng_ui64_t ary_rng_ui64[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_ui64_array1", 0, 1, 2,
	  (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui64_array2", 0, 1, 2,
	  (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui64_array_inv1", EINVAL, 1, 2,
	  (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui64_array_inv2", EINVAL, 1, 2,
	  (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ui64_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	  (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	  (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_ui64_array1", 0, 1, 2,
	   (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, 
	  {(UINT64_MAX - 1), UINT64_MAX}},
	 {{"test_conf_map_ui64_array2", 0, 1, 2,
	   (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT64_MAX - 1), UINT64_MAX}},
	 {{"test_conf_map_ui64_array_inv1", EINVAL, 1, 2,
	   (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT64_MAX - 1), UINT64_MAX}},
	 {{"test_conf_map_ui64_array_inv2", EINVAL, 1, 2,
	   (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT64_MAX - 1), UINT64_MAX}},
	 {{"test_conf_map_ui64_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT64_MAX - 1), UINT64_MAX}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(uint64_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(UINT64_MAX - 1), UINT64_MAX}}
	}};

/*
 * Test Case
 *      pfc_conf_array_uint64_range()
 */
TEST(conf, pfc_conf_array_uint64_range)
{
	RANGE_PREPARE("test_conf_blk_ui64", "test_conf_map_ui64",
		      "test_conf_map_ui64_k1");

	RANGE_ROUTINE(uint64_t, ary_rng_ui64, pfc_conf_array_uint64_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(uint64_t, ary_rng_ui64, pfc_conf_array_uint64_range);
}



/*
 * Test Data
 *      pfc_conf_array_long_range()
 */
tc_ary_rng_lng_t ary_rng_lng[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_lng_array1", 0, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MIN + 1), (PFC_LONG_MIN + 2)}},
	 {{"test_conf_blk_lng_array2", 0, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MIN + 1), (PFC_LONG_MIN + 2)}},
	 {{"test_conf_blk_lng_array_inv1", EINVAL, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MIN + 1), (PFC_LONG_MIN + 2)}},
	 {{"test_conf_blk_lng_array_inv2", EINVAL, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MIN + 1), (PFC_LONG_MIN + 2)}},
	 {{"test_conf_blk_lng_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MIN + 1), (PFC_LONG_MIN + 2)}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MIN + 1), (PFC_LONG_MIN + 2)}}
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_lng_array1", 0, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MAX - 1), PFC_LONG_MAX}},
	 {{"test_conf_map_lng_array2", 0, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MAX - 1), PFC_LONG_MAX}},
	 {{"test_conf_map_lng_array_inv1", EINVAL, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MAX - 1), PFC_LONG_MAX}},
	 {{"test_conf_map_lng_array_inv2", EINVAL, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MAX - 1), PFC_LONG_MAX}},
	 {{"test_conf_map_lng_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MAX - 1), PFC_LONG_MAX}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2, 
	   (sizeof(pfc_long_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_LONG_MAX - 1), PFC_LONG_MAX}}
	}};

/*
 * Test Case
 *      pfc_conf_array_long_range()
 */
TEST(conf, pfc_conf_array_long_range)
{
	RANGE_PREPARE("test_conf_blk_lng_min", "test_conf_map_lng",
		      "test_conf_map_lng_k2");

	RANGE_ROUTINE(pfc_long_t, ary_rng_lng, pfc_conf_array_long_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(pfc_long_t, ary_rng_lng, pfc_conf_array_long_range);
}

/*
 * Test Data
 *      pfc_conf_array_ulong_range()
 */
tc_ary_rng_ulng_t ary_rng_ulng[TEST_CONF_HDL_CNT][ARY_RNG_TEST_COUNT] = {
	/*
	 * from block
	 */
	{
	 {{"test_conf_blk_ulng_array1", 0, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ulng_array2", 0, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ulng_array_inv1", EINVAL, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ulng_array_inv2", EINVAL, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_blk_ulng_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	},

	/*
	 * from map
	 */
	{
	 {{"test_conf_map_ulng_array1", 0, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_ULONG_MAX - 1), PFC_ULONG_MAX}},
	 {{"test_conf_map_ulng_array2", 0, 1, 2,
	   (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)},
	  {(PFC_ULONG_MAX - 1), PFC_ULONG_MAX}},
	 {{"test_conf_map_ulng_array_inv1", EINVAL, 1, 2,
	  (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_map_ulng_array_inv2", EINVAL, 1, 2,
	  (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{"test_conf_map_ulng_array1", ERANGE, TEST_CONF_INVALID_INDEX, 2,
	  (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	 {{TEST_CONF_UNDEF_PARAM, ENOENT, 1, 2,
	  (sizeof(pfc_ulong_t) * TEST_CONF_ARRAY_RANGE_ELEM)}, {1, 2}},
	}};

/*
 * Test Case
 *      pfc_conf_array_ulong_range()
 */
TEST(conf, pfc_conf_array_ulong_range)
{
	RANGE_PREPARE("test_conf_blk_ulng", "test_conf_map_ulng",
		      "test_conf_map_ulng_k1");

	RANGE_ROUTINE(pfc_ulong_t, ary_rng_ulng, pfc_conf_array_ulong_range,
		      ASSERT_EQ);

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	RANGE_INVALID_HND(pfc_ulong_t, ary_rng_ulng,
			  pfc_conf_array_ulong_range);
}

/*
 * Test Case
 *      underscore name
 */
TEST(conf, pfc_conf_underscore_name)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	pfc_cfblk_t cfmap;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(0, ret);

	cfblk = pfc_conf_get_block(confp, "_test_conf_invalid_block_name");
	ASSERT_NE(PFC_CFBLK_INVALID, cfblk);

	ret = pfc_conf_get_byte(cfblk, "_test_conf_invalid_block_name", -1);
	ASSERT_EQ(2, ret);

	cfmap = pfc_conf_get_map(confp, "_test_conf_invalid_map_name",
				 "_test_conf_invalid_map_name_key");
	ASSERT_NE(PFC_CFBLK_INVALID, cfmap);

	ret = pfc_conf_get_byte(cfmap, "_test_conf_invalid_map_name", -1);
	ASSERT_EQ(5, ret);

	cfblk = pfc_conf_get_block(confp, "test_conf_invalid_param_name");
	ASSERT_NE(PFC_CFBLK_INVALID, cfblk);

	ret = pfc_conf_get_byte(cfblk, "_test_conf_invalid_param_name1", -1);
	ASSERT_EQ(8, ret);

	pfc_conf_close(confp);
}


/*
 * Reflect result of review
 */

/*
 * Test Data
 *      pfc_conf_copy_string() 2
 */
test_case_cpy_str_t tc_cpy_str2[TEST_CONF_HDL_CNT][CPY_STRING_2_TEST_COUNT] 
	= {
	/*
	 * from block
	 */
	{{"test_conf_blk_str", "test_string_blk",
	  (sizeof("test_string_blk") + 1)},
	 {"test_conf_blk_str", "test_", (sizeof("test_") + 1)},
	/* different type1_1 */
	 {"test_conf_blk_str_inv1", defvalue_string,
	  (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type1_2 */
	 {"test_conf_blk_str_inv1", "inval", (sizeof("inval") + 1)},
	/* different type2_1 */
	 {"test_conf_blk_str_inv2", defvalue_string,
	  (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type2_2 */
	 {"test_conf_blk_str_inv2", "inval", (sizeof("inval") + 1)},
	/* undefined Parameter_1 */
	 {TEST_CONF_UNDEF_PARAM, defvalue_string,
	  (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* undefined Parameter_2 */
	 {TEST_CONF_UNDEF_PARAM, "inval", (sizeof("inval") + 1)}},

	/*
	 * from map
	 */
	{{"test_conf_map_str", "test_string_map",
	  (sizeof("test_string_map") + 1)},
	 {"test_conf_map_str", "test_", (sizeof("test_") + 1)},
	/* different type1_1 */
	 {"test_conf_map_str_inv1", defvalue_string,
	  (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type1_2 */
	 {"test_conf_map_str_inv1", "inval", (sizeof("inval") + 1)},
	/* different type2_1 */
	 {"test_conf_map_str_inv2", defvalue_string,
	  (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type2_2 */
	 {"test_conf_map_str_inv2", "inval", (sizeof("inval") + 1)},
	/* undefined Parameter_1 */
	 {TEST_CONF_UNDEF_PARAM, defvalue_string,
	  (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* undefined Parameter_2 */
	 {TEST_CONF_UNDEF_PARAM, "inval", (sizeof("inval") + 1)}}
	};

/*
 * Test Case
 *      pfc_conf_copy_string() 2
 */
TEST(conf, pfc_conf_copy_string_2)
{
	int ret, i, j;
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];
	pfc_conf_t confp;
	char *buf;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cf[0] = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);
	cf[1] = pfc_conf_get_map(confp, "test_conf_map_str",
				 "test_conf_map_str_k1");
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {
		for (j = 0; j < CPY_STRING_2_TEST_COUNT; j++) {
			buf = (char *)malloc(tc_cpy_str2[i][j].bufsz);
			ASSERT_TRUE(buf != NULL);
			memset(buf, TEST_CONF_DUMMY_DATA,
			       tc_cpy_str2[i][j].bufsz);
			pfc_conf_copy_string(cf[i], tc_cpy_str2[i][j].param,
					     defvalue_string,
					     buf,
					     (tc_cpy_str2[i][j].bufsz - 1));
			ASSERT_STREQ(tc_cpy_str2[i][j].expect, buf);
			/* overflow check */
			ASSERT_EQ(TEST_CONF_DUMMY_DATA,
				  *(buf + tc_cpy_str2[i][j].bufsz - 1));
			free(buf);
		}
	}

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	buf = (char *)malloc(tc_cpy_str2[0][1].bufsz);
	memset(buf, TEST_CONF_DUMMY_DATA, tc_cpy_str2[0][1].bufsz);
	pfc_conf_copy_string(PFC_CFBLK_INVALID, tc_cpy_str2[0][0].param,
			     defvalue_string, buf,
			     (tc_cpy_str2[0][1].bufsz - 1));
	ASSERT_STREQ("inval", buf);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA, *(buf + tc_cpy_str2[0][1].bufsz - 1));
	free(buf);

	pfc_conf_close(confp);
}

/*
 * Test Data
 *      pfc_conf_array_copy_string() 2
 */
tc_ary_cpy_str_t tc_ary_cpy_str2[TEST_CONF_HDL_CNT][ARY_CPY_STR_2_TEST_COUNT] 
	= {
	/*
	 * from block
	 */
	{{"test_conf_blk_str_array1", "test_string-1", TEST_CONF_MIN_INDEX,
	  (sizeof("test_string-1") + 1)},
	 {"test_conf_blk_str_array1", "test_", TEST_CONF_MIN_INDEX,
	  (sizeof("test_") + 1)},
	 {"test_conf_blk_str_array2", "test_string-1", TEST_CONF_MIN_INDEX,
	  (sizeof("test_string-1") + 1)},
	 {"test_conf_blk_str_array2", "test_s", TEST_CONF_MIN_INDEX,
	  (sizeof("test_s") + 1)},
	/* different type1_1 */
	 {"test_conf_blk_str_array_inv1", defvalue_string,
	  TEST_CONF_MIN_INDEX, (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type1_2 */
	 {"test_conf_blk_str_array_inv1", "inval", 
	  TEST_CONF_MIN_INDEX, (sizeof("inval") + 1)},
	/* different type2_1 */
	 {"test_conf_blk_str_array_inv2", defvalue_string,
	  TEST_CONF_MIN_INDEX, (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type2_2 */
	 {"test_conf_blk_str_array_inv2", "inval",
	  TEST_CONF_MIN_INDEX, (sizeof("inval") + 1)},
	/* invalid array index_1 */
	 {"test_conf_blk_str_array1", defvalue_string,
	  TEST_CONF_MAX_ARRAY_SIZE, (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* invalid array index_2 */
	 {"test_conf_blk_str_array1", "inval",
	  TEST_CONF_MAX_ARRAY_SIZE, (sizeof("inval") + 1)},
	},

	/*
	 * from map
	 */
	{{"test_conf_map_str_array1", "test_string-6", TEST_CONF_MAX_INDEX,
	  (sizeof("test_string-6") + 1)},
	 {"test_conf_map_str_array1", "test_", TEST_CONF_MAX_INDEX,
	  (sizeof("test_") + 1)},
	 {"test_conf_map_str_array2", "test_string-6", TEST_CONF_MAX_INDEX,
	  (sizeof("test_string-6") + 1)},
	 {"test_conf_map_str_array2", "test_s", TEST_CONF_MAX_INDEX,
	  (sizeof("test_s") + 1)},
	/* different type1_1 */
	 {"test_conf_map_str_array_inv1", defvalue_string,
	  TEST_CONF_MIN_INDEX, (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type1_2 */
	 {"test_conf_map_str_array_inv1", "inval",
	  TEST_CONF_MIN_INDEX, (sizeof("inval") + 1)},
	/* different type2_1 */
	 {"test_conf_map_str_array_inv2", defvalue_string,
	  TEST_CONF_MIN_INDEX, (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* different type2_2 */
	 {"test_conf_map_str_array_inv2", "inval",
	  TEST_CONF_MIN_INDEX, (sizeof("inval") + 1)},
	/* invalid array index_1 */
	 {"test_conf_blk_str_array1", defvalue_string, UINT32_MAX,
	   (TEST_CONF_DEFVALUE_STR_LEN + 1)},
	/* invalid array index_2 */
	 {"test_conf_blk_str_array1", "inval", UINT32_MAX,
	   (sizeof("inval") + 1)}}
};

/*
 * Test Case
 *      pfc_conf_array_copy_string() 2
 */
TEST(conf, pfc_conf_array_copy_string_2)
{
	int ret, i, j;
	pfc_cfblk_t cf[TEST_CONF_HDL_CNT];
	pfc_conf_t confp;
	char *buf;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cf[0] = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cf[0], PFC_CFBLK_INVALID);
	cf[1] = pfc_conf_get_map(confp, "test_conf_map_str",
				 "test_conf_map_str_k1");
	ASSERT_NE(cf[1], PFC_CFBLK_INVALID);

	for (i = 0; i < TEST_CONF_HDL_CNT; i++) {
		for (j = 0; j < ARY_CPY_STR_2_TEST_COUNT; j++) {
			buf = (char *)malloc(tc_ary_cpy_str2[i][j].bufsz);
			ASSERT_TRUE(buf != NULL);
			memset(buf, TEST_CONF_DUMMY_DATA,
			       tc_ary_cpy_str2[i][j].bufsz);
			pfc_conf_array_copy_string(cf[i],
						   tc_ary_cpy_str2[i][j].param,
						   tc_ary_cpy_str2[i][j].index,
						   defvalue_string, buf,
						   (tc_ary_cpy_str2[i][j].
						    bufsz - 1));
			/* overflow check */
			ASSERT_EQ(TEST_CONF_DUMMY_DATA,
				  *(buf + tc_ary_cpy_str2[i][j].bufsz - 1));
			ASSERT_STREQ(tc_ary_cpy_str2[i][j].expect, buf);
			free(buf);
		}
	}

	/*
	 * invalid handle
	 *     Handle and Parameter value are no object.
	 */
	buf = (char *)malloc(tc_ary_cpy_str2[0][1].bufsz);
	memset(buf, TEST_CONF_DUMMY_DATA, tc_ary_cpy_str2[0][1].bufsz);
	pfc_conf_array_copy_string(PFC_CFBLK_INVALID,
				    tc_ary_cpy_str2[0][0].param, 
				    tc_ary_cpy_str2[0][0].index,
				    defvalue_string, buf,
				    (tc_ary_cpy_str2[0][1].bufsz - 1));
	ASSERT_STREQ("inval", buf);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA,
		  *(buf + tc_ary_cpy_str2[0][1].bufsz - 1));
	free(buf);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_byte_range() 2
 */
TEST(conf, pfc_conf_array_byte_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	uint8_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						 TEST_CONF_DUMMY_DATA,
						 TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_byte");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_byte_range(cfblk, "test_conf_blk_byte_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_byte_range(cfblk, "test_conf_blk_byte_array1",
					TEST_CONF_MIN_INDEX,
					TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_string_range() 2
 */
TEST(conf, pfc_conf_array_string_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	const char **buf;

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_str");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	buf = (const char **)malloc(ARY_RNG_STR_BUF_SIZE);
	ASSERT_TRUE(buf != NULL);

	memset(buf, TEST_CONF_DUMMY_DATA, ARY_RNG_STR_BUF_SIZE);

	/* nelems > array size */
	ret = pfc_conf_array_string_range(cfblk, "test_conf_blk_str_array1",
					 TEST_CONF_MIN_INDEX,
					 (TEST_CONF_MAX_ARRAY_SIZE + 1),
					 buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_string_range(cfblk, "test_conf_blk_str_array1",
					  TEST_CONF_MIN_INDEX,
					  TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA,
		  *(uint8_t *)(buf + TEST_CONF_ARRAY_RANGE_ELEM));

	free(buf);
	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_bool_range() 2
 */
TEST(conf, pfc_conf_array_bool_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	pfc_bool_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						    TEST_CONF_DUMMY_DATA,
						    TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_bool");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_bool_range(cfblk, "test_conf_blk_bool_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_bool_range(cfblk, "test_conf_blk_bool_array1",
					TEST_CONF_MIN_INDEX,
					TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_int32_range() 2
 */
TEST(conf, pfc_conf_array_int32_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	int32_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						 TEST_CONF_DUMMY_DATA,
						 TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i32_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_int32_range(cfblk, "test_conf_blk_i32_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_int32_range(cfblk, "test_conf_blk_i32_array1",
					 TEST_CONF_MIN_INDEX,
					 TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_uint32_range() 2
 */
TEST(conf, pfc_conf_array_uint32_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	uint32_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						  TEST_CONF_DUMMY_DATA,
						  TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ui32");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_uint32_range(cfblk, "test_conf_blk_ui32_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_uint32_range(cfblk, "test_conf_blk_ui32_array1",
					  TEST_CONF_MIN_INDEX,
					  TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ((uint32_t)TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_int64_range() 2
 */
TEST(conf, pfc_conf_array_int64_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	int64_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						 TEST_CONF_DUMMY_DATA,
						 TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_i64_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_int64_range(cfblk, "test_conf_blk_i64_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_int64_range(cfblk, "test_conf_blk_i64_array1",
					 TEST_CONF_MIN_INDEX,
					 TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_uint64_range() 2
 */
TEST(conf, pfc_conf_array_uint64_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	uint64_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						  TEST_CONF_DUMMY_DATA,
						  TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ui64");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_uint64_range(cfblk, "test_conf_blk_ui64_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_uint64_range(cfblk, "test_conf_blk_ui64_array1",
					  TEST_CONF_MIN_INDEX,
					  TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ((uint64_t)TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_long_range() 2
 */
TEST(conf, pfc_conf_array_long_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	pfc_long_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						    TEST_CONF_DUMMY_DATA,
						    TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_lng_min");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_long_range(cfblk, "test_conf_blk_lng_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_long_range(cfblk, "test_conf_blk_lng_array1",
					 TEST_CONF_MIN_INDEX,
					 TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ(TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}

/*
 * Test Case
 *      pfc_conf_array_ulong_range() 2
 */
TEST(conf, pfc_conf_array_ulong_range_2)
{
	int ret;
	pfc_conf_t confp;
	pfc_cfblk_t cfblk;
	pfc_ulong_t buf[TEST_CONF_MAX_ARRAY_SIZE] = {TEST_CONF_DUMMY_DATA,
						     TEST_CONF_DUMMY_DATA,
						     TEST_CONF_DUMMY_DATA};

        TmpFile tmpf("conf_test");
        setup_conffile(tmpf);
        RETURN_ON_ERROR();

	ret = pfc_conf_open(&confp, tmpf.getPath(), &test_conf_cfdef);
	ASSERT_EQ(ret, 0);
	cfblk = pfc_conf_get_block(confp, "test_conf_blk_ulng");
	ASSERT_NE(cfblk, PFC_CFBLK_INVALID);

	/* nelems > array size */
	ret = pfc_conf_array_ulong_range(cfblk, "test_conf_blk_ulng_array1",
					TEST_CONF_MIN_INDEX,
					(TEST_CONF_MAX_ARRAY_SIZE + 1),
					buf);
	ASSERT_EQ(ERANGE, ret);

	ret = pfc_conf_array_ulong_range(cfblk, "test_conf_blk_ulng_array1",
					  TEST_CONF_MIN_INDEX,
					  TEST_CONF_ARRAY_RANGE_ELEM, buf);
	ASSERT_EQ(0, ret);
	/* overflow check */
	ASSERT_EQ((pfc_ulong_t)TEST_CONF_DUMMY_DATA, buf[TEST_CONF_MAX_INDEX]);

	pfc_conf_close(confp);
}
