#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of C++ library for IPC server.
##

include ../defs.mk

SHLIB_NAME	= libpfcxx_ipcsrv

CXX_SOURCES	=		\
	event.cc		\
	session.cc

PFCLIB_INCDIRS	= libpfc_ipc libpfc_ipcsrv
EXTRA_CPPFLAGS	= -D_PFC_LIBPFCXX_IPCSRV_BUILD

PFC_LIBS	= libpfc_ipcsrv libpfc_util

include ../rules.mk
