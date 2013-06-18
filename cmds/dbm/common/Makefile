#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of shell scripts for the UNC system.
##

include ../sed_defs.mk
include ../../../build/config.mk

# Script files to be installed to cluster management script directory.
# Typically, db management script directory is sbin/dbm.
DBM_SCRIPTS_IN	=		\
	dbm_functions.in


# sed(1) rules to generate shell scripts.
SED_RULES	= $(SED_RULE_LIST:%=-e %)

# Generated scripts.
OBJ_SCRIPTS	= $(DBM_SCRIPTS_IN:%.in=$(OBJDIR)/%)
CLEANFILES	= $(OBJ_SCRIPTS)

# Installation targets.
DEST_DBMDIR		= /dbm
DEST_DBM_SCRIPTDIR	= $(DESTDIR)$(INST_SYSSCRIPTDIR)$(DEST_DBMDIR)
DEST_DBM_SCRIPTS	= $(DBM_SCRIPTS_IN:%.in=$(DEST_DBM_SCRIPTDIR)/%)

DEST_SCRIPTS	= $(DEST_DBM_SCRIPTS)

all:	$(OBJ_SCRIPTS)

install:	$(DEST_SCRIPTS)

include $(BLDDIR)/rules.mk
include $(CORE_BLDDIR)/sed-rules.mk

# Install cluster management shell scripts.
$(DEST_DBM_SCRIPTDIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_SCRIPTS):	PERM_FILES = $(PERM_PROGS)
