#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC IPC client library.
##

include ../defs.mk

SHLIB_NAME	= libpfc_ipcclnt

C_SOURCES	=		\
	cancel.c		\
	channel.c		\
	connection.c		\
	evattr.c		\
	event.c			\
	eventsys.c		\
	hostset.c		\
	misc.c			\
	server.c		\
	session.c

PFC_LIBS	= libpfc_util libpfc_ipc

# Import system library private header files.
PFCLIB_INCDIRS	= libpfc_util libpfc_ipc

IPCSRV_INCDIR	= ../libpfc_ipcsrv

EXTRA_CPPFLAGS	= -D_PFC_LIBPFC_IPCCLNT_BUILD

# Define log identifier.
USE_LOG_IDENT	= 1

include ../rules.mk

# server.c requires internal headers for libpfc_ipcsrv.
$(OBJDIR)/server.o:	EXTRA_INCDIRS += $(IPCSRV_INCDIR)
