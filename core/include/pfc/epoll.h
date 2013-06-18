/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_EPOLL_H
#define	_PFC_EPOLL_H

/*
 * Low level utilities for I/O event notification facility. (Linux specific)
 */

#include <pfc/base.h>

#ifdef	PFC_HAVE_EPOLL

#include <sys/epoll.h>

PFC_C_BEGIN_DECL

struct pfc_ephdlr;
typedef struct pfc_ephdlr	pfc_ephdlr_t;

/*
 * Prototype of I/O event handler function.
 */
typedef int	(*pfc_epfunc_t)(pfc_ephdlr_t *evp, uint32_t events, void *arg);

/*
 * Interface of I/O event handler.
 *
 * Usage:
 *
 * 1. Embed pfc_ephdlr_t into data structure which manages the target file
 *    descriptor.
 *
 *	struct fd_data {
 *		int		f_fd;
 *		void		*f_data;
 *		pfc_ephdlr_t	f_event;
 *	};
 *
 * 2. Initialize pfc_ephdlr_t with a pointer to function which handles
 *    events on the target file descriptor.
 *
 *	struct fd_data	*fdp = (struct fd_data *)malloc(sizeof(fd_data));
 *
 *	fdp->f_fd = target_fd;
 *	fdp->f_data = your_data;
 *	pfc_epoll_handler_init(&fdp->f_event, event_handler);
 *
 * 3. Implement event handler function.
 *    PFC_CAST_CONTAINER() is useful to obtain a pointer to data structure
 *    associated with the target file descriptor.
 *
 *	int
 *	event_handler(pfc_ephdlr_t *evp, uint32_t events, void *arg)
 *	{
 *		struct fd_data	*fdp =
 *			PFC_CAST_CONTAINER(evp, struct fd_data, f_event);
 *
 *		...
 *		return 0;
 *	}
 *
 * 4. Create an event poll instance, and register the target file descriptor.
 *    Note that all target file descriptors must be registered by the call
 *    of pfc_epoll_ctl().
 *
 *	int		epfd = pfc_epoll_create();
 *	struct fd_data	*fdp = your_fd_data_pointer;
 *
 *	pfc_epoll_ctl(epfd, EPOLL_CTL_ADD, target_fd, EPOLLIN, &fdp->f_event);
 *
 * 5. Wait for an I/O event on an epoll instance, and dispatch event to
 *    I/O event handler.
 *
 *	struct epoll_event	events[8];
 *	int	nfds;
 *
 *	nfds = epoll_wait(epfd, events, 8, -1);
 *	pfc_epoll_dispatch(events, 8, your_data);
 */
struct pfc_ephdlr {
	/*
	 * int
	 * pev_handler(pfc_ephdlr_t *evp, uint32_t events, void *arg)
	 *	I/O event handler.
	 *
	 *	This function is called when an event on the target file
	 *	descriptor associated with `evp' is detected.
	 *
	 *	A pointer to pfc_ephdlr_t itself is passed to `evp',
	 *	and an I/O event bitmask (EPOLLXXX) which represents detected
	 *	events is passed to `events'. `arg' is an arbitrary pointer
	 *	passed to pfc_epoll_dispatch().
	 *
	 * Calling/Exit State:
	 *	Zero must be returned on success.
	 *	Otherwise error number which indicates the cause of error
	 *	is returned.
	 */
	pfc_epfunc_t	pev_handler;
};

/*
 * int
 * pfc_epoll_create(void)
 *	Create an event poll instance.
 *
 *	The close-on-exec bit is always set to the file descriptor associated
 *	with the new event poll instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a valid file descriptor associated with
 *	an event poll instance is returned.
 *	Otherwise, error number which indicates the cause of error is set
 *	to errno, and then -1 is returned.
 */
#if	defined(PFC_HAVE_EPOLL_CLOEXEC) &&		\
	!defined(__PFC_UTIL_DONT_DEFINE_EPOLL_CREATE)
#define	pfc_epoll_create()		epoll_create1(EPOLL_CLOEXEC)
#else	/* !(PFC_HAVE_EPOLL_CLOEXEC && !__PFC_UTIL_DONT_DEFINE_EPOLL_CREATE) */
extern int	pfc_epoll_create(void);
#endif	/* PFC_HAVE_EPOLL_CLOEXEC && !__PFC_UTIL_DONT_DEFINE_EPOLL_CREATE */

/*
 * static int void PFC_FATTR_ALWAYS_INLINE
 * pfc_epoll_ctl(int epfd, int op, int fd, uint32_t events, pfc_ephdlr_t *evp)
 *	Control interface for an I/O event poll descriptor associated with
 *	`epfd'.
 *
 *	This function is a wrapper interface for epoll_ctl(2).
 *
 *	`op' determines behavior of this function.
 *
 *	EPOLL_CTL_ADD
 *	    Register the target file descriptor specified by `fd' to the
 *	    event poll instance. `event' is a bitmask of I/O event bits to
 *	    watch. `evp' is an I/O event handler interface to run when an event
 *	    is detected on the target file descriptor.
 *
 *	EPOLL_CTL_DEL
 *	    Remove the target file descriptor specified by `fd' from the
 *	    event poll instance. `events' and `evp' are simply ignored.
 *
 *	EPOLL_CTL_MOD
 *	    Change the event associated with the target file descriptor.
 *	    `event' is a bitmask of I/O event bits to watch. `evp' is an I/O
 *	    event handler interface to run when an event is detected on the
 *	    target file descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is set
 *	to errno, and then -1 is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_epoll_ctl(int epfd, int op, int fd, uint32_t events, pfc_ephdlr_t *evp)
{
	struct epoll_event	ev;

#ifndef	PFC_LP64
	/* Suppress unwanted valgrind warning. */
	ev.data.u64 = 0;
#endif	/* !PFC_LP64 */
	ev.data.ptr = evp;
	ev.events = events;

	return epoll_ctl(epfd, op, fd, &ev);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_epoll_handler_init(pfc_ephdlr_t *evp, pfc_epfunc_t handler)
 * 	Initialize I/O event handler interface specified by `evp'.
 *
 *	`func' must be a pointer to a function which handles I/O events on
 *	a file descriptor associated with `evp'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_epoll_handler_init(pfc_ephdlr_t *evp, pfc_epfunc_t handler)
{
	evp->pev_handler = handler;
}

/*
 * Static initializer of pfc_ephdlr_t.
 */
#define	PFC_EPOLL_HANDLER_INITIALIZER(handler)	{handler}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_epoll_dispatch(struct epoll_event *events, int nevents, void *arg)
 *	Dispatch I/O events detected by the epoll instance.
 *
 *	This function must be returned just after successful return of
 *	epoll_wait(2) or epoll_pwait(2). `events' is an event array passed
 *	to epoll_wait(2) or epoll_pwait(2), and `nevents' must be a return
 *	value of epoll_wait(2) or epoll_pwait(2).
 *
 *	`arg' is an arbitrary pointer passed to I/O event handler.
 *
 * Calling/Exit State:
 *	Zero is returned if all I/O event handlers are successfully called.
 *	If at least one I/O event handler returns non-zero value, the value is
 *	returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_epoll_dispatch(struct epoll_event *events, int nevents, void *arg)
{
	struct epoll_event	*ep;
	struct epoll_event	*elimit = events + nevents;

	for (ep = events; ep < elimit; ep++) {
		pfc_ephdlr_t	*evp = (pfc_ephdlr_t *)ep->data.ptr;
		int		err;

		err = evp->pev_handler(evp, ep->events, arg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

PFC_C_END_DECL

#endif	/* PFC_HAVE_EPOLL */

#endif	/* !_PFC_EPOLL_H */
