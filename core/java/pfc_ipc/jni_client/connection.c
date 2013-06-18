/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * connection.c - IPC client connection management.
 */

#include <org_opendaylight_vtn_core_ipc_IpcConnection.h>
#include <org_opendaylight_vtn_core_ipc_DefaultConnection.h>
#include <org_opendaylight_vtn_core_ipc_AltConnection.h>
#include <org_opendaylight_vtn_core_ipc_ConnectionPool.h>
#include "ipcclnt_jni.h"

#if	org_opendaylight_vtn_core_ipc_IpcConnection_CONN_INVALID !=	\
	PFC_IPCCONN_INVALID
#error	Invalid value: IpcConnection.CONN_INVALID
#endif	/* IpcConnection.CONN_INVALID != PFC_IPCCONN_INVALID */

#if	org_opendaylight_vtn_core_ipc_ConnectionPool_CPOOL_INVALID != \
	PFC_IPCCPOOL_INVALID
#error	Invalid value: ConnectionPool.CPOOL_INVALID
#endif	/* ConnectionPool.CPOOL_INVALID != PFC_IPCCPOOL_INVALID */

#if	org_opendaylight_vtn_core_ipc_ConnectionPool_CPOOL_GLOBAL != \
	PFC_IPCCPOOL_GLOBAL
#error	Invalid value: ConnectionPool.CPOOL_GLOBAL
#endif	/* ConnectionPool.CPOOL_GLOBAL != PFC_IPCCPOOL_GLOBAL */

#if	org_opendaylight_vtn_core_ipc_ConnectionPool_CPOOL_CLOSE_FORCE != \
	PFC_IPCPLF_C_FORCE
#error	Invalid value: ConnectionPool.CPOOL_CLOSE_FORCE
#endif	/* ConnectionPool.CPOOL_CLOSE_FORCE != PFC_IPCPLF_C_FORCE */

/*
 * Internal prototypes.
 */
static void	cpool_not_found(JNIEnv *env, pfc_ipccpool_t pool);

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_DefaultConnection_setAddress(
 *	JNIEnv *env, jobject this, jstring chaddr)
 *
 *	Change IPC channel address of the default connection.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_DefaultConnection_setAddress(
	JNIEnv *env, jobject this, jstring chaddr)
{
	const char	*addr;
	int		err;

	PFC_ASSERT(chaddr != NULL);
	addr = pjni_string_get(env, chaddr);
	if (PFC_EXPECT_TRUE(addr != NULL)) {
		err = pfc_ipcclnt_setdefault(addr);
		if (PFC_EXPECT_FALSE(err != 0)) {
			jipc_throw(env, err, NULL,
				   "Failed to change default address: "
				   "addr=%s, err=%d", addr, err);
		}

		pjni_string_release(env, chaddr, addr);
	}
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_AltConnection_openImpl(
 *	JNIEnv *env, jobject this, jstring chaddr)
 *
 *	Open an alternative connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, a connection handle associated with a
 *	created alternative connection is returned.
 *
 *	Otherwise PFC_IPCCONN_INVALID is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_AltConnection_openImpl(
	JNIEnv *env, jobject this, jstring chaddr)
{
	const char	*addr;
	pfc_ipcconn_t	conn;
	int		err;

	if (chaddr == NULL) {
		addr = NULL;
	}
	else {
		addr = pjni_string_get(env, chaddr);
		if (PFC_EXPECT_FALSE(addr == NULL)) {
			return PFC_IPCCONN_INVALID;
		}
	}

	err = pfc_ipcclnt_altopen(addr, &conn);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to open an alternative connection: "
			   "addr=%s, err=%d",
			   (addr == NULL) ? jipc_str_null : addr, err);
		pjni_string_release(env, chaddr, addr);

		return PFC_IPCCONN_INVALID;
	}

	pjni_string_release(env, chaddr, addr);

	return conn;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_AltConnection_closeImpl(
 *	JNIEnv *env, jobject this, jint handle)
 *
 *	Close an alternative connection specified by `handle'.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_AltConnection_closeImpl(
	JNIEnv *env, jobject this, jint handle)
{
	int	err;

	err = pfc_ipcclnt_altclose((pfc_ipcconn_t)handle);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return;
	}

	/* Ignore EBADF error. */
	if (PFC_EXPECT_FALSE(err != EBADF)) {
		jipc_throw(env, err, NULL,
			   "Failed to close an alternative connection(%u): "
			   "err=%d", handle, err);
	}
}

/*
 * JNIEXPORT jint
 * JNICALL Java_org_opendaylight_vtn_core_ipc_ConnectionPool_create(
 *	JNIEnv *env, jclass cls, jint capacity)
 *
 *	Create a new connection pool.
 *
 * Calling/Exit State:
 *	Upon successful completion, a connection pool handle is returned.
 *	Otherwise PFC_IPCCPOOL_INVALID is returned with throwing an exception.
 */
JNIEXPORT jint
JNICALL Java_org_opendaylight_vtn_core_ipc_ConnectionPool_create(
	JNIEnv *env, jclass cls, jint capacity)
{
	pfc_ipccpool_t	pool;
	int		err;

	err = pfc_ipcclnt_cpool_create(&pool, capacity);
	if (PFC_EXPECT_FALSE(err != 0)) {
		jipc_throw(env, err, NULL,
			   "Failed to create a connection pool: err=%d", err);
		pool = PFC_IPCCPOOL_INVALID;
	}

	return (jint)pool;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ConnectionPool_destroy(
 *	JNIEnv *env, jclass cls, jint pool)
 *
 *	Destroy the connection pool specified by `pool'.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ConnectionPool_destroy(
	JNIEnv *env, jclass cls, jint pool)
{
	int	err;

	err = pfc_ipcclnt_cpool_destroy((pfc_ipccpool_t)pool);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (PFC_EXPECT_TRUE(err == ENOENT)) {
			cpool_not_found(env, pool);
		}
		else {
			jipc_throw(env, err, NULL,
				   "Failed to destroy a connection pool(%u): "
				   "err=%d", pool, err);
		}
	}
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ConnectionPool_open(
 *	JNIEnv *env, jclass cls, jint pool, jstring chaddr)
 *
 *	Create an alternative connection via the connection pool specified
 *	by `pool'.
 *
 * Calling/Exit State:
 *	Upon successful completion, an alternative connection handle is
 *	returned.
 *
 *	Otherwise PFC_IPCCONN_INVALID is returned with throwing an exception.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_ConnectionPool_open(
	JNIEnv *env, jclass cls, jint pool, jstring chaddr)
{
	const char	*addr;
	pfc_ipcconn_t	conn;
	int		err;

	if (chaddr == NULL) {
		addr = NULL;
	}
	else {
		addr = pjni_string_get(env, chaddr);
		if (PFC_EXPECT_FALSE(addr == NULL)) {
			return PFC_IPCCONN_INVALID;
		}
	}

	err = pfc_ipcclnt_cpool_open((pfc_ipccpool_t)pool, addr, &conn);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (PFC_EXPECT_TRUE(err == ENOENT)) {
			cpool_not_found(env, pool);
		}
		else {
			jipc_throw(env, err, NULL,
				   "Failed to open an alternative connection "
				   "from connection pool: pool=%u, addr=%s, "
				   "err=%d",
				   pool, (addr == NULL) ? jipc_str_null : addr,
				   err);
		}
		conn = PFC_IPCCONN_INVALID;
	}

	pjni_string_release(env, chaddr, addr);

	return conn;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ConnectionPool_close(
 *	JNIEnv *env, jclass cls, jint pool, jint handle, jint flags)
 *
 *	Close the alternative connection specified by `handle' in the
 *	connection pool specified by `pool'.
 *
 * Calling/Exit State:
 *	An exception is thrown on error.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ConnectionPool_close(
	JNIEnv *env, jclass cls, jint pool, jint handle, jint flags)
{
	int		err;

	err = pfc_ipcclnt_cpool_close((pfc_ipccpool_t)pool,
				      (pfc_ipcconn_t)handle,
				      (uint32_t)flags);
	/* Ignore EBADF error. */
	if (PFC_EXPECT_TRUE(err == 0 || err == EBADF)) {
		return;
	}

	if (err == ENOENT) {
		uint32_t	size;

		/*
		 * In this case, we need to check whether the pool exists
		 * or not.
		 */
		size = pfc_ipcclnt_cpool_getsize((pfc_ipccpool_t)pool);
		if (size == UINT32_MAX) {
			cpool_not_found(env, pool);
			return;
		}
	}

	jipc_throw(env, err, NULL,
		   "Failed to close an alternative connection(%u): "
		   "pool=%u, err=%d", handle, pool, err);
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ConnectionPool_getSize(
 *	JNIEnv *env, jobject this, jint pool)
 *
 *	Return the number of connections in the connection pool.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of connections is returned.
 *	Otherwise UINT32_MAX is returned with throwing an IpcBadPoolException.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_ConnectionPool_getSize(
	JNIEnv *env, jobject this, jint pool)
{
	uint32_t	size = pfc_ipcclnt_cpool_getsize(pool);

	if (PFC_EXPECT_FALSE(size == UINT32_MAX)) {
		cpool_not_found(env, pool);
	}

	return size;
}

/*
 * JNIEXPORT jint JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ConnectionPool_getCapacity(
 *	JNIEnv *env, jobject this, jint pool)
 *
 *	Return the capacity of the connection pool.
 *
 * Calling/Exit State:
 *	Upon successful completion, the capacity of the pool is returned.
 *	Otherwise UINT32_MAX is returned with throwing an IpcBadPoolException.
 */
JNIEXPORT jint JNICALL
Java_org_opendaylight_vtn_core_ipc_ConnectionPool_getCapacity(
	JNIEnv *env, jobject this, jint pool)
{
	uint32_t	capacity = pfc_ipcclnt_cpool_getcapacity(pool);

	if (PFC_EXPECT_FALSE(capacity == UINT32_MAX)) {
		cpool_not_found(env, pool);
	}

	return capacity;
}

/*
 * JNIEXPORT void JNICALL
 * Java_org_opendaylight_vtn_core_ipc_ConnectionPool_reap(
 *	JNIEnv *env, jclass cls, jboolean forced)
 *
 *	Reap unused connections in all connection pools.
 */
JNIEXPORT void JNICALL
Java_org_opendaylight_vtn_core_ipc_ConnectionPool_reap(
	JNIEnv *env, jclass cls, jboolean forced)
{
	PFC_ASSERT(forced == PFC_TRUE || forced == PFC_FALSE);

	pfc_ipcclnt_cpool_reap((pfc_bool_t)forced);
}

/*
 * static void
 * cpool_not_found(JNIEnv *env, pfc_ipccpool_t pool)
 *	Throw an IpcBadPoolException which indicates the connection pool
 *	does not exist.
 */
static void
cpool_not_found(JNIEnv *env, pfc_ipccpool_t pool)
{
	pjni_throw(env, PJNI_CLASS(IpcBadPoolException),
		   "The connection pool does not exist: pool=%u", pool);
}
