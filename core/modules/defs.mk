#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build PFC modules.
##

NEED_OBJDIR	:= 1

# Don't use SO version for module object.
NO_SO_VERSION	:= 1

MODULE_SRCROOT	?= ../..
MODULE_BASEDIR	?= ..

include $(MODULE_SRCROOT)/build/config.mk
include $(BLDDIR)/shared-defs.mk

# Module name must be identical to build directory name.
MODULE_NAME	:= $(notdir $(CURDIR))

# Common options for modtool.
MODTOOL_OPTS	:= -D $(MODULE_BASEDIR) -m

# Ensure that the module name is valid.
MODULE_NAME_CHECK	:= $(shell $(MODTOOL) $(MODTOOL_OPTS) -c $(MODULE_NAME))
$(eval $(call MODTOOL_ASSERT,$(MODULE_NAME_CHECK)))

# Module dependency file.
MODULE_DEP	:= $(call TEST_FILE_EXISTS,$(MODULE_NAME).dep)

ifneq	($(strip $(MODULE_DEP)),)

# Ensure that the contents of module dependency file is valid.
MODULE_DEPENDS	:= $(shell $(MODTOOL) $(MODTOOL_OPTS) -s $(MODULE_DEP))
$(eval $(call MODTOOL_ASSERT,$(MODULE_DEPENDS)))

endif	# !empty(MODULE_DEP)

# Module configuration definition file.
MODULE_CFDEF	:= $(call TEST_FILE_EXISTS,$(MODULE_NAME).cfdef)

# Name of module configuration file.
# This file will be installed to module directory.
MODULE_CONF	:= $(MODULE_NAME).conf

# Name of public module configuration file.
# This file will be installed to modconf directory.
MODULE_PUBLIC_CONF	:= $(MODULE_NAME)_public.conf
