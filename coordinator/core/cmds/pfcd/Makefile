#
# Copyright (c) 2010-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC daemon.
##

# Use PFC daemon maintenance API.
USE_PFCD_MAINT	= 1

include ../defs.mk

EXEC_NAME	= pfcd
INST_EXEC_NAME	= uncd

C_SOURCES	=		\
	child.c			\
	control.c		\
	event.c			\
	ipc.c			\
	main.c			\
	module.c		\
	path.c			\
	resource.c		\
	signal.c		\
	stderr.c

# Link copyright.o
LINK_COPYRIGHT	= 1

CFDEF_FILES	= attr.cfdef

# Import system library private header files.
IPC_INCDIRS	= libpfc_ipc libpfc_ipcsrv libpfc_ipcclnt
PFCLIB_INCDIRS	= libpfc_util libpfc libpfc_cmd $(IPC_INCDIRS)
EXTRA_INCDIRS	= $(PFCLIB_INCDIRS:%=../../libs/%)

# Link libraries for CSM and SDB support.
EXT_LIBDIRS	:= $(sort $(abspath $(CSM_LIBRARIES) $(SDB_LIBRARIES)))
EXTRA_LIBDIRS	+= $(EXT_LIBDIRS)

# Search required libraries in the specified directories.
LDFLAGS		+= $(EXT_LIBDIRS:%=-Wl,-rpath-link,%)

PFC_LIBS	= libpfc libpfc_cmd libpfc_util libpfc_ipcsrv libpfc_ipcclnt
LDLIBS		+= -ldl
LDLIBS_PREP	= $(PFCD_EXTRA_LDLIBS)

# Embed build timestamp.
# Note that this always causes rebuild of pfcd.
GENCOPY_FLAGS	= -s

# Install pfcd to sbin.
CMD_BINDIR	= $(INST_SBINDIR)

# Directories to be created on "make install".
PFCD_INSTDIRS	= $(DESTDIR)$(INST_MODULEDIR) $(DESTDIR)$(INST_MODCONFDIR)

EXTRA_INSTALL	= install-dirs

include ../rules.mk

# Install PFC system directories.
install-dirs:	$(PFCD_INSTDIRS)

$(PFCD_INSTDIRS):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

# Include UNC header files.
$(OBJDIR)/main.o:	EXTRA_INCDIRS += $(UNC_INCDIRS)
