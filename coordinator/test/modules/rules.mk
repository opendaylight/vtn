#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build unit tests for VTN Coordinator modules.
## defs.mk must be included in advance.
##

ALT_CFDEF_NAMES	= $(notdir $(ALT_CFDEF_FILES))
CFDEF_SOURCES	+= $(ALT_CFDEF_NAMES:%.cfdef=$(OBJDIR)/$(CFDEF_PREFIX)%.c)
CFDEF_DEPFILES	+= $(ALT_CFDEF_NAMES:%=$(OBJ_DEPDIR)/%.d)
OBJECTS		+= $(ALT_CFDEF_NAMES:%.cfdef=$(CFDEF_PREFIX)%.o)

include $(GTEST_BLDDIR)/gtest-rules.mk

# Define rules to build source files under ALT_SRCDIRS.
define COMPILE_TEMPLATE
$$(OBJDIR)/%.o $$(OBJ_CMDDIR)/%.cc.cmd:	$(1)/%.cc FRC
	@$$(call CMD_EXECUTE,CXX_O,$$(OBJ_CMDDIR)/$$*.cc.cmd,$$?)
endef

$(foreach dir,$(ALT_SRCDIRS),$(eval $(call COMPILE_TEMPLATE,$(dir))))

# Define rules to compile cfdef files in ALT_CFDEF_FILES.
ALT_CFDEF_DIRS	= $(sort $(dir $(ALT_CFDEF_FILES)))

define CFDEF_TEMPLATE
$$(OBJDIR)/$$(CFDEF_PREFIX)%.c $$(OBJ_CMDDIR)/%.cfdef.cmd:	$(1)%.cfdef FRC
	@$$(call CMD_EXECUTE,CFDEF_C,$$(OBJ_CMDDIR)/$$*.cfdef.cmd,$$?)
endef

$(foreach dir,$(ALT_CFDEF_DIRS),$(eval $(call CFDEF_TEMPLATE,$(dir))))
