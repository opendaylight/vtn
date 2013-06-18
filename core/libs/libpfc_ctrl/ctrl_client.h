/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_LIBPFC_CTRL_CTRL_CLIENT_H
#define	_LIBPFC_CTRL_CTRL_CLIENT_H

/*
 * Common definitions for PFC daemon control protocol client.
 */

#include <stdarg.h>
#include <errno.h>
#include <pfc/clock.h>
#include <pfc/iostream.h>
#include <ctrl_proto.h>

PFC_C_BEGIN_DECL

/*
 * Prototype of error logging handler.
 */
typedef void	(*pfc_ctrl_err_t)(const char *fmt, va_list ap);

/*
 * Prototype of debug message handler.
 */
typedef void	(*pfc_ctrl_dbg_t)(int level, const char *fmt, va_list ap);

/*
 * Control protocol command operations.
 */
typedef struct {
	/*
	 * Send optional arguments after protocol command.
	 * User-specified argument passed to pfc_ctrl_client_execute()
	 * will be passed to `arg'.
	 *
	 * Zero must be returned on success, and an error number must be
	 * returned on failure.
	 */
	int	(*send)(cproto_sess_t *sess, pfc_ptr_t arg);

	/*
	 * Receive additional response after protocol command response.
	 * User-specified argument passed to pfc_ctrl_client_execute()
	 * will be passed to `arg'.
	 * Note that this method is never called on error response.
	 *
	 * Zero must be returned on success, and an error number must be
	 * returned on failure.
	 */
	int	(*receive)(cproto_sess_t *sess, pfc_ptr_t arg);
} pfc_ctrl_ops_t;

/*
 * Prototypes.
 */
extern int	pfc_ctrl_client_init(const char *PFC_RESTRICT workdir,
				     pfc_ctrl_err_t errfunc);
extern void	pfc_ctrl_debug(pfc_ctrl_dbg_t dbgfunc);
extern int	pfc_ctrl_client_create_c(cproto_sess_t *PFC_RESTRICT sess,
					 int canceller,
					 const pfc_timespec_t *PFC_RESTRICT
					 timeout,
					 const pfc_iowait_t *PFC_RESTRICT
					 iowait);
extern void	pfc_ctrl_client_destroy(cproto_sess_t *sess);
extern int	pfc_ctrl_client_execute(cproto_sess_t *PFC_RESTRICT sess,
					ctrl_cmdtype_t cmd,
					const pfc_ctrl_ops_t *PFC_RESTRICT ops,
					pfc_ptr_t arg);
extern int	pfc_ctrl_check_permission(void);

/*
 * int
 * pfc_ctrl_client_create(cproto_sess_t *PFC_RESTRICT sess,
 *			  const pfc_timespec_t *PFC_RESTRICT timeout,
 *			  const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Create control protocol client session.
 *
 *	If `timeout' is not NULL, it is used as session timeout.
 *	If NULL, the default value is used, which is specified by
 *	options.ctrl_timeout in pfcd.conf.
 *
 *	`iowait' determines behavior of I/O wait. The specified value is passed
 *	to pfc_iostream_create().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ctrl_client_create(cproto_sess_t *PFC_RESTRICT sess,
		       const pfc_timespec_t *PFC_RESTRICT timeout,
		       const pfc_iowait_t *PFC_RESTRICT iowait)
{
	return pfc_ctrl_client_create_c(sess, -1, timeout, iowait);
}

PFC_C_END_DECL

#endif	/* !_LIBPFC_CTRL_CTRL_CLIENT_H */
