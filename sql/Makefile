#
# Copyright (c) 2012-2013 NEC Corporation
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

# Installation targets.
DEST_SQL_SCRIPTDIR	= $(DESTDIR)$(INST_SQLDIR)
DEST_SQL_SCRIPTS	= $(SQL_SCRIPTS:%=$(DEST_SQL_SCRIPTDIR)/%)

all:

install:	$(DEST_SQL_SCRIPTS)

include $(BLDDIR)/rules.mk

# Install SQL scripts.
$(DEST_SQL_SCRIPTDIR)/%:	%
	@$(call CMD_INSTALL_FILE,$<,$@)
