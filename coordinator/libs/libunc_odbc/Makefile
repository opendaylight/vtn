#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of libunc_odbc library.
## libunc_odbc serializes unixODBC library call in order to avoid bug of
## PostgreSQL ODBC driver.
##

UNC_ODBC_NOWRAPPER	:= 1

include ../defs.mk

C_SOURCES	= wrapper.c
UNC_LIBS	= libpfc_util
USE_ODBC	= 1

include ../rules.mk
