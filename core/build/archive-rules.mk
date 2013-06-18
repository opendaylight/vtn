#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build archive library.
## This makefile is designed to be included after archive-defs.mk is included.
##

ifndef	BUILD_ARCHIVE_RULES_MK_INCLUDED

BUILD_ARCHIVE_RULES_MK_INCLUDED	:= 1

OBJ_LIB_FILE	= $(OBJDIR)/$(LIB_FILE)
OBJ_OBJECTS	= $(OBJECTS:%=$(OBJDIR)/%)

ifdef	EXPORT_LIBS
EXPORT_LIBDIR	= $(LINK_LIBDIR)
LINK_OBJFILES	= $(LIB_FILE:%=$(LINK_LIBDIR)/%)
ARLIB_TARGET	= $(LINK_OBJFILES)
else	# !EXPORT_LIBS
ARLIB_TARGET	= $(OBJ_LIB_FILE)
endif	# EXPORT_LIBS

CLEANFILES	+= $(OBJ_LIB_FILE) $(LINK_OBJFILES) $(OBJ_OBJECTS)

all:	$(ARLIB_TARGET) $(EXTRA_TARGET)

include	$(BLDDIR)/rules.mk

CMDRULE_LINK_AR	=							\
	set -e;								\
	echo 'cd $(OBJDIR);' $(AR) $(ARFLAGS) $(LIB_FILE) $(OBJECTS);	\
	cd $(OBJDIR);							\
	$(AR) $(ARFLAGS) $(LIB_FILE) $(OBJECTS)

$(OBJ_LIB_FILE) $(OBJ_CMDDIR)/$(LIB_NAME).cmd:	$(OBJ_OBJECTS) FRC
	@$(call CMD_EXECUTE,LINK_AR,$(OBJ_CMDDIR)/$(LIB_NAME).cmd,$?)

endif	# !BUILD_ARCHIVE_RULES_MK_INCLUDED
