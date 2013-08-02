/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_IPC_SERVER_H
#define	_PFC_IPC_SERVER_H

/*
 * Common definition for PFC Inter-Process Communication framework server.
 */

#include <pfc/base.h>
#include <pfc/ipc.h>
#include <pfc/clock.h>
#include <netinet/in.h>

PFC_C_BEGIN_DECL

/*
 * IPC event delivery descriptor.
 */
typedef uint32_t	pfc_ipcevdesc_t;

/*
 * Invalid IPC event delivery descriptor.
 */
#define	PFC_IPCEVDESC_INVALID		PFC_CONST_U(0)

/*
 * Callback function used by IPC service handler.
 */
typedef struct {
	/*
	 * void
	 * isc_callback(pfc_ipcsrv_t *srv, pfc_ipcsrvcb_type_t type,
	 *		pfc_ptr_t arg)
	 *	Callback function.
	 *
	 *	`srv' is a pointer to IPC server session connected to an
	 *	IPC client.
	 *	`type' is a type of IPC server session callback.
	 *	`arg' is an arbitrary pointer specified by user.
	 */
	void	(*isc_callback)(pfc_ipcsrv_t *srv, pfc_ipcsrvcb_type_t type,
				pfc_ptr_t arg);

	/*
	 * void
	 * isc_argdtor(pfc_ptr_t arg)
	 *	Destructor of callback argument.
	 *
	 *	If a non-NULL pointer is set, it is called when a callback
	 *	function is removed from the IPC server subsystem.
	 *
	 *	`arg' is an arbitrary pointer specified by user.
	 */
	void	(*isc_argdtor)(pfc_ptr_t arg);
} pfc_ipcsrvcb_t;

#ifndef	PFC_MODULE_BUILD

/*
 * Prototype of IPC service handler.
 * `srv' is a pointer to IPC server session.
 * `service' is an identifier used to distinguish IPC service.
 * `arg' is an arbitrary pointer value specified to pfc_ipcsrv_add_handler().
 */
typedef pfc_ipcresp_t	(*pfc_ipchdlr_t)(pfc_ipcsrv_t *srv,
					 pfc_ipcid_t service,
					 pfc_ptr_t arg);

/*
 * Prototype of IPC service destructor, which will be called when the IPC
 * service handler is removed.
 * `name' is name of the IPC service handler, and `arg' is an arbitrary pointer
 * passed to pfc_ipcsrv_add_handler_dtor().
 */
typedef void		(*pfc_ipcsvdtor_t)(const char *name, pfc_ptr_t arg);

#endif	/* !PFC_MODULE_BUILD */

/*
 * PFC_IPCSRV_OUTPUT_STRUCT(srv, stname, data)
 *	Append a struct data to the IPC output stream for the client.
 *
 *	`srv' must be a pointer to server session instance.
 *	`stname' must be a literal which represents struct name defined in
 *	IPC struct template file.
 *	`data' must be a non-NULL pointer to struct data to be appended.
 *	It must be a pointer to `struct stname' data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ENODEV is returned if IPC struct information associated with the
 *	given struct name is not found or invalid.
 *	EFAULT is returned if `data' is not aligned on required boundary.
 *	Otherwise error number which indicates the cause of error is returned.
 */
#define	PFC_IPCSRV_OUTPUT_STRUCT(srv, stname, data)			\
	__pfc_ipcsrv_output_struct((srv), (const uint8_t *)(data),	\
				   __PFC_IPC_STRUCT_SIZE(stname),	\
				   __PFC_IPC_STRUCT_STRINGIFY(stname),	\
				   __PFC_IPC_STRUCT_SIGNATURE(stname))

/*
 * PFC_IPCSRV_GETARG_STRUCT(srv, index, stname, datap)
 *	Fetch a struct data at the given index sent by the client.
 *
 *	`srv' must be a pointer to server session instance.
 *	`index' is a PDU index which specifies the target PDU.
 *	`stname' must be a literal which represents struct name defined in
 *	IPC struct template file.
 *	`datap' must be a non-NULL pointer to buffer to store struct data.
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
 */
#define	PFC_IPCSRV_GETARG_STRUCT(srv, index, stname, datap)		\
	__pfc_ipcsrv_getarg_struct((srv), (index), (uint8_t *)(datap),	\
				   __PFC_IPC_STRUCT_SIZE(stname),	\
				   __PFC_IPC_STRUCT_STRINGIFY(stname),	\
				   __PFC_IPC_STRUCT_SIGNATURE(stname))

/*
 * Prototypes.
 */
extern int	pfc_ipcsrv_output_int8(pfc_ipcsrv_t *srv, int8_t data);
extern int	pfc_ipcsrv_output_uint8(pfc_ipcsrv_t *srv, uint8_t data);
extern int	pfc_ipcsrv_output_int16(pfc_ipcsrv_t *srv, int16_t data);
extern int	pfc_ipcsrv_output_uint16(pfc_ipcsrv_t *srv, uint16_t data);
extern int	pfc_ipcsrv_output_int32(pfc_ipcsrv_t *srv, int32_t data);
extern int	pfc_ipcsrv_output_uint32(pfc_ipcsrv_t *srv, uint32_t data);
extern int	pfc_ipcsrv_output_int64(pfc_ipcsrv_t *srv, int64_t data);
extern int	pfc_ipcsrv_output_uint64(pfc_ipcsrv_t *srv, uint64_t data);
extern int	pfc_ipcsrv_output_float(pfc_ipcsrv_t *srv, float data);
extern int	pfc_ipcsrv_output_double(pfc_ipcsrv_t *srv, double data);
extern int	pfc_ipcsrv_output_ipv4(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       struct in_addr *PFC_RESTRICT data);
extern int	pfc_ipcsrv_output_ipv6(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       struct in6_addr *PFC_RESTRICT data);
extern int	pfc_ipcsrv_output_string(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 const char *PFC_RESTRICT data);
extern int	pfc_ipcsrv_output_binary(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 const uint8_t *PFC_RESTRICT data,
					 uint32_t length);
extern int	pfc_ipcsrv_output_null(pfc_ipcsrv_t *srv);
extern int	pfc_ipcsrv_output_stdef(pfc_ipcsrv_t *PFC_RESTRICT srv,
					const pfc_ipcstdef_t *PFC_RESTRICT defp,
					pfc_cptr_t data);

extern int	pfc_ipcsrv_getarg_int8(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       uint32_t index,
				       int8_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_uint8(pfc_ipcsrv_t *PFC_RESTRICT srv,
					uint32_t index,
					uint8_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_int16(pfc_ipcsrv_t *PFC_RESTRICT srv,
					uint32_t index,
					int16_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_uint16(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 uint32_t index,
					 uint16_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_int32(pfc_ipcsrv_t *PFC_RESTRICT srv,
					uint32_t index,
					int32_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_uint32(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 uint32_t index,
					 uint32_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_int64(pfc_ipcsrv_t *PFC_RESTRICT srv,
					uint32_t index,
					int64_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_uint64(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 uint32_t index,
					 uint64_t *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_float(pfc_ipcsrv_t *PFC_RESTRICT srv,
					uint32_t index,
					float *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_double(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 uint32_t index,
					 double *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_ipv4(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       uint32_t index,
				       struct in_addr *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_ipv6(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       uint32_t index,
				       struct in6_addr *PFC_RESTRICT datap);
extern int	pfc_ipcsrv_getarg_string(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 uint32_t index,
					 const char **PFC_RESTRICT datapp);
extern int	pfc_ipcsrv_getarg_binary(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 uint32_t index,
					 const uint8_t **PFC_RESTRICT datapp,
					 uint32_t *PFC_RESTRICT lengthp);
extern int	pfc_ipcsrv_getarg_stdef(pfc_ipcsrv_t *PFC_RESTRICT srv,
					uint32_t index,
					const pfc_ipcstdef_t *PFC_RESTRICT defp,
					pfc_ptr_t datap);

extern uint32_t	pfc_ipcsrv_getargcount(pfc_ipcsrv_t *srv);
extern int	pfc_ipcsrv_getargtype(pfc_ipcsrv_t *PFC_RESTRICT srv,
				      uint32_t index,
				      pfc_ipctype_t *PFC_RESTRICT typep);
extern int	pfc_ipcsrv_settimeout(pfc_ipcsrv_t *PFC_RESTRICT srv,
				      const pfc_timespec_t *PFC_RESTRICT
				      timeout);
extern int	pfc_ipcsrv_getarg_structname(pfc_ipcsrv_t *PFC_RESTRICT srv,
					     uint32_t index,
					     const char **PFC_RESTRICT namepp);

extern int	pfc_ipcsrv_getcladdr(pfc_ipcsrv_t *PFC_RESTRICT srv,
				     pfc_ipccladdr_t *PFC_RESTRICT claddr);

extern int	pfc_ipcsrv_setcallback(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       pfc_ipcsrvcb_type_t type,
				       const pfc_ipcsrvcb_t *PFC_RESTRICT
				       callback, pfc_ptr_t arg);
extern void	pfc_ipcsrv_unsetcallback(pfc_ipcsrv_t *srv,
					 pfc_ipcsrvcb_type_t type);
extern void	pfc_ipcsrv_clearcallbacks(pfc_ipcsrv_t *srv);

extern int	pfc_ipcsrv_event_destroy(pfc_ipcsrv_t *srv);
extern int	pfc_ipcsrv_event_post(pfc_ipcsrv_t *srv);
extern int	pfc_ipcsrv_event_postto(pfc_ipcsrv_t *PFC_RESTRICT srv,
					const pfc_ipccladdr_t *PFC_RESTRICT
					claddr);

extern int	pfc_ipcsrv_evdesc_create(pfc_ipcevdesc_t *PFC_RESTRICT descp,
					 pfc_ipcsrv_t *PFC_RESTRICT srv);
extern int	pfc_ipcsrv_evdesc_destroy(pfc_ipcevdesc_t desc);
extern int	pfc_ipcsrv_evdesc_wait(pfc_ipcevdesc_t desc,
				       const pfc_timespec_t *timeout);
extern int	pfc_ipcsrv_evdesc_wait_abs(pfc_ipcevdesc_t desc,
					   const pfc_timespec_t *abstime);

#ifndef	PFC_MODULE_BUILD

/*
 * IPC server operations.
 */
typedef struct {
	/*
	 * int
	 * isvops_thread_create(void *(*func)(void *), void *arg)
	 *	Create a new thread for an IPC server session.
	 *
	 *	libpfc_ipcsrv creates a dedicated thread per IPC server
	 *	session. This function is used to create a thread in own way.
	 *	If omitted, pthread_create() with default attributes is used.
	 *
	 *	`func' is a pointer to thread start routine, and `arg' is an
	 *	argument to be passed to `func' call. `func' must be called on
	 *	a new session thread.
	 *
	 * Calling/Exit State:
	 *	Zero must be returned on success.
	 *	An error number defined in errno.h must be returned on failure.
	 *
	 * Remarks:
	 *	This function must create a detached thread.
	 *	There is no way to join with a server session thread.
	 */
	int	(*isvops_thread_create)(void *(*func)(void *), void *arg);
} pfc_ipcsrvops_t;

extern int	pfc_ipcsrv_init(const char *channel,
				const pfc_ipcsrvops_t *ops);
extern int	pfc_ipcsrv_main(void);
extern int	pfc_ipcsrv_fini(void);
extern void	pfc_ipcsrv_enable_log(pfc_bool_t enable);

extern int	pfc_ipcsrv_add_handler(const char *name, uint32_t nservices,
				       pfc_ipchdlr_t handler, pfc_ptr_t arg);
extern int	pfc_ipcsrv_add_handler_dtor(const char *name,
					    uint32_t nservices,
					    pfc_ipchdlr_t handler,
					    pfc_ptr_t arg,
					    pfc_ipcsvdtor_t dtor);
extern void	pfc_ipcsrv_remove_handler(const char *name);

extern int	pfc_ipcsrv_event_create(pfc_ipcsrv_t **PFC_RESTRICT srvp,
					const char *PFC_RESTRICT name,
					pfc_ipcevtype_t type);

#endif	/* !PFC_MODULE_BUILD */

/*
 * Below are non-public interfaces.
 * Never call them directly.
 */
extern int	__pfc_ipcsrv_output_struct(pfc_ipcsrv_t *PFC_RESTRICT srv,
					   const uint8_t *PFC_RESTRICT data,
					   uint32_t length,
					   const char *PFC_RESTRICT stname,
					   const char *PFC_RESTRICT sig);
extern int	__pfc_ipcsrv_getarg_struct(pfc_ipcsrv_t *PFC_RESTRICT srv,
					   uint32_t index,
					   uint8_t *PFC_RESTRICT datap,
					   uint32_t length,
					   const char *PFC_RESTRICT stname,
					   const char *PFC_RESTRICT sig);

PFC_C_END_DECL

#endif	/* !_PFC_IPC_SERVER_H */
