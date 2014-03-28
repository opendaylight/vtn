#
# Copyright (c) 2011-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC ipc library.
##

include ../defs.mk

SHLIB_NAME	= libpfc_ipc

C_SOURCES	=		\
	conf.c			\
	event.c			\
	pdu.c			\
	message.c		\
	misc.c			\
	stream.c		\
	struct.c

PFC_LIBS	= libpfc_util

# Import system library private header files.
PFCLIB_INCDIRS	= libpfc_util

EXTRA_CPPFLAGS	= -D_PFC_LIBPFC_IPC_BUILD

# Define log identifier.
USE_LOG_IDENT	= 1

include ../rules.mk
