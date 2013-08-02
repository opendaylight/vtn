/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * constructor and destructor for libpfc callers
 */

#include <unistd.h>
#include <dirent.h>
#include <pfc/base.h>
#include <pfc/log.h>

extern void libpfc_init(void);
extern void libpfc_fini(void);

static pid_t	mypid;

static void	show_unclosed_files(void);

static void PFC_FATTR_INIT
init (void)
{
    mypid = getpid();
    libpfc_init();
    pfc_log_init("libpfc_util test", stderr, PFC_LOGLVL_DEBUG, NULL);
}


static void PFC_FATTR_FINI
fini (void)
{
    if (getpid() != mypid) {
        /* Do nothing. */
        return;
    }

    libpfc_fini();
    show_unclosed_files();
}

/*
 * static void
 * show_unclosed_files(void)
 *	Show unclosed file descriptors.
 */
static void
show_unclosed_files(void)
{
#ifdef	__linux
    DIR	  *dirp;
    int	  dfd;
    struct dirent   buf, *dp;

    dirp = opendir("/proc/self/fd");
    if (dirp == NULL) {
        return;
    }
    
    dfd = dirfd(dirp);
    while (readdir_r(dirp, &buf, &dp) == 0 && dp != NULL) {
        ssize_t	sz;
        char	first = dp->d_name[0];
        char	name[256], path[256], *p;
        long	fd;

        if (first < '0' || first > '9') {
            continue;
        }

        fd = strtol(dp->d_name, &p, 10);
        if (*p != '\0' || fd == dfd || (fd >= 0 && fd <= 2)) {
            continue;
        }

        snprintf(path, sizeof(path), "/proc/self/fd/%s", dp->d_name);
        if ((sz = readlink(path, name, sizeof(name))) != -1) {
            if (sz == 0) {
                snprintf(name, sizeof(name), "<unknown>");
            }
            else {
                if ((size_t)sz >= sizeof(name)) {
                    sz = sizeof(name) - 1;
                }
                name[sz] = '\0';
            }
        }
        else {
            snprintf(name, sizeof(name), "<unknown>");
        }
        fprintf(stderr, "*** WARNING: Unclosed file: %3s: %s\n",
                dp->d_name, name);
    }

    closedir(dirp);
#endif	/* __linux */
}
