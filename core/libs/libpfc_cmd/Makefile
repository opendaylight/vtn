#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC command library.
##

include ../defs.mk

SHLIB_NAME	= libpfc_cmd

C_SOURCES	=		\
	cmdopt_help.c		\
	cmdopt_parser.c		\
	conf.c			\
	ctrl_proto.c		\
	misc.c			\
	modcache.c		\
	pidfile.c

PFC_LIBS	= libpfc_util

include ../rules.mk
