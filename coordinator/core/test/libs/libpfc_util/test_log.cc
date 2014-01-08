/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_log.cc - Test for PFC logging system.
 */

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <sysexits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <pfc/log.h>
#include <pfc/conf.h>
#include <pfc/util.h>
#include <pfc/strtoint.h>
#include <log_impl.h>
#include "test.h"
#include "misc.hh"
#include "tmpfile.hh"
#include "child.hh"

#define IS_LOGLVL_VALID(level)                                          \
    ((level) >= PFC_LOGLVL_FATAL && (level) <= PFC_LOGLVL_VERBOSE)

#define IS_LOGLVL_CHAR_VALID(c)                 \
    (((c) >= '0' && (c) <= '7') || (c) == 'n')

#define IS_SYSLOGLVL_VALID(level)                                       \
    ((level) >= PFC_LOGLVL_FATAL && (level) <= PFC_LOGLVL_DEBUG)

#define IS_SYSLOGLVL_CHAR_VALID(c)              \
    (((c) >= '0' && (c) <= '5') || (c) == 'n')

/*
 * Logging levels.
 */
static const char    *log_levels[] = {
    "fatal",
    "error",
    "warning",
    "notice",
    "info",
    "debug",
    "trace",
    "verbose"
};

/*
 * Use configuration file defined by test_loginit.cc.
 */
extern pfc_cfdef_t  test_loginit_cfdef;

static uint32_t   log_path_index = 0;

#define TEST_LOG_IDENT       "test_log"

/*
 * Log file instance.
 */
class LogFile
{
public:
    typedef enum {
        CHECK_NONE,             // Log file must not exist.
        CHECK_EXISTS,           // Log file must exist, and not be empty.
        CHECK_EMPTY,            // Log file must exist, and be empty.
    } checkmode_t;

    LogFile(const char *prefix = "test_log");
    ~LogFile();

    void   check(checkmode_t cmode = CHECK_EMPTY);
    void   search(const char *pattern, bool &found);

    inline void
    setPath(pfc_log_conf_t &conf)
    {
        pfc_logconf_setpath(&conf, _base.c_str(), _base.length(),
                            _parent.c_str(), _parent.length(),
                            _name.c_str(), _name.length());
    }

    inline void
    getPath(std::string &path)
    {
        path.assign(_base);
        path.append("/");
        path.append(_parent);
        path.append("/");
        path.append(_name);
    }

    inline void
    getParent(std::string &path)
    {
        path.assign(_base);
        path.append("/");
        path.append(_parent);
    }

    inline std::string &
    getBase(void)
    {
        return _base;
    }

    inline std::string &
    getName(void)
    {
        return _name;
    }

private:
    std::string       _base;            // base directory path
    std::string       _parent;          // parent directory path
    std::string       _name;            // file name
};

LogFile::LogFile(const char *prefix)
{
    char   base[32], parent[32], name[32];
    pid_t  pid(getpid());

    char   curdir[PATH_MAX];
    char   *cwd(getcwd(curdir, sizeof(curdir)));
    if (cwd == NULL) {
        curdir[0] = '.';
        curdir[1] = '\0';
    }

    uint32_t  idx(log_path_index);
    log_path_index++;

    snprintf(base, sizeof(base), "%s.%u", prefix, pid);
    snprintf(parent, sizeof(parent), "logdir_%u", idx);
    snprintf(name, sizeof(name), "log_file_%u", idx);

    _base.assign(curdir);
    _base.append("/");
    _base.append(base);

    _parent.assign(parent);
    _name.assign(name);
}

LogFile::~LogFile()
{
    (void)pfc_rmpath(_base.c_str());
}

void
LogFile::check(checkmode_t cmode)
{
    struct stat  sbuf;

    int  ret(lstat(_base.c_str(), &sbuf));

    if (cmode == CHECK_NONE) {
        ASSERT_EQ(-1, ret);
        ASSERT_EQ(ENOENT, errno);

        return;
    }

    ASSERT_EQ(0, ret)
        << "*** ERROR: " << strerror(errno);
    ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
    ASSERT_EQ(static_cast<mode_t>(0700), sbuf.st_mode & 07777);

    std::string  parent(_base);
    parent.append("/");
    parent.append(_parent);
    int  dfd(pfc_open_cloexec(parent.c_str(), O_RDONLY));
    ASSERT_NE(-1, dfd);
    FdRef  dref(dfd);

    ASSERT_EQ(0, fstat(dfd, &sbuf));
    ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
    ASSERT_EQ(static_cast<mode_t>(0700), sbuf.st_mode & 07777);

    ASSERT_EQ(0, fstatat(dfd, _name.c_str(), &sbuf, AT_SYMLINK_NOFOLLOW));
    ASSERT_TRUE(S_ISREG(sbuf.st_mode));
    ASSERT_EQ(static_cast<mode_t>(0600), sbuf.st_mode & 07777);

    if (cmode == CHECK_EMPTY) {
        ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);
    }
    else {
        ASSERT_NE(static_cast<off_t>(0), sbuf.st_size);
    }
}

void
LogFile::search(const char *pattern, bool &found)
{
    found = false;

    std::string  path;
    getPath(path);

    FILE  *fp(pfc_fopen_cloexec(path.c_str(), "r"));
    ASSERT_TRUE(fp != NULL);
    StdioRef fpref(fp);

    char   line[1024];
    RegEx  regex(pattern);
    RETURN_ON_ERROR();

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (regex.search(line)) {
            found = true;
            break;
        }
    }
}

/*
 * Log level file instance.
 */
class LogLevelFile
{
public:
    static inline char
    toChar(pfc_log_level_t level)
    {
        if (level >= PFC_LOGLVL_FATAL && level <= PFC_LOGLVL_VERBOSE) {
            uint32_t  i(static_cast<uint32_t>(level));

            return '0' + i;
        }

        return 'n';
    }

    static inline pfc_log_level_t
    toLevel(char c)
    {
        if (c == 'n') {
            return PFC_LOGLVL_NONE;
        }

        int lvl(c - '0');

        return static_cast<pfc_log_level_t>(lvl);
    }

    LogLevelFile(const char *prefix = "loglevel");
    LogLevelFile(const char *data, size_t size,
                 const char *prefix = "loglevel");
    ~LogLevelFile();

    int   exists(void);
    void  check(pfc_log_level_t level);

    inline void
    setLevelPath(pfc_log_conf_t &conf)
    {
        pfc_logconf_setlvlpath(&conf, _parent.c_str(), _parent.length(),
                               _name.c_str(), _name.length());
    }

    inline void
    getPath(std::string &path)
    {
        path.assign(_parent);
        path.append("/");
        path.append(_name);
    }

private:
    void  setUp(const char *prefix, const char *data, size_t size);

    std::string   _parent;           // parent directory
    std::string   _name;             // name of log level file
};

LogLevelFile::LogLevelFile(const char *prefix)
{
    setUp(prefix, NULL, 0);
}

LogLevelFile::LogLevelFile(const char *data, size_t size, const char *prefix)
{
    setUp(prefix, data, size);
}

LogLevelFile::~LogLevelFile()
{
    (void)pfc_rmpath(_parent.c_str());
}

int
LogLevelFile::exists(void)
{
    std::string  path;
    getPath(path);

    struct stat  sbuf;
    if (lstat(path.c_str(), &sbuf) == 0) {
        return 0;
    }

    return errno;
}

void
LogLevelFile::check(pfc_log_level_t level)
{
    std::string  path;
    getPath(path);

    int  fd(pfc_open_cloexec(path.c_str(), O_RDONLY));
    ASSERT_NE(-1, fd);
    FdRef fref(fd);

    struct stat  sbuf;
    ASSERT_EQ(0, fstat(fd, &sbuf)) << "*** ERROR: " << strerror(errno);
    ASSERT_TRUE(S_ISREG(sbuf.st_mode));
    ASSERT_EQ(static_cast<mode_t>(0600), sbuf.st_mode & 07777);
    ASSERT_EQ(1U, sbuf.st_size);

    char  data[1];
    ASSERT_EQ(static_cast<ssize_t>(sizeof(data)),
              read(fd, data, sizeof(data)));

    pfc_log_level_t lvl(toLevel(data[0]));
    ASSERT_EQ(level, lvl);
}

void
LogLevelFile::setUp(const char *prefix, const char *data, size_t size)
{
    char   curdir[PATH_MAX];
    char   *cwd(getcwd(curdir, sizeof(curdir)));
    if (cwd == NULL) {
        curdir[0] = '.';
        curdir[1] = '\0';
    }

    char   parent[32];
    snprintf(parent, sizeof(parent), "%s.%u", prefix, getpid());
    _parent.assign(curdir);
    _parent.append("/");
    _parent.append(parent);
    ASSERT_EQ(0, pfc_mkdir(_parent.c_str(), 0700));

    uint32_t  idx(log_path_index);
    log_path_index++;

    char   name[32];
    snprintf(name, sizeof(name), "log_level.%u", idx);
    _name.assign(name);

    if (data != NULL) {
        std::string  path(_parent);

        path.append("/");
        path.append(_name);

        int  fd(pfc_open_cloexec(path.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0600));
        ASSERT_NE(-1, fd);
        FdRef  fref(fd);

        if (size != 0) {
            ASSERT_EQ(static_cast<ssize_t>(size), write(fd, data, size));
        }
    }
}

typedef boost::function<void (const char *, std::string &)> lvlfile_create_t;

/*
 * static void
 * lvlfile_create_badsize(const char *path, std::string &errmsg, size_t size)
 *      Create a log level file whose size is invalid.
 */
static void
lvlfile_create_badsize(const char *path, std::string &errmsg, size_t size)
{
    int  fd(pfc_open_cloexec(path, O_RDWR|O_CREAT|O_TRUNC, 0600));

    ASSERT_NE(-1, fd);
    FdRef  fref(fd);

    ASSERT_EQ(0, ftruncate(fd, size)) << "*** ERROR: " << strerror(errno);

    char  buf[128];
    snprintf(buf, sizeof(buf), "Invalid loglevel file size: %s: %"
             PFC_PFMT_SIZE_T, path, size);
    errmsg.assign(buf);
}

/*
 * static void
 * lvlfile_create_badlevel(const char *path, std::string &errmsg, char lvlc)
 *      Create a log level file which contains the specified characters.
 */
static void
lvlfile_create_badlevel(const char *path, std::string &errmsg, char lvlc)
{
    int   fd(pfc_open_cloexec(path, O_RDWR|O_CREAT|O_TRUNC, 0600));
    FdRef fref(fd);
    char  data[] = {lvlc};
    ASSERT_EQ(static_cast<ssize_t>(sizeof(data)),
              write(fd, data, sizeof(data)));

    char  buf[128];
    if (!IS_LOGLVL_CHAR_VALID(lvlc)) {
        snprintf(buf, sizeof(buf),
                 "Broken trace log level in the loglevel file: %s: 0x%x",
                 path, lvlc);
    }
    else {
        buf[0] = '\0';
    }

    errmsg.assign(buf);
}

/*
 * static void
 * lvlfile_create_symlink(const char *path, std::string &errmsg)
 *      Create a symbolic link as the log level file.
 */
static void
lvlfile_create_symlink(const char *path, std::string &errmsg)
{
    int  err(pfc_rmpath(path));
    if (err != 0) {
        ASSERT_EQ(ENOENT, err);
    }

    ASSERT_EQ(0, symlink("/foo/bar", path))
        << "*** ERROR: " << strerror(errno);

    char  buf[128];
    snprintf(buf, sizeof(buf), "Loglevel file is not a regular file: %s",
             path);
    errmsg.assign(buf);
}

/*
 * static void
 * lvlfile_create_directory(const char *path, std::string &errmsg)
 *      Create a directory as the log level file.
 */
static void
lvlfile_create_directory(const char *path, std::string &errmsg)
{
    int  err(pfc_rmpath(path));
    if (err != 0) {
        ASSERT_EQ(ENOENT, err);
    }

    ASSERT_EQ(0, pfc_mkdir(path, 0700));

    char  buf[128];
    snprintf(buf, sizeof(buf), "Loglevel file is not a regular file: %s",
             path);
    errmsg.assign(buf);
}

/*
 * static void
 * lvlfile_create_node(const char *path, std::string &errmsg, mode_t type)
 *      Create a file node using mknod(2) as the log level file.
 */
static void
lvlfile_create_node(const char *path, std::string &errmsg, mode_t type)
{
    int  err(pfc_rmpath(path));
    if (err != 0) {
        ASSERT_EQ(ENOENT, err);
    }

    ASSERT_EQ(0, mknod(path, type | 0644, 0))
        << "*** ERROR: " << strerror(errno);

    char  buf[128];
    snprintf(buf, sizeof(buf), "Loglevel file is not a regular file: %s",
             path);
    errmsg.assign(buf);
}

/*
 * List of level file creator which create invalid file.
 */
typedef std::list<lvlfile_create_t>  lvlfile_crlist_t;

class BadLevelFileCreator
{
public:
    BadLevelFileCreator();

    lvlfile_crlist_t &
    getList(void)
    {
        return _list;
    }

private:
    lvlfile_crlist_t    _list;
};

BadLevelFileCreator::BadLevelFileCreator()
{
    // Empty log level file.
    {
        lvlfile_create_t  func(boost::bind(lvlfile_create_badsize, _1, _2, 0));
        _list.push_back(func);
    }

    // Invalid size of log level file.
    for (size_t size(3); size <= 10; size++) {
        lvlfile_create_t  func(boost::bind(lvlfile_create_badsize, _1, _2,
                                           size));
        _list.push_back(func);
    }

    // Symbolic link.
    {
        lvlfile_create_t  func(boost::bind(lvlfile_create_symlink, _1, _2));
        _list.push_back(func);
    }

    // Directory.
    {
        lvlfile_create_t  func(boost::bind(lvlfile_create_directory, _1, _2));
        _list.push_back(func);
    }

    // FIFO.
    {
        lvlfile_create_t  func(boost::bind(lvlfile_create_node, _1, _2,
                                           S_IFIFO));
        _list.push_back(func);
    }

    // Socket.
    {
        lvlfile_create_t  func(boost::bind(lvlfile_create_node, _1, _2,
                                           S_IFSOCK));
        _list.push_back(func);
    }
}

/*
 * Configuration file instance
 */
class ConfFile
{
public:
    ConfFile() : _conf(PFC_CONF_INVALID), _modconf(NULL), _modconfcnt(0) {}
    ~ConfFile();

    void  setUp(pfc_log_level_t level, pfc_cfblk_t &block);
    void  setUp(const char *level, pfc_cfblk_t &block);
    void  setUp(const pfc_log_modconf_t *mconf, uint32_t nmods,
                pfc_cfblk_t &block);
    int   copyModLevels(pfc_log_modconf_t *&levels);

private:
    pfc_conf_t          _conf;            // configuration file handle
    std::string         _path;            // configuration file path
    pfc_log_modconf_t   *_modconf;        // pfc_log_modconf_t array
    uint32_t            _modconfcnt;      // number of elements in _modconf
};

ConfFile::~ConfFile()
{
    pfc_log_modlevel_free(_modconf, _modconfcnt);

    if (_conf != PFC_CONF_INVALID) {
        pfc_conf_close(_conf);
    }

    if (_path.length() != 0) {
        (void)unlink(_path.c_str());
    }
}

void
ConfFile::setUp(pfc_log_level_t level, pfc_cfblk_t &block)
{
    char  name[32];

    snprintf(name, sizeof(name), "test_log_conf.%u", getpid());
    FILE  *fp(pfc_fopen_cloexec(name, "w"));
    ASSERT_TRUE(fp != NULL);
    _path.assign(name);

    {
        StdioRef fpref(fp);

        fputs("options {\n", fp);
        if (level != PFC_LOGLVL_NONE) {
            fprintf(fp, "    log_level = %s;\n", log_levels[level]);
        }
        fputs("}\n", fp);
    }

    ASSERT_EQ(0, pfc_conf_open(&_conf, name, &test_loginit_cfdef));
    block = pfc_conf_get_block(_conf, "options");
    ASSERT_NE(PFC_CFBLK_INVALID, block);
}

void
ConfFile::setUp(const char *level, pfc_cfblk_t &block)
{
    char  name[32];

    snprintf(name, sizeof(name), "test_log_conf.%u", getpid());
    FILE  *fp(pfc_fopen_cloexec(name, "w"));
    ASSERT_TRUE(fp != NULL);
    _path.assign(name);

    {
        StdioRef fpref(fp);

        fputs("options {\n", fp);
        if (level != NULL) {
            fprintf(fp, "    log_level = \"%s\";\n", level);
        }
        fputs("}\n", fp);
    }

    ASSERT_EQ(0, pfc_conf_open(&_conf, name, &test_loginit_cfdef));
    block = pfc_conf_get_block(_conf, "options");
    ASSERT_NE(PFC_CFBLK_INVALID, block);
}

void
ConfFile::setUp(const pfc_log_modconf_t *mconf, uint32_t nmods,
                pfc_cfblk_t &block)
{
    char  name[32];

    snprintf(name, sizeof(name), "test_log_conf.%u", getpid());
    FILE  *fp(pfc_fopen_cloexec(name, "w"));
    ASSERT_TRUE(fp != NULL);
    _path.assign(name);

    {
        StdioRef fpref(fp);

        fputs("options {\n", fp);
        if (nmods > 0) {
            fputs("    log_modlevel = [\n", fp);
            for (const pfc_log_modconf_t *plmp(mconf); plmp < mconf + nmods;
                 plmp++) {
                fprintf(fp, "        \"%s:%s\",\n", plmp->plm_name,
                        log_levels[plmp->plm_level]);
            }
            fputs("    ];\n", fp);
        }
        fputs("}\n", fp);
    }

    ASSERT_EQ(0, pfc_conf_open(&_conf, name, &test_loginit_cfdef));
    block = pfc_conf_get_block(_conf, "options");
    ASSERT_NE(PFC_CFBLK_INVALID, block);
}

int
ConfFile::copyModLevels(pfc_log_modconf_t *&levels)
{
    pfc_log_modlevel_free(_modconf, _modconfcnt);
    _modconfcnt = pfc_log_modlevel_copy(&_modconf);
    levels = _modconf;

    return _modconfcnt;
}

/*
 * static void
 * check_log_sysinit(pfc_log_conf_t &conf, pfc_log_level_t reqlvl,
 *                   bool do_fini)
 *      Initialize PFC logging system, and verifies logging level.
 */
static void
check_log_sysinit(pfc_log_conf_t &conf, pfc_log_level_t reqlvl,
                  bool do_fini = true)
{
    if (do_fini) {
        // Clear configuration done by other tests.
        pfc_log_fini();
    }

    pfc_log_sysinit(&conf);

    pfc_log_level_t  level(pfc_log_current_level);
    ASSERT_EQ(reqlvl, level);
}

/*
 * static void
 * check_logconf_early_explicit(pfc_cfblk_t options, LogLevelFile *lvfp)
 *      Ensure that logging levels passed to pfc_logconf_early() are always
 *      used.
 */
static void
check_logconf_early_explicit(pfc_cfblk_t options, LogLevelFile *lvfp)
{
    pfc_log_conf_t  conf;

    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));

        pfc_logconf_early(&conf, options, TEST_LOG_IDENT,
                          stderr, level, NULL);
        if (lvfp != NULL) {
            lvfp->setLevelPath(conf);
        }

        check_log_sysinit(conf, level);
        RETURN_ON_ERROR();
    }
}

/*
 * static void
 * check_logconf_init(pfc_log_conf_t &conf, pfc_log_level_t reqlvl)
 *      Initialize PFC logging system, and verifies logging level.
 *
 *      The caller must initialize `conf' using pfc_logconf_init().
 */
static void
check_logconf_init(pfc_log_conf_t &conf, pfc_log_level_t reqlvl)
{
    LogFile  file;

    file.setPath(conf);
    check_log_sysinit(conf, reqlvl);
    file.check();
}

/*
 * static void
 * check_logconf_init(LogFile &file)
 *      Initialize PFC logging system with default log level, and verifies
 *      log directory and file.
 */
static void
check_logconf_init(LogFile &file)
{
    pfc_log_conf_t  conf;
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);

    file.setPath(conf);
    check_log_sysinit(conf, PFC_LOGLVL_INFO);
    file.check();
}

/*
 * static void
 * check_logconf_init_explicit(pfc_cfblk_t options, LogLevelFile *lvfp)
 *      Ensure that logging levels set by pfc_logconf_setlevel() are always
 *      used.
 */
static void
check_logconf_init_explicit(pfc_cfblk_t options, LogLevelFile *lvfp)
{
    pfc_log_conf_t  conf;

    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        pfc_logconf_init(&conf, options, TEST_LOG_IDENT, NULL);
        if (lvfp != NULL) {
            lvfp->setLevelPath(conf);
        }

        pfc_logconf_setlevel(&conf, level);
        check_logconf_init(conf, level);
        RETURN_ON_ERROR();
    }
}

/*
 * static void
 * check_logconf_init_rotate(uint32_t nrotate)
 *      Ensure that pfc_log_sysinit() rotates the log file correctly.
 */
static void
check_logconf_init_rotate(uint32_t nrotate)
{
    const size_t  limit(1024);

    // Prepare log directory.
    LogFile     file;
    std::string parent;
    file.getParent(parent);
    ASSERT_EQ(0, pfc_mkdir(parent.c_str(), 0700));

    std::string  path;
    file.getPath(path);

    // Create old logs.
    for (uint32_t i(1); i <= nrotate; i++) {
        char  buf[32];

        snprintf(buf, sizeof(buf), "%u", i);
        std::string  fname(path);
        fname.append(".");
        fname.append(buf);

        FILE *fp(pfc_fopen_cloexec(fname.c_str(), "w"));
        ASSERT_TRUE(fp != NULL);
        StdioRef  fpref(fp);
        ASSERT_EQ(0, fchmod(fileno(fp), 0600));

        for (uint32_t sz(0); sz < i; sz++) {
            putc('0', fp);
        }
        ASSERT_EQ(0, ferror(fp));
    }

    // Create log file, and expand the file size to the limit.
    struct stat  sbuf_prev;
    {
        int  fd(pfc_open_cloexec(path.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0600));
        ASSERT_NE(-1, fd);
        FdRef fref(fd);

        ASSERT_EQ(0, ftruncate(fd, limit));
        ASSERT_EQ(0, fstat(fd, &sbuf_prev));
        ASSERT_EQ(static_cast<off_t>(limit), sbuf_prev.st_size);
    }

    // Initialize the logging system.
    pfc_log_conf_t  conf;
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
    pfc_logconf_setrotate(&conf, nrotate, limit);
    file.setPath(conf);
    check_log_sysinit(conf, PFC_LOGLVL_INFO);
    file.check();

    // Current log file must be empty.
    struct stat  sbuf;
    ASSERT_EQ(0, lstat(path.c_str(), &sbuf))
        << "*** ERROR: " << strerror(errno);
    ASSERT_TRUE(S_ISREG(sbuf.st_mode));
    ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);

    // Old log files must be rotated.
    std::string   &fnm(file.getName());
    const char    *fname(fnm.c_str());
    size_t        fnamelen(fnm.length());
    DIR           *dirp(opendir(parent.c_str()));
    ASSERT_TRUE(dirp != NULL);
    DirEntRef     dref(dirp);
    struct dirent *dp;

    while ((dp = readdir(dirp)) != NULL) {
        const char  *name(dp->d_name);
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
            strcmp(name, fname) == 0) {
            continue;
        }

        ASSERT_EQ(0, strncmp(name, fname, fnamelen));
        const char  *suffix(name + fnamelen);
        std::string  lpath(path);
        lpath.append(suffix);

        ASSERT_EQ('.', *suffix);
        suffix++;

        uint32_t  idx;
        ASSERT_EQ(0, pfc_strtou32(suffix, &idx));
        ASSERT_LE(idx, nrotate);

        if (idx == 1) {
            // This must be previous log file.
            ASSERT_EQ(0, lstat(lpath.c_str(), &sbuf))
                << "*** ERROR: " << strerror(errno);
            ASSERT_EQ(sbuf_prev.st_dev, sbuf.st_dev);
            ASSERT_EQ(sbuf_prev.st_ino, sbuf.st_ino);
            ASSERT_EQ(sbuf_prev.st_mode, sbuf.st_mode);
            ASSERT_EQ(sbuf_prev.st_mtime, sbuf.st_mtime);
            ASSERT_EQ(sbuf_prev.st_size, sbuf.st_size);
        }
        else {
            FILE      *fp(pfc_fopen_cloexec(lpath.c_str(), "r"));
            ASSERT_TRUE(fp != NULL);
            StdioRef  fpref(fp);

            uint32_t  sz(0);
            for (;;) {
                int  c(getc(fp));
                if (c == EOF) {
                    break;
                }

                ASSERT_EQ('0', c);
                sz++;
            }

            ASSERT_EQ(idx - 1, sz) << lpath;
        }
    }
}

/*
 * static void
 * check_logconf_init_badbase(ChildContext *child, TmpFile *errfile,
 *                            const char *base)
 *      Initialize the logging system on a child process, with specifying
 *      invalid base directory path.
 *
 *      This function is used to test error case in pfc_log_sysinit().
 */
static void
check_logconf_init_badbase(ChildContext *child, TmpFile *errfile,
                           const char *base)
{
    // Redirect stderr to the given file.
    int  fd(errfile->getDescriptor());
    ASSERT_NE(-1, fd);
    ASSERT_EQ(2, dup2(fd, 2)) << "*** ERROR: " << strerror(errno);
    ASSERT_EQ(0, close(fd)) << "*** ERROR: " << strerror(errno);

    pfc_log_conf_t  conf;
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);

    size_t  baselen(strlen(base));
    pfc_logconf_setpath(&conf, base, baselen, NULL, 0, "test.log", 8);

    pfc_log_fini();
    pfc_log_sysinit(&conf);
}

/*
 * static void
 * check_logconf_init_fail(ChildContext *child, TmpFile *errfile, LogFile *lfp)
 *      Initialize the logging system on a child process when no file
 *      descriptor is available.
 *
 *      This function is used to test error case in pfc_log_sysinit().
 */
static void
check_logconf_init_fail(ChildContext *child, TmpFile *errfile, LogFile *lfp)
{
    // Redirect stderr to the given file.
    int  fd(errfile->getDescriptor());
    ASSERT_NE(-1, fd);
    ASSERT_EQ(2, dup2(fd, 2)) << "*** ERROR: " << strerror(errno);
    ASSERT_EQ(0, close(fd)) << "*** ERROR: " << strerror(errno);

    pfc_log_conf_t  conf;
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);

    // Prepare log directory.
    std::string  parent;
    lfp->getParent(parent);
    ASSERT_EQ(0, pfc_mkdir(parent.c_str(), 0700));
    lfp->setPath(conf);

    pfc_log_fini();

    // Exhaust file descriptors.
    struct rlimit rlim;
    rlim.rlim_cur = 20;
    rlim.rlim_max = 20;
    if (!is_under_valgrind()) {
        ASSERT_EQ(0, setrlimit(RLIMIT_NOFILE, &rlim))
            << "*** ERROR: " << strerror(errno);
    }
    for (;;) {
        int  fd(dup(1));

        if (fd == -1) {
            ASSERT_EQ(EMFILE, errno);
            break;
        }
    }

    pfc_log_sysinit(&conf);
}

/*
 * static void
 * check_broken_levelfile(lvlfile_create_t &func)
 *      Ensure that broken log level file is always ignored.
 *      `func' must create an invalid log level file.
 */
static void
check_broken_levelfile(lvlfile_create_t &func)
{
    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        // Create broken log level file.
        LogLevelFile lvlfile;
        RETURN_ON_ERROR();

        std::string  errmsg, path;
        ASSERT_EQ(ENOENT, lvlfile.exists());
        lvlfile.getPath(path);
        (func)(path.c_str(), errmsg);
        RETURN_ON_ERROR();
        ASSERT_EQ(0, lvlfile.exists());

        // Ensure that broken level file is always ignored.
        pfc_log_conf_t  conf;
        pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
        pfc_logconf_setdeflevel(&conf, level);
        lvlfile.setLevelPath(conf);

        LogFile  file;
        LogFile::checkmode_t  cmode(LogFile::CHECK_EMPTY);
        file.setPath(conf);
        check_log_sysinit(conf, level);

        // Broken level file must be removed.
        ASSERT_EQ(ENOENT, lvlfile.exists());

        // Ensure that a warning message is logged.
        bool found(false);
        bool required;

        if (level >= PFC_LOGLVL_WARN) {
            required = true;
            cmode = LogFile::CHECK_EXISTS;
        }
        else {
            required = false;
        }
        file.search("Bogus loglevel file was reset\\.", found);
        ASSERT_EQ(required, found);

        // Ensure that an error message is logged.
        if (errmsg.length() != 0) {
            file.search(errmsg.c_str(), found);
            if (level >= PFC_LOGLVL_ERROR) {
                required = true;
                cmode = LogFile::CHECK_EXISTS;
            }
            else {
                required = false;
            }
            ASSERT_EQ(required, found);
        }

        file.check(cmode);
        RETURN_ON_ERROR();
    }
}

/*
 * static void
 * check_broken_conf(const char *conf_level, pfc_log_level_t level)
 *      Ensure that broken log level in the configuration file is always
 *      ignored.
 */
static void
check_broken_conf(const char *conf_level, pfc_log_level_t level)
{
    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  deflevel(static_cast<pfc_log_level_t>(lvl));
        pfc_log_level_t  reqlevel((level == PFC_LOGLVL_NONE)
                                  ? deflevel : level);

        // Create configuration file and define broken log level.
        pfc_cfblk_t  options;
        ConfFile     cf;
        cf.setUp(conf_level, options);
        RETURN_ON_ERROR();

        // Initialize the logging system.
        pfc_log_conf_t  conf;
        pfc_logconf_init(&conf, options, TEST_LOG_IDENT, NULL);
        pfc_logconf_setdeflevel(&conf, deflevel);

        LogFile  file;
        LogFile::checkmode_t  cmode(LogFile::CHECK_EMPTY);
        file.setPath(conf);
        check_log_sysinit(conf, reqlevel);

        // Ensure that a warning message is logged.
        bool found(false);
        char buf[64];
        snprintf(buf, sizeof(buf), "Invalid logging level: %s",
                 conf_level);
        bool required;

        if (reqlevel >= PFC_LOGLVL_WARN && level == PFC_LOGLVL_NONE) {
            required = true;
            cmode = LogFile::CHECK_EXISTS;
        }
        else {
            required = false;
        }
        file.search(buf, found);
        ASSERT_EQ(required, found);
        file.check(cmode);
        RETURN_ON_ERROR();
    }
}

#define RECONF_LOGFILE               0x1U
#define RECONF_LEVELFILE             0x2U
#define RECONF_EARLY_LEVELFILE       0x4U

#define RECONF_MAX                                                      \
    (RECONF_LOGFILE | RECONF_LEVELFILE | RECONF_EARLY_LEVELFILE)

/*
 * static void
 * check_reconfigure(uint32_t flags)
 *      Ensure that the logging system can be reconfigured until syslog is
 *      initialized.
 */
static void
check_reconfigure(uint32_t flags)
{
    // Initialize the logging system with default configuration.
    pfc_log_conf_t  conf;
    pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                      stderr, PFC_LOGLVL_NONE, NULL);
    check_log_sysinit(conf, PFC_LOGLVL_INFO);
    RETURN_ON_ERROR();

    // Initialize the logging system again with log level file.
    char   lvldata[] = {LogLevelFile::toChar(PFC_LOGLVL_DEBUG)};
    LogLevelFile lvlfile_early(lvldata, sizeof(lvldata), "loglevel_early");
    RETURN_ON_ERROR();

    pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                      stderr, PFC_LOGLVL_NONE, NULL);
    lvlfile_early.setLevelPath(conf);
    check_log_sysinit(conf, PFC_LOGLVL_DEBUG, false);
    RETURN_ON_ERROR();

    if ((flags & RECONF_EARLY_LEVELFILE) == 0) {
        // Initialize the logging system again without log level file.
        pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                          stderr, PFC_LOGLVL_WARN, NULL);
        check_log_sysinit(conf, PFC_LOGLVL_WARN, false);
        RETURN_ON_ERROR();
    }

    // Initialize the logging system using pfc_logconf_init().
    pfc_log_level_t  level(PFC_LOGLVL_NOTICE);
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
    pfc_logconf_setlevel(&conf, level);

    lvldata[0] = LogLevelFile::toChar(level);
    LogLevelFile  lvlfile(lvldata, sizeof(lvldata));

    LogFile  file;
    LogFile::checkmode_t  cmode;
    if (flags & RECONF_LOGFILE) {
        pfc_logconf_setsyslog(&conf, PFC_FALSE);
        file.setPath(conf);
        cmode = LogFile::CHECK_EMPTY;
    }
    else {
        pfc_logconf_setsyslog(&conf, PFC_TRUE);
        cmode = LogFile::CHECK_NONE;
    }

    if (flags & RECONF_LEVELFILE) {
        lvlfile.setLevelPath(conf);
    }

    check_log_sysinit(conf, level, false);
    RETURN_ON_ERROR();

    file.check(cmode);
    RETURN_ON_ERROR();

    char            buf[128];
    bool            found(false);
    pfc_timespec_t  ts;

    // Ensure that the configuration is fixed.
    for (int lvl(PFC_LOGLVL_FATAL); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t dummy_level(static_cast<pfc_log_level_t>(lvl));

        pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                          stderr, dummy_level, NULL);
        check_log_sysinit(conf, level, false);
        RETURN_ON_ERROR();

        if (cmode == LogFile::CHECK_EMPTY) {
            ASSERT_EQ(0, pfc_clock_gettime(&ts));
            snprintf(buf, sizeof(buf), "test message:1: %lu.%lu",
                     ts.tv_sec, ts.tv_nsec);
            pfc_log_notice("%s", buf);
            file.search(buf, found);
            ASSERT_TRUE(found);

            snprintf(buf, sizeof(buf), "test message:2: %lu.%lu",
                     ts.tv_sec, ts.tv_nsec);
            pfc_log_info("%s", buf);
            file.search(buf, found);
            ASSERT_FALSE(found);
        }

        lvldata[0] = LogLevelFile::toChar(dummy_level);
        LogLevelFile lvlfile1(lvldata, sizeof(lvldata), "loglevel_1");
        LogFile      file1("test_log_1");

        pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
        lvlfile1.setLevelPath(conf);
        file1.setPath(conf);
        check_log_sysinit(conf, level, false);

        file1.check(LogFile::CHECK_NONE);

        if (cmode == LogFile::CHECK_EMPTY) {
            ASSERT_EQ(0, pfc_clock_gettime(&ts));
            snprintf(buf, sizeof(buf), "test message:3: %lu.%lu",
                     ts.tv_sec, ts.tv_nsec);
            pfc_log_notice("%s", buf);
            file.search(buf, found);
            ASSERT_TRUE(found);

            snprintf(buf, sizeof(buf), "test message:4: %lu.%lu",
                     ts.tv_sec, ts.tv_nsec);
            pfc_log_info("%s", buf);
            file.search(buf, found);
            ASSERT_FALSE(found);
        }
    }
}

/*
 * static void
 * check_rotate(uint32_t nrotate)
 *      Ensure that automatic file rotation works.
 */
static void
check_rotate(uint32_t nrotate)
{
    const size_t  limit(1024);

    // Calculate the length of log prefix.
    // We can substitute getpid() for gettid() because this function should
    // be called on a main thread.
    size_t  pflen;
    {
        char  buf[32];
        int   len(snprintf(buf, sizeof(buf), ": [%u]: INFO: ", getpid()));
        ASSERT_GT(len, 0);
        pflen = static_cast<size_t>(len);
        ASSERT_LT(pflen, sizeof(buf));

        // Append length of timestamp.
        pflen += 26;
    }

    // Prepare log directory.
    LogFile     file;
    std::string parent;
    file.getParent(parent);
    ASSERT_EQ(0, pfc_mkdir(parent.c_str(), 0700));

    std::string  path;
    file.getPath(path);

    // Initialize the logging system.
    pfc_log_conf_t  conf;
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
    pfc_logconf_setrotate(&conf, nrotate, limit);
    file.setPath(conf);
    check_log_sysinit(conf, PFC_LOGLVL_INFO);
    file.check();

    // Current log file must be empty.
    struct stat  sbuf;
    ASSERT_EQ(0, lstat(path.c_str(), &sbuf))
        << "*** ERROR: " << strerror(errno);
    ASSERT_TRUE(S_ISREG(sbuf.st_mode));
    ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);

    // Create old logs.
    for (uint32_t i(1); i <= nrotate; i++) {
        char  buf[32];

        snprintf(buf, sizeof(buf), "%u", i);
        std::string  fname(path);
        fname.append(".");
        fname.append(buf);

        FILE *fp(pfc_fopen_cloexec(fname.c_str(), "w"));
        ASSERT_TRUE(fp != NULL);
        StdioRef  fpref(fp);
        ASSERT_EQ(0, fchmod(fileno(fp), 0600));

        for (uint32_t sz(0); sz < i; sz++) {
            putc('0', fp);
        }
        ASSERT_EQ(0, ferror(fp));
    }

    // Generate log messages until the log file is rotated.
    size_t   total(0);
    uint32_t index(0);
    for (; total < limit; index++) {
        char   buf[128];
        int    len(snprintf(buf, sizeof(buf), "test message: %u", index));
        ASSERT_GT(len, 0);
        size_t size(static_cast<size_t>(len));
        ASSERT_LT(size, sizeof(buf));
        total += (pflen + size + 1);

        pfc_log_info("%s", buf);
    }

    // Old log files must be rotated.
    struct stat  sbuf_cur;
    ASSERT_EQ(0, lstat(path.c_str(), &sbuf_cur))
        << "*** ERROR: " << strerror(errno);
    ASSERT_TRUE(S_ISREG(sbuf_cur.st_mode));
    ASSERT_EQ(static_cast<mode_t>(0600), sbuf_cur.st_mode & 07777);
    ASSERT_EQ(static_cast<off_t>(0), sbuf_cur.st_size);

    if (nrotate != 0) {
        std::string  prev(path);
        prev.append(".1");
        struct stat  sbuf_prev;
        ASSERT_EQ(0, lstat(prev.c_str(), &sbuf_prev))
            << "*** ERROR: " << strerror(errno) << ":" << prev;
        ASSERT_EQ(sbuf.st_dev, sbuf_prev.st_dev);
        ASSERT_EQ(sbuf.st_ino, sbuf_prev.st_ino);
        ASSERT_EQ(sbuf.st_mode, sbuf_prev.st_mode);
        ASSERT_EQ(static_cast<off_t>(total), sbuf_prev.st_size);

        FILE     *fp(pfc_fopen_cloexec(prev.c_str(), "r"));
        ASSERT_TRUE(fp != NULL);
        StdioRef fpref(fp);

        char  line[1024];
        for (uint32_t idx(0); idx < index; idx++) {
            char  buf[128];
            snprintf(buf, sizeof(buf), "test message: %u\n", idx);
            ASSERT_TRUE(fgets(line, sizeof(line), fp) != NULL);
            char  *p(strstr(line, buf));
            ASSERT_TRUE(p != NULL);
        }

        ASSERT_TRUE(fgets(line, sizeof(line), fp) == NULL);
    }

    std::string   &fnm(file.getName());
    const char    *fname(fnm.c_str());
    size_t        fnamelen(fnm.length());
    DIR           *dirp(opendir(parent.c_str()));
    ASSERT_TRUE(dirp != NULL);
    DirEntRef     dref(dirp);
    struct dirent *dp;

    while ((dp = readdir(dirp)) != NULL) {
        const char  *name(dp->d_name);
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
            strcmp(name, fname) == 0) {
            continue;
        }

        ASSERT_EQ(0, strncmp(name, fname, fnamelen));
        const char  *suffix(name + fnamelen);
        std::string  lpath(path);
        lpath.append(suffix);

        ASSERT_EQ('.', *suffix);
        suffix++;

        uint32_t  idx;
        ASSERT_EQ(0, pfc_strtou32(suffix, &idx));
        ASSERT_LE(idx, nrotate);

        if (idx == 1) {
            continue;
        }

        FILE      *fp(pfc_fopen_cloexec(lpath.c_str(), "r"));
        ASSERT_TRUE(fp != NULL);
        StdioRef  fpref(fp);

        uint32_t  sz(0);
        for (;;) {
            int  c(getc(fp));
            if (c == EOF) {
                break;
            }

            ASSERT_EQ('0', c);
            sz++;
        }

        ASSERT_EQ(idx - 1, sz) << lpath;
    }

    // Ensure that the size of current log file was reset.
    {
        char   buf[128];
        int    len(snprintf(buf, sizeof(buf), "Final message: %lu",
                            time(NULL)));
        ASSERT_GT(len, 0);
        size_t size(static_cast<size_t>(len));
        ASSERT_LT(size, sizeof(buf));
        size += (pflen + 1);

        pfc_log_info("%s", buf);

        struct stat  sbuf_new;
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf_new))
            << "*** ERROR: " << strerror(errno);
        ASSERT_TRUE(S_ISREG(sbuf_new.st_mode));
        ASSERT_EQ(static_cast<mode_t>(0600), sbuf_new.st_mode & 07777);
        ASSERT_EQ(sbuf_cur.st_dev, sbuf_new.st_dev);
        ASSERT_EQ(sbuf_cur.st_ino, sbuf_new.st_ino);
        ASSERT_EQ(static_cast<off_t>(size), sbuf_new.st_size);
    }
}

/*
 * Test fixture.
 */
class log
    : public ::testing::Test
{
protected:
    virtual void
    SetUp(void)
    {
        // Clear configuration done by other tests.
        pfc_log_fini();

        // Change umask to 022.
        _umask = umask(022);
    }

    virtual void
    TearDown(void)
    {
        // Reset log levels, and clear log configuration made by the test.
        pfc_log_fini();

        pfc_log_conf_t conf;
        pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                          stderr, PFC_LOGLVL_NONE, NULL);
        pfc_log_sysinit(&conf);
        pfc_log_fini();

        // Restore umask.
        (void)umask(_umask);
    }

private:
    mode_t  _umask;
};

/*
 * Below are test cases.
 */

/*
 * Initialize the logging system using pfc_logconf_early().
 *
 *   - No log level file.
 *   - No configuration file.
 */
TEST_F(log, early)
{
    // Use default log levels.
    pfc_log_conf_t  conf;
    pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                      stderr, PFC_LOGLVL_NONE, NULL);
    check_log_sysinit(conf, PFC_LOGLVL_INFO);
    RETURN_ON_ERROR();

    // Ensure that default levels can be changed.
    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        pfc_logconf_early(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT,
                          stderr, PFC_LOGLVL_NONE, NULL);
        pfc_logconf_setdeflevel(&conf, level);

        check_log_sysinit(conf, level);
        RETURN_ON_ERROR();

        // Ensure that logging levels passed to pfc_logconf_early()
        // always precede other configurations.
        check_logconf_early_explicit(PFC_CFBLK_INVALID, NULL);
    }
}

/*
 * Initialize the logging system using pfc_logconf_early().
 *
 *   - No log level file.
 *   - Use configuration file.
 */
TEST_F(log, early_conf)
{
    for (int lvl(-1); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        // Create configuration file.
        pfc_cfblk_t  options;
        ConfFile     cf;
        cf.setUp(level, options);
        RETURN_ON_ERROR();

        // Ensure that logging levels are derived from the configuration
        // file.
        pfc_log_conf_t  conf;
        pfc_logconf_early(&conf, options, TEST_LOG_IDENT,
                          stderr, PFC_LOGLVL_NONE, NULL);

        pfc_log_level_t  reqlvl((level == PFC_LOGLVL_NONE)
                                ? PFC_LOGLVL_INFO : level);
        check_log_sysinit(conf, reqlvl);
        RETURN_ON_ERROR();

        // Ensure that logging levels passed to pfc_logconf_early()
        // always precede other configurations.
        check_logconf_early_explicit(options, NULL);
    }
}

/*
 * Initialize the logging system using pfc_logconf_early().
 *
 *   - Use log level file.
 *   - Use configuration file.
 */
TEST_F(log, early_levelfile)
{
    for (int lvl(-1); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        // Create log level file.
        char   lvldata[] = {LogLevelFile::toChar(level)};
        LogLevelFile lvlfile(lvldata, sizeof(lvldata));
        RETURN_ON_ERROR();

        // Create configuration file, which should be ignored.
        pfc_cfblk_t  options;
        ConfFile     cf;
        cf.setUp(PFC_LOGLVL_NOTICE, options);
        RETURN_ON_ERROR();

        // Ensure that logging levels are derived from the log level file.
        pfc_log_conf_t  conf;
        pfc_logconf_early(&conf, options, TEST_LOG_IDENT,
                          stderr, PFC_LOGLVL_NONE, NULL);
        lvlfile.setLevelPath(conf);

        pfc_log_level_t  reqlvl((level == PFC_LOGLVL_NONE)
                                ? PFC_LOGLVL_NOTICE : level);
        check_log_sysinit(conf, reqlvl);
        RETURN_ON_ERROR();

        // Ensure that logging levels passed to pfc_logconf_early()
        // always precede other configurations.
        check_logconf_early_explicit(options, &lvlfile);
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - No log level file.
 *   - No configuration file.
 */
TEST_F(log, init)
{
    // Use default log levels.
    pfc_log_conf_t  conf;
    pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
    check_logconf_init(conf, PFC_LOGLVL_INFO);
    RETURN_ON_ERROR();

    // Ensure that default levels can be changed.
    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
        pfc_logconf_setdeflevel(&conf, level);

        check_logconf_init(conf, level);
        RETURN_ON_ERROR();

        // Ensure that logging levels set by pfc_logconf_setlevel() are
        // always precede other configurations.
        check_logconf_init_explicit(PFC_CFBLK_INVALID, NULL);
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - Ensure that the log directory and file are initialized correctly.
 */
TEST_F(log, init_file)
{
    // Prepare log directory with invalid permission.
    {
        LogFile      file;
        std::string  parent;
        file.getParent(parent);
        const char   *path(parent.c_str());

        ASSERT_EQ(0, pfc_mkdir(path, 0755));

        struct stat  sbuf;
        ASSERT_EQ(0, lstat(path, &sbuf)) << "*** ERROR: " << strerror(errno);
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(static_cast<mode_t>(0755), sbuf.st_mode & 07777);

        std::string  &base(file.getBase());
        ASSERT_EQ(0, chmod(base.c_str(), 0700));

        check_logconf_init(file);
    }

    // Create symbolic link at the log directory.
    {
        LogFile      file;
        std::string  &base(file.getBase());
        ASSERT_EQ(0, pfc_mkdir(base.c_str(), 0700));

        std::string  parent;
        file.getParent(parent);
        const char   *path(parent.c_str());

        ASSERT_EQ(0, symlink("/foo/bar", path))
            << "*** ERROR: " << strerror(errno);
        check_logconf_init(file);
    }

    // Put an invalid file at the log directory path.
    const mode_t  types[] = {S_IFREG, S_IFIFO, S_IFSOCK};
    for (const mode_t *tp(types); tp < PFC_ARRAY_LIMIT(types); tp++) {
        LogFile      file;
        std::string  &base(file.getBase());
        ASSERT_EQ(0, pfc_mkdir(base.c_str(), 0700));

        std::string  parent;
        file.getParent(parent);
        const char   *path(parent.c_str());

        ASSERT_EQ(0, mknod(path, *tp | 0644, 0))
            << "*** ERROR: " << strerror(errno);
        check_logconf_init(file);
    }

    // Put a directory at the log file path.
    {
        LogFile      file;
        std::string  p;
        file.getPath(p);
        const char   *path(p.c_str());

        ASSERT_EQ(0, pfc_mkdir(path, 0700));
        check_logconf_init(file);
    }

    // Put a symbolic link at the log file path.
    {
        LogFile      file;
        std::string  parent;
        file.getParent(parent);
        ASSERT_EQ(0, pfc_mkdir(parent.c_str(), 0700));

        std::string  p;
        file.getPath(p);
        const char   *path(p.c_str());

        ASSERT_EQ(0, symlink("/foo/bar", path))
            << "*** ERROR: " << strerror(errno);
        check_logconf_init(file);
    }

    // Put an invalid file at the log file path.
    for (const mode_t *tp(types); tp < PFC_ARRAY_LIMIT(types); tp++) {
        LogFile      file;
        std::string  parent;
        file.getParent(parent);
        ASSERT_EQ(0, pfc_mkdir(parent.c_str(), 0700));

        std::string  p;
        file.getPath(p);
        const char   *path(p.c_str());

        ASSERT_EQ(0, mknod(path, *tp | 0644, 0))
            << "*** ERROR: " << strerror(errno);
        check_logconf_init(file);
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - Ensure that the file is rotated correctly.
 */
TEST_F(log, init_rotate)
{
    for (uint32_t nrotate(0); nrotate <= 20; nrotate++) {
        check_logconf_init_rotate(nrotate);
        RETURN_ON_ERROR();
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - No log level file.
 *   - Use configuration file.
 */
TEST_F(log, init_conf)
{
    for (int lvl(-1); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        // Create configuration file.
        pfc_cfblk_t  options;
        ConfFile     cf;
        cf.setUp(level, options);
        RETURN_ON_ERROR();

        // Ensure that logging levels are derived from the configuration
        // file.
        pfc_log_conf_t  conf;
        pfc_logconf_init(&conf, options, TEST_LOG_IDENT, NULL);

        pfc_log_level_t  reqlvl((level == PFC_LOGLVL_NONE)
                                ? PFC_LOGLVL_INFO : level);
        check_logconf_init(conf, reqlvl);
        RETURN_ON_ERROR();

        // Ensure that logging levels set by pfc_logconf_setlevel() are
        // always precede other configurations.
        check_logconf_init_explicit(options, NULL);
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - Use log level file.
 *   - Use configuration file.
 */
TEST_F(log, init_levelfile)
{
    for (int lvl(-1); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));
        // Create log level file.
        char   lvldata[] = {LogLevelFile::toChar(level)};
        LogLevelFile lvlfile(lvldata, sizeof(lvldata));
        RETURN_ON_ERROR();

        // Create configuration file, which should be ignored.
        pfc_cfblk_t  options;
        ConfFile     cf;
        cf.setUp(PFC_LOGLVL_NOTICE, options);
        RETURN_ON_ERROR();

        // Ensure that logging levels are derived from the log level file.
        pfc_log_conf_t  conf;
        pfc_logconf_init(&conf, options, TEST_LOG_IDENT, NULL);
        lvlfile.setLevelPath(conf);

        pfc_log_level_t  reqlvl((level == PFC_LOGLVL_NONE)
                                ? PFC_LOGLVL_NOTICE : level);
        check_logconf_init(conf, reqlvl);
        RETURN_ON_ERROR();

        // Ensure that logging levels set by pfc_logconf_setlevel() are
        // always precede other configurations.
        check_logconf_init_explicit(options, &lvlfile);
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - A broken level file exists.
 *   - No configuration file.
 */
TEST_F(log, init_levelfile_broken)
{
    BadLevelFileCreator  creator;
    lvlfile_crlist_t     &list(creator.getList());

    for (lvlfile_crlist_t::iterator it(list.begin()); it != list.end(); it++) {
        check_broken_levelfile(*it);
        RETURN_ON_ERROR();
    }
}

/*
 * Initialize the logging system using pfc_logconf_init().
 *
 *   - A level file contains invalid character.
 *   - No configuration file.
 */
TEST_F(log, init_levelfile_badchar)
{
    lvlfile_create_t  func(boost::bind(lvlfile_create_badlevel, _1, _2, 'a'));
    check_broken_levelfile(func);
}

/*
 * Initialize the logging system using pfc_logconf_init()
 *
 *   - Broken log level is defined in the configuration file.
 */
TEST_F(log, init_conf_broken)
{
    // Broken trace log level is defined.
    check_broken_conf("broken", PFC_LOGLVL_NONE);
}

/*
 * Initialize the logging system using pfc_logconf_init()
 *
 *   - Unable to create log file.
 */
TEST_F(log, init_err)
{
    if (getuid() == 0 || geteuid() == 0) {
        // This test does not work with root privilege.
        return;
    }

    char  buf[128];

    // pfc_logconf_init() calls exit(EX_CANTCREAT) if the log directory could
    // not be created.
    {
        TmpFile       errfile("test_log_err");
        ASSERT_EQ(0, errfile.createFile());

        const char    *path("/invalid_dir_on_root/log");
        ChildContext  ctx;
        child_func_t  func(boost::bind(check_logconf_init_badbase, _1,
                                       &errfile, path));
        ctx.run(func);
        RETURN_ON_ERROR();

        int status;
        ctx.wait(status);
        RETURN_ON_ERROR();
        ASSERT_EQ(EX_CANTCREAT, status);

        // An error message is dumped to the standard error output.
        bool  found(false);
        snprintf(buf, sizeof(buf), "%s: Failed to setup log: %d: ",
                 path, LOG_ERRLOC_DIR_MKDIR);
        errfile.search(buf, found);
        RETURN_ON_ERROR();
        ASSERT_TRUE(found);
    }

    // pfc_logconf_init() calls exit(EX_CANTCREAT) if the log file could not
    // be created.
    {
        TmpFile       errfile("test_log_err");
        ASSERT_EQ(0, errfile.createFile());

        LogFile       file;
        std::string   path;
        file.getPath(path);
        ChildContext  ctx;
        child_func_t  func(boost::bind(check_logconf_init_fail, _1, &errfile,
                                       &file));
        ctx.run(func);
        RETURN_ON_ERROR();

        int status;
        ctx.wait(status);
        RETURN_ON_ERROR();
        ASSERT_EQ(EX_CANTCREAT, status);

        // An error message is dumped to the standard error output.
        bool found(false);
        snprintf(buf, sizeof(buf), "%s: Failed to open log file: %d: ",
                 path.c_str(), LOG_ERRLOC_CREATE);
        errfile.search(buf, found);
        RETURN_ON_ERROR();
        ASSERT_TRUE(found);
    }

    // Run the same test again with preparing log file.
    {
        TmpFile       errfile("test_log_err");
        ASSERT_EQ(0, errfile.createFile());

        LogFile       file;
        std::string   parent;
        file.getParent(parent);
        ASSERT_EQ(0, pfc_mkdir(parent.c_str(), 0700));
        std::string   p;
        file.getPath(p);
        const char    *path(p.c_str());
        int  fd(pfc_open_cloexec(path, O_CREAT|O_WRONLY|O_CREAT, 0600));
        ASSERT_NE(-1, fd);
        ASSERT_EQ(0, close(fd));

        ChildContext  ctx;
        child_func_t  func(boost::bind(check_logconf_init_fail, _1, &errfile,
                                       &file));
        ctx.run(func);
        RETURN_ON_ERROR();

        int status;
        ctx.wait(status);
        RETURN_ON_ERROR();
        ASSERT_EQ(EX_CANTCREAT, status);

        // An error message is dumped to the standard error output.
        bool found(false);
        snprintf(buf, sizeof(buf), "%s: Failed to open log file: %d: ",
                 path, LOG_ERRLOC_APPEND);
        errfile.search(buf, found);
        RETURN_ON_ERROR();
        ASSERT_TRUE(found);
    }
}

/*
 * Test case for pfc_log_set_level().
 *
 *   - No log level file.
 */
TEST_F(log, set_level)
{
    for (int l(PFC_LOGLVL_NONE); l <= PFC_LOGLVL_DEBUG; l++) {
        for (int deflvl(0); deflvl <= PFC_LOGLVL_VERBOSE; deflvl++) {
            pfc_log_level_t  deflevel(static_cast<pfc_log_level_t>(deflvl));

            pfc_log_conf_t  conf;
            pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
            pfc_logconf_setdeflevel(&conf, deflevel);

            LogFile  file;
            file.setPath(conf);
            check_log_sysinit(conf, deflevel);
            file.check();
            RETURN_ON_ERROR();

            for (int lvl(-20); lvl <= 20; lvl++) {
                pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));

                if (level == PFC_LOGLVL_NONE) {
                    continue;
                }

                int  ret(pfc_log_set_level(level));

                if (level == deflevel) {
                    ASSERT_EQ(1, ret);
                }
                else if (IS_LOGLVL_VALID(level)) {
                    ASSERT_EQ(0, ret);
                }
                else {
                    // Invalid log level must be rejected.
                    ASSERT_EQ(-EINVAL, ret);
                    ASSERT_EQ(deflevel, pfc_log_current_level);
                    continue;
                }

                ASSERT_EQ(level, pfc_log_current_level);

                // 1 must be returned if level is not changed.
                ASSERT_EQ(1, pfc_log_set_level(level));
                ASSERT_EQ(level, pfc_log_current_level);

                // Reset log level.
                ret = pfc_log_set_level(PFC_LOGLVL_NONE);

                if (level == deflevel) {
                    ASSERT_EQ(1, ret);
                }
                else {
                    ASSERT_EQ(0, ret);
                }

                ASSERT_EQ(deflevel, pfc_log_current_level);
            }
        }
    }
}

/*
 * Test case for pfc_log_set_level().
 *
 *   - Use log level file.
 */
TEST_F(log, set_level_file)
{
    for (int l(PFC_LOGLVL_NONE); l <= PFC_LOGLVL_DEBUG; l++) {
        for (int deflvl(0); deflvl <= PFC_LOGLVL_VERBOSE; deflvl++) {
            pfc_log_level_t  deflevel(static_cast<pfc_log_level_t>(deflvl));

            pfc_log_conf_t  conf;
            pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
            pfc_logconf_setdeflevel(&conf, deflevel);

            LogLevelFile  lvlfile;
            lvlfile.setLevelPath(conf);

            LogFile  file;
            file.setPath(conf);
            check_log_sysinit(conf, deflevel);
            file.check();
            RETURN_ON_ERROR();

            // Log level file must not exist yet.
            ASSERT_EQ(ENOENT, lvlfile.exists());

            for (int lvl(-20); lvl <= 20; lvl++) {
                pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));

                if (level == PFC_LOGLVL_NONE) {
                    continue;
                }

                pfc_log_level_t  flevel;
                int   ret(pfc_log_set_level(level));

                if (level == deflevel) {
                    ASSERT_EQ(1, ret);

                    // In this case, pfc_log_set_level() never updates the log
                    // level file.
                    flevel = PFC_LOGLVL_NONE;
                }
                else if (IS_LOGLVL_VALID(level)) {
                    ASSERT_EQ(0, ret);
                    flevel = level;
                }
                else {
                    // Invalid log level must be rejected.
                    ASSERT_EQ(-EINVAL, ret);
                    ASSERT_EQ(deflevel, pfc_log_current_level);
                    continue;
                }

                bool lvlfile_exists((deflevel == PFC_LOGLVL_FATAL &&
                                     level == PFC_LOGLVL_FATAL) ? false : true);

                ASSERT_EQ(level, pfc_log_current_level);
                if (lvlfile_exists) {
                    lvlfile.check(flevel);
                    RETURN_ON_ERROR();
                }

                // 1 must be returned if level is not changed.
                ASSERT_EQ(1, pfc_log_set_level(level));
                ASSERT_EQ(level, pfc_log_current_level);

                if (lvlfile_exists) {
                    lvlfile.check(flevel);
                    RETURN_ON_ERROR();
                }

                // Reset log level.
                ret = pfc_log_set_level(PFC_LOGLVL_NONE);

                if (level == deflevel) {
                    ASSERT_EQ(1, ret);
                }
                else {
                    ASSERT_EQ(0, ret);
                }

                ASSERT_EQ(deflevel, pfc_log_current_level);

                // Reset always updates the log level file.
                lvlfile.check(PFC_LOGLVL_NONE);
                RETURN_ON_ERROR();
            }
        }
    }
}

/*
 * Test case for pfc_log_set_level().
 *
 *   - A broken level file exists.
 */
TEST_F(log, set_level_file_broken)
{
    BadLevelFileCreator  creator;
    lvlfile_crlist_t     &list(creator.getList());

    for (lvlfile_crlist_t::iterator it(list.begin()); it != list.end(); it++) {
        for (int lvl(PFC_LOGLVL_NONE); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
            pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));

            if (level == PFC_LOGLVL_INFO) {
                continue;
            }

            pfc_log_level_t  reqlevel((level == PFC_LOGLVL_NONE)
                                      ? PFC_LOGLVL_INFO : level);

            pfc_log_conf_t  conf;
            pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);

            LogLevelFile  lvlfile;
            lvlfile.setLevelPath(conf);

            LogFile  file;
            file.setPath(conf);
            check_log_sysinit(conf, PFC_LOGLVL_INFO);
            file.check();
            RETURN_ON_ERROR();

            // Create broken log level file.
            std::string  errmsg, path;
            ASSERT_EQ(ENOENT, lvlfile.exists());
            lvlfile.getPath(path);
            (*it)(path.c_str(), errmsg);
            RETURN_ON_ERROR();
            ASSERT_EQ(0, lvlfile.exists());

            int  ret(pfc_log_set_level(level));
            if (reqlevel == PFC_LOGLVL_INFO) {
                ASSERT_EQ(1, ret);
            }
            else {
                ASSERT_EQ(0, ret);
            }
            ASSERT_EQ(reqlevel, pfc_log_current_level);

            // Broken log level file must be removed, and a new one must
            // be created.
            lvlfile.check(level);
            RETURN_ON_ERROR();

            // Ensure that a warning message is logged.
            bool found(false);
            bool required((reqlevel >= PFC_LOGLVL_WARN) ? true : false);
            file.search("Bogus loglevel file was reset\\.", found);
            ASSERT_EQ(required, found);

            // Ensure that an error message is logged.
            if (errmsg.length() != 0) {
                file.search(errmsg.c_str(), found);
                required = (reqlevel >= PFC_LOGLVL_ERROR) ? true : false;
                ASSERT_EQ(required, found);
            }
        }
    }
}

/*
 * Ensure that the logging system can be reconfigured until syslog is
 * initialized.
 */
TEST_F(log, reconfigure)
{
    for (uint32_t flags(0); flags <= RECONF_MAX; flags++) {
        check_reconfigure(flags);
        RETURN_ON_ERROR();
    }
}

/*
 * Ensure that automatic file rotation works.
 */
TEST_F(log, rotate)
{
    for (uint32_t nrotate(0); nrotate <= 20; nrotate++) {
        check_rotate(nrotate);
        RETURN_ON_ERROR();
    }
}

/*
 * Test case for module-specific log level feature.
 */
TEST_F(log, modlevel)
{
    for (int lvl(0); lvl <= PFC_LOGLVL_VERBOSE; lvl++) {
        pfc_log_level_t  level(static_cast<pfc_log_level_t>(lvl));

        pfc_log_conf_t  conf;
        pfc_logconf_init(&conf, PFC_CFBLK_INVALID, TEST_LOG_IDENT, NULL);
        pfc_logconf_setlevel(&conf, level);

        LogFile  file;
        file.setPath(conf);
        check_log_sysinit(conf, level);

        // Invalid log level must be rejected.
        for (int i(-20); i <= 30; i++) {
            pfc_log_level_t  l(static_cast<pfc_log_level_t>(i));

            if (!IS_LOGLVL_VALID(l)) {
                ASSERT_EQ(-EINVAL, pfc_log_modlevel_set("modname", l));
                ASSERT_EQ(level, __pfc_log_current_modlevel("modname"));
            }
        }

        // No module-specific log level is defined.
        pfc_cfblk_t  options;
        {
            ConfFile     cf;

            cf.setUp(reinterpret_cast<const pfc_log_modconf_t *>(NULL), 0,
                     options);
            pfc_log_modlevel_init(options);

            RETURN_ON_ERROR();

            pfc_log_modconf_t *levels;
            ASSERT_EQ(0, cf.copyModLevels(levels));
            ASSERT_TRUE(levels == NULL);
            ASSERT_FALSE(__PFC_LOG_MODLEVEL_IS_DEFINED());
            ASSERT_EQ(level, __pfc_log_current_modlevel("module1"));
            ASSERT_EQ(level, __pfc_log_current_modlevel("module2"));
        }

        // Define module-specific log levels in the configuration file.
        pfc_log_modconf_t  modlevels[] = {
            {"module_1", PFC_LOGLVL_VERBOSE},
            {"module_2", PFC_LOGLVL_ERROR},
            {"module_3", PFC_LOGLVL_NOTICE},
            {"module_4", PFC_LOGLVL_TRACE},
            {"module_5", PFC_LOGLVL_DEBUG},
            {"module_6", PFC_LOGLVL_INFO},
            {"module_7", PFC_LOGLVL_WARN},
            {"module_8", PFC_LOGLVL_FATAL},
        };
        const uint32_t  nlevels(PFC_ARRAY_CAPACITY(modlevels));

        ConfFile  cf;
        cf.setUp(modlevels, nlevels, options);
        pfc_log_modlevel_init(options);

        pfc_log_modconf_t *levels;
        ASSERT_EQ(static_cast<int>(nlevels), cf.copyModLevels(levels));
        ASSERT_TRUE(levels != NULL);
        ASSERT_TRUE(__PFC_LOG_MODLEVEL_IS_DEFINED());

        // pfc_log_modconf_t are sorted by module name in dictionary order.
        pfc_log_modconf_t *mdef(modlevels), *plmp(levels);
        for (; plmp < levels + nlevels;
             plmp++, mdef++) {
            ASSERT_STREQ(mdef->plm_name, plmp->plm_name);
            ASSERT_EQ(mdef->plm_level, plmp->plm_level);
            ASSERT_EQ(plmp->plm_level,
                      __pfc_log_current_modlevel(plmp->plm_name));
        }

        for (uint32_t i(9); i <= 30; i++) {
            char  buf[64];

            snprintf(buf, sizeof(buf), "module_%u", i);
            ASSERT_EQ(level, __pfc_log_current_modlevel(buf));
        }

        // Update existing module-specific level configuration.
        modlevels[2].plm_level = PFC_LOGLVL_DEBUG;
        modlevels[5].plm_level = PFC_LOGLVL_ERROR;
        for (uint32_t i(0); i < nlevels; i++) {
            mdef = &modlevels[i];
            int  ret(pfc_log_modlevel_set(mdef->plm_name, mdef->plm_level));

            if (i == 2 || i == 5) {
                ASSERT_EQ(0, ret);
            }
            else {
                ASSERT_EQ(1, ret);
            }
        }

        // Insert new configuration.
        ASSERT_EQ(0, pfc_log_modlevel_set("new_mod1", PFC_LOGLVL_TRACE));
        ASSERT_EQ(0, pfc_log_modlevel_set("new_mod2", PFC_LOGLVL_WARN));

        ASSERT_EQ(static_cast<int>(nlevels + 2), cf.copyModLevels(levels));
        ASSERT_TRUE(levels != NULL);
        ASSERT_TRUE(__PFC_LOG_MODLEVEL_IS_DEFINED());

        mdef = modlevels;
        for (plmp = levels; plmp < levels + nlevels; plmp++, mdef++) {
            ASSERT_STREQ(mdef->plm_name, plmp->plm_name);
            ASSERT_EQ(mdef->plm_level, plmp->plm_level);
            ASSERT_EQ(plmp->plm_level,
                      __pfc_log_current_modlevel(plmp->plm_name));
        }

        ASSERT_STREQ("new_mod1", plmp->plm_name);
        ASSERT_EQ(PFC_LOGLVL_TRACE, plmp->plm_level);
        ASSERT_EQ(PFC_LOGLVL_TRACE,
                  __pfc_log_current_modlevel(plmp->plm_name));
        plmp++;

        ASSERT_STREQ("new_mod2", plmp->plm_name);
        ASSERT_EQ(PFC_LOGLVL_WARN, plmp->plm_level);
        ASSERT_EQ(PFC_LOGLVL_WARN,
                  __pfc_log_current_modlevel(plmp->plm_name));

        for (uint32_t i(9); i <= 30; i++) {
            char  buf[64];

            snprintf(buf, sizeof(buf), "module_%u", i);
            ASSERT_EQ(level, __pfc_log_current_modlevel(buf));
        }

        // Reset log levels for some modules.
        modlevels[1].plm_level = PFC_LOGLVL_NONE;
        modlevels[5].plm_level = PFC_LOGLVL_NONE;
        modlevels[6].plm_level = PFC_LOGLVL_NONE;
        for (uint32_t i(0); i < nlevels; i++) {
            mdef = &modlevels[i];
            if (mdef->plm_level == PFC_LOGLVL_NONE) {
                pfc_log_modlevel_reset(mdef->plm_name);
            }
        }
        pfc_log_modlevel_reset("new_mod1");

        ASSERT_EQ(static_cast<int>(nlevels - 2), cf.copyModLevels(levels));

        
        plmp = levels;
        for (mdef = modlevels; mdef < modlevels + nlevels; mdef++) {
            pfc_log_level_t  lvl(mdef->plm_level);

            if (lvl != PFC_LOGLVL_NONE) {
                ASSERT_STREQ(mdef->plm_name, plmp->plm_name);
                ASSERT_EQ(mdef->plm_level, plmp->plm_level);
                plmp++;
            }
            else {
                lvl = level;
            }
            ASSERT_EQ(lvl, __pfc_log_current_modlevel(mdef->plm_name));
        }

        ASSERT_EQ(level, __pfc_log_current_modlevel("new_mod1"));

        ASSERT_STREQ("new_mod2", plmp->plm_name);
        ASSERT_EQ(PFC_LOGLVL_WARN, plmp->plm_level);
        ASSERT_EQ(PFC_LOGLVL_WARN,
                  __pfc_log_current_modlevel(plmp->plm_name));

        // Reset all module-specific log levels.
        pfc_log_modlevel_reset(NULL);
        ASSERT_EQ(0, cf.copyModLevels(levels));
        ASSERT_TRUE(levels == NULL);
        ASSERT_FALSE(__PFC_LOG_MODLEVEL_IS_DEFINED());
        for (mdef = modlevels; mdef < modlevels + nlevels; mdef++) {
            ASSERT_EQ(level, __pfc_log_current_modlevel(mdef->plm_name));
        }

        ASSERT_EQ(level, __pfc_log_current_modlevel("new_mod1"));
        ASSERT_EQ(level, __pfc_log_current_modlevel("new_mod2"));
    }
}
