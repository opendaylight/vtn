#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build UNC libraries.
##

# Determine shared library name.
ifndef	SHLIB_NAME
SHLIB_NAME	:= $(notdir $(CURDIR))
endif	# !SHLIB_NAME

# Add header search path to include library internal header files.
EXTRA_INCDIRS	+= $(UNCLIB_INCDIRS:%=../%)

# Disallow unresolved symbol.
LD_LDFLAGS	+= $(LD_ZDEFS)

ifeq	($(strip $(USE_LOG_IDENT)),1)
# Use library name as log identifier.
CPPFLAGS	+= -DPFC_LOG_IDENT='"$(SHLIB_NAME)"'
else ifneq	($(strip $(USE_LOG_IDENT)),)
# Use specified string as log identifier.
CPPFLAGS	+= -DPFC_LOG_IDENT='"$(USE_LOG_IDENT)"'
endif	# USE_LOG_IDENT == 1

include $(BLDDIR)/shared-rules.mk

install:	all $(EXTRA_INSTALL) $(SYS_EXTRA_INSTALL)
	@$(call CMD_DEST_INSTALL,$(LIBS_TARGETDIR),$(DESTDIR))
