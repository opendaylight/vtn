#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs system files for Java applications.
##

NEED_OBJDIR	:= 1

include ../../build/config.mk

# Files to be installed to INST_DATADIR/java.
JAVA_DATAFILES_IN	= functions.in
OBJ_JAVA_DATAFILES	= $(JAVA_DATAFILES_IN:%.in=$(OBJDIR)/%)
CLEANFILES		= $(OBJ_JAVA_DATAFILES)

# Installation targets.
DEST_JAVA_DATADIR	= $(DESTDIR)$(INST_DATADIR)/java
DEST_JAVA_DATA		= $(JAVA_DATAFILES_IN:%.in=$(DEST_JAVA_DATADIR)/%)

# sed(1) rules to generate shell scripts.
SED_RULE_LIST	= 's,%INST_SYSCONFDIR%,$(INST_SYSCONFDIR),g'

SED_RULES	= $(SED_RULE_LIST:%=-e %)

DEST_DIRS		= $(DEST_JAVA_DATADIR)
DEST_FILES		= $(DEST_JAVA_DATA)

all:	$(OBJ_JAVA_DATAFILES)

install:	$(DEST_FILES)

include $(BLDDIR)/rules.mk
include $(CORE_BLDDIR)/sed-rules.mk

# Install data files to be shared with Java applications.
$(DEST_JAVA_DATADIR)/%:	$(OBJDIR)/% $(DEST_JAVA_DATADIR) FRC
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_DIRS):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@
