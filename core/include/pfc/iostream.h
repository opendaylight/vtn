/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_IOSTREAM_H
#define	_PFC_IOSTREAM_H

/*
 * Definitions for PFC I/O stream.
 */

#include <errno.h>
#include <signal.h>
#include <pfc/base.h>
#include <pfc/clock.h>

PFC_C_BEGIN_DECL

/*
 * Buffered input/output stream using non-blocking file descriptor.
 */
struct __pfc_iostream;
typedef struct __pfc_iostream	*pfc_iostream_t;

/*
 * EINTR callback.
 */
typedef pfc_bool_t	(*pfc_iointr_cb_t)(pfc_ptr_t arg);

/*
 * Maximum I/O size.
 */
#define	PFC_IOSTREAM_MAXSIZE	((size_t)SSIZE_MAX)

/*
 * Flags for pfc_iostream_shutdown().
 */
#define	PFC_IOSTREAM_SHUT_RD	PFC_CONST_U(0x1)	/* read shutdown */
#define	PFC_IOSTREAM_SHUT_WR	PFC_CONST_U(0x2)	/* write shutdown */
#define	PFC_IOSTREAM_SHUT_RDWR	(PFC_IOSTREAM_SHUT_RD | PFC_IOSTREAM_SHUT_WR)

/*
 * I/O wait parameters.
 */
typedef struct {
	pfc_iointr_cb_t		iw_intrfunc;	/* EINTR callback */
	pfc_ptr_t		iw_intrarg;	/* argument for iw_intrfunc */
	sigset_t		*iw_sigmask;	/* signal mask */
} pfc_iowait_t;

/*
 * Prototypes.
 */
extern int	pfc_iostream_create(pfc_iostream_t *PFC_RESTRICT streamp,
				    int fd, uint32_t insize, uint32_t outsize,
				    const pfc_iowait_t *PFC_RESTRICT iowait);
extern int	pfc_iostream_destroy(pfc_iostream_t stream);
extern int	pfc_iostream_read(pfc_iostream_t PFC_RESTRICT stream,
				  void *PFC_RESTRICT buf,
				  size_t *PFC_RESTRICT sizep,
				  const pfc_timespec_t *PFC_RESTRICT timeout);
extern int	pfc_iostream_read_abs(pfc_iostream_t PFC_RESTRICT stream,
				      void *PFC_RESTRICT buf,
				      size_t *PFC_RESTRICT sizep,
				      const pfc_timespec_t *PFC_RESTRICT
				      abstime);
extern int	pfc_iostream_write(pfc_iostream_t PFC_RESTRICT stream,
				   const void *PFC_RESTRICT buf,
				   size_t *PFC_RESTRICT sizep,
				   const pfc_timespec_t *PFC_RESTRICT timeout);
extern	int	pfc_iostream_write_abs(pfc_iostream_t PFC_RESTRICT stream,
				       const void *PFC_RESTRICT buf,
				       size_t *PFC_RESTRICT sizep,
				       const pfc_timespec_t *PFC_RESTRICT
				       abstime);
extern int	pfc_iostream_flush(pfc_iostream_t PFC_RESTRICT stream,
				   const pfc_timespec_t *PFC_RESTRICT timeout);
extern int	pfc_iostream_flush_abs(pfc_iostream_t PFC_RESTRICT stream,
				       const pfc_timespec_t *PFC_RESTRICT
				       abstime);
extern int	pfc_iostream_shutdown(pfc_iostream_t stream, uint32_t how);
extern int	pfc_iostream_setcanceller(pfc_iostream_t stream, int fd);
extern int	pfc_iostream_getfd(pfc_iostream_t stream);

PFC_C_END_DECL

#endif	/* !_PFC_IOSTREAM_H */
