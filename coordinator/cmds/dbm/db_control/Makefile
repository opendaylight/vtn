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

include ../sed_defs.mk
include ../../../build/config.mk
include $(JAVA_CONFIG_MK)

# Script files to be installed to system admin script directory.
# Typically, system admin script directory is sbin.
SYS_SCRIPTS_IN	=	\
	db_setup.in	\
	db_start.in	\
	db_stop.in

# Use db management specific configuration.
DBM_CONF_IN	= 		\
	unc_db_env.conf.in	\
	pg_hba.conf.in		\
	postgresql.conf.single.in

DBM_PATH_CONF_IN	= 		\
	unc_db_path_info.conf.in

# Install LOCALSTATEDIR on "make install".
DBM_DIRS	= $(DESTDIR)$(INST_LOCALSTATEDIR)

ifneq	($(INST_LOCALSTATEDIR),$(INST_UNCWORKDIR))
DBM_DIRS	+= $(DESTDIR)$(INST_UNCWORKDIR)
endif	# INST_LOCALSTATEDIR != INST_UNCWORKDIR

# sed(1) rules to generate shell scripts.
SED_RULES	= $(SED_RULE_LIST:%=-e %)

# Generated scripts.
OBJ_SCRIPTS	= $(SYS_SCRIPTS_IN:%.in=$(OBJDIR)/%)
OBJ_SCRIPTS	+= $(DBM_CONF_IN:%.in=$(OBJDIR)/%)
OBJ_SCRIPTS	+= $(DBM_PATH_CONF_IN:%.in=$(OBJDIR)/%)
CLEANFILES	= $(OBJ_SCRIPTS)

# Installation targets.
DEST_SYS_SCRIPTDIR	= $(DESTDIR)$(INST_SYSSCRIPTDIR)
DEST_SYS_SCRIPTS	= $(SYS_SCRIPTS_IN:%.in=$(DEST_SYS_SCRIPTDIR)/%)
DEST_DBMCONFDIR		= /dbm
DEST_DBM_CONFDIR	= $(DESTDIR)$(INST_SYSCONFDIR)$(DEST_DBMCONFDIR)
DEST_DBM_CONF		= $(DBM_CONF_IN:%.in=$(DEST_DBM_CONFDIR)/%)
DEST_DBMPATHCONFDIR	= /dbm/pathinfo
DEST_DBM_PATH_CONFDIR	= $(DESTDIR)$(INST_SYSCONFDIR)$(DEST_DBMPATHCONFDIR)
DEST_DBM_PATH_CONF	= $(DBM_PATH_CONF_IN:%.in=$(DEST_DBM_PATH_CONFDIR)/%)

DEST_SCRIPTS	=		\
	$(DEST_SYS_SCRIPTS)	\
	$(DEST_DBM_CONF)	\
	$(DEST_DBM_PATH_CONF)	\
	$(DBM_DIRS)

all:	$(OBJ_SCRIPTS)

install:	$(DEST_SCRIPTS)

include $(BLDDIR)/rules.mk
include $(CORE_BLDDIR)/sed-rules.mk

# Install system admin shell scripts.
$(DEST_SYS_SCRIPTDIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_SYS_SCRIPTS):	PERM_FILES = $(PERM_PROGS)

# Install conf.
$(DEST_DBM_CONFDIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

# Install path info conf.
$(DEST_DBM_PATH_CONFDIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)

# Install mandatory directories.
$(DBM_DIRS):
	$(INSTALL_DIRS) $@
