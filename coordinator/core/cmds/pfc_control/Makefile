#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of pfc_control command.
##

NO_PTHREAD	= 1

include ../defs.mk

EXEC_NAME	= pfc_control
INST_EXEC_NAME	= unc_control

C_SOURCES	=		\
	client.c		\
	cmd_checkconf.c		\
	cmd_event.c		\
	cmd_loglevel.c		\
	cmd_modlist.c		\
	cmd_pid.c		\
	cmd_ping.c		\
	cmd_stop.c		\
	ipc.c			\
	main.c			\
	misc.c			\
	modinfo.c

# Sources which requires libpfc internal header.
IMPORT_LIBPFC_SRCS	=	\
	cmd_event.c		\
	cmd_modlist.c		\
	modinfo.c

IMPORT_LIBPFC_OBJS	= $(IMPORT_LIBPFC_SRCS:%.c=$(OBJDIR)/%.o)

# Link copyright.o
LINK_COPYRIGHT	= 1

PFC_LIBS	= libpfc_util libpfc_cmd libpfc_ctrl libpfc_ipcclnt

# Import system library private header files.
EXTRA_INCDIRS	= $(PFC_LIBS:%=../../libs/%)

# Install symbolic link to alternative command path.
ALTLINK_INSTALL	:= 1

include ../rules.mk

$(IMPORT_LIBPFC_OBJS):		EXTRA_INCDIRS += ../../libs/libpfc
