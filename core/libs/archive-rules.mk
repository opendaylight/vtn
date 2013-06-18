#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build PFC core archive libraries.
##

ifeq	($(strip $(USE_LOG_IDENT)),1)

# Use library name as log identifier.
CPPFLAGS	+= -DPFC_LOG_IDENT='"$(notdir $(CURDIR))"'

endif	# USE_LOG_IDENT == 1

include $(BLDDIR)/archive-rules.mk

# Don't install archive library.
install:	all
