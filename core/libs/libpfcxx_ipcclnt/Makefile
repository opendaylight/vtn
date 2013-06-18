#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of C++ library for IPC client.
##

include ../defs.mk

SHLIB_NAME	= libpfcxx_ipcclnt

CXX_SOURCES	=		\
	event.cc

PFCLIB_INCDIRS	= libpfc_ipc libpfc_ipcclnt
EXTRA_CPPFLAGS	= -D_PFC_LIBPFCXX_IPCCLNT_BUILD

PFC_LIBS	= libpfc_util libpfc_ipcclnt

include ../rules.mk
