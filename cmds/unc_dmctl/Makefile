#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of unc_dmctl command.
##

NO_PTHREAD	= 1

include ../defs.mk

C_SOURCES	=		\
	cmd_clevent.c		\
	cmd_clstate.c		\
	cmd_list.c		\
	cmd_status.c		\
	dmconf.c		\
	ipc.c			\
	main.c			\
	uniqstr.c

# Link copyright.o
LINK_COPYRIGHT	= 1

UNC_LIBS	= libpfc_util libpfc_cmd libpfc_ipcclnt libunc_launcher

# Import system library private header files.
PFCLIB_INCDIRS	= libpfc_util libpfc_cmd
EXTRA_INCDIRS	+= $(PFCLIB_INCDIRS:%=$(CORE_SRCROOT)/libs/%)

include ../rules.mk
