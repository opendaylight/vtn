/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_IOSTREAM_IMPL_H
#define	_PFC_LIBPFC_UTIL_IOSTREAM_IMPL_H

/*
 * Internal definitions for I/O stream.
 */

#include <sys/types.h>
#include <pfc/base.h>
#include <pfc/socket.h>
#include <pfc/iostream.h>

PFC_C_BEGIN_DECL

/*
 * Pseudo buffer size which indicates that the size must be retained.
 */
#define	PFC_IOSSZ_RETAIN	UINT32_MAX

/*
 * Non-public APIs.
 */
extern void	__pfc_iostream_log_enable(void);
extern void	__pfc_iostream_dispose(pfc_iostream_t stream);
extern int	__pfc_iostream_setfd(pfc_iostream_t stream, int newfd);
extern int	__pfc_iostream_resize(pfc_iostream_t stream,
				      uint32_t insize, uint32_t outsize);

extern int	__pfc_iostream_sendcred(pfc_iostream_t PFC_RESTRICT stream,
					const void *PFC_RESTRICT buf,
					size_t *PFC_RESTRICT sizep,
					pfc_cucred_t *PFC_RESTRICT credp,
					const pfc_timespec_t *PFC_RESTRICT
					timeout);
extern int	__pfc_iostream_sendcred_abs(pfc_iostream_t PFC_RESTRICT stream,
					    const void *PFC_RESTRICT buf,
					    size_t *PFC_RESTRICT sizep,
					    pfc_cucred_t *PFC_RESTRICT credp,
					    const pfc_timespec_t *PFC_RESTRICT
					    abstime);
extern int	__pfc_iostream_recvcred(pfc_iostream_t PFC_RESTRICT stream,
					void *PFC_RESTRICT buf,
					size_t *PFC_RESTRICT sizep,
					pfc_ucred_t *PFC_RESTRICT credp,
					const pfc_timespec_t *PFC_RESTRICT
					timeout);
extern int	__pfc_iostream_recvcred_abs(pfc_iostream_t PFC_RESTRICT stream,
					    void *PFC_RESTRICT buf,
					    size_t *PFC_RESTRICT sizep,
					    pfc_ucred_t *PFC_RESTRICT credp,
					    const pfc_timespec_t *PFC_RESTRICT
					    abstime);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_IOSTREAM_IMPL_H */
