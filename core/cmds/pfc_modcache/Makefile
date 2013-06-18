#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of pfc_modcache command.
##

NO_PTHREAD	= 1

include ../defs.mk

EXEC_NAME	= pfc_modcache
INST_EXEC_NAME	= unc_modcache

C_SOURCES	=		\
	dump.c			\
	main.c			\
	strtable.c		\
	update.c

# Link copyright.o
LINK_COPYRIGHT	= 1

# Import system library private header files.
PFCLIB_INCDIRS	= libpfc_util libpfc_cmd
EXTRA_INCDIRS	= $(PFCLIB_INCDIRS:%=../../libs/%)

PFC_LIBS	= libpfc_util libpfc_cmd

include ../rules.mk
