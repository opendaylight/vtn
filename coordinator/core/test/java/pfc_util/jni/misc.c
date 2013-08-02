/*
 * Copyright (c) 2013 NEC Corporation
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
#include <unistd.h>
#include <dirent.h>
#include <pfc/jni.h>
#include <pfc/util.h>
#include <pfc/strtoint.h>
#include <org_opendaylight_vtn_core_util_TestBase.h>

/*
 * The name of JAR file.
 */
static const char	test_jarfile[] = "pfc_util_test.jar";

#define	TEST_JARFILE_LEN	(sizeof(test_jarfile) - 1)

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_util_TestBase_removePath(
 *	JNIEnv *env, jclass cls, jstring jpath)
 *
 *	Remove the file specified by the given path.
 *
 * Calling/Exit Status:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_util_TestBase_removePath(
	JNIEnv *env, jclass cls, jstring jpath)
{
	const char	*path;

	if (PFC_EXPECT_FALSE(jpath == NULL)) {
		/* Nothing to do. */
		return;
	}

	path = pjni_string_get(env, jpath);
	if (PFC_EXPECT_TRUE(path != NULL)) {
		int	err = pfc_rmpath(path);

		if (PFC_EXPECT_FALSE(err != 0 && err != ENOENT)) {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "Unable to remove the file: path=%s, "
				   "err=%d", path, err);
			/* FALLTHROUGH */
		}
	}

	pjni_string_release(env, jpath, path);
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_util_TestBase_getJavaModeOption(
 *	JNIEnv *env, jclass cls)
 *
 *	Return a java command option to specify data model.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_util_TestBase_getJavaModeOption(
	JNIEnv *env, jclass cls)
{
	const char	*mode;

#ifdef	PFC_LP64
	mode = "-d64";
#else	/* !PFC_LP64 */
	mode = "-d32";
#endif	/* PFC_LP64 */

	return pjni_newstring(env, mode);
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_util_TestBase_getJarFilePath(
 *	JNIEnv *env, jclass cls)
 *
 *	Return a path to JAR file which contains test classes.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_util_TestBase_getJarFilePath(
	JNIEnv *env, jclass cls)
{
#ifdef	__linux
	DIR		*dirp;
	int		dfd;
	char		path[1024];
	pfc_bool_t	found = PFC_FALSE;

	dirp = opendir("/proc/self/fd");
	if (PFC_EXPECT_FALSE(dirp == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to open /proc/self/fd: %d", errno);

		return NULL;
	}

	dfd = dirfd(dirp);
	for (;;) {
		struct dirent	dent, *dp;
		int		err, sz, fd;

		err = readdir_r(dirp, &dent, &dp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "Unable to read /proc/self/fd entry: %d",
				   errno);
			goto error;
		}
		if (dp == NULL) {
			break;
		}

		err = pfc_strtoi32(dp->d_name, &fd);
		if (PFC_EXPECT_FALSE(err != 0)) {
			continue;
		}
		if (fd == dfd || (fd >= 0 && fd <= 2)) {
			continue;
		}

		sz = readlinkat(dfd, dp->d_name, path, sizeof(path));
		if (PFC_EXPECT_TRUE(sz > 0)) {
			size_t	len = (size_t)sz;

			if (PFC_EXPECT_FALSE(len >= sizeof(path))) {
				len = sizeof(path) - 1;
			}
			path[len] = '\0';
			if (len >= TEST_JARFILE_LEN + 1 &&
			    path[len - (TEST_JARFILE_LEN + 1)] == '/' &&
			    strcmp(&path[len - TEST_JARFILE_LEN],
				   test_jarfile) == 0) {
				/* Found. */
				found = PFC_TRUE;
				break;
			}
		}
                else {
			pjni_throw(env, PJNI_CLASS(IllegalStateException),
				   "readlinkat(%d, %s) failed: %d",
				   dfd, dp->d_name, errno);
                        goto error;
                }
	}

	if (PFC_EXPECT_FALSE(!found)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Not found.");
		goto error;
	}

	closedir(dirp);

	return pjni_newstring(env, path);

error:
	closedir(dirp);

	return NULL;
#else	/* !__linux */
#error	Port me!
#endif	/* __linux */
}
