/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_SYNCBLOCK_H
#define	_PFC_SYNCBLOCK_H

/*
 * Definitions to implement synchronized block.
 * This header file is assumed to be used by PFC system library.
 * Common user should not use contents in this file.
 */

#include <pfc/base.h>
#include <pfc/debug.h>
#include <pthread.h>

#ifdef	__cplusplus

/*
 * Current glibc's pthread_cleanup_push() / pthread_cleanup_pop()
 * implementation allows C++ to return in cleanup section.
 */
#define	__PFC_SYNCBLOCK_BEGIN()
#define	__PFC_SYNCBLOCK_END()

#else	/* !__cplusplus */

/*
 * Define assertion to detect unexpected return in synchronized block.
 */
#define	__PFC_SYNCBLOCK_BEGIN()		PFC_DISABLE_RETURN_BEGIN()
#define	__PFC_SYNCBLOCK_END()		PFC_DISABLE_RETURN_END()

#endif	/* __cplusplus */

PFC_C_BEGIN_DECL

/*
 * Synchronized block frame.
 */
struct __pfc_syncblock;
typedef struct __pfc_syncblock	__pfc_syncblock_t;

struct __pfc_syncblock {
	pfc_cptr_t		psb_target;	/* target object */
	uint32_t		psb_depth;	/* lock depth */
	uint32_t		psb_flags;	/* flags */
	__pfc_syncblock_t	*psb_next;	/* next link */
};

/*
 * Flags for synchronized block frame.
 */
#define	PFC_SYNCBLOCK_WRITE	PFC_CONST_U(0x1)	/* write mode */

/*
 * Start synchronized block.
 * __pfc_wsync_block_begin() starts synchronized block as writer mode,
 * __pfc_rsync_block_begin() starts as reader.
 *
 * __pfc_wsync_block_begin(), __pfc_rsync_block_begin() and
 * __pfc_sync_block_end() are implemented as macros and must always be
 * used in matching pairs at the same block.
 */

#define	__pfc_syncblock_begin_common(target, beginfunc, endfunc, flags)	\
	do {								\
		__pfc_syncblock_t __frame = {				\
			(pfc_cptr_t)(target), 1, (flags), NULL,		\
		};							\
		beginfunc(&__frame);					\
		pthread_cleanup_push(endfunc, &__frame);		\
		__PFC_SYNCBLOCK_BEGIN()

#define	__pfc_sync_block_end()					\
		__PFC_SYNCBLOCK_END();				\
		pthread_cleanup_pop(1);				\
	} while (0)

#define	__pfc_wsync_block_begin(target, beginfunc, endfunc)		\
	__pfc_syncblock_begin_common(target, beginfunc, endfunc,	\
				     PFC_SYNCBLOCK_WRITE)

#define	__pfc_rsync_block_begin(target, beginfunc, endfunc)		\
	__pfc_syncblock_begin_common(target, beginfunc, endfunc, 0)

PFC_C_END_DECL

#endif	/* !_PFC_SYNCBLOCK_H */
