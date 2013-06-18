#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC IPC server library.
##

include ../defs.mk

SHLIB_NAME	= libpfc_ipcsrv

C_SOURCES	=		\
	channel.c		\
	event.c			\
	handler.c		\
	misc.c			\
	session.c

PFC_LIBS	= libpfc_util libpfc_ipc

# Import system library private header files.
PFCLIB_INCDIRS	= libpfc_util libpfc_ipc

EXTRA_CPPFLAGS	= -D_PFC_LIBPFC_IPCSRV_BUILD

# Define log identifier.
USE_LOG_IDENT	= 1

include ../rules.mk
