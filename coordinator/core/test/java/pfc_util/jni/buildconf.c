/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * buildconf.c - Utilities for build configuration test.
 */

#include <stdlib.h>
#include <string.h>
#include <org_opendaylight_vtn_core_CoreSystemTest.h>
#include <pfc/rbtree.h>
#include <pfc/debug.h>
#include <pfc/jni.h>

/*
 * Build configuration.
 */
typedef struct {
	const char	*bc_key;		/* configuration key string */
	union {
		const char	*string;	/* string value */
		int32_t		integer;	/* integer value */
	} bc_value;
	pfc_rbnode_t	bc_node;		/* Red-Black tree node */
} bldconf_t;

#define	BLDCONF_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), bldconf_t, bc_node)

/*
 * Declare expected build configuration strings.
 */
#define	BLDCONF_STRING_DECL(name)				\
	{							\
		.bc_key		= #name,			\
		.bc_value 	= {				\
			.string	= BLDCONF_STRING_VALUE_##name,	\
		},						\
	}

#define	BLDCONF_STRING_VALUE_PRODUCT_NAME	PFC_PRODUCT_NAME
#define	BLDCONF_STRING_VALUE_VERSION			\
	PFC_VERSION_STRING PFC_BUILD_TYPE_SUFFIX
#define	BLDCONF_STRING_VALUE_INST_PREFIX	PFC_ROOTDIR
#define	BLDCONF_STRING_VALUE_INST_BINDIR	PFC_BINDIR
#define	BLDCONF_STRING_VALUE_INST_DATADIR	PFC_DATADIR
#define	BLDCONF_STRING_VALUE_INST_LIBDIR	PFC_LIBDIR
#define	BLDCONF_STRING_VALUE_INST_LIBEXECDIR	PFC_LIBEXECDIR
#define	BLDCONF_STRING_VALUE_INST_LOCALSTATEDIR	PFC_LOCALSTATEDIR
#define	BLDCONF_STRING_VALUE_INST_SBINDIR	PFC_SBINDIR
#define	BLDCONF_STRING_VALUE_INST_SYSCONFDIR	PFC_SYSCONFDIR
#define	BLDCONF_STRING_VALUE_JAVA_LIBDIR	PFC_JAVADIR "/jar"
#define	BLDCONF_STRING_VALUE_JNI_LIBDIR		PFC_JAVADIR "/jni"

static bldconf_t	bldconf_string_nodes[] = {
	BLDCONF_STRING_DECL(PRODUCT_NAME),
	BLDCONF_STRING_DECL(VERSION),
	BLDCONF_STRING_DECL(INST_PREFIX),
	BLDCONF_STRING_DECL(INST_BINDIR),
	BLDCONF_STRING_DECL(INST_DATADIR),
	BLDCONF_STRING_DECL(INST_LIBDIR),
	BLDCONF_STRING_DECL(INST_LIBEXECDIR),
	BLDCONF_STRING_DECL(INST_LOCALSTATEDIR),
	BLDCONF_STRING_DECL(INST_SBINDIR),
	BLDCONF_STRING_DECL(INST_SYSCONFDIR),
	BLDCONF_STRING_DECL(JAVA_LIBDIR),
	BLDCONF_STRING_DECL(JNI_LIBDIR),
};

/*
 * Declare expected build configuration integers.
 */
#define	BLDCONF_INT_DECL(name)						\
	{								\
		.bc_key		= #name,				\
		.bc_value 	= {					\
			.integer	= BLDCONF_INT_VALUE_##name,	\
		},							\
	}

#define	BLDCONF_INT_VALUE_VERSION_MAJOR		PFC_VERSION_MAJOR
#define	BLDCONF_INT_VALUE_VERSION_MINOR		PFC_VERSION_MINOR
#define	BLDCONF_INT_VALUE_VERSION_REVISION	PFC_VERSION_REVISION
#define	BLDCONF_INT_VALUE_VERSION_PATCHLEVEL	PFC_VERSION_PATCHLEVEL

#ifdef	PFC_VERBOSE_DEBUG
#define	BLDCONF_INT_VALUE_DEBUG			1
#else	/* !PFC_VERBOSE_DEBUG */
#define	BLDCONF_INT_VALUE_DEBUG			0
#endif	/* PFC_VERBOSE_DEBUG */

static bldconf_t	bldconf_int_nodes[] = {
	BLDCONF_INT_DECL(VERSION_MAJOR),
	BLDCONF_INT_DECL(VERSION_MINOR),
	BLDCONF_INT_DECL(VERSION_REVISION),
	BLDCONF_INT_DECL(VERSION_PATCHLEVEL),
	BLDCONF_INT_DECL(DEBUG),
};

/*
 * Internal prototypes.
 */
static bldconf_t	*bldconf_lookup(JNIEnv *PFC_RESTRICT env,
					pfc_rbtree_t *PFC_RESTRICT tree,
					jstring jkey);
static pfc_cptr_t	bldconf_getkey(pfc_rbnode_t *node);

/*
 * Red-Black tree which keeps expected build configuration.
 */
#define	BLDCONF_TREE_INITIALIZER					\
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, bldconf_getkey)

static pfc_rbtree_t	bldconf_string = BLDCONF_TREE_INITIALIZER;
static pfc_rbtree_t	bldconf_integer = BLDCONF_TREE_INITIALIZER;

/*
 * static void PFC_FATTR_INIT
 * bldconf_init(void)
 *	Initialize build configuration.
 */
static void PFC_FATTR_INIT
bldconf_init(void)
{
	bldconf_t	*bcp;
	pfc_rbtree_t	*tree = &bldconf_string;

	for (bcp = bldconf_string_nodes;
	     bcp < PFC_ARRAY_LIMIT(bldconf_string_nodes); bcp++) {
		PFC_ASSERT_INT(pfc_rbtree_put(tree, &bcp->bc_node), 0);
	}

	tree = &bldconf_integer;
	for (bcp = bldconf_int_nodes;
	     bcp < PFC_ARRAY_LIMIT(bldconf_int_nodes); bcp++) {
		PFC_ASSERT_INT(pfc_rbtree_put(tree, &bcp->bc_node), 0);
	}
}

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_CoreSystemTest_getConfiguration(
 *	JNIEnv *env, jobject this, jstring jkey)
 *
 *	Return a string which repsents build configuration value associated
 *	with the given key.
 *
 * Calling/Exit State:
 *	NULL is returned with throwing an IllegalStateException if the given
 *	key is not found in the build configuration.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_CoreSystemTest_getConfiguration(
	JNIEnv *env, jobject this, jstring jkey)
{
	bldconf_t	*bcp;
	jstring		ret;

	bcp = bldconf_lookup(env, &bldconf_string, jkey);
	if (PFC_EXPECT_TRUE(bcp != NULL)) {
		ret = pjni_newstring(env, bcp->bc_value.string);
	}
	else {
		ret = NULL;
	}

	return ret;
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_CoreSystemTest_getIntConfiguration(
 *	JNIEnv *env, jobject this, jstring jkey)
 *
 *	Return an integer value which represents build configuration value
 *	associated with the given key.
 *
 * Calling/Exit State:
 *	Zero is returned with throwing an IllegalStateException if the given
 *	key is not found in the build configuration.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_CoreSystemTest_getIntConfiguration(
	JNIEnv *env, jobject this, jstring jkey)
{
	bldconf_t	*bcp;
	jint		ret;

	bcp = bldconf_lookup(env, &bldconf_integer, jkey);
	if (PFC_EXPECT_TRUE(bcp != NULL)) {
		ret = bcp->bc_value.integer;
	}
	else {
		ret = 0;
	}

	return ret;
}

/*
 * static bldconf_t *
 * bldconf_lookup(JNIEnv *PFC_RESTRICT env, pfc_rbtree_t *PFC_RESTRICT tree,
 *		  jstring jkey)
 *	Search for a build configuration associated with `jkey' in the given
 *	build confiuguration tree.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to bldconf_t is
 *	returned.
 *	If not found, NULL is returned with throwing an IllegalStateException.
 */
static bldconf_t *
bldconf_lookup(JNIEnv *PFC_RESTRICT env, pfc_rbtree_t *PFC_RESTRICT tree,
	       jstring jkey)
{
	const char	*key;
	pfc_rbnode_t	*node;
	bldconf_t	*bcp;

	if (PFC_EXPECT_FALSE(jkey == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Configuration key is null.");

		return NULL;
	}

	key = pjni_string_get(env, jkey);
	if (PFC_EXPECT_FALSE(key == NULL)) {
		return NULL;
	}

	node = pfc_rbtree_get(tree, (pfc_cptr_t)key);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		bcp = BLDCONF_NODE2PTR(node);
	}
	else {
		bcp = NULL;
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Configuration not found: %s", key);
	}

	pjni_string_release(env, jkey, key);

	return bcp;
}

/*
 * static pfc_cptr_t
 * bldconf_getkey(pfc_rbnode_t *node)
 *	Return the key of the given build configuration node.
 *	`node' must be a pointer to bc_node in bldconf_t.
 */
static pfc_cptr_t
bldconf_getkey(pfc_rbnode_t *node)
{
	bldconf_t	*bcp = BLDCONF_NODE2PTR(node);

	return (pfc_cptr_t)bcp->bc_key;
}
