#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build PFC commands.
##

NEED_OBJDIR	:= 1
BLD_CONFIG_MK	?= ../../build/config.mk

# Installation directory for the command.
CMD_BINDIR	= $(INST_BINDIR)

include $(BLD_CONFIG_MK)
include $(BLDDIR)/exec-defs.mk
