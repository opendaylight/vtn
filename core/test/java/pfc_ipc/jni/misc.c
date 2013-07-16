/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include <fcntl.h>
#include <dirent.h>
#include <pfc/jni.h>
#include <pfc/config.h>
#include <pfc/clock.h>
#include <pfc/strtoint.h>
#include <pfc/log.h>
#include <pfc/rbtree.h>
#include <pfc/util.h>
#include <pfc/ipc_client.h>
#include <ipcclnt_impl.h>
#include <org_opendaylight_vtn_core_ipc_ClientLibraryTest.h>
#include <org_opendaylight_vtn_core_ipc_TestBase.h>

#define	SESS_PTR(session)	((pfc_ipcsess_t *)(uintptr_t)(session))

/*
 * The name of JAR file.
 */
static const char	test_jarfile[] = "pfc_ipc_test.jar";

#define	TEST_JARFILE_LEN	(sizeof(test_jarfile) - 1)

/*
 * Fully qualified class name.
 */
#define	PJNI_CLASS_TimeSpec			\
	"org/opendaylight/vtn/core/util/TimeSpec"

/*
 * JNI signature of TimeSpec(long, long)
 */
#define	JIPC_CTORSIG_TimeSpec			\
	PJNI_SIG_METHOD2(void, long, long)

/*
 * A pair of error number and its name.
 */
typedef struct {
	const char	*je_name;		/* name of error number */
	pfc_rbnode_t	je_node;		/* Red-Black tree node */
	int		je_errno;		/* error number */
} jipc_errno_t;

#define	JIPC_ERRNO_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), jipc_errno_t, je_node)

#define	JIPC_ERRNO_DECL(err)			\
	{					\
		.je_name	= #err,		\
		.je_errno	= err,		\
	}

/*
 * Internal prototypes.
 */
static pfc_cptr_t	jipc_errno_getkey(pfc_rbnode_t *node);

/*
 * Red-Black tree which keeps pairs of error number and its name.
 */
static pfc_rbtree_t	jipc_errno_tree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, jipc_errno_getkey);

/*
 * Error numbers suppored by getErrorNumber().
 */
static jipc_errno_t	jipc_errno_nodes[] = {
	JIPC_ERRNO_DECL(EINVAL),
	JIPC_ERRNO_DECL(EBADFD),
	JIPC_ERRNO_DECL(EPERM),
	JIPC_ERRNO_DECL(EBUSY),
	JIPC_ERRNO_DECL(ENOENT),
	JIPC_ERRNO_DECL(EEXIST),
	JIPC_ERRNO_DECL(ETIMEDOUT),
	JIPC_ERRNO_DECL(ECONNREFUSED),
	JIPC_ERRNO_DECL(ECONNRESET),
	JIPC_ERRNO_DECL(EPIPE),
	JIPC_ERRNO_DECL(ECANCELED),
	JIPC_ERRNO_DECL(ESHUTDOWN),
	JIPC_ERRNO_DECL(EPROTO),
	JIPC_ERRNO_DECL(ENOSPC),
};

/*
 * static void PFC_FATTR_INIT
 * jipc_test_libinit(void)
 *	Initialize unit test library.
 */
static void PFC_FATTR_INIT
jipc_test_libinit(void)
{
	jipc_errno_t	*ep;
	pfc_log_conf_t	lconf;

	/* Initialize errno tree. */
	for (ep = jipc_errno_nodes; ep < PFC_ARRAY_LIMIT(jipc_errno_nodes);
	     ep++) {
		PFC_ASSERT_INT(pfc_rbtree_put(&jipc_errno_tree, &ep->je_node),
			       0);
	}

	/* Suppress logs. */
	pfc_logconf_early(&lconf, PFC_CFBLK_INVALID, "pfc_ipc_test",
			  stderr, PFC_LOGLVL_FATAL, NULL);
	pfc_log_sysinit(&lconf);
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestBase_getJavaModeOption(
 *	JNIEnv *env, jclass cls)
 *
 *	Return a java command option to specify data model.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_TestBase_getJavaModeOption(
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
 * Java_org_opendaylight_vtn_core_ipc_TestBase_getJarFilePath(
 *	JNIEnv *env, jclass cls)
 *	Return a path to JAR file which contains test classes.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_TestBase_getJarFilePath(
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

/*
 * JNIEXPORT jobject JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestBase_getCurrentTime(
 *	JNIEnv *env, jclass cls)
 *	Return TimeSpec instance which contains the current system time.
 */
JNIEXPORT jobject JNICALL
Java_org_opendaylight_vtn_core_ipc_TestBase_getCurrentTime(
	JNIEnv *env, jclass cls)
{
	pfc_timespec_t	ts;
	int		err;

	err = pfc_clock_get_realtime(&ts);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to get system time: err=%d", err);

		return NULL;
	}

	return pjni_newobject(env, PJNI_CLASS(TimeSpec), JIPC_CTORSIG_TimeSpec,
			      (jlong)(uintptr_t)ts.tv_sec,
			      (jlong)(uintptr_t)ts.tv_nsec);
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestBase_getErrorNumber(
 *	JNIEnv *env, jclass cls, jstring jname)
 *
 *	Convert the given name of error number into a numeric error number.
 *
 * Calling/Exit State:
 *	Upon successful completion, a error number associated with the given
 *	name is returned.
 *	Otherwise zero is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_TestBase_getErrorNumber(
	JNIEnv *env, jclass cls, jstring jname)
{
	const char	*name;
	pfc_rbnode_t	*node;
	int		value;

	name = pjni_string_get(env, jname);
	if (PFC_EXPECT_FALSE(name == NULL)) {
		return 0;
	}

	node = pfc_rbtree_get(&jipc_errno_tree, (pfc_cptr_t)name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalArgumentException),
			   "Unknown errno name: %s", name);
		value = 0;
	}
	else {
		value = JIPC_ERRNO_NODE2PTR(node)->je_errno;
	}

	pjni_string_release(env, jname, name);

	return value;
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestBase_toHexPointer(
 *	JNIEnv *env, jclass cls, jlong value)
 *
 *	Convert the given long value into a string using "%p".
 *
 * Calling/Exit State:
 *	Upon successful completion, a string which represents the given
 *	long value is returned.
 *	name is returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_ipc_TestBase_toHexPointer(
	JNIEnv *env, jclass cls, jlong value)
{
	char	buf[32];

	snprintf(buf, sizeof(buf), "%p", (void *)(uintptr_t)value);

	return pjni_newstring(env, buf);
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_TestBase_removePath(
 *	JNIEnv *env, jclass cls, jstring jpath)
 *
 *	Remove the file specified by the given path.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_TestBase_removePath(
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
 * static pfc_cptr_t
 * jipc_errno_getkey(pfc_rbnode_t *node)
 *	Return the key of the given errno node.
 *	`node' must be a pointer to je_node in jipc_errno_t.
 */
static pfc_cptr_t
jipc_errno_getkey(pfc_rbnode_t *node)
{
	jipc_errno_t	*ep = JIPC_ERRNO_NODE2PTR(node);

	return (pfc_cptr_t)ep->je_name;
}
