/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPC_STRUCT_JNI_H
#define	_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPC_STRUCT_JNI_H

/*
 * Internal definitions for IPC structure management.
 */

#include <pfc/jni.h>
#include <ipc_struct_impl.h>

/*
 * JNI signature of IpcStruct(String, long, int, long).
 */
#define	JIPC_CTORSIG_IpcStruct	PJNI_SIG_METHOD4(void, String, long, int, long)

/*
 * JNI signature of IpcStructField(String, String, int, int, String).
 */
#define	JIPC_CTORSIG_IpcStructField					\
	PJNI_SIG_METHOD5(void, String, String, int, int, String)

/*
 * Pointer cast macros.
 */
#define	JIPC_STRBUF_PTR(buffer)		((ipc_strbuf_t *)(uintptr_t)(buffer))
#define	JIPC_STRBUF_HANDLE(sbp)		((jlong)(uint64_t)(uintptr_t)(sbp))
#define	JIPC_STRINFO_PTR(info)		((ipc_strinfo_t *)(uintptr_t)(info))
#define	JIPC_STRINFO_HANDLE(sip)	((jlong)(uint64_t)(uintptr_t)(sip))
#define	JIPC_FLDMETA_PTR(field)		((ipc_fldmeta_t *)(uintptr_t)(field))
#define	JIPC_FLDMETA_HANDLE(fmp)	((jlong)(uint64_t)(uintptr_t)(fmp))

#endif	/* !_PFC_JAVA_PFC_IPC_JNI_CLIENT_IPC_STRUCT_JNI_H */
