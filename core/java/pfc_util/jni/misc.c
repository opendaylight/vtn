/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.c - Miscellaneous routines.
 */

#include "pfc_util_jni.h"

/*
 * jint
 * JNI_OnLoad(JavaVM *jvm, void *reserved)
 *	Notify required JNI version to the Java VM.
 */
jint
JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	/* Return minimum JNI version. */
	return PJNI_MIN_VERSION;
}

/*
 * void
 * JNI_OnUnload(JavaVM *jvm, void *reserved)
 *	Clean up handler of the native library.
 */
void
JNI_OnUnload(JavaVM *jvm, void *reserved)
{
	log_cleanup(jvm);
}
