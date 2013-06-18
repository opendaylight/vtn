#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of libunc_launcher library.
##

include ../defs.mk

C_SOURCES	=		\
	conf.c

# Define configuration file format for the launcher module and daemon process.
# daemon
CFDEF_FILES	= launcher.cfdef daemon.cfdef

UNC_LIBS	= libpfc_util

# Define log identifier.
USE_LOG_IDENT	= 1

include ../rules.mk
