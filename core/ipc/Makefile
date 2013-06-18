#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of header files from IPC struct
## template files.
##

NEED_OBJDIR	:= 1

include ../build/config.mk

# Regular expression to collect IPC struct template files.
IPC_TMPL_REGEXP	= ^[a-zA-Z]\w*\.ipct$$

# Files to preserve compiled templates.
IPC_STAMP_FILE	= $(OBJDIR)/templates

# Access permission for IPC struct information file.
PERM_STRUCT_BIN	= 0444

# Installation path of IPC struct information file.
IPC_DATADIR	= $(INST_DATADIR)/ipc
DEST_IPCDIR	= $(DESTDIR)$(IPC_DATADIR)
DEST_IPC_BIN	= $(DEST_IPCDIR)/$(IPC_STRUCT_BIN)

# Flags for IPC struct template file compiler.
IPCTC_FLAGS	= $(OBJ_STRUCT_H:%=-h %) $(OBJ_IPC_BIN:%=-i %)
IPCTC_FLAGS	+= $(OBJ_SERVER_P_HH:%=-S %) $(OBJ_SERVER_I_HH:%=-L %)
IPCTC_FLAGS	+= $(OBJ_CLIENT_P_HH:%=-C %) $(OBJ_CLIENT_I_HH:%=-l %)
IPCTC_FLAGS	+= $(IPC_TMPL_REGEXP:%=-a '%') $(IPC_STAMP_FILE:%=-s %)
IPCTC_FLAGS	+= $(IPC_TMPL_SRCDIR:%=-T %)

CLEANFILES	= $(IPC_STAMP_FILE) $(OBJ_IPC_BIN)
DISTCLEANFILES	= $(IPC_HEADERS)

all:	$(OBJ_IPC_BIN)

# Remarks: Header files will be installed by rules in "include" directory.
install dist:	install-bin

# Do nothing on "make uninstall".
uninstall:

$(OBJ_IPC_BIN):	FRC
	@$(IPCTC) $(IPCTC_FLAGS);					\
	ret=$$?;							\
	if [ $$ret -eq 0 ]; then					\
	    $(MAKE) -C $(SRCROOT)/libs					\
		EXPORT_HEADER_DIR=$(UNC_CORE_INCLUDE) export-header ||	\
		exit 1;							\
	elif [ $$ret -ne 75 ]; then					\
	    exit $$ret;							\
	fi

install-bin:	$(DEST_IPC_BIN)

$(DEST_IPC_BIN):	$(OBJ_IPC_BIN)
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_IPC_BIN):	PERM_FILES=$(PERM_STRUCT_BIN)

clean:	FRC
	$(RM) -rf $(CLEANFILES)

clobber:	FRC
	$(RM) -rf $(CLOBBERFILES)

distclean:	clobber
	$(RM) -rf $(DISTCLEANFILES)

.DELETE_ON_ERROR:

FRC:

.PHONY:	FRC
