#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC core library for C++ programs.
##

include ../defs.mk

SHLIB_NAME	= libpfcxx

CXX_SOURCES	=		\
	event.cc		\
	module.cc		\
	task_queue.cc		\
	thread_pool.cc		\
	thread.cc		\
	timer.cc

# Library internal headers are required.
PFCLIB_INCDIRS	= libpfc libpfc_ipc libpfc_ipcsrv libpfc_ipcclnt
PFCLIB_INCDIRS	+= libpfcxx_ipcclnt

PFC_LIBS	= libpfc libpfc_util libpfc_ipcsrv libpfc_ipcclnt
PFC_LIBS	+= libpfcxx_ipcsrv libpfcxx_ipcclnt

include ../rules.mk
