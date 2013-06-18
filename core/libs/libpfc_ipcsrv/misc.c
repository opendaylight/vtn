/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.c - Miscellaneous utilities.
 */

#include <pfc/util.h>
#include "ipcsrv_impl.h"

const char	ipcsrv_str_unknown[] PFC_ATTR_HIDDEN = "<unknown>";
const char	ipcsrv_str_empty[] PFC_ATTR_HIDDEN = "";

/*
 * Flag which indicates whether internal log should be recorded by PFC logging
 * architecture.
 * This flag is updated by atomic operation.
 */
volatile pfc_bool_t	ipcsrv_log_enabled PFC_ATTR_HIDDEN = PFC_FALSE;

/*
 * static void PFC_FATTR_FINI
 * ipcsrv_libfini(void)
 *	Destructor of libpfc_ipcsrv library.
 */
static void PFC_FATTR_FINI
ipcsrv_libfini(void)
{
	pfc_ipcsrv_channel_cleanup();
}

/*
 * Declare accessor of IPC output stream.
 */
#define	IPCSRV_OUTPUT_DECL_BODY(srv, suffix, args, invalid_args)	\
	int		err;						\
									\
	if (PFC_EXPECT_FALSE(invalid_args)) {				\
		return EINVAL;						\
	}								\
									\
	IPCSRV_LOCK(srv);						\
	if (IPCSRV_IS_RESET(srv)) {					\
		/* Connection is already reset. */			\
		err = ECONNRESET;					\
	}								\
	else {								\
		err = pfc_ipcstream_add_##suffix args;			\
		if (PFC_EXPECT_FALSE(err != 0)) {			\
			IPCSRV_LOG_ERROR("%s.%u: Failed to add %s "	\
					 "argument: %s",		\
					 srv->isv_name, srv->isv_service, \
					 #suffix, strerror(err));	\
		 }							\
	}								\
	IPCSRV_UNLOCK(srv);						\
									\
	return err

#define	IPCSRV_OUTPUT_DECL_NUMERIC(argtype, suffix)			\
	int								\
	pfc_ipcsrv_output_##suffix(pfc_ipcsrv_t *srv, argtype data)	\
	{								\
		IPCSRV_OUTPUT_DECL_BODY(srv, suffix,			\
					(&(srv)->isv_output, data),	\
					(srv == NULL));			\
	}

#define	IPCSRV_OUTPUT_DECL_IP(argtype, suffix)				\
	int								\
	pfc_ipcsrv_output_##suffix(pfc_ipcsrv_t *PFC_RESTRICT srv,	\
				   argtype *PFC_RESTRICT data)		\
	{								\
		IPCSRV_OUTPUT_DECL_BODY(srv, suffix,			\
					(&(srv)->isv_output, data),	\
					(srv == NULL || data == NULL));	\
	}

#define	IPCSRV_OUTPUT_DECL_PTR(argtype, suffix)				\
	int								\
	pfc_ipcsrv_output_##suffix(pfc_ipcsrv_t *PFC_RESTRICT srv,	\
				   argtype *PFC_RESTRICT data)		\
	{								\
		IPCSRV_OUTPUT_DECL_BODY(srv, suffix,			\
					(&(srv)->isv_output, data),	\
					(srv == NULL));			\
	}

#define	IPCSRV_OUTPUT_DECL_PTR_LENGTH(argtype, suffix)			\
	int								\
	pfc_ipcsrv_output_##suffix(pfc_ipcsrv_t *PFC_RESTRICT srv,	\
				   argtype *PFC_RESTRICT data,		\
				   uint32_t length)			\
	{								\
		IPCSRV_OUTPUT_DECL_BODY(srv, suffix,			\
					(&(srv)->isv_output, data, length), \
					(srv == NULL));			\
	}

IPCSRV_OUTPUT_DECL_NUMERIC(int8_t, int8);
IPCSRV_OUTPUT_DECL_NUMERIC(uint8_t, uint8);
IPCSRV_OUTPUT_DECL_NUMERIC(int16_t, int16);
IPCSRV_OUTPUT_DECL_NUMERIC(uint16_t, uint16);
IPCSRV_OUTPUT_DECL_NUMERIC(int32_t, int32);
IPCSRV_OUTPUT_DECL_NUMERIC(uint32_t, uint32);
IPCSRV_OUTPUT_DECL_NUMERIC(int64_t, int64);
IPCSRV_OUTPUT_DECL_NUMERIC(uint64_t, uint64);
IPCSRV_OUTPUT_DECL_NUMERIC(float, float);
IPCSRV_OUTPUT_DECL_NUMERIC(double, double);

IPCSRV_OUTPUT_DECL_IP(struct in_addr, ipv4);
IPCSRV_OUTPUT_DECL_IP(struct in6_addr, ipv6);
IPCSRV_OUTPUT_DECL_PTR(const char, string);

IPCSRV_OUTPUT_DECL_PTR_LENGTH(const uint8_t, binary);

/*
 * int
 * pfc_ipcsrv_output_null(pfc_ipcsrv_t *srv)
 *	Append a NULL data to the IPC output stream for the client.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_output_null(pfc_ipcsrv_t *srv)
{
	IPCSRV_OUTPUT_DECL_BODY(srv, null, (&(srv)->isv_output),
				(srv == NULL));
}

/*
 * int
 * __pfc_ipcsrv_output_struct(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			      const uint8_t *PFC_RESTRICT data, uint32_t length,
 *			      const char *PFC_RESTRICT stname,
 *			      const char *PFC_RESTRICT sig)
 *	Append a struct data to the IPC output stream for the client.
 *
 *	`stname' must be a name of struct to be sent. It must be defined by
 *	IPC struct template file.
 *	`sig' must be the layout signature required by the struct.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ENODEV is returned if IPC struct information associated with the
 *	given struct name is not found or invalid.
 *	EFAULT is returned if `data' is not aligned on required boundary.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This is not a public interface.
 *	Never call this function directly.
 */
int
__pfc_ipcsrv_output_struct(pfc_ipcsrv_t *PFC_RESTRICT srv,
			   const uint8_t *PFC_RESTRICT data, uint32_t length,
			   const char *PFC_RESTRICT stname,
			   const char *PFC_RESTRICT sig)
{
	IPCSRV_OUTPUT_DECL_BODY(srv, struct,
				(&(srv)->isv_output, data, length, stname,
				 sig),
				(srv == NULL || data == NULL ||
				 stname == NULL || sig == NULL));
}

/*
 * int
 * pfc_ipcsrv_output_stdef(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			   const pfc_ipcstdef_t *PFC_RESTRICT defp,
 *			   pfc_cptr_t data)
 *	Append a struct data to the IPC output stream for the client.
 *
 *	`defp' must be a pointer to pfc_ipcstdef_t, which represents
 *	definition of IPC structure. It must be initialized by
 *	PFC_IPC_STDEF_INIT() or PFC_IPC_STDEF_INITIALIZER() macro.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ENODEV is returned if IPC struct information specified by `defp' is
 *	is not found or invalid.
 *	EFAULT is returned if `data' is not aligned on required boundary.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_output_stdef(pfc_ipcsrv_t *PFC_RESTRICT srv,
			const pfc_ipcstdef_t *PFC_RESTRICT defp,
			pfc_cptr_t data)
{
	if (PFC_EXPECT_FALSE(defp == NULL)) {
		return EINVAL;
	}

	return __pfc_ipcsrv_output_struct(srv, (const uint8_t *)data,
					  defp->ist_size, defp->ist_name,
					  defp->ist_signature);
}

/*
 * Declare accessor of IPC message.
 */
#define	IPCSRV_GETARG_DECL_BODY(suffix, args, is_invalid)		\
	int	err;							\
									\
	if (PFC_EXPECT_FALSE(is_invalid)) {				\
		return EINVAL;						\
	}								\
									\
	err = ipcsrv_is_reset(srv);					\
	if (PFC_EXPECT_TRUE(err == 0)) {				\
		err = pfc_ipcmsg_get_##suffix args;			\
	}								\
									\
	return err

#define	IPCSRV_GETARG_DECL(argtype, suffix)				\
	int								\
	pfc_ipcsrv_getarg_##suffix(pfc_ipcsrv_t *PFC_RESTRICT srv,	\
				   uint32_t index,			\
				   argtype *PFC_RESTRICT datap)		\
	{								\
		IPCSRV_GETARG_DECL_BODY(suffix,				\
					(&(srv)->isv_args, index, datap), \
					(srv == NULL || datap == NULL)); \
	}

#define	IPCSRV_GETARG_DECL_PP(argtype, suffix)				\
	int								\
	pfc_ipcsrv_getarg_##suffix(pfc_ipcsrv_t *PFC_RESTRICT srv,	\
				   uint32_t index,			\
				   argtype **PFC_RESTRICT datapp)	\
	{								\
		IPCSRV_GETARG_DECL_BODY(suffix,				\
					(&(srv)->isv_args, index, datapp), \
					(srv == NULL || datapp == NULL)); \
	}

#define	IPCSRV_GETARG_DECL_PP_LENGTH(argtype, suffix)			\
	int								\
	pfc_ipcsrv_getarg_##suffix(pfc_ipcsrv_t *PFC_RESTRICT srv,	\
				   uint32_t index,			\
				   argtype **PFC_RESTRICT datapp,	\
				   uint32_t *PFC_RESTRICT lengthp)	\
	{								\
		IPCSRV_GETARG_DECL_BODY(suffix,				\
					(&(srv)->isv_args, index, datapp, \
					 lengthp),			\
					(srv == NULL || datapp == NULL || \
					 lengthp == NULL));		\
	}

IPCSRV_GETARG_DECL(int8_t, int8);
IPCSRV_GETARG_DECL(uint8_t, uint8);
IPCSRV_GETARG_DECL(int16_t, int16);
IPCSRV_GETARG_DECL(uint16_t, uint16);
IPCSRV_GETARG_DECL(int32_t, int32);
IPCSRV_GETARG_DECL(uint32_t, uint32);
IPCSRV_GETARG_DECL(int64_t, int64);
IPCSRV_GETARG_DECL(uint64_t, uint64);
IPCSRV_GETARG_DECL(float, float);
IPCSRV_GETARG_DECL(double, double);
IPCSRV_GETARG_DECL(struct in_addr, ipv4);
IPCSRV_GETARG_DECL(struct in6_addr, ipv6);

IPCSRV_GETARG_DECL_PP(const char, string);
IPCSRV_GETARG_DECL_PP_LENGTH(const uint8_t, binary);

/*
 * int
 * __pfc_ipcsrv_getarg_struct(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
 *			      uint8_t *PFC_RESTRICT datap, uint32_t length,
 *			      const char *PFC_RESTRICT stname,
 *			      const char *PFC_RESTRICT sig)
 *	Fetch a struct data sent by the IPC client.
 *
 *	Data is copied to the buffer pointed by `datap'. It must be `length'
 *	long, and `length' must equal to the size of struct.
 *
 *	`stname' must be a name of struct to be sent. It must be defined by
 *	IPC struct template file.
 *	`sig' must be the layout signature required by the struct.
 *
 * Calling/Exit State:
 *	Upon successful completion, struct data at the given index is copied
 *	to the buffer pointed by `datap'.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if data type specified by `stname' does not match
 *	the type of actual PDU type.
 *	ENODEV is returned if IPC struct information associated with the
 *	given struct name is not found or invalid.
 *	EFAULT is returned if `datap' is not aligned on required boundary.
 *	EPROTO is returned if the PDU is broken.
 *	EBADMSG is returned if struct layout in the PDU does not match.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This is not a public interface.
 *	Never call this function directly.
 */
int
__pfc_ipcsrv_getarg_struct(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
			   uint8_t *PFC_RESTRICT datap, uint32_t length,
			   const char *PFC_RESTRICT stname,
			   const char *PFC_RESTRICT sig)
{
	IPCSRV_GETARG_DECL_BODY(struct,
				(&(srv)->isv_args, index, datap, length,
				 stname, sig),
				(srv == NULL || datap == NULL ||
				 stname == NULL || sig == NULL));
}

/*
 * int
 * pfc_ipcsrv_getarg_stdef(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
 *			   const pfc_ipcstdef_t *PFC_RESTRICT defp,
 *			   pfc_ptr_t datap)
 *	Fetch a struct data sent by the IPC client.
 *
 *	`defp' must be a pointer to pfc_ipcstdef_t, which represents
 *	definition of IPC structure. It must be initialized by
 *	PFC_IPC_STDEF_INIT() or PFC_IPC_STDEF_INITIALIZER() macro.
 *
 *	IPC structure is copied to the buffer pointed by `datap'. It must be
 *	defp->ist_size long.
 *
 * Calling/Exit State:
 *	Upon successful completion, struct data at the given index is copied
 *	to the buffer pointed by `datap'.
 *
 *	EINVAL is returned if an invalid index is specified.
 *	EPERM is returned if data type specified by `defp' does not match
 *	the type of actual PDU type.
 *	ENODEV is returned if IPC struct information specified by `defp' is
 *	is not found or invalid.
 *	EFAULT is returned if `datap' is not aligned on required boundary.
 *	EPROTO is returned if the PDU is broken.
 *	EBADMSG is returned if struct layout in the PDU does not match.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_getarg_stdef(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
			const pfc_ipcstdef_t *PFC_RESTRICT defp,
			pfc_ptr_t datap)
{
	if (PFC_EXPECT_FALSE(defp == NULL)) {
		return EINVAL;
	}

	return __pfc_ipcsrv_getarg_struct(srv, index, (uint8_t *)datap,
					  defp->ist_size, defp->ist_name,
					  defp->ist_signature);
}

/*
 * int
 * pfc_ipcsrv_getarg_structname(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
 *				const char **PFC_RESTRICT namepp)
 *	Determine struct name of received additional data at the given index.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to struct name is
 *	set to `*namepp', and zero is returned.
 *
 *	EINVAL is returned if invalid argument is specified.
 *	EPERM is returned if the type of the specified PDU is not STRUCT.
 *	EPROTO is returned if the PDU is broken.
 *	ECONNRESET is returned if the connection associated with `srv' is
 *	already reset.
 */
int
pfc_ipcsrv_getarg_structname(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
			     const char **PFC_RESTRICT namepp)
{
	int	err;

	if (PFC_EXPECT_FALSE(srv == NULL || namepp == NULL)) {
		return EINVAL;
	}

	err = ipcsrv_is_reset(srv);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_ipcmsg_get_struct_name(&srv->isv_args, index,
						 namepp);
	}

	return err;
}

/*
 * uint32_t
 * pfc_ipcsrv_getargcount(pfc_ipcsrv_t *srv)
 *	Return the number of PDUs sent by the IPC client.
 *
 * Remarks:
 *	This function returns zero if NULL is specified to `srv'.
 */
uint32_t
pfc_ipcsrv_getargcount(pfc_ipcsrv_t *srv)
{
	if (PFC_EXPECT_FALSE(srv == NULL || ipcsrv_is_reset(srv) != 0)) {
		return 0;
	}

	return pfc_ipcmsg_getcount(&srv->isv_args);
}

/*
 * int
 * pfc_ipcsrv_getargtype(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
 *			 pfc_ipctype_t *PFC_RESTRICT typep)
 *	Get type of argument PDU at the index specified by `index'.
 *
 * Calling/Exit State:
 *	Upon successful completion, PDU type identifier is set to `*typep',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_getargtype(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t index,
		      pfc_ipctype_t *PFC_RESTRICT typep)
{
	int	err;

	if (PFC_EXPECT_FALSE(srv == NULL || typep == NULL)) {
		return EINVAL;
	}

	err = ipcsrv_is_reset(srv);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_ipcmsg_gettype(&srv->isv_args, index, typep);
	}

	return err;
}

/*
 * int
 * pfc_ipcsrv_settimeout(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			 const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Set the timeout of the server session.
 *
 *	If the IPC service handler does not return within the period specified
 *	by `timeout', the IPC service is considered as timed out.
 *	Specifying NULL to `timeout' means the server session timeout should
 *	be disabled.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_settimeout(pfc_ipcsrv_t *PFC_RESTRICT srv,
		      const pfc_timespec_t *PFC_RESTRICT timeout)
{
	int	err;

	if (PFC_EXPECT_FALSE(srv == NULL)) {
		return EINVAL;
	}

	IPCSRV_LOCK(srv);

	if (IPCSRV_IS_RESET(srv)) {
		err = ECONNRESET;
	}
	else if (timeout == NULL) {
		err = 0;
		srv->isv_flags |= (IPCSRVF_NOTIMEOUT | IPCSRVF_HASTIMEOUT);
	}
	else if (PFC_EXPECT_FALSE(!PFC_CLOCK_IS_VALID(timeout))) {
		err = EINVAL;
	}
	else {
		err = 0;
		srv->isv_flags |= IPCSRVF_HASTIMEOUT;
		srv->isv_flags &= ~IPCSRVF_NOTIMEOUT;
		srv->isv_timeout = *timeout;
	}

	IPCSRV_UNLOCK(srv);

	return err;
}

/*
 * void
 * pfc_ipcsrv_enable_log(pfc_bool_t enable)
 *	Enable or disable internal logging.
 *
 *	If PFC_TRUE is specified to `enable', internal logging by PFC log
 *	architecture is enabled. It is up to the caller for initialization of
 *	PFC log.
 *
 *	Internal logging is disabled by default.
 */
void
pfc_ipcsrv_enable_log(pfc_bool_t enable)
{
	(void)pfc_atomic_swap_uint8((uint8_t *)&ipcsrv_log_enabled, enable);

	pfc_ipc_enable_log(enable);
}
