#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build UNC modules in pfcd.
##

# Append UNC_LIBS to PFC_LIBS because PFC-Core build environment does not
# see UNC_LIBS.
UNC_LIBS	:= $(filter-out $(PFC_LIBS), $(UNC_LIBS))
PFC_LIBS	+= $(UNC_LIBS)

# Include UNC header files before PFC-Core header files.
CC_INCDIRS_PREP	= $(UNC_INCDIRS)

include $(ODBC_RULES_MK)
include $(SRCROOT)/modules/rules.mk
