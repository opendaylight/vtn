#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC ipc library.
##

include ../defs.mk

SHLIB_NAME	= libpfc_ipc

C_SOURCES	=		\
	conf.c			\
	event.c			\
	pdu.c			\
	message.c		\
	misc.c			\
	stream.c		\
	struct.c

PFC_LIBS	= libpfc_util

# Import system library private header files.
PFCLIB_INCDIRS	= libpfc_util

EXTRA_CPPFLAGS	= -D_PFC_LIBPFC_IPC_BUILD

# Define log identifier.
USE_LOG_IDENT	= 1

# Create system directories on "make install".
INST_WORKDIR	= $(DESTDIR)$(INST_IPCWORKDIR)
INST_CHANNELDIR	= $(INST_WORKDIR)/.channel

PERM_CHANNELDIR	= 01777
INST_IPCDIRS	= $(INST_WORKDIR) $(INST_CHANNELDIR)

INSTALL_IPCDIR	=						\
	echo "=== Installing $@ (permission: $(PERM_DIR))";	\
	$(INSTALL_DIRS) $@

EXTRA_INSTALL	+= install-dirs

include ../rules.mk

install-dirs:	$(INST_IPCDIRS)

$(INST_IPCDIRS):
	@$(INSTALL_IPCDIR)

$(INST_CHANNELDIR):	$(INST_WORKDIR)

$(INST_CHANNELDIR):	PERM_DIR = $(PERM_CHANNELDIR)
