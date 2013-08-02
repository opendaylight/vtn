#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build PFC commands.
##

include $(BLDDIR)/exec-rules.mk

ifdef	SKIP_INSTALL

# Don't install command.
install:	all

else	# !SKIP_INSTALL

install:	all $(EXTRA_INSTALL) $(SYS_EXTRA_INSTALL)
	@$(call CMD_INSTALL,$(CMD_BINDIR))

endif	# SKIP_INSTALL
