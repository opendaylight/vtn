#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of SQL scripts for the UNC system.
##

include ../build/config.mk

# Collect SQL files.
SQL_SCRIPTS	:= $(wildcard *.sql)

# Define sub-directories for SQL files.
SQL_SUBDIRS	:= upgrade downgrade optimize

# Collect SQL files in SQL_SUBDIRS.
SQL_SUBSCRIPTS	:= $(wildcard $(SQL_SUBDIRS:%=%/*.sql))

# Installation targets.
DEST_SQL_SCRIPTDIR	= $(DESTDIR)$(INST_SQLDIR)
DEST_SQL_SCRIPTS	= $(SQL_SCRIPTS:%=$(DEST_SQL_SCRIPTDIR)/%)

DEST_SQL_SUBDIRS	= $(SQL_SUBDIRS:%=$(DEST_SQL_SCRIPTDIR)/%)
DEST_SQL_SCRIPTS	+= $(SQL_SUBSCRIPTS:%=$(DEST_SQL_SCRIPTDIR)/%)

INSTALL_TARGET	:= $(DEST_SQL_SUBDIRS) $(DEST_SQL_SCRIPTS)

all:

install:	$(INSTALL_TARGET)

include $(BLDDIR)/rules.mk

# Install SQL scripts.
$(DEST_SQL_SCRIPTDIR)/%:	%
	@$(call CMD_INSTALL_FILE,$<,$@)

# Install SQL sub-directories.
$(DEST_SQL_SUBDIRS):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@
