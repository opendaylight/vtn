/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * hostaddr.c - Host address utility.
 */

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <org_opendaylight_vtn_core_util_HostAddress.h>
#include "pfc_util_jni.h"

/*
 * JNIEXPORT jstring JNICALL
 * Java_org_opendaylight_vtn_core_util_HostAddress_toInet6TextAddress(
 *	JNIEnv *env, jclass cls, jbyteArray addr)
 *
 *	Convert raw IPv6 address into a string.
 *
 * Calling/Exit State:
 *	Upon successful completion, a reference to String is returned.
 *	Otherwise an exception is thrown and null is returned.
 */
JNIEXPORT jstring JNICALL
Java_org_opendaylight_vtn_core_util_HostAddress_toInet6TextAddress(
	JNIEnv *env, jclass cls, jbyteArray addr)
{
	jsize		nelems;
	jbyte		*ptr;
	char		buf[INET6_ADDRSTRLEN];
	const char	*ret;

	nelems = (*env)->GetArrayLength(env, addr);
	if (PFC_EXPECT_FALSE(nelems != sizeof(struct in6_addr))) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unexpected IPv6 address size: %u", nelems);

		return NULL;
	}

	ptr = pjni_bytearray_get(env, addr);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		return NULL;
	}

	ret = inet_ntop(AF_INET6, ptr, buf, sizeof(buf));
	pjni_bytearray_release(env, addr, ptr, PFC_TRUE);

	if (PFC_EXPECT_FALSE(ret == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Failed to convert IPv6 address: err=%d", errno);

		return NULL;
	}

	return pjni_newstring(env, ret);
}
