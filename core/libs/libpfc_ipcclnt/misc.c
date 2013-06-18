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
#include "ipcclnt_impl.h"
#include "ipcclnt_event.h"

/*
 * Flag which indicates whether internal log should be recorded by PFC logging
 * architecture.
 * This flag is updated by atomic operation.
 */
volatile pfc_bool_t	ipcclnt_log_enabled PFC_ATTR_HIDDEN = PFC_FALSE;

/*
 * Client process name.
 */
pfc_refptr_t	*ipcclnt_procname PFC_ATTR_HIDDEN;

/*
 * Global IPC client option.
 */
ipc_option_t	ipcclnt_option PFC_ATTR_HIDDEN = IPC_OPTION_INITIALIZER;

/*
 * String table.
 */
const char	ipc_str_empty[] PFC_ATTR_HIDDEN = {'\0'};
const char	ipc_str_null[] PFC_ATTR_HIDDEN = "<null>";
const char	ipc_str_any[] PFC_ATTR_HIDDEN = "<any>";
const char	ipc_str_svname_chstate[] PFC_ATTR_HIDDEN = IPC_SVNAME_CHSTATE;
const char	ipc_str_chaddr[] PFC_ATTR_HIDDEN = "IPC Channel Address";
const char	ipc_str_hostset[] PFC_ATTR_HIDDEN = "IPC Host Set";

/*
 * Internal prototypes.
 */
static void	ipcclnt_fork_prepare(void);
static void	ipcclnt_fork_parent(void);
static void	ipcclnt_fork_child(void);

/*
 * fork(2) handlers.
 */
static const ipc_fork_t		fork_handler = {
	.if_prepare	= ipcclnt_fork_prepare,
	.if_parent	= ipcclnt_fork_parent,
	.if_child	= ipcclnt_fork_child,
};

/*
 * Ticket for library destructor.
 */
static uint8_t		libfini_ticket = 0;

/*
 * static void PFC_FATTR_INIT
 * ipcclnt_libinit(void)
 *	Constructor of libpfc_ipcclnt library.
 */
static void PFC_FATTR_INIT
ipcclnt_libinit(void)
{
	/* Register fork(2) handlers to libpfc_ipc. */
	pfc_ipc_client_init(&fork_handler);

	/* Initialize default channel name. */
	pfc_ipcclnt_channel_init();

	/* Initialize default event attributes. */
	pfc_ipcevent_attr_sysinit();
}

/*
 * static void PFC_FATTR_FINI
 * ipcclnt_libfini(void)
 *	Destructor of libpfc_ipcclnt library.
 */
static void PFC_FATTR_FINI
ipcclnt_libfini(void)
{
	uint8_t	ticket;

	ticket = pfc_atomic_swap_uint8(&libfini_ticket, 1);
	if (ticket == 0) {
		pfc_ipcclnt_conn_cleanup();
		pfc_ipcclnt_chan_cleanup();
		pfc_ipcclnt_canceller_cleanup();
	}
}

/*
 * Declare accessor of IPC output stream.
 */
#define	IPCCLNT_OUTPUT_DECL_BODY(sess, suffix, args, invalid_args)	\
	ipc_sstate_t	state;						\
	int		err;						\
									\
	if (PFC_EXPECT_FALSE(invalid_args)) {				\
		return EINVAL;						\
	}								\
									\
	IPC_CLSESS_LOCK(sess);						\
									\
	/* Check session state. */					\
	state = sess->icss_state;					\
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_READY)) {		\
		err = pfc_ipcstream_add_##suffix args;			\
	}								\
	else {								\
		err = pfc_ipcclnt_sess_state_error(state);		\
	}								\
									\
	IPC_CLSESS_UNLOCK(sess);					\
									\
	return err

#define	IPCCLNT_OUTPUT_DECL_NUMERIC(argtype, suffix)			\
	int								\
	pfc_ipcclnt_output_##suffix(pfc_ipcsess_t *sess, argtype data)	\
	{								\
		IPCCLNT_OUTPUT_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_output, data),	\
					 (sess == NULL));		\
	}

#define	IPCCLNT_OUTPUT_DECL_PTR(argtype, suffix)			\
	int								\
	pfc_ipcclnt_output_##suffix(pfc_ipcsess_t *PFC_RESTRICT sess,	\
				    argtype *PFC_RESTRICT data)		\
	{								\
		IPCCLNT_OUTPUT_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_output, data),	\
					 (sess == NULL));		\
	}

#define	IPCCLNT_OUTPUT_DECL_IP(argtype, suffix)				\
	int								\
	pfc_ipcclnt_output_##suffix(pfc_ipcsess_t *PFC_RESTRICT sess,	\
				    argtype *PFC_RESTRICT data)		\
	{								\
		IPCCLNT_OUTPUT_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_output, data),	\
					 (sess == NULL || data == NULL)); \
	}

#define	IPCCLNT_OUTPUT_DECL_PTR_LENGTH(argtype, suffix)			\
	int								\
	pfc_ipcclnt_output_##suffix(pfc_ipcsess_t *PFC_RESTRICT sess,	\
				    argtype *PFC_RESTRICT data,		\
				    uint32_t length)			\
	{								\
		IPCCLNT_OUTPUT_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_output, data,	\
					  length), (sess == NULL));	\
	}

IPCCLNT_OUTPUT_DECL_NUMERIC(int8_t, int8);
IPCCLNT_OUTPUT_DECL_NUMERIC(uint8_t, uint8);
IPCCLNT_OUTPUT_DECL_NUMERIC(int16_t, int16);
IPCCLNT_OUTPUT_DECL_NUMERIC(uint16_t, uint16);
IPCCLNT_OUTPUT_DECL_NUMERIC(int32_t, int32);
IPCCLNT_OUTPUT_DECL_NUMERIC(uint32_t, uint32);
IPCCLNT_OUTPUT_DECL_NUMERIC(int64_t, int64);
IPCCLNT_OUTPUT_DECL_NUMERIC(uint64_t, uint64);
IPCCLNT_OUTPUT_DECL_NUMERIC(float, float);
IPCCLNT_OUTPUT_DECL_NUMERIC(double, double);

IPCCLNT_OUTPUT_DECL_IP(struct in_addr, ipv4);
IPCCLNT_OUTPUT_DECL_IP(struct in6_addr, ipv6);
IPCCLNT_OUTPUT_DECL_PTR(const char, string);

IPCCLNT_OUTPUT_DECL_PTR_LENGTH(const uint8_t, binary);

/*
 * int
 * pfc_ipcclnt_output_null(pfc_ipcsess_t *sess)
 *	Append a NULL data to the IPC output stream for the server.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_output_null(pfc_ipcsess_t *sess)
{
	IPCCLNT_OUTPUT_DECL_BODY(sess, null, (&(sess)->icss_output),
				 (sess == NULL));
}

/*
 * int
 * __pfc_ipcclnt_output_struct(pfc_ipcclnt_t *PFC_RESTRICT sess,
 *			       const uint8_t *PFC_RESTRICT data,
 *			       uint32_t length,
 *			       const char *PFC_RESTRICT stname,
 *			       const char *PFC_RESTRICT sig)
 *	Append a struct data to the IPC output stream for the server.
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
__pfc_ipcclnt_output_struct(pfc_ipcsess_t *PFC_RESTRICT sess,
			    const uint8_t *PFC_RESTRICT data, uint32_t length,
			    const char *PFC_RESTRICT stname,
			    const char *PFC_RESTRICT sig)
{
	IPCCLNT_OUTPUT_DECL_BODY(sess, struct,
				 (&(sess)->icss_output, data, length, stname,
				  sig),
				 (sess == NULL || data == NULL ||
				  stname == NULL || sig == NULL));
}

/*
 * int
 * pfc_ipcclnt_output_stdef(pfc_ipcsess_t *PFC_RESTRICT sess,
 *			    const pfc_ipcstdef_t *PFC_RESTRICT defp,
 *			    pfc_cptr_t data)
 *	Append a struct data to the IPC output stream for the server.
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
pfc_ipcclnt_output_stdef(pfc_ipcsess_t *PFC_RESTRICT sess,
			 const pfc_ipcstdef_t *PFC_RESTRICT defp,
			 pfc_cptr_t data)
{
	if (PFC_EXPECT_FALSE(defp == NULL)) {
		return EINVAL;
	}

	return __pfc_ipcclnt_output_struct(sess, (const uint8_t *)data,
					   defp->ist_size, defp->ist_name,
					   defp->ist_signature);
}

/*
 * Declare accessor of IPC message.
 */
#define	IPCCLNT_GETRES_DECL_BODY(sess, suffix, args, is_invalid)	\
	ipc_sstate_t	state;						\
	int		err;						\
									\
	if (PFC_EXPECT_FALSE(is_invalid)) {				\
		return EINVAL;						\
	}								\
									\
	IPC_CLSESS_LOCK(sess);						\
									\
	/* Check session state. */					\
	state = sess->icss_state;					\
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_RESULT)) {		\
		err = pfc_ipcmsg_get_##suffix args;			\
	}								\
	else {								\
		err = pfc_ipcclnt_sess_state_error(state);		\
	}								\
									\
	IPC_CLSESS_UNLOCK(sess);					\
									\
	return err

#define	IPCCLNT_GETRES_DECL(argtype, suffix)				\
	int								\
	pfc_ipcclnt_getres_##suffix(pfc_ipcsess_t *PFC_RESTRICT sess,	\
				    uint32_t index,			\
				    argtype *PFC_RESTRICT datap)	\
	{								\
		IPCCLNT_GETRES_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_msg, index, datap), \
					 (sess == NULL || datap == NULL)); \
	}

#define	IPCCLNT_GETRES_DECL_PP(argtype, suffix)				\
	int								\
	pfc_ipcclnt_getres_##suffix(pfc_ipcsess_t *PFC_RESTRICT sess,	\
				    uint32_t index,			\
				    argtype **PFC_RESTRICT datapp)	\
	{								\
		IPCCLNT_GETRES_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_msg, index, datapp), \
					 (sess == NULL || datapp == NULL)); \
	}

#define	IPCCLNT_GETRES_DECL_PP_LENGTH(argtype, suffix)			\
	int								\
	pfc_ipcclnt_getres_##suffix(pfc_ipcsess_t *PFC_RESTRICT sess,	\
				    uint32_t index,			\
				    argtype **PFC_RESTRICT datapp,	\
				    uint32_t *lengthp)			\
	{								\
		IPCCLNT_GETRES_DECL_BODY(sess, suffix,			\
					 (&(sess)->icss_msg, index, datapp, \
					  lengthp),			\
					 (sess == NULL || datapp == NULL || \
					  lengthp == NULL));		\
	}

IPCCLNT_GETRES_DECL(int8_t, int8);
IPCCLNT_GETRES_DECL(uint8_t, uint8);
IPCCLNT_GETRES_DECL(int16_t, int16);
IPCCLNT_GETRES_DECL(uint16_t, uint16);
IPCCLNT_GETRES_DECL(int32_t, int32);
IPCCLNT_GETRES_DECL(uint32_t, uint32);
IPCCLNT_GETRES_DECL(int64_t, int64);
IPCCLNT_GETRES_DECL(uint64_t, uint64);
IPCCLNT_GETRES_DECL(float, float);
IPCCLNT_GETRES_DECL(double, double);
IPCCLNT_GETRES_DECL(struct in_addr, ipv4);
IPCCLNT_GETRES_DECL(struct in6_addr, ipv6);

IPCCLNT_GETRES_DECL_PP(const char, string);
IPCCLNT_GETRES_DECL_PP_LENGTH(const uint8_t, binary);

/*
 * int
 * __pfc_ipcclnt_getres_struct(pfc_ipcsess_t *PFC_RESTRICT sess, uint32_t index,
 *			       uint8_t *PFC_RESTRICT datap, uint32_t length,
 *			       const char *PFC_RESTRICT stname,
 *			       const char *PFC_RESTRICT sig)
 *	Fetch a struct data sent by the IPC server.
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
 *	to the buffer pointed by `datap', and zero is returned.
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
__pfc_ipcclnt_getres_struct(pfc_ipcsess_t *PFC_RESTRICT sess, uint32_t index,
			    uint8_t *PFC_RESTRICT datap, uint32_t length,
			    const char *PFC_RESTRICT stname,
			    const char *PFC_RESTRICT sig)
{
	IPCCLNT_GETRES_DECL_BODY(sess, struct,
				 (&(sess)->icss_msg, index, datap, length,
				  stname, sig),
				 (sess == NULL || datap == NULL ||
				  stname == NULL || sig == NULL));
}

/*
 * int
 * pfc_ipcclnt_getres_stdef(pfc_ipcsess_t *PFC_RESTRICT sess, uint32_t index,
 *			    const pfc_ipcstdef_t *PFC_RESTRICT defp,
 *			    pfc_ptr_t datap)
 *	Fetch an IPC structure sent by the IPC server.
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
 *	to the buffer pointed by `datap', and zero is returned.
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
pfc_ipcclnt_getres_stdef(pfc_ipcsess_t *PFC_RESTRICT sess, uint32_t index,
			 const pfc_ipcstdef_t *PFC_RESTRICT defp,
			 pfc_ptr_t datap)
{
	if (PFC_EXPECT_FALSE(defp == NULL)) {
		return EINVAL;
	}

	return __pfc_ipcclnt_getres_struct(sess, index, (uint8_t *)datap,
					   defp->ist_size, defp->ist_name,
					   defp->ist_signature);
}

/*
 * int
 * pfc_ipcclnt_getres_structname(pfc_ipcsess_t *PFC_RESTRICT sess,
 *				 uint32_t index,
 *				 const char **PFC_RESTRICT namepp)
 *	Determine struct name of received additional data at the given index.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to struct name is
 *	set to `*namepp', and zero is returned.
 *
 *	EINVAL is returned if invalid argument is specified.
 *	EPERM is returned if the type of the specified PDU is not STRUCT.
 *	EPROTO is returned if the PDU is broken.
 */
int
pfc_ipcclnt_getres_structname(pfc_ipcsess_t *PFC_RESTRICT sess,
			      uint32_t index, const char **PFC_RESTRICT namepp)
{
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(sess == NULL || namepp == NULL)) {
		return EINVAL;
	}

	IPC_CLSESS_LOCK(sess);

	/* Check session state. */
	state = sess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_RESULT)) {
		err = pfc_ipcmsg_get_struct_name(&sess->icss_msg, index,
						 namepp);
	}
	else {
		err = pfc_ipcclnt_sess_state_error(state);
	}

	IPC_CLSESS_UNLOCK(sess);

	return err;
}

/*
 * uint32_t
 * pfc_ipcclnt_getrescount(pfc_ipcsess_t *sess)
 *	Return the number of PDUs in the result of the last IPC service
 *	request.
 *
 * Remarks:
 *	This function returns zero if an invalid session instance is specified.
 */
uint32_t
pfc_ipcclnt_getrescount(pfc_ipcsess_t *sess)
{
	uint32_t	count;

	if (PFC_EXPECT_TRUE(sess != NULL)) {
		IPC_CLSESS_LOCK(sess);
		count = (sess->icss_state == IPC_SSTATE_RESULT)
			? pfc_ipcmsg_getcount(&sess->icss_msg)
			: 0;
		IPC_CLSESS_UNLOCK(sess);
	}
	else {
		count = 0;
	}

	return count;
}

/*
 * int
 * pfc_ipcclnt_getrescount2(pfc_ipcsess_t *PFC_RESTRICT sess,
 *			    uint32_t *PFC_RESTRICT countp)
 *	Get the number of PDUs in the result of the last IPC service request.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of PDUs is set to the buffer
 *	pointed by `countp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is provided only for internal use.
 */
int
pfc_ipcclnt_getrescount2(pfc_ipcsess_t *PFC_RESTRICT sess,
			 uint32_t *PFC_RESTRICT countp)
{
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(sess == NULL || countp == NULL)) {
		return EINVAL;
	}

	IPC_CLSESS_LOCK(sess);

	state = sess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_RESULT)) {
		*countp = pfc_ipcmsg_getcount(&sess->icss_msg);
		err = 0;
	}
	else {
		err = pfc_ipcclnt_sess_state_error(state);
	}

	IPC_CLSESS_UNLOCK(sess);

	return err;
}

/*
 * int
 * pfc_ipcclnt_getrestype(pfc_ipcsess_t *PFC_RESTRICT sess,
 *			  uint32_t index, pfc_ipctype_t *PFC_RESTRICT typep)
 *	Get type of the PDU specified by `index' in the result of the last
 *	IPC service.
 *
 * Calling/Exit State:
 *	Upon successful completion, PDU type identifier is set to `*typep',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_getrestype(pfc_ipcsess_t *PFC_RESTRICT sess,
		       uint32_t index, pfc_ipctype_t *PFC_RESTRICT typep)
{
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(sess == NULL || typep == NULL)) {
		return EINVAL;
	}

	IPC_CLSESS_LOCK(sess);

	state = sess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_RESULT)) {
		err = pfc_ipcmsg_gettype(&sess->icss_msg, index, typep);
	}
	else {
		err = pfc_ipcclnt_sess_state_error(state);
	}

	IPC_CLSESS_UNLOCK(sess);

	return err;
}

/*
 * pfc_bool_t
 * pfc_ipcclnt_isdisabled(void)
 *	Determine whether the IPC client library is disabled permanently.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the IPC client library is disabled by the
 *	call of pfc_ipcclnt_cancel(PFC_TRUE).
 *	Otherwise PFC_FALSE is returned.
 */
pfc_bool_t
pfc_ipcclnt_isdisabled(void)
{
	pfc_bool_t	disabled;

	IPC_CLIENT_RDLOCK();
	disabled = ipc_disabled;
	IPC_CLIENT_UNLOCK();

	return disabled;
}

/*
 * void
 * pfc_ipcclnt_enable_log(pfc_bool_t enable)
 *	Enable or disable internal logging.
 *
 *	If PFC_TRUE is specified to `enable', internal logging by PFC log
 *	architecture is enabled. It is up to the caller for initialization of
 *	PFC log.
 *
 *	Internal logging is disabled by default.
 */
void
pfc_ipcclnt_enable_log(pfc_bool_t enable)
{
	(void)pfc_atomic_swap_uint8((uint8_t *)&ipcclnt_log_enabled, enable);

	pfc_ipc_enable_log(enable);
}

/*
 * void
 * pfc_ipcclnt_libfini(void)
 *	Call library destructor by force.
 *	This is non-public interface.
 */
void
pfc_ipcclnt_libfini(void)
{
	ipcclnt_libfini();
}

/*
 * static void
 * ipcclnt_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 */
static void
ipcclnt_fork_prepare(void)
{
	IPC_CLIENT_WRLOCK();
	pfc_ipcclnt_event_fork_prepare();
	pfc_ipcclnt_conn_fork_prepare();
	pfc_ipcclnt_canceller_fork_prepare();
}

/*
 * static void
 * ipcclnt_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
static void
ipcclnt_fork_parent(void)
{
	pfc_ipcclnt_canceller_fork_parent();
	pfc_ipcclnt_conn_fork_parent();
	pfc_ipcclnt_event_fork_parent();
	IPC_CLIENT_UNLOCK();
}

/*
 * static void
 * ipcclnt_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
static void
ipcclnt_fork_child(void)
{
	pfc_refptr_t	*pname = ipcclnt_procname;

	if (pname != NULL) {
		/* Release cached process name. */
		pfc_refptr_put(pname);
		ipcclnt_procname = NULL;
	}

	/* Initialize client lock. */
	PFC_ASSERT_INT(pfc_rwlock_init(&ipc_client_lock), 0);

	/*
	 * Disable IPC event subsystem if it is initialized.
	 * All IPC channel locks are initialized here.
	 */
	pfc_ipcclnt_event_fork_child();

	/* Invalidate all IPC connections. */
	pfc_ipcclnt_conn_fork_child();

	/* Discard all session cancellers. */
	pfc_ipcclnt_canceller_fork_child();
}
