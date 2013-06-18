#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build PFC core libraries.
##

CPPFLAGS	+= -D_PFC_CORE_LIBS_BUILD

# Add header search path to include library internal header files.
EXTRA_INCDIRS	+= $(PFCLIB_INCDIRS:%=../%)

# Disallow unresolved symbol.
LD_LDFLAGS	+= $(LD_ZDEFS)

ifeq	($(strip $(USE_LOG_IDENT)),1)

# Use library name as log identifier.
CPPFLAGS	+= -DPFC_LOG_IDENT='"$(notdir $(CURDIR))"'

endif	# USE_LOG_IDENT == 1

LIBS_DESTDIR	= $(DESTDIR)

include $(BLDDIR)/shared-rules.mk

install:	all $(EXTRA_INSTALL) $(SYS_EXTRA_INSTALL)
	@$(call CMD_DEST_INSTALL,$(LIBS_TARGETDIR),$(LIBS_DESTDIR))
