#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to install daemon configuration file used by
## UNC daemon launcher.
##
## This makefile is designed to be included after common dmconf-defs.mk is
## included.
##

# Installation path for the daemon configuration file.
DMCONF_DIR	= $(DESTDIR)$(INST_SYSCONFDIR)/launcher.d
DEST_DMCONF	= $(DMCONF:%=$(DMCONF_DIR)/%)

dmconf:	$(OBJ_DMCONF)

include $(CORE_BLDDIR)/repl-rules.mk

$(OBJ_DMCONF):	REPL_RULES = $(DMCONF_RULES)

install-dmconf:	$(DEST_DMCONF)

$(DMCONF_DIR)/%:	$(OBJDIR)/%
	@$(call CMD_INSTALL_FILE,$<,$@)
