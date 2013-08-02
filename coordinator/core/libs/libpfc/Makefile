#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC core library.
##

# Use PFC daemon maintenance API.
USE_PFCD_MAINT	= 1

include ../defs.mk

SHLIB_NAME	= libpfc

C_SOURCES	=		\
	event.c			\
	eventsys.c		\
	libpfc.c		\
	module.c		\
	property.c		\
	ptimer.c		\
	taskq.c			\
	thread.c		\
	thread_exit.c		\
	timer.c			\
	tpool.c

# thread_exit.c contains pthread_exit() call which throws internal exception
# for thread cancellation on C++. So it must be compiled with C++ exception
# support.
EXP_C_SOURCES	= thread_exit.c

PFC_LIBS	= libpfc_util libpfc_ipc libpfc_ipcsrv libpfc_ipcclnt
LDLIBS		+= -ldl

# Include path for internal headers of IPC libraries.
IPC_LIBDIRS	= libpfc_ipc libpfc_ipcsrv libpfc_ipcclnt
IPC_INCDIRS	= $(IPC_LIBDIRS:%=../%)

EXTRA_CPPFLAGS	= -D_PFC_LIBPFC_BUILD

include ../rules.mk

# module.c requires libpfc_cmd and IPC library internal header.
$(OBJDIR)/module.o:	EXTRA_INCDIRS = ../libpfc_cmd $(IPC_INCDIRS)

# libpfc.c requires libpfc_util and IPC library internal header.
$(OBJDIR)/libpfc.o:	EXTRA_INCDIRS = ../libpfc_util $(IPC_INCDIRS)
