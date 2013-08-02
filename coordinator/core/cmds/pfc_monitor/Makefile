#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of pfc_monitor command.
##

NO_PTHREAD	= 1

include ../defs.mk

EXEC_NAME	= pfc_monitor
INST_EXEC_NAME	= unc_monitor

C_SOURCES	=		\
	main.c			\
	monitor.c		\
	pidlock.c

# Link copyright.o
LINK_COPYRIGHT	= 1

PFC_LIBS	= libpfc_util libpfc_cmd libpfc_ctrl

# Import system library private header files.
EXTRA_INCDIRS	= $(PFC_LIBS:%=../../libs/%)

# Define PFC daemon monitor configuration file format.
CFDEF_FILES	= pfc_monitor.cfdef

include ../rules.mk
