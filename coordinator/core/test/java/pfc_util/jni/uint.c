/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * uint.c - Utilities to handle unsigned integer tests.
 */

#include <stdlib.h>
#include <org_opendaylight_vtn_core_util_UnsignedIntegerTest.h>
#include <pfc/jni.h>

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_util_UnsignedIntegerTest_compare(
 *	 JNIEnv *env, jclass cls, jlong l1, jlong l2)
 *
 *	Compare the specified two long values.
 *	The specified values are compared as unsigned integer.
 *
 * Calling/Exit State:
 *	A negative integer, zero, or a positive integer is returned if
 *	`l1' is less than, equal to, or greater than `l2' respectively.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_util_UnsignedIntegerTest_compare(
	 JNIEnv *env, jclass cls, jlong l1, jlong l2)
{
	if (l1 == l2) {
		return 0;
	}

	return ((uint64_t)l1 < (uint64_t)l2) ? -1 : 1;
}
