/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for conflog init
 */

#include <iostream>
#include <cstring>
#include <pfcxx/conf.hh>
#include <pfc/log.h>
#include "log_impl.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include "test.h"
#include "tmpfile.hh"

extern pfc_cfdef_t	test_loginit_cfdef;

/*
 * Contents of test configuration files.
 */
static const char  *test_options[] = {
    "",
    "log_syslog = true;",
    "log_syslog = false;",
    "log_facility = \"daemon\";",
    "log_facility = \"local0\";",
    "log_facility = \"local1\";",
    "log_facility = \"local2\";",
    "log_facility = \"local3\";",
    "log_facility = \"local4\";",
    "log_facility = \"local5\";",
    "log_facility = \"local6\";",
    "log_facility = \"local7\";",
    "message_rotate = 0;",
    "message_rotate = 100;",
    "message_size = 0;",
    "message_size = 50000000;",
    "log_level = fatal;",
    "log_level = error;",
    "log_level = warning;",
    "log_level = notice;",
    "log_level = info;",
    "log_level = debug;",
    "log_level = trace;",
    "log_level = verbose;",
};

namespace pfc {
namespace core {
namespace test_log_init {

class LogConf
{
public:
    LogConf();
    ~LogConf();

    pfc_log_conf_t &
    getConf(void)
    {
        return _conf;
    }

private:
    pfc_log_conf_t       _conf;
};

LogConf::LogConf()
{
    memset(&_conf, 0, sizeof(_conf));
    _conf.plc_level.plvc_level = PFC_LOGLVL_NONE;
    _conf.plc_level.plvc_deflevel = PFC_LOGLVL_INFO;
}

LogConf::~LogConf()
{
    if (_conf.plc_logdir != NULL) {
        free(const_cast<char *>(_conf.plc_logdir));
    }
    if (_conf.plc_logpath != NULL) {
        free(const_cast<char *>(_conf.plc_logpath));
    }
    if (_conf.plc_lvlpath != NULL) {
        free(const_cast<char *>(_conf.plc_lvlpath));
    }
}

/*
 * Valid logging facilities.
 */
static const char	*log_facilities[] = {
    "daemon",
    "local0",
    "local1",
    "local2",
    "local3",
    "local4",
    "local5",
    "local6",
    "local7",
};

static inline int
log_facility_value(const char *name, pfc_log_facl_t *fap)
{
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(log_facilities); i++) {
        const char  *facility(log_facilities[i]);

        if (strcasecmp(name, facility) == 0) {
            *fap = static_cast<pfc_log_facl_t>(i);
            return 0;
        }
    }

    return EINVAL;
}

#define LOG_IDENT	"test"
#ifndef LOG_INIT

#define DEFAULT_RCOUNT	10U
#define DEFAULT_RSIZE	10000000U
#define DEFAULT_LOGLVL	"info"

#define	IS_LOGLVL_VALID(level)                                          \
    ((level) >= PFC_LOGLVL_FATAL && (level) <= PFC_LOGLVL_VERBOSE)

#define	IS_SYSLOGLVL_VALID(level)                                       \
    ((level) >= PFC_LOGLVL_FATAL && (level) <= PFC_LOGLVL_DEBUG)

/* Check value of pfc_log_conf_t */
static void
check_log_conf(pfc_log_conf_t conf, const char* PFC_RESTRICT ident,
               pfc_log_fatal_t handler, pfc_cfblk_t blk, pfc_bool_t early,
               FILE *out, pfc_log_level_t level)
{ 
    ASSERT_EQ(PFC_LOGLVL_INFO, conf.plc_level.plvc_deflevel);

    if (early) {
        ASSERT_EQ(LOG_TYPE_STREAM, conf.plc_type);

        pfc_log_level_t required((IS_LOGLVL_VALID(level)) ? level
                                 : PFC_LOGLVL_NONE);
        ASSERT_EQ(required, conf.plc_level.plvc_level);
        ASSERT_EQ(out, conf.plc_output);
    } else {
        if (blk == PFC_CFBLK_INVALID) {
            ASSERT_EQ(LOG_TYPE_FILE, conf.plc_type);
            ASSERT_EQ(PFC_LOGFACL_LOCAL0, conf.plc_facility);
            ASSERT_EQ(DEFAULT_RCOUNT, conf.plc_rcount);
            ASSERT_EQ(DEFAULT_RSIZE, conf.plc_rsize);
        } else {
            pfc_bool_t log_syslog;
            const char *facility;
            uint32_t rcount;
            uint32_t rsize;

            log_syslog = pfc_conf_get_bool(blk, "log_syslog", PFC_FALSE);
            facility = pfc_conf_get_string(blk, "log_facility", "local0");
            rcount = pfc_conf_get_uint32(blk, "message_rotate", DEFAULT_RCOUNT);
            rsize = static_cast<size_t>(pfc_conf_get_uint32(blk, "message_size",
                                        DEFAULT_RSIZE));

            if (log_syslog) {
                ASSERT_EQ(LOG_TYPE_SYSLOG, conf.plc_type);
            } else {
                ASSERT_EQ(LOG_TYPE_FILE, conf.plc_type);
            }

            pfc_log_facl_t  facl(PFC_LOGFACL_LOCAL0);
            (void)log_facility_value(facility, &facl);
            ASSERT_EQ(facl, conf.plc_facility);
            ASSERT_EQ(rcount, conf.plc_rcount);
            ASSERT_EQ(rsize, conf.plc_rsize);
        }

        ASSERT_EQ(PFC_LOGLVL_NONE, conf.plc_level.plvc_level);
        ASSERT_EQ(reinterpret_cast<FILE *>(NULL), conf.plc_output);
    }

    ASSERT_EQ(blk, conf.plc_cfblk);
    ASSERT_EQ(ident, conf.plc_ident);
    ASSERT_EQ((void*)handler, (void*)conf.plc_handler);
    ASSERT_EQ(reinterpret_cast<const char *>(NULL), conf.plc_logdir);
    ASSERT_EQ(reinterpret_cast<const char *>(NULL), conf.plc_logpath);
    ASSERT_EQ(reinterpret_cast<const char *>(NULL), conf.plc_lvlpath);
}

/* Check to pfc_logconf_early() */
static void
check_log_conf_early(pfc_log_conf_t conf, const char* PFC_RESTRICT ident,
                     pfc_log_fatal_t handler, pfc_cfblk_t blk,
                     FILE *PFC_RESTRICT out, pfc_log_level_t level)
{
    check_log_conf(conf, ident, handler, blk, PFC_TRUE, out, level);
}

/* Check to pfc_logconf_init() */
static void
check_log_conf_normal(pfc_log_conf_t conf, const char* PFC_RESTRICT ident,
                      pfc_log_fatal_t handler, pfc_cfblk_t blk)
{
    check_log_conf(conf, ident, handler, blk, PFC_FALSE, NULL,
                   PFC_LOGLVL_NONE);
}

/* Cleanup pfc_log_conf_t */
static void
cleanup_logconf(pfc_log_conf_t conf)
{
    if (conf.plc_logdir != NULL) {
        free((void *)conf.plc_logdir);
    }
    if (conf.plc_logpath != NULL) {
        free((void *)conf.plc_logpath);
    }
    if (conf.plc_lvlpath != NULL) {
        free((void *)conf.plc_lvlpath);
    }
}

static void
check_loglevel_early(FILE *out, pfc_cfblk_t blck)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    int level;

    for (level = -20; level <= 20; level++) {
        pfc_logconf_early(&conf, blck, LOG_IDENT, out,
                          (pfc_log_level_t)level, NULL);
        check_log_conf_early(conf, LOG_IDENT, NULL, blck,
                             out, (pfc_log_level_t)level);
        RETURN_ON_ERROR();
    }

    cleanup_logconf(conf);
}

class Rename
{
public:
    Rename(std::string oldname, std::string newname) {
        tmp_name = std::string("./test_loginit.tmp");
        old_name = std::string("./") + oldname;
        new_name = std::string("./") + newname;
        rename(new_name.c_str(), tmp_name.c_str());
        rename(old_name.c_str(), new_name.c_str());
    }

    ~Rename() {
        rename(new_name.c_str(), old_name.c_str());
        rename(tmp_name.c_str(), new_name.c_str());
     }

private:
    std::string tmp_name;
    std::string old_name;
    std::string new_name;
};

/*
 *
 * test cases
 *
 */ 

/* test for log */
TEST(loginit, pfc_logconf_init)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());

    // Create an empty configuration file.
    TmpFile tmpf("loginit_test");
    ASSERT_EQ(0, tmpf.createFile());
    const char *cfpath(tmpf.getPath());

    ConfHandle conf_handle(cfpath, &test_loginit_cfdef);
    ASSERT_EQ(0, conf_handle.getError());
    ConfBlock undef_options(conf_handle, "options");

    pfc_logconf_init(&conf, undef_options.getBlock(), LOG_IDENT, NULL);
    ASSERT_EQ(PFC_CFBLK_INVALID,undef_options.getBlock());
    check_log_conf_normal(conf, LOG_IDENT, NULL, undef_options.getBlock());

    // Create configuration files which have valid options.
    FILE *fp(tmpf.getFile());
    int fd(fileno(fp));
    for (const char **optpp(test_options);
         optpp < PFC_ARRAY_LIMIT(test_options); optpp++) {
        rewind(fp);
        ASSERT_EQ(0, ftruncate(fd, 0)) << "*** ERROR: " << strerror(errno);
        fprintf(fp, "options {\n    %s\n}\n", *optpp);
        fflush(fp);

        ASSERT_EQ(0, conf_handle.reload());
        ConfBlock def_options(conf_handle, "options");
        pfc_logconf_init(&conf, def_options.getBlock(), LOG_IDENT, NULL);
        check_log_conf_normal(conf, LOG_IDENT, NULL, def_options.getBlock());
    }

    pfc_log_fini();
    cleanup_logconf(conf);
}

TEST(loginit, pfc_logconf_early)
{
    // Create an empty configuration file.
    TmpFile tmpf("loginit_test");
    ASSERT_EQ(0, tmpf.createFile());
    const char *cfpath(tmpf.getPath());

    ConfHandle conf_handle(cfpath, &test_loginit_cfdef);
    ASSERT_EQ(0, conf_handle.getError());

    ConfBlock undef_options(conf_handle, "options");
    check_loglevel_early(stdin, PFC_CFBLK_INVALID);
    RETURN_ON_ERROR();

    check_loglevel_early(stdout, PFC_CFBLK_INVALID);
    RETURN_ON_ERROR();

    check_loglevel_early(stderr, PFC_CFBLK_INVALID);
    RETURN_ON_ERROR();

    // Create configuration files which have valid options.
    FILE *fp(tmpf.getFile());
    int fd(fileno(fp));
    for (const char **optpp(test_options);
         optpp < PFC_ARRAY_LIMIT(test_options); optpp++) {
        rewind(fp);
        ASSERT_EQ(0, ftruncate(fd, 0)) << "*** ERROR: " << strerror(errno);
        fprintf(fp, "options {\n    %s\n}\n", *optpp);
        fflush(fp);

        ASSERT_EQ(0, conf_handle.reload());
        ConfBlock def_options(conf_handle, "options");
        check_loglevel_early(stdin, def_options.getBlock());
        RETURN_ON_ERROR();

        check_loglevel_early(stdout, def_options.getBlock());
        RETURN_ON_ERROR();

        check_loglevel_early(stderr, def_options.getBlock());
        RETURN_ON_ERROR();
    }
}

TEST(loginit, pfc_logconf_setpath)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    std::string basename("test_log/configure/base");
    std::string dirname("test_dir");
    std::string filename("test_filename.txt");

    pfc_logconf_early(&conf, PFC_CFBLK_INVALID, LOG_IDENT, stdout,
                      PFC_LOGLVL_NONE, NULL);
    pfc_logconf_setpath(&conf,
                        basename.c_str(), basename.length(),
                        dirname.c_str(), dirname.length(),
                        filename.c_str(), filename.length());
    ASSERT_EQ(conf.plc_logdir, basename + "/" + dirname);
    ASSERT_EQ(conf.plc_logpath, basename + "/" + dirname + "/" + filename);
}

TEST(loginit, pfc_logconf_setpath_nodir)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    std::string basename("test_log/configure/base/logdir");
    std::string filename("test_filename_nodir.txt");

    pfc_logconf_early(&conf, PFC_CFBLK_INVALID, LOG_IDENT, stdout,
                      PFC_LOGLVL_NONE, NULL);
    pfc_logconf_setpath(&conf,
                        basename.c_str(), basename.length(),
                        NULL, 0,
                        filename.c_str(), filename.length());
    ASSERT_EQ(conf.plc_logdir, basename);
    ASSERT_EQ(conf.plc_logpath, basename + "/" + filename);
}

TEST(loginit, pfc_logconf_setlvlpath)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    std::string basename("test_log/configure/base");
    std::string filename("test_filename.txt");

    pfc_logconf_early(&conf, PFC_CFBLK_INVALID, LOG_IDENT, stdout,
                      PFC_LOGLVL_NONE, NULL);
    pfc_logconf_setlvlpath(&conf,
                           basename.c_str(), basename.length(),
                           filename.c_str(), filename.length());
    ASSERT_EQ(conf.plc_lvlpath, basename + "/" + filename);
}

TEST(loginit, pfc_logconf_setrotate)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    uint32_t rcount;
    size_t rsize;

    rcount = 0;
    rsize = 0;
    pfc_logconf_setrotate(&conf, rcount, rsize);
    ASSERT_EQ(rcount, conf.plc_rcount);
    ASSERT_EQ(rsize, conf.plc_rsize);

    rcount = 100;
    rsize = 100;
    pfc_logconf_setrotate(&conf, rcount, rsize);
    ASSERT_EQ(rcount, conf.plc_rcount);
    ASSERT_EQ(rsize, conf.plc_rsize);

    rcount = 100;
    rsize = 10000000;
    pfc_logconf_setrotate(&conf, rcount, rsize);
    ASSERT_EQ(rcount, conf.plc_rcount);
    ASSERT_EQ(rsize, conf.plc_rsize);
}

TEST(loginit, pfc_logconf_setlevel)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    int level;
    pfc_log_level_t prev_lvl(PFC_LOGLVL_NONE);

    /* Check to set logging level */
    for (level = -20; level <= 20; level++) {
        pfc_log_level_t  lvl(static_cast<pfc_log_level_t>(level));
        pfc_logconf_setlevel(&conf, lvl);

        pfc_log_level_t reqlvl;
        if (IS_LOGLVL_VALID(lvl)) {
            reqlvl = lvl;
            prev_lvl = lvl;
        }
        else {
            reqlvl = prev_lvl;
        }

        ASSERT_EQ(reqlvl, conf.plc_level.plvc_level);
        ASSERT_EQ(PFC_LOGLVL_INFO, conf.plc_level.plvc_deflevel);
    }
}

TEST(loginit, pfc_logconf_setdeflevel)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    int level;
    pfc_log_level_t prev_lvl(PFC_LOGLVL_INFO);

    /* Check to set logging level */
    for (level = -20; level <= 20; level++) {
        pfc_log_level_t  lvl(static_cast<pfc_log_level_t>(level));
        pfc_logconf_setdeflevel(&conf, lvl);

        pfc_log_level_t reqlvl;
        if (IS_LOGLVL_VALID(lvl)) {
            reqlvl = lvl;
            prev_lvl = lvl;
        }
        else {
            reqlvl = prev_lvl;
        }

        ASSERT_EQ(reqlvl, conf.plc_level.plvc_deflevel);
        ASSERT_EQ(PFC_LOGLVL_NONE, conf.plc_level.plvc_level);
    }
}

TEST(loginit, pfc_logconf_setsyslog)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    pfc_logconf_setsyslog(&conf, PFC_TRUE);
    ASSERT_EQ(LOG_TYPE_SYSLOG, conf.plc_type);
    pfc_logconf_setsyslog(&conf, PFC_FALSE);
    ASSERT_EQ(LOG_TYPE_FILE, conf.plc_type);
}

TEST(loginit, pfc_logconf_setfacility)
{
    LogConf lc;
    pfc_log_conf_t &conf(lc.getConf());
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, LOG_IDENT, NULL);

    pfc_log_facl_t  prev(conf.plc_facility);
    ASSERT_EQ(PFC_LOGFACL_LOCAL0, prev);

    for (int facl(-50); facl <= 50; facl++) {
        pfc_log_facl_t  facility(static_cast<pfc_log_facl_t>(facl));
        pfc_log_facl_t  required;

        if ((uint32_t)facility >= PFC_ARRAY_CAPACITY(log_facilities)) {
            required = prev;
        }
        else {
            required = facility;
        }

        pfc_logconf_setfacility(&conf, facility);
        ASSERT_EQ(required, conf.plc_facility);
        prev = conf.plc_facility;
    }
}

#else

static int fatal_handler_invoked;

static void
fatal_handler()
{
    fatal_handler_invoked += 1;
} 

/*
 * This test creates log file "test_loginit.log".
 * If you finish this test, remove the log file.
 * You can change output stream for this test.
 *  - Defined macro "TEST_LOGINIT_STDOUT" -> out to stdout.
 *  - Defined macro "TEST_LOGINIT_STDERR" -> out to stderr.
 *  - Not defined macro "STDOUT" & "STDERR" -> out to file.
 */ 
TEST(loginit, pfc_log_init)
{
    pfc_log_level_t level = PFC_LOGLVL_VERBOSE;

#ifdef TEST_LOGINIT_STDOUT
    /* STDOUT */
    pfc_log_init(LOG_IDENT, stdout, level, fatal_handler);
#elif TEST_LOGINIT_STDERR
    /* STDERR */
    pfc_log_init(LOG_IDENT, stderr, level, fatal_handler);
#else
    /* FILE */
    FILE *file;
    file = fopen("./test_loginit.log", "w");
    ASSERT_TRUE(file != NULL);
    pfc_log_init(LOG_IDENT, file, level, fatal_handler);
#endif

    fatal_handler_invoked = 0;
    pfc_log_fatal("Fatal log message.");
    ASSERT_EQ(1, fatal_handler_invoked);
    pfc_log_error("Error log message.");
    pfc_log_warn("Warn log message.");
    pfc_log_notice("Notice log message.");
    pfc_log_info("Info log message.");
    pfc_log_debug("Debug log message.");
    pfc_log_trace("Trace log message.");
    pfc_log_verbose("Verbose log message.");

    pfc_syslog_fatal("Fatal syslog message.");
    ASSERT_EQ(2, fatal_handler_invoked);
    pfc_syslog_error("Error syslog message.");
    pfc_syslog_warn("Warn syslog message.");
    pfc_syslog_notice("Notice syslog message.");
    pfc_syslog_info("Info syslog message.");
    pfc_syslog_debug("Debug syslog message.");

#ifndef TEST_LOGINIT_STDOUT
#ifndef TEST_LOGINIT_STDERR
    fclose(file);
#endif
#endif

}
#endif

}// namespace test_loginit
}// namespace core
}// namespace pfc
