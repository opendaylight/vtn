#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build native executable.
##

# Undefine INST_LIBDIR.
INST_LIBDIR	:=

# Create .deps and .cmds on "make all".
PREPEND_TARGET	= $(DEPDIR) $(CMDDIR)

# Filter out rules to create .deps and .cmds from object dependency.
PHONY_TARGET	+= $(PREPEND_TARGET)

# Don't remove OBJDIR on "make clobber".
CLOBBERFILES	= $(PREPEND_TARGET)
CLOBBER_DEPS	= clean

# No need to update IPC struct headers on tool build.
SKIP_IPC_UPDATE	= 1
export	SKIP_IPC_UPDATE

include $(BLDDIR)/exec-rules.mk

install:	all $(EXTRA_INSTALL)
	@$(call CMD_INSTALL,$(TOOLBIN))

$(OBJECTS):	$(PREPEND_TARGET)

$(PREPEND_TARGET):
	$(INSTALL_DIRS) $@

distclean:	clobber

