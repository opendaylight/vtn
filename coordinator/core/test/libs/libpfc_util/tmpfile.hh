/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for temporary file class.
 */

#ifndef	_TEST_TMPFILE_HH
#define	_TEST_TMPFILE_HH

#include <cstdio>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <pfc/synch.h>

/*
 * Class which represents a temporary file.
 * Created file will be removed by destructor.
 */
class TmpFile
{
public:
    TmpFile(const char *base);
    ~TmpFile();

    int		createFile(void);
    void	print(const char *fmt, ...);
    void	flush(void);
    int		getSize(size_t &size);
    int		readAsString(std::string &str);
    int		getDescriptor(void) const;
    const char	*getPath(void) const;
    void        search(const char *pattern, bool &found);

    inline FILE *
    getFile(void)
    {
        return _fp;
    }

private:
    /*
     * Temporary file path.
     */
    std::string	_path;

    /*
     * File handle.
     */
    FILE	*_fp;

    /*
     * Mutex to serialize file access.
     */
    pfc_mutex_t	_mutex;
};

/*
 * Class which represents a temporary directory.
 * Created directory will be removed by destructor.
 */
class TmpDir
{
public:
    TmpDir(const char *base, bool setpath = false);
    ~TmpDir();

    int		createDirectory(void);
    const char	*getPath(void) const;
    int		cleanup(void);

private:
    int		cleanup(std::string dir, struct dirent *buf);

    /*
     * Base name of temporary directory path.
     */
    std::string	_base;

    /*
     * Temporary directory path.
     */
    std::string	_path;
};

/*
 * Preserve umask, and set zero to umask.
 */
class Umask
{
public:
    Umask()
    {
        _umask = umask(0);
    }

    ~Umask()
    {
        (void)umask(_umask);
    }

    inline mode_t
    getOldMask(void)
    {
        return _umask;
    }

private:
    mode_t	_umask;
};

/*
 * Rename the specified path, and restore it on exit.
 */
class RenameFile
{
public:
    RenameFile()
        : _dfd(-1), _renamed(false) {}
    ~RenameFile();

    void  rename(const char *path);

private:
    int           _dfd;
    bool          _renamed;
    std::string   _fname;
    std::string   _saved;
};

/*
 * Prototypes.
 */
extern void     has_fs_capability(bool &privileged);
extern void     is_testdir_safe(bool &safe);

#endif	/* !_TEST_TMPFILE_HH */
