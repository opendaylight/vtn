#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of shell scripts for the UNC system.
##

NEED_OBJDIR	:= 1

include ../build/config.mk
include $(JAVA_CONFIG_MK)

# Script files to be installed to system admin script directory.
# Typically, system admin script directory is sbin.
SYS_SCRIPTS_IN	=		\
	uncd_start.in		\
	uncd_stop.in		\
	uncd_monitor.in

# Script files to be installed to bin.
BIN_SCRIPTS_IN	=		\
	vtn_start.in		\
	vtn_stop.in

# Script files to be installed to INST_DATADIR.
DATA_SCRIPTS_IN	=		\
	vtn_funcs.sh.in

# sed(1) rules to generate shell scripts.
SED_RULE_LIST	= 's,%INST_BINDIR%,$(INST_BINDIR),g'
SED_RULE_LIST	+= 's,%INST_DATADIR%,$(INST_DATADIR),g'
SED_RULE_LIST	+= 's,%INST_JARDIR%,$(INST_JARDIR),g'
SED_RULE_LIST	+= 's,%INST_LIBEXECDIR%,$(INST_LIBEXECDIR),g'
SED_RULE_LIST	+= 's,%INST_LOCALSTATEDIR%,$(INST_LOCALSTATEDIR),g'
SED_RULE_LIST	+= 's,%INST_SBINDIR%,$(INST_SBINDIR),g'
SED_RULE_LIST	+= 's,%INST_SYSCONFDIR%,$(INST_SYSCONFDIR),g'
SED_RULE_LIST	+= 's,%INST_SYSSCRIPTDIR%,$(INST_SYSSCRIPTDIR),g'
SED_RULE_LIST	+= 's,%INST_UNCWORKDIR%,$(INST_UNCWORKDIR),g'
SED_RULE_LIST	+= 's,%INST_DATADIR%,$(INST_DATADIR),g'
SED_RULE_LIST	+= 's,%PATH_SCRIPT%,$(PATH_SCRIPT),g'
SED_RULE_LIST	+= 's,%SHELL_PATH%,$(SHELL_PATH),g'

SED_RULES	= $(SED_RULE_LIST:%=-e %)

# Generated scripts.
OBJ_SCRIPTS	= $(SYS_SCRIPTS_IN:%.in=$(OBJDIR)/%)
OBJ_SCRIPTS	+= $(BIN_SCRIPTS_IN:%.in=$(OBJDIR)/%)
OBJ_SCRIPTS	+= $(DATA_SCRIPTS_IN:%.in=$(OBJDIR)/%)
CLEANFILES	= $(OBJ_SCRIPTS)

# Installation targets.
DEST_SYS_SCRIPTDIR	= $(DESTDIR)$(INST_SYSSCRIPTDIR)
DEST_SYS_SCRIPTS	= $(SYS_SCRIPTS_IN:%.in=$(DEST_SYS_SCRIPTDIR)/%)
DEST_BINDIR		= $(DESTDIR)$(INST_BINDIR)
DEST_BIN_SCRIPTS	= $(BIN_SCRIPTS_IN:%.in=$(DEST_BINDIR)/%)
DEST_DATADIR		= $(DESTDIR)$(INST_DATADIR)
DEST_DATA_SCRIPTS	= $(DATA_SCRIPTS_IN:%.in=$(DEST_DATADIR)/%)

DEST_SCRIPTS	= $(DEST_SYS_SCRIPTS) $(DEST_BIN_SCRIPTS)

all:	$(OBJ_SCRIPTS)

install:	$(DEST_SCRIPTS) $(DEST_DATA_SCRIPTS)

include $(BLDDIR)/rules.mk
include $(CORE_BLDDIR)/sed-rules.mk

# Install system admin shell scripts.
$(DEST_SYS_SCRIPTDIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

# Install shell scripts.
$(DEST_BINDIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

# Install data files for shell script.
$(DEST_DATADIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_SCRIPTS):	PERM_FILES = $(PERM_PROGS)
